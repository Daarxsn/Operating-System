#ifndef XYRIS_IDLE_H
#define XYRIS_IDLE_H

#include "process.h"
#include "thread.h"

/*
 * ============================================================
 * XyrisOS Idle Process
 * ============================================================
 */

void idle_initialize(void);

void idle_thread(void);

thread_t *idle_get_thread(void);

process_t *idle_get_process(void);

#endif