#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "lwp.h"

#define MEGABYTE 1048576
#define KILOBYTE 1024
#define LWP_STACK_SIZE 2*MEGABYTE

int mmap_flags = MAP_PRIVATE | MAP_ANONYMOUS;
#ifdef __linux__
    mmap_flags |= MAP_STACK;
#endif
int tid_incr = 0;

/* Rounds up to the nearest multiple of 16 greater than `x` */
uintptr_t roundUp(uintptr_t x, int multiple) {
  return ((x + multiple - 1) / multiple) * multiple;
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
  /* Allocate a context for the lwp */
  thread lwp = (thread)calloc(1, sizeof(context));
  if (lwp == NULL) {
    perror("malloc");
    return NO_THREAD;
  }
  lwp->stack = base_ptr;
  lwp->stacksize = stack_size;
  lwp->tid = tid_incr++;
  lwp->state.fxsave = FPU_INIT;
  /* Set base pointer to function to return to this thread */
  lwp->state.rbp = (uintptr_t)function - 1;
  /* In x86, the first function argument is stored in rdi */
  lwp->state.rdi = (uintptr_t)argument;

  /* Add lwp to scheduler */
  scheduler sched = lwp_get_scheduler();
  sched->admit(lwp);

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
  
}

int main() {
  return 0;
}