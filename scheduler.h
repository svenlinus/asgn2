#ifndef SCHED_H
#define SCHED_H

#include "lwp.h"
extern struct scheduler current_scheduler;
extern struct scheduler round_robin_scheduler;

#endif

scheduler round_robin_scheduler = {init, shutdown, admit, remove, next, qlen};
scheduler current_scheduler = {init, shutdown, admit, remove, next, qlen};