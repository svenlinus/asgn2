#include "lwp.h"
#include "scheduler.h"

int main(){

    lwp_create(test_func, NULL);
    lwp_start();

    return 0;
}

void test_func(){
    printf("Hello World");
}