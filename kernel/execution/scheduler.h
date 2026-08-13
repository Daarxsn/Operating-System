#ifndef XYRIS_SCHEDULER_H
#define XYRIS_SCHEDULER_H

#include "thread.h"

void scheduler_init(void);

void scheduler_add(thread_t* thread);

void scheduler_remove(thread_t* thread);

thread_t* scheduler_next(void);

void scheduler_tick(void);

thread_t* scheduler_current(void);

#endif