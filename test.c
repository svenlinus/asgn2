#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "lwp.h"

int thread_code(void *arg) {
  int val = *(int *)arg;
  int i, j;
  for (j = 0; j < val; j++) {
    for (i = 0; i < val * 2; i++) {
      printf(" ");
    }
    printf("%d\n", val);
    lwp_yield();
  }
  lwp_exit(val);
  return val;
}

int main() {
  int i;
  int args[6];
  for (i = 1; i <= 1; i++) {
    args[i-1] = i;
    lwp_create(thread_code, &args[i-1]);
    printf("Created %d\n", i);
  }
  lwp_start();
  printf("starting...\n");
  for (i = 0; i < 1; i++) {
    int status;
    tid_t tid = lwp_wait(&status);
    printf("%d exited with status %d\n", (int)tid, status);
  }
  return 0;
}