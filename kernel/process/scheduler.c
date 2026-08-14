#include "scheduler.h"
#include "context.h"
#include "idle.h"
#include "process.h"
#include "thread.h"
#include <stddef.h>

static scheduler_t scheduler;
static context_t bootstrap_context;
static bool reschedule_pending = false;

static void scheduler_wake_sleeping(void)
{
    thread_t *thread = scheduler.sleeping_queue.head;
    while (thread != NULL)
    {
        thread_t *next = thread->next;
        if (thread->wake_tick <= scheduler.ticks)
        {
            queue_remove(&scheduler.sleeping_queue, thread);
            thread->wake_tick = 0;
            thread->time_slice = 0;
            thread->state = THREAD_READY;
            queue_push(&scheduler.ready_queue, thread);
        }
        thread = next;
    }
}

void scheduler_initialize(void)
{
    scheduler.current = NULL;
    queue_initialize(&scheduler.ready_queue);
    queue_initialize(&scheduler.blocked_queue);
    queue_initialize(&scheduler.sleeping_queue);
    scheduler.ticks = 0;
    scheduler.context_switches = 0;
    scheduler.total_threads = 0;
    scheduler.time_slice = SCHEDULER_DEFAULT_TIME_SLICE;
    reschedule_pending = false;
}

void scheduler_start(void) { scheduler_schedule(); }

void scheduler_tick(void)
{
    scheduler.ticks++;
    scheduler_wake_sleeping();
    thread_t *current = scheduler.current;
    if (current == NULL || current == idle_get_thread()) return;
    current->cpu_time++;
    current->time_slice++;
    if (current->time_slice >= scheduler.time_slice) reschedule_pending = true;
}

bool scheduler_preemption_pending(void) { return reschedule_pending; }
void scheduler_clear_preemption(void) { reschedule_pending = false; }

void scheduler_add_thread(thread_t *thread)
{
    if (thread == NULL || thread == idle_get_thread() || thread->state != THREAD_CREATED) return;
    if (!thread->scheduler_managed)
    {
        thread->scheduler_managed = true;
        scheduler.total_threads++;
    }
    thread->state = THREAD_READY;
    thread->time_slice = 0;
    thread->wake_tick = 0;
    queue_push(&scheduler.ready_queue, thread);
}

void scheduler_remove_thread(thread_t *thread)
{
    if (thread == NULL || thread == scheduler.current) return;
    queue_remove(&scheduler.ready_queue, thread);
    queue_remove(&scheduler.blocked_queue, thread);
    queue_remove(&scheduler.sleeping_queue, thread);
    if (thread->scheduler_managed)
    {
        thread->scheduler_managed = false;
        if (scheduler.total_threads > 0) scheduler.total_threads--;
    }
}

thread_t *scheduler_current_thread(void) { return scheduler.current; }
void scheduler_yield(void) { scheduler_schedule(); }

void scheduler_block_current(void)
{
    thread_t *current = scheduler.current;
    if (current == NULL || current == idle_get_thread()) return;
    current->state = THREAD_BLOCKED;
    current->time_slice = 0;
    queue_push(&scheduler.blocked_queue, current);
    scheduler_schedule();
}

void scheduler_unblock_thread(thread_t *thread)
{
    if (thread == NULL || thread->state != THREAD_BLOCKED) return;
    queue_remove(&scheduler.blocked_queue, thread);
    thread->state = THREAD_READY;
    thread->time_slice = 0;
    queue_push(&scheduler.ready_queue, thread);
}

void scheduler_sleep_current(uint64_t ticks)
{
    thread_t *current = scheduler.current;
    if (current == NULL || current == idle_get_thread()) return;
    if (ticks == 0) { scheduler_schedule(); return; }
    current->wake_tick = scheduler.ticks + ticks;
    current->time_slice = 0;
    current->state = THREAD_SLEEPING;
    queue_push(&scheduler.sleeping_queue, current);
    scheduler_schedule();
}

void scheduler_exit_current(void)
{
    thread_t *current = scheduler.current;
    if (current == NULL || current == idle_get_thread()) return;
    current->state = THREAD_TERMINATED;
    current->wake_tick = 0;
    current->time_slice = 0;
    if (current->scheduler_managed)
    {
        current->scheduler_managed = false;
        if (scheduler.total_threads > 0) scheduler.total_threads--;
    }
    process_thread_exited(current->owner);
    scheduler_schedule();
    for (;;) __asm__ volatile("hlt");
}

void scheduler_schedule(void)
{
    thread_t *previous = scheduler.current;
    reschedule_pending = false;

    if (previous != NULL && previous != idle_get_thread() && previous->state == THREAD_RUNNING)
    {
        previous->state = THREAD_READY;
        previous->time_slice = 0;
        queue_push(&scheduler.ready_queue, previous);
    }

    thread_t *next = queue_pop(&scheduler.ready_queue);
    if (next == NULL) next = idle_get_thread();
    if (next == NULL)
    {
        scheduler.current = NULL;
        thread_set_current(NULL);
        process_set_current(NULL);
        return;
    }

    if (next == previous)
    {
        next->state = THREAD_RUNNING;
        scheduler.current = next;
        thread_set_current(next);
        if (next->owner != NULL)
        {
            process_set_current(next->owner);
            next->owner->state = PROCESS_RUNNING;
        }
        return;
    }

    scheduler.current = next;
    thread_set_current(next);
    if (next->owner != NULL)
    {
        process_set_current(next->owner);
        next->owner->state = PROCESS_RUNNING;
    }
    if (previous != NULL && previous != idle_get_thread() && previous->owner != NULL &&
        previous->owner != next->owner && previous->owner->state == PROCESS_RUNNING)
        previous->owner->state = PROCESS_READY;

    next->state = THREAD_RUNNING;
    next->started = true;
    next->time_slice = 0;
    scheduler.context_switches++;

    if (previous == NULL) context_switch(&bootstrap_context, next->context);
    else context_switch(previous->context, next->context);
}
