/*
 * scheduler.c
 * XyrisOS Kernel
 *
 * Round-Robin Scheduler
 */

#include "scheduler.h"
#include "switch.h"

#include <stddef.h>

static thread_t *ready_head = NULL;
static thread_t *ready_tail = NULL;
static thread_t *current = NULL;

/* --------------------------------------------------
   Initialize Scheduler
   -------------------------------------------------- */

void scheduler_init(void)
{
    ready_head = NULL;
    ready_tail = NULL;
    current = NULL;
}

/* --------------------------------------------------
   Add Thread
   -------------------------------------------------- */

void scheduler_add(thread_t *thread)
{
    if (thread == NULL)
        return;

    thread->next_ready = NULL;
    thread->state = THREAD_READY;

    if (ready_head == NULL)
    {
        ready_head = thread;
        ready_tail = thread;
        return;
    }

    ready_tail->next_ready = thread;
    ready_tail = thread;
}

/* --------------------------------------------------
   Remove Thread
   -------------------------------------------------- */

void scheduler_remove(thread_t *thread)
{
    if (thread == NULL)
        return;

    /*
     * Thread is the head of the ready queue.
     */
    if (ready_head == thread)
    {
        ready_head = thread->next_ready;

        if (ready_tail == thread)
            ready_tail = NULL;

        thread->next_ready = NULL;
        return;
    }

    /*
     * Find the thread in the ready queue.
     */
    thread_t *current_thread = ready_head;

    while (current_thread != NULL &&
           current_thread->next_ready != thread)
    {
        current_thread = current_thread->next_ready;
    }

    /*
     * Thread was not found.
     */
    if (current_thread == NULL)
        return;

    /*
     * Unlink the thread.
     */
    current_thread->next_ready = thread->next_ready;

    /*
     * Update tail if required.
     */
    if (ready_tail == thread)
        ready_tail = current_thread;

    /*
     * Fully detach the thread.
     */
    thread->next_ready = NULL;
}

/* --------------------------------------------------
   Select Next Thread
   -------------------------------------------------- */

thread_t *scheduler_next(void)
{
    if (ready_head == NULL)
        return NULL;

    thread_t *next = ready_head;

    ready_head = ready_head->next_ready;

    if (ready_head == NULL)
        ready_tail = NULL;

    next->next_ready = NULL;

    return next;
}

/* --------------------------------------------------
   Scheduler Tick / Cooperative Yield
   -------------------------------------------------- */

void scheduler_tick(void)
{
    thread_t *previous = current;
    thread_t *next = scheduler_next();

    /*
     * No runnable thread.
     */
    if (next == NULL)
        return;

    /*
     * First thread.
     *
     * There is no previous execution context to save.
     */
    if (previous == NULL)
    {
        current = next;
        next->state = THREAD_RUNNING;

        switch_to_context(&next->context);

        __builtin_unreachable();
    }

    /*
     * Only one runnable thread.
     *
     * Continue running the current thread.
     */
    if (previous == next)
    {
        previous->state = THREAD_RUNNING;
        return;
    }

    /*
     * Current thread voluntarily yielded.
     */
    if (previous->state == THREAD_RUNNING)
    {
        previous->state = THREAD_READY;
        scheduler_add(previous);
    }

    /*
     * Run the next thread.
     */
    current = next;
    next->state = THREAD_RUNNING;

    switch_context(
        &previous->context,
        &next->context
    );
}
/* --------------------------------------------------
   Current Thread
   -------------------------------------------------- */

thread_t *scheduler_current(void)
{
    return current;
}