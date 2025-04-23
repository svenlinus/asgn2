#include "scheduler.h"
#include <stdlib.h>

void init(void){
    return;
}

void shutdown(void){
    return;
}

void admit(thread new){
    if (thread_list_head == NULL){
        thread_list_head = new;
    }
    else{
        new->sched_one = thread_list_head;
        (thread_list_head->sched_two)->sched_one = new;
        new->sched_two = thread_list_head->sched_two;
        thread_list_head->sched_two = new;

    }
}

void remove(thread victim){
    thread curr_thread = thread_list_head;
    while (curr_thread != NULL){
        if(curr_thread == victim){
            (curr_thread->sched_two)->sched_one = curr_thread->sched_one;
            (curr_thread->sched_one)->sched_two = curr_thread->sched_two;
            curr_thread->sched_one = NULL;
            curr_thread->sched_two = NULL;
            break;
        }
        curr_thread = curr_thread->sched_one;
    }

}

thread next(void){
    if (thread_list_head == NULL){
        return NULL;
    }
    thread return_thread = thread_list_head;
    while (return_thread != NULL){
        if (return_thread->exited == NULL){
            break;
        }

    }
    thread_list_head = return_thread;
    return return_thread;
}

int qlen(void){
    int counter = 0;
    thread current_thread = thread_list_head;
    if (current_thread == NULL){
        return counter;
    }
    counter++;
    current_thread = current_thread->sched_one;
    while (current_thread != thread_list_head){
        if (!current_thread->exited){
            counter++;
        }
        current_thread = current_thread->sched_one;
    }
    return counter;
}
