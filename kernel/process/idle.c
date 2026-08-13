#include "idle.h"

#include <stddef.h>
#include "process.h"
#include "thread.h"
#include "scheduler.h"

#include "../cpu/cpu.h"

/*
 * ============================================================
 * XyrisOS Idle Process / Thread
 * ============================================================
 *
 * The idle process exists permanently and runs whenever there
 * are no other runnable threads.
 * ============================================================
 */

static process_t *idle_process = NULL;
static thread_t *idle_thread_handle = NULL;


/*
 * ------------------------------------------------------------
 * Idle Thread
 * ------------------------------------------------------------
 */

void idle_thread(void)
{
    while (1)
    {
        /*
         * Halt until an interrupt arrives, then give the
         * scheduler a chance to run a newly-ready thread.
         */
        cpu_halt();
        scheduler_yield();
    }
}


/*
 * ------------------------------------------------------------
 * Initialize Idle Process
 * ------------------------------------------------------------
 */

void idle_initialize(void)
{
    /*
     * Prevent duplicate initialization.
     */
    if (idle_process != NULL)
    {
        return;
    }

    /*
     * Create the kernel idle process.
     */
    idle_process =
        process_create(
            "idle",
            true
        );

    if (idle_process == NULL)
    {
        return;
    }

    idle_process->state =
        PROCESS_READY;

    /*
     * Create the idle thread.
     */
    idle_thread_handle =
        thread_create(
            idle_process,
            idle_thread,
            THREAD_PRIORITY_IDLE
        );

    if (idle_thread_handle == NULL)
    {
        process_destroy(idle_process);
        idle_process = NULL;

        return;
    }

    idle_thread_handle->state =
        THREAD_READY;

    idle_process->main_thread =
        idle_thread_handle;

    /*
     * Do NOT add the idle thread to the normal
     * ready queue.
     *
     * The scheduler uses it only when there are
     * no normal runnable threads.
     */
}


/*
 * ------------------------------------------------------------
 * Idle Thread Accessor
 * ------------------------------------------------------------
 */

thread_t *idle_get_thread(void)
{
    return idle_thread_handle;
}


/*
 * ------------------------------------------------------------
 * Idle Process Accessor
 * ------------------------------------------------------------
 */

process_t *idle_get_process(void)
{
    return idle_process;
}