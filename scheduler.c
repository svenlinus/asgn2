#include "scheduler.h"
#include <stdlib.h>

thread thread_list_head = NULL;

void init(void){
    return;
}

void shutdown(void){
    return;
}

void admit(thread new){
    if (thread_list_head == NULL){
        thread_list_head = new;
        new->sched_one = new;
        new->sched_two = new;
    }
    else{
        new->sched_one = thread_list_head;
        new->sched_two = thread_list_head->sched_two;
        (thread_list_head->sched_two)->sched_one = new;
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
    thread starting = thread_list_head;
    thread current = thread_list_head->sched_one;
    while (current->exited != NULL){
        if (current == starting){
            return NULL;
        }
        current = current->sched_one;
    }
    thread_list_head = current;
    return current;
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


struct scheduler round_robin_sched = {init, shutdown, admit, remove, next, qlen};
struct scheduler current_sched = {init, shutdown, admit, remove, next, qlen};