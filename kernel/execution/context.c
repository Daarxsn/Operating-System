/*
 * context.c
 * XyrisOS Kernel
 *
 * Thread Bootstrap
 */

#include "thread.h"
#include "scheduler.h"

void thread_bootstrap(void)
{
    thread_t *current = scheduler_current();

    if (current == NULL)
    {
        for (;;)
            __asm__ volatile ("hlt");
    }

    /*
     * Execute the thread's entry function.
     */
    if (current->entry != NULL)
    {
        current->entry(current->argument);
    }

    /*
     * The entry function returned, so this
     * thread has finished execution.
     */
    current->state = THREAD_TERMINATED;

    /*
     * Do not return from the bootstrap function.
     *
     * Thread destruction/reaping will be handled
     * by the execution manager in a later phase.
     */
    for (;;)
    {
        __asm__ volatile ("hlt");
    }
}
