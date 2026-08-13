/*
 * execution.c
 * XyrisOS Kernel
 *
 * Execution Manager
 */

#include "execution.h"
#include "scheduler.h"
#include <stddef.h>

static thread_t* thread_list = NULL;

void execution_init(void)
{
    thread_init();
    scheduler_init();

    thread_list = NULL;
}

thread_t* execution_create(
    void (*entry)(void*),
    void* argument)
{
    thread_t* thread = thread_create(entry, argument);

    if (thread == NULL)
        return NULL;

    if (thread_list == NULL)
    {
        thread_list = thread;
    }
    else
    {
        thread_t* current = thread_list;

        while (current->next_all != NULL)
        {
            current = current->next_all;
        }

        current->next_all = thread;
    }

    scheduler_add(thread);
    return thread;
}

void execution_destroy(thread_t* thread)
{
    if (thread == NULL)
        return;

    if (thread == thread_list)
    {
        thread_list = thread->next_all;
    }
    else
    {
        thread_t* current = thread_list;

        while (current != NULL &&
               current->next_all != thread)
        {
            current = current->next_all;
        }

        if (current != NULL)
            current->next_all = thread->next_all;
    }

    scheduler_remove(thread);
    thread_destroy(thread);
}

void execution_yield(void)
{
    scheduler_tick();
}

thread_t* execution_first(void)
{
    return thread_list;
}