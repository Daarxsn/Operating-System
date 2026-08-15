#ifndef XYRIS_SCHEDULER_H
#define XYRIS_SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>
#include "queue.h"

#define SCHEDULER_DEFAULT_TIME_SLICE 10

typedef struct scheduler
{
    thread_t *current;
    thread_queue_t ready_queue;
    thread_queue_t blocked_queue;
    thread_queue_t sleeping_queue;
    uint64_t ticks;
    uint64_t context_switches;
    uint64_t total_threads;
    uint32_t time_slice;
} scheduler_t;

void scheduler_initialize(void);
void scheduler_start(void);
void scheduler_tick(void);
uint64_t scheduler_debug_get_tick_count(void);
uint64_t pit_debug_get_handler_count(void);
void scheduler_schedule(void);
void scheduler_add_thread(thread_t *thread);
void scheduler_remove_thread(thread_t *thread);
thread_t *scheduler_current_thread(void);
void scheduler_yield(void);
void scheduler_block_current(void);
void scheduler_unblock_thread(thread_t *thread);
void scheduler_sleep_current(uint64_t ticks);
void scheduler_exit_current(void);
bool scheduler_preemption_pending(void);
void scheduler_clear_preemption(void);

#endif
