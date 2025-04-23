#include "lwp.h"
#include <stdio.h>

scheduler round_robin_scheduler = {init, shutdown, admit, remove, next, qlen};
scheduler current_scheduler = {init, shutdown, admit, remove, next, qlen};
thread thread_list_head = NULL;

void init(void);

void shutdown(void); 

void admit(thread new);

void remove(thread victim);

thread next(void);

int qlen(void);
