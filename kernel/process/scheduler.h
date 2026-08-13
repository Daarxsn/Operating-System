#ifndef XYRIS_SCHEDULER_H
#define XYRIS_SCHEDULER_H

#include <stdint.h>

#include "queue.h"

/*
 * ============================================================
 * XyrisOS Scheduler
 * ------------------------------------------------------------
 * Round-Robin Scheduler
 * ============================================================
 */

#define SCHEDULER_DEFAULT_TIME_SLICE 10

/*
 * ------------------------------------------------------------
 * Scheduler
 * ------------------------------------------------------------
 */

typedef struct scheduler
{
    /*
     * Currently Running Thread
     */

    thread_t *current;

    /*
     * Scheduler Queues
     */

    thread_queue_t ready_queue;

    thread_queue_t blocked_queue;

    thread_queue_t sleeping_queue;

    /*
     * Statistics
     */

    uint64_t ticks;

    uint64_t context_switches;

    uint64_t total_threads;

    /*
     * Time Slice
     */

    uint32_t time_slice;

} scheduler_t;

/*
 * ------------------------------------------------------------
 * Scheduler API
 * ------------------------------------------------------------
 */

void scheduler_initialize(void);

void scheduler_start(void);

void scheduler_tick(void);

void scheduler_schedule(void);

void scheduler_add_thread(
    thread_t *thread
);

void scheduler_remove_thread(
    thread_t *thread
);

thread_t *scheduler_current_thread(void);

void scheduler_yield(void);

void scheduler_block_current(void);

void scheduler_unblock_thread(
    thread_t *thread
);

void scheduler_sleep_current(
    uint64_t ticks
);

void scheduler_exit_current(void);

#endif