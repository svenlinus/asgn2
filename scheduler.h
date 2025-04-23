#include "lwp.h"

scheduler round_robin_scheduler;
scheduler current_scheduler;
thread thread_list_head = NULL;

void init(void);

void shutdown(void); 

void admit(thread new);

void remove(thread victim);

int qlen(void);
