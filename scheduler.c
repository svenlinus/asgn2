#include "scheduler.h"

void init(void){
    return;
}

void shutdown(void){
    return;
}

void admit(thread new){
    if(thread_list_tail != NULL){
        thread_list_tail->lib_one = new;
    }
    else{
        thread_list_head = new;
    }
    thread_list_tail = new;

}

void remove(thread victim){

}

thread next(void){
    
}

int qlen(void){
    thread current = thread_list_head;
    int counter = 0;
    while (current != NULL){
        counter++;
        current = current->lib_one;
    }
    return counter;
}
