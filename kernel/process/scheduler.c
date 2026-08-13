#include "scheduler.h"

#include "context.h"
#include "idle.h"

#include <stddef.h>

/*
 * ============================================================
 * XyrisOS Round-Robin Scheduler
 * ============================================================
 *
 * Scheduler responsibilities:
 *
 *   - Maintain the ready queue
 *   - Maintain blocked and sleeping queues
 *   - Select the next runnable thread
 *   - Perform context switches
 *   - Track scheduler ticks
 *   - Wake sleeping threads
 *   - Handle thread termination
 *
 * Scheduling model:
 *
 *   Cooperative round-robin scheduling.
 *
 * scheduler_tick() only accounts for time.
 * scheduler_yield(), block, sleep, and exit perform
 * the actual scheduling transition.
 *
 * ============================================================
 */


/*
 * ------------------------------------------------------------
 * Global Scheduler State
 * ------------------------------------------------------------
 */

static scheduler_t scheduler;


/*
 * ------------------------------------------------------------
 * Bootstrap Context
 * ------------------------------------------------------------
 *
 * This context represents the execution state of kernel_main()
 * before the first scheduler-controlled thread starts running.
 *
 * The first scheduler transition is:
 *
 *     kernel_main()
 *          |
 *          v
 *     bootstrap_context
 *          |
 *          v
 *     first thread
 *
 * ------------------------------------------------------------
 */

static context_t bootstrap_context;


/*
 * ------------------------------------------------------------
 * Initialize Scheduler
 * ------------------------------------------------------------
 */

void scheduler_initialize(void)
{
    /*
     * No thread is running yet.
     */
    scheduler.current = NULL;

    /*
     * Initialize all scheduler queues.
     */
    queue_initialize(
        &scheduler.ready_queue
    );

    queue_initialize(
        &scheduler.blocked_queue
    );

    queue_initialize(
        &scheduler.sleeping_queue
    );

    /*
     * Reset scheduler statistics.
     */
    scheduler.ticks = 0;

    scheduler.context_switches = 0;

    scheduler.total_threads = 0;

    /*
     * Default scheduler time slice.
     */
    scheduler.time_slice =
        SCHEDULER_DEFAULT_TIME_SLICE;
}


/*
 * ============================================================
 * Scheduler Start
 * ============================================================
 *
 * Starts scheduling by selecting the first runnable thread.
 *
 * ============================================================
 */

void scheduler_start(void)
{
    scheduler_schedule();
}


/*
 * ============================================================
 * Wake Sleeping Threads
 * ============================================================
 *
 * Move every sleeping thread whose wake-up time has arrived
 * into the ready queue.
 *
 * ============================================================
 */

static void scheduler_wake_sleeping(void)
{
    thread_t *thread =
        scheduler.sleeping_queue.head;

    while (thread != NULL)
    {
        /*
         * Save the next pointer before removing the current
         * thread from the queue.
         */
        thread_t *next =
            thread->next;

        if (thread->wake_tick <= scheduler.ticks)
        {
            /*
             * Remove from sleeping queue first.
             */
            queue_remove(
                &scheduler.sleeping_queue,
                thread
            );

            /*
             * Reset sleep state.
             */
            thread->wake_tick = 0;

            thread->time_slice = 0;

            /*
             * Make the thread runnable.
             */
            thread->state =
                THREAD_READY;

            /*
             * Put it into the ready queue.
             */
            queue_push(
                &scheduler.ready_queue,
                thread
            );
        }

        thread = next;
    }
}


/*
 * ============================================================
 * Scheduler Tick
 * ============================================================
 *
 * Called by the timer interrupt.
 *
 * This function deliberately does NOT perform a context switch
 * directly from interrupt context.
 *
 * Instead it:
 *
 *   1. Updates scheduler time.
 *   2. Wakes sleeping threads.
 *   3. Accounts CPU time.
 *   4. Accounts the current thread's time slice.
 *
 * A thread can subsequently call scheduler_yield() to perform
 * the actual context switch.
 *
 * ============================================================
 */

void scheduler_tick(void)
{
    scheduler.ticks++;

    /*
     * Wake threads whose sleep interval has expired.
     */
    scheduler_wake_sleeping();

    /*
     * No current thread yet.
     */
    thread_t *current =
        scheduler.current;

    if (current == NULL)
    {
        return;
    }

    /*
     * Idle time is not counted as normal thread CPU time.
     */
    if (current != idle_get_thread())
    {
        current->cpu_time++;
        current->time_slice++;
    }
}


/*
 * ============================================================
 * Add Thread
 * ============================================================
 *
 * Adds a newly created thread to the scheduler.
 *
 * Only THREAD_CREATED threads may enter the scheduler through
 * this function.
 *
 * Existing READY/BLOCKED/SLEEPING threads must use their
 * respective transition functions.
 *
 * ============================================================
 */

void scheduler_add_thread(thread_t *thread)
{
    if (thread == NULL)
    {
        return;
    }

    /*
     * Never schedule a terminated thread.
     */
    if (thread->state == THREAD_TERMINATED)
    {
        return;
    }

    /*
     * The idle thread is managed separately.
     */
    if (thread == idle_get_thread())
    {
        return;
    }

    /*
     * A thread should only be registered once.
     *
     * THREAD_CREATED is the only valid state for initial
     * scheduler registration.
     */
    if (thread->state != THREAD_CREATED)
    {
        return;
    }

    /*
     * Mark the thread as scheduler-managed.
     */
    if (!thread->scheduler_managed)
    {
        thread->scheduler_managed = true;

        scheduler.total_threads++;
    }

    /*
     * Make the new thread runnable.
     */
    thread->state =
        THREAD_READY;

    thread->time_slice = 0;

    thread->wake_tick = 0;

    /*
     * Insert into ready queue.
     *
     * queue_push() also protects against duplicate queue
     * membership.
     */
    queue_push(
        &scheduler.ready_queue,
        thread
    );
}


/*
 * ============================================================
 * Remove Thread
 * ============================================================
 *
 * Remove a thread from all scheduler queues.
 *
 * The currently executing thread cannot be removed here.
 * Use scheduler_exit_current() for that case.
 *
 * ============================================================
 */

void scheduler_remove_thread(thread_t *thread)
{
    if (thread == NULL)
    {
        return;
    }

    /*
     * The current thread is still executing.
     *
     * Do not detach it here.
     */
    if (thread == scheduler.current)
    {
        return;
    }

    /*
     * Remove from every possible scheduler queue.
     *
     * queue_remove() safely does nothing if the thread isn't
     * present in that queue.
     */
    queue_remove(
        &scheduler.ready_queue,
        thread
    );

    queue_remove(
        &scheduler.blocked_queue,
        thread
    );

    queue_remove(
        &scheduler.sleeping_queue,
        thread
    );

    /*
     * Update scheduler accounting.
     */
    if (thread->scheduler_managed)
    {
        thread->scheduler_managed = false;

        if (scheduler.total_threads > 0)
        {
            scheduler.total_threads--;
        }
    }
}


/*
 * ============================================================
 * Get Current Thread
 * ============================================================
 */

thread_t *scheduler_current_thread(void)
{
    return scheduler.current;
}


/*
 * ============================================================
 * Yield CPU
 * ============================================================
 *
 * The current thread remains runnable and gives the scheduler
 * an opportunity to select another thread.
 *
 * ============================================================
 */

void scheduler_yield(void)
{
    scheduler_schedule();
}


/*
 * ============================================================
 * Block Current Thread
 * ============================================================
 *
 * The current thread becomes BLOCKED and is placed into the
 * blocked queue.
 *
 * It will not run again until scheduler_unblock_thread()
 * transitions it back to READY.
 *
 * ============================================================
 */

void scheduler_block_current(void)
{
    thread_t *current =
        scheduler.current;

    /*
     * Nothing to block.
     */
    if (current == NULL)
    {
        return;
    }

    /*
     * The idle thread must never be blocked.
     */
    if (current == idle_get_thread())
    {
        return;
    }

    /*
     * Change state before scheduling.
     *
     * scheduler_schedule() checks the state and therefore
     * will NOT place this thread back into the ready queue.
     */
    current->state =
        THREAD_BLOCKED;

    current->time_slice = 0;

    /*
     * Put the thread into the blocked queue.
     */
    queue_push(
        &scheduler.blocked_queue,
        current
    );

    /*
     * Select another runnable thread.
     */
    scheduler_schedule();
}


/*
 * ============================================================
 * Unblock Thread
 * ============================================================
 *
 * Moves a BLOCKED thread into the READY state.
 *
 * ============================================================
 */

void scheduler_unblock_thread(thread_t *thread)
{
    if (thread == NULL)
    {
        return;
    }

    /*
     * Only blocked threads can be unblocked.
     */
    if (thread->state != THREAD_BLOCKED)
    {
        return;
    }

    /*
     * Remove from blocked queue.
     */
    queue_remove(
        &scheduler.blocked_queue,
        thread
    );

    /*
     * Make runnable.
     */
    thread->state =
        THREAD_READY;

    thread->time_slice = 0;

    /*
     * Insert into ready queue.
     */
    queue_push(
        &scheduler.ready_queue,
        thread
    );
}


/*
 * ============================================================
 * Sleep Current Thread
 * ============================================================
 *
 * Places the current thread into the sleeping queue until:
 *
 *     scheduler.ticks >= wake_tick
 *
 * ============================================================
 */

void scheduler_sleep_current(uint64_t ticks)
{
    thread_t *current =
        scheduler.current;

    /*
     * Nothing to sleep.
     */
    if (current == NULL)
    {
        return;
    }

    /*
     * Idle thread never sleeps.
     */
    if (current == idle_get_thread())
    {
        return;
    }

    /*
     * A zero-length sleep is equivalent to yielding.
     */
    if (ticks == 0)
    {
        scheduler_schedule();

        return;
    }

    /*
     * Calculate wake-up tick.
     */
    current->wake_tick =
        scheduler.ticks + ticks;

    current->time_slice = 0;

    /*
     * Change state before scheduling so the current thread
     * cannot be reinserted into the ready queue.
     */
    current->state =
        THREAD_SLEEPING;

    /*
     * Put into sleeping queue.
     */
    queue_push(
        &scheduler.sleeping_queue,
        current
    );

    /*
     * Select another runnable thread.
     */
    scheduler_schedule();
}


/*
 * ============================================================
 * Terminate Current Thread
 * ============================================================
 *
 * Permanently terminates the currently executing thread.
 *
 * IMPORTANT:
 *
 * Once this function is called, the current thread must never
 * be placed back into the ready queue.
 *
 * The scheduler therefore changes the state to TERMINATED
 * BEFORE calling scheduler_schedule().
 *
 * ============================================================
 */

void scheduler_exit_current(void)
{
    thread_t *current =
        scheduler.current;

    /*
     * Nothing to terminate.
     */
    if (current == NULL)
    {
        return;
    }

    /*
     * The idle thread is permanent.
     */
    if (current == idle_get_thread())
    {
        return;
    }

    /*
     * Mark terminated FIRST.
     *
     * scheduler_schedule() will see THREAD_TERMINATED and
     * will therefore not requeue this thread.
     */
    current->state =
        THREAD_TERMINATED;

    current->wake_tick = 0;

    current->time_slice = 0;

    /*
     * Remove it from scheduler accounting.
     */
    if (current->scheduler_managed)
    {
        current->scheduler_managed = false;

        if (scheduler.total_threads > 0)
        {
            scheduler.total_threads--;
        }
    }

    /*
     * Switch to another runnable thread.
     *
     * This function should never return to the terminated
     * thread if the scheduler is operating correctly.
     */
    scheduler_schedule();

    /*
     * Defensive fallback.
     *
     * If execution somehow returns here, the terminated
     * thread must never continue executing as a normal thread.
     */
    for (;;)
    {
        __asm__ volatile("hlt");
    }
}


/*
 * ============================================================
 * Round-Robin Scheduling
 * ============================================================
 *
 * Select the next runnable thread.
 *
 * State transition rules:
 *
 *     RUNNING -> READY
 *         when yielding
 *
 *     BLOCKED
 *         remains in blocked queue
 *
 *     SLEEPING
 *         remains in sleeping queue
 *
 *     TERMINATED
 *         is discarded from scheduling
 *
 *     READY
 *         is selected for execution
 *
 * If no normal READY thread exists, the idle thread is selected.
 *
 * ============================================================
 */

void scheduler_schedule(void)
{
    thread_t *previous =
        scheduler.current;


    /*
     * --------------------------------------------------------
     * Requeue Previous Running Thread
     * --------------------------------------------------------
     *
     * Only a RUNNING thread is returned to the ready queue.
     *
     * This is the key distinction between:
     *
     *     yield
     *     block
     *     sleep
     *     terminate
     *
     * Blocked, sleeping and terminated threads have already
     * changed their state before reaching this function.
     */

    if (previous != NULL &&
        previous != idle_get_thread() &&
        previous->state == THREAD_RUNNING)
    {
        previous->state =
            THREAD_READY;

        previous->time_slice = 0;

        queue_push(
            &scheduler.ready_queue,
            previous
        );
    }


    /*
     * --------------------------------------------------------
     * Select Next Runnable Thread
     * --------------------------------------------------------
     */

    thread_t *next =
        queue_pop(
            &scheduler.ready_queue
        );


    /*
     * --------------------------------------------------------
     * No Ready Thread
     * --------------------------------------------------------
     *
     * Run the idle thread.
     */

    if (next == NULL)
    {
        next =
            idle_get_thread();

        /*
         * No idle thread available.
         *
         * This should only happen during an incomplete boot
         * sequence or a serious scheduler initialization
         * problem.
         */
        if (next == NULL)
        {
            scheduler.current = NULL;

            thread_set_current(NULL);

            return;
        }
    }


    /*
     * --------------------------------------------------------
     * Same Thread Selected
     * --------------------------------------------------------
     *
     * This can happen when a thread yields while no other
     * runnable thread exists.
     *
     * No context switch is necessary.
     */

    if (next == previous)
    {
        next->state =
            THREAD_RUNNING;

        scheduler.current =
            next;

        thread_set_current(next);

        if (next->owner != NULL)
        {
            process_set_current(next->owner);
            next->owner->state = PROCESS_RUNNING;
        }

        return;
    }


    /*
     * --------------------------------------------------------
     * Install New Current Thread
     * --------------------------------------------------------
     *
     * Do this BEFORE context_switch().
     *
     * thread_bootstrap() and other thread code may query the
     * current thread immediately after the context is restored.
     */

    scheduler.current =
        next;

    thread_set_current(next);

    /*
     * Keep the process manager synchronized with the scheduler.
     * System calls and process lifecycle code use process_current()
     * to determine the address space and trust boundary of the
     * executing thread.  Without this update, switching between
     * threads belonging to different processes leaves stale process
     * ownership behind.
     */
    if (next->owner != NULL)
    {
        process_set_current(next->owner);
        next->owner->state = PROCESS_RUNNING;
    }

    if (previous != NULL &&
        previous != idle_get_thread() &&
        previous->owner != NULL &&
        previous->owner != next->owner &&
        previous->owner->state == PROCESS_RUNNING)
    {
        previous->owner->state = PROCESS_READY;
    }

    next->state =
        THREAD_RUNNING;


    /*
     * Reset the newly scheduled thread's time slice.
     */
    next->time_slice = 0;


    /*
     * Count the context switch.
     */
    scheduler.context_switches++;


    /*
     * --------------------------------------------------------
     * First Scheduler Transition
     * --------------------------------------------------------
     *
     * No previous scheduler-managed thread exists.
     *
     * Save kernel_main() execution state in bootstrap_context
     * and start the first scheduled thread.
     */

    if (previous == NULL)
    {
        context_switch(
            &bootstrap_context,
            next->context
        );

        return;
    }


    /*
     * --------------------------------------------------------
     * Normal Thread-to-Thread Switch
     * --------------------------------------------------------
     */

    context_switch(
        previous->context,
        next->context
    );
}