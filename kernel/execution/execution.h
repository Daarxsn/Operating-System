#ifndef XYRIS_EXECUTION_H
#define XYRIS_EXECUTION_H

#include "thread.h"

void execution_init(void);

thread_t* execution_create(
    void (*entry)(void*),
    void* argument);

void execution_destroy(thread_t* thread);

void execution_yield(void);

thread_t* execution_first(void);

#endif