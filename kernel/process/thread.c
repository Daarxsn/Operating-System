#include "thread.h"

#include "context.h"
#include "scheduler.h"

#include <stddef.h>
#include <stdint.h>

#include "../lib/string.h"
#include "../memory/heap.h"

/*
 * ============================================================
 * XyrisOS Thread Manager
 * ------------------------------------------------------------
 * Process-owned kernel thread management.
 *
 * Responsibilities:
 *   - Thread table management
 *   - Thread creation
 *   - Kernel stack allocation
 *   - Initial CPU context creation
 *   - Thread destruction
 *   - Current-thread tracking
 * ============================================================
 */

/*
 * ------------------------------------------------------------
 * Thread Table
 * ------------------------------------------------------------
 */

static thread_t thread_table[THREAD_MAX_COUNT];

static thread_t *current_thread = NULL;

static thread_id_t next_tid = 1;


/*
 * ------------------------------------------------------------
 * Thread Bootstrap
 * ------------------------------------------------------------
 *
 * A newly-created thread begins execution here.
 *
 * The scheduler prepares the initial context so that RIP points
 * to this function. The bootstrap then calls the actual thread
 * entry function.
 *
 * A thread entry may yield, block, sleep or return normally.
 * Returning from the entry function is treated as thread exit.
 * ------------------------------------------------------------
 */

static void thread_bootstrap(void)
{
    thread_t *thread = current_thread;

    if (thread == NULL || thread->entry == NULL)
    {
        /*
         * A malformed thread must not remain RUNNING forever.
         * Terminate it through the normal scheduler path.
         */
        scheduler_exit_current();

        for (;;)
        {
            __asm__ volatile ("hlt");
        }
    }

    thread->entry();

    /*
     * Returning from the entry function means the thread has
     * completed. The scheduler marks it TERMINATED and switches
     * to another runnable thread.
     */

    scheduler_exit_current();

    /*
     * scheduler_exit_current() switches away from this context.
     * If a future scheduler implementation ever returns here,
     * keep the terminated thread parked safely.
     */
    for (;;)
    {
        __asm__ volatile ("hlt");
    }
}


/*
 * ------------------------------------------------------------
 * Initialize Thread Manager
 * ------------------------------------------------------------
 */

void thread_initialize(void)
{
    memset(
        thread_table,
        0,
        sizeof(thread_table)
    );

    current_thread = NULL;

    next_tid = 1;
}


/*
 * ------------------------------------------------------------
 * Create Thread
 * ------------------------------------------------------------
 */

thread_t *thread_create(
    process_t *owner,
    void (*entry)(void),
    thread_priority_t priority)
{
    if (owner == NULL || entry == NULL)
    {
        return NULL;
    }

    /*
     * Find an unused TCB.
     */
    for (uint32_t i = 0;
         i < THREAD_MAX_COUNT;
         i++)
    {
        if (thread_table[i].tid != 0)
        {
            continue;
        }

        thread_t *thread =
            &thread_table[i];

        /*
         * Clear the complete TCB before initialization.
         */
        memset(
            thread,
            0,
            sizeof(thread_t)
        );

        /*
         * Basic identity.
         */
        thread->tid = next_tid++;

        thread->owner = owner;

        thread->entry = entry;

        thread->state =
            THREAD_CREATED;

        thread->priority =
            priority;


        /*
         * ----------------------------------------------------
         * Allocate Kernel Stack
         * ----------------------------------------------------
         */

        thread->stack_size =
            THREAD_STACK_SIZE;

        thread->stack =
            kmalloc(thread->stack_size);

        if (thread->stack == NULL)
        {
            memset(
                thread,
                0,
                sizeof(thread_t)
            );

            return NULL;
        }


        /*
         * ----------------------------------------------------
         * Allocate CPU Context
         * ----------------------------------------------------
         */

        thread->context =
            kmalloc(sizeof(context_t));

        if (thread->context == NULL)
        {
            kfree(thread->stack);

            memset(
                thread,
                0,
                sizeof(thread_t)
            );

            return NULL;
        }

        memset(
            thread->context,
            0,
            sizeof(context_t)
        );


        /*
         * ----------------------------------------------------
         * Prepare Initial Stack
         * ----------------------------------------------------
         *
         * x86-64 stacks grow downward.
         *
         * Keep the stack 16-byte aligned.
         */

        uintptr_t stack_top =
            (uintptr_t)thread->stack +
            thread->stack_size;

        stack_top &=
            ~((uintptr_t)0xFULL);


        /*
         * Leave space for the initial return address.
         *
         * The context switch implementation will restore RSP
         * and jump directly to RIP.
         */

        stack_top -= sizeof(uintptr_t);

        *(uintptr_t *)stack_top = 0;


        /*
         * ----------------------------------------------------
         * Initial CPU Context
         * ----------------------------------------------------
         */

        thread->context->rsp =
            stack_top;

        thread->context->rbp =
            stack_top;

        thread->context->rbx = 0;
        thread->context->r12 = 0;
        thread->context->r13 = 0;
        thread->context->r14 = 0;
        thread->context->r15 = 0;

        /*
         * Start through the bootstrap wrapper rather than
         * jumping directly into the user-supplied entry.
         */
        thread->context->rip =
            (uint64_t)thread_bootstrap;

        /*
         * Interrupt-enabled kernel context.
         *
         * IF = bit 9.
         */
        thread->context->rflags =
            0x202;


        /*
         * ----------------------------------------------------
         * Runtime Statistics
         * ----------------------------------------------------
         */

        thread->cpu_time = 0;

        thread->time_slice = 0;
        thread->wake_tick = 0;
        thread->scheduler_managed = false;


        /*
         * ----------------------------------------------------
         * Queue Links
         * ----------------------------------------------------
         */

        thread->next = NULL;

        thread->previous = NULL;


        /*
         * ----------------------------------------------------
         * Process Main Thread
         * ----------------------------------------------------
         */

        if (owner->main_thread == NULL)
        {
            owner->main_thread =
                thread;
        }


        return thread;
    }

    return NULL;
}


/*
 * ------------------------------------------------------------
 * Destroy Thread
 * ------------------------------------------------------------
 */

void thread_destroy(thread_t *thread)
{
    if (thread == NULL)
    {
        return;
    }

    /*
     * Never release the stack/context of the currently executing
     * thread. It is still using both.
     */
    if (thread == current_thread)
    {
        thread->state = THREAD_TERMINATED;
        return;
    }

    /*
     * Detach the thread from every scheduler queue before freeing
     * its links and storage.
     */
    scheduler_remove_thread(thread);

    /*
     * Mark terminated before releasing resources.
     */
    thread->state =
        THREAD_TERMINATED;


    /*
     * Do not leave the process pointing at a destroyed
     * main thread.
     */
    if (thread->owner != NULL &&
        thread->owner->main_thread == thread)
    {
        thread->owner->main_thread = NULL;
    }


    /*
     * Free kernel stack.
     */
    if (thread->stack != NULL)
    {
        kfree(thread->stack);

        thread->stack = NULL;
    }


    /*
     * Free CPU context.
     */
    if (thread->context != NULL)
    {
        kfree(thread->context);

        thread->context = NULL;
    }


    /*
     * Clear the TCB.
     *
     * tid = 0 means this table slot is available again.
     */
    memset(
        thread,
        0,
        sizeof(thread_t)
    );
}


void thread_destroy_process_threads(
    process_t *owner
)
{
    if (owner == NULL)
    {
        return;
    }

    for (uint32_t i = 0;
         i < THREAD_MAX_COUNT;
         i++)
    {
        thread_t *thread =
            &thread_table[i];

        if (thread->tid == 0 ||
            thread->owner != owner)
        {
            continue;
        }

        if (thread == current_thread)
        {
            /*
             * A running thread cannot safely free its own stack
             * while executing on it. Mark it terminated and leave
             * reclamation to a future reaper.
             */
            thread->state =
                THREAD_TERMINATED;
            continue;
        }

        thread_destroy(thread);
    }

    owner->main_thread = NULL;
}

/*
 * ------------------------------------------------------------
 * Current Thread
 * ------------------------------------------------------------
 */

thread_t *thread_current(void)
{
    return current_thread;
}


/*
 * ------------------------------------------------------------
 * Set Current Thread
 * ------------------------------------------------------------
 */

void thread_set_current(thread_t *thread)
{
    current_thread = thread;
}
