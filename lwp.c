/* TODO: tid2thread(tid t tid), stack round 16, Wrap */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "lwp.h"
#include "scheduler.h"

#define MEGABYTE 1048576
#define KILOBYTE 1024
#define LWP_STACK_SIZE 2*MEGABYTE

int mmap_flags = MAP_PRIVATE | MAP_ANONYMOUS;
#ifdef __linux__
    mmap_flags |= MAP_STACK;
#endif
int tid_incr = 0;
thread curr_thread = NULL;
thread exited_threads = NULL;
thread waiting_threads = NULL;
thread all_threads = NULL;

/* Rounds up to the nearest multiple of 16 greater than `x` */
uintptr_t roundUp(uintptr_t x, int multiple) {
  return ((x + multiple - 1) / multiple) * multiple;
}

/* Call the given lwpfunction with the given argument.
 * Calls lwp exit() with its return value
 */
void lwp_wrap(lwpfun fun, void *arg) {
  int rval;
  rval = fun(arg);
  lwp_exit(rval);
}

/* Allocates resources for a LWP, returns thread ID */
tid_t lwp_create(lwpfun function, void *argument) {
  /* Figure out the size of the LWP's stack */
  /* Set stack size to default */
  size_t stack_size = LWP_STACK_SIZE;
  long page_size = sysconf(_SC_PAGESIZE); /* _SC_PAGE_SIZE */
  if (page_size == -1) {
      perror("sysconf");
  }
  /* Set stack size to resource limit if it exists */
  struct rlimit limits;
  if (getrlimit(RLIMIT_STACK, &limits) == -1) {
      perror("getrlimit");
  }
  if (limits.rlim_cur != RLIM_INFINITY) {
    stack_size = limits.rlim_cur;
  }
  /* Round to nearest multiple of page size */
  if (stack_size % page_size != 0) {
    stack_size = roundUp(stack_size, page_size);
  }

  /* Create stack for the LWP */
  uintptr_t stack_top = mmap(
    NULL, 
    stack_size, 
    PROT_READ|PROT_WRITE, 
    mmap_flags, 
    -1, 
    0
  );
  if (stack_top == MAP_FAILED) {
    perror("mmap");
    return NO_THREAD;
  }
  /* Because stacks grow downward, compute the top of the allocated memory */
  /* This pointer will be the base of the stack */
  uintptr_t base_ptr = stack_top + stack_size;
  /* Add return address to stack so that the program jumps to the function */
  ((unsigned char *)base_ptr)[1] = (uintptr_t)lwp_wrap;
  ((unsigned char *)base_ptr)[2] = base_ptr;

  /* Allocate a context for the lwp */
  thread lwp = (thread)calloc(1, sizeof(context));
  if (lwp == NULL) {
    perror("calloc");
    return NO_THREAD;
  }
  lwp->stack = base_ptr;
  lwp->stacksize = stack_size;
  lwp->tid = tid_incr++;
  lwp->status = MKTERMSTAT(LWP_LIVE, 0);
  lwp->state.fxsave = FPU_INIT;
  /* Set base pointer to this threads stack */
  lwp->state.rbp = base_ptr + 2;
  /* In x86, the first function argument is stored in rdi */
  lwp->state.rdi = (uintptr_t)function;
  lwp->state.rsi = (uintptr_t)argument;

  /* Add lwp to scheduler */
  scheduler sched = lwp_get_scheduler();
  sched->admit(lwp);

  /* Manage datastructure for gettid (lib_two) */
  lwp->lib_two = all_threads;
  all_threads = lwp;
  
  if (curr_thread == NULL) {
    curr_thread = lwp;
  }

  return lwp->tid;
}


void lwp_start() {
  /* Allocate a context for the main thread */
  thread lwp = (thread)calloc(1, sizeof(context));
  if (lwp == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  lwp->stack = NULL;
  lwp->tid = tid_incr++;
  lwp->state.fxsave = FPU_INIT;

  /* Add lwp to scheduler */
  scheduler sched = lwp_get_scheduler();
  sched->admit(lwp);

  lwp_yield();
}

void lwp_yield() {
  if (curr_thread == NULL) {
    fprintf(STDERR_FILENO, "Can't yield with no threads running\n");
    return;
  } 
  /* Get the next thread to run */
  scheduler sched = lwp_get_scheduler();
  thread next = sched->next();
  if (next == NULL) {
    exit(curr_thread->status);
  }
  /* Save current context and load next */
  rfile curr_state;
  swap_rfiles(&curr_state, &(next->state));
  curr_thread->state = curr_state;
  curr_thread = next;
}


void lwp_exit(int status) {
  if (curr_thread == NULL) {
    fprintf(STDERR_FILENO, "Can't exit with no threads running\n");
    return;
  }
  scheduler sched = lwp_get_scheduler();
  int exit_code = LWPTERMSTAT(status);
  curr_thread->status = MKTERMSTAT(LWP_TERM, exit_code);
  /* Check for waiting threads */
  if (waiting_threads != NULL) {
    /* Reschedule waiting thread and associate with it */
    waiting_threads->exited = curr_thread;
    sched->admit(waiting_threads);
    /* Deque waiting thread */
    waiting_threads = waiting_threads->lib_one;
  }
  else {
    /* Add curr_thread to end of exited threads list */
    if (exited_threads == NULL) {
      exited_threads = curr_thread;
    }
    else {
      thread iter_thread = exited_threads;
      while (iter_thread->exited) {
        iter_thread = iter_thread->exited;
      }
      iter_thread->exited = curr_thread;
    }
  }
  /* Remove curr thread from schedule */
  sched->remove(curr_thread);
  /* Remove curr thread from all threads */
  thread iter_thread = all_threads;
  thread pthread = NULL;
  while (iter_thread) {
    if (iter_thread->tid == curr_thread->tid) {
      if (pthread) {
        pthread->lib_two = iter_thread->lib_two;
      }
      else {
        all_threads = iter_thread->lib_two;
      }
    }
    pthread = iter_thread;
    iter_thread = iter_thread->lib_two;
  }
}

tid_t lwp_wait(int *status) {
  if (curr_thread == NULL) {
    fprintf(STDERR_FILENO, "Can't wait with no threads running\n");
    return;
  }
  scheduler sched = lwp_get_scheduler();
  if (sched->qlen() <= 1) {
    return NO_THREAD;
  } 
  thread exit_thread;
  /* Check for exited threads */
  if (exited_threads != NULL) {
    /* Deque the oldest exited thread (head) */
    exit_thread = exited_threads;
    exited_threads = exited_threads->exited;
  }
  else {
    /* Add curr_thread to end of waiting list */
    if (waiting_threads == NULL) {
      waiting_threads = curr_thread;
    }
    else {
      thread iter_thread = waiting_threads;
      while (iter_thread->lib_one) {
        iter_thread = iter_thread->lib_one;
      }
      iter_thread->lib_one = curr_thread;
    }
    /* Block (deschedule and yield) */
    sched->remove(curr_thread);
    curr_thread->exited = NULL;
    lwp_yield();

    /* Thread rescheduled */
    exit_thread = curr_thread->exited;
    if (exit_thread == NULL) {
      fprintf(STDERR_FILENO, "Thread rescheduled with no associated exits\n");
      return NO_THREAD;
    }
  }
  tid_t tid = exit_thread->tid;
  *status = exit_thread->status ? LWPTERMSTAT(exit_thread->status) : NULL;
  /* Deallocate exited thread */
  if (exit_thread->stack) {
    munmap(exit_thread->stack, exit_thread->stacksize);
  }
  free(exit_thread);

  return tid;
}

thread tid2thread(tid_t tid) {
  thread iter_thread = all_threads;
  while (iter_thread) {
    if (iter_thread->tid == tid) {
      return iter_thread;
    }
    iter_thread = iter_thread->lib_two;
  }
  return NO_THREAD;
}

tid_t lwp_gettid(){
  return curr_thread ? curr_thread->tid : NO_THREAD;
}

void lwp_set_scheduler(scheduler fun){
  thread current_thread = current_scheduler->next();
  if (fun == NULL) {
    current_scheduler->init = round_robin_scheduler->init;
    current_scheduler->admit = round_robin_scheduler->admit;
    current_scheduler->next = round_robin_scheduler->next;
    current_scheduler->qlen = round_robin_scheduler->qlen;
    current_scheduler->remove = round_robin_scheduler->remove;
    current_scheduler->shutdown = round_robin_scheduler->shutdown;
    return;
  }

  if(fun->init != NULL){
    fun->init();
  }
  while(current_thread != NULL){
    fun->admit(current_thread);
    current_scheduler->remove(current_thread);  
    current_thread = current_scheduler->next();
  }
  current_scheduler->init = fun->init;
  current_scheduler->admit = fun->admit;
  current_scheduler->remove = fun->remove;
  current_scheduler->next = fun->next;
  current_scheduler->qlen = fun->qlen;

  if (current_scheduler->shutdown != NULL){
    current_scheduler->shutdown();
  }
  current_scheduler->shutdown = fun->shutdown;
}

scheduler lwp_get_scheduler(){
  return current_scheduler;
}