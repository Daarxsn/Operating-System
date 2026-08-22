#include "thread.h"
#include "context.h"
#include "scheduler.h"
#include <stddef.h>
#include <stdint.h>
#include "../lib/string.h"
#include "../memory/heap.h"

extern void user_thread_bootstrap(void);

static thread_t thread_table[THREAD_MAX_COUNT];
static thread_t *current_thread = NULL;
static thread_id_t next_tid = 1;

static void thread_bootstrap(void)
{
    thread_t *thread = current_thread;
    if (thread == NULL || thread->entry == NULL)
    {
        scheduler_exit_current();
        for (;;) __asm__ volatile("hlt");
    }

    thread->entry();
    scheduler_exit_current();
    for (;;) __asm__ volatile("hlt");
}

void thread_initialize(void)
{
    memset(thread_table, 0, sizeof(thread_table));
    current_thread = NULL;
    next_tid = 1;
}

thread_t *thread_create_user(
    process_t *owner,
    uint64_t entry,
    uint64_t user_stack,
    thread_priority_t priority)
{
    if (owner == NULL ||
        entry == 0 ||
        user_stack == 0)
    {
        return NULL;
    }

    for (uint32_t i = 0; i < THREAD_MAX_COUNT; ++i)
    {
        if (thread_table[i].tid != 0)
            continue;

        thread_t *thread = &thread_table[i];

        memset(thread, 0, sizeof(*thread));

        thread->tid = next_tid++;
        thread->owner = owner;
        thread->entry = NULL;
        thread->state = THREAD_CREATED;
        thread->priority = priority;

        thread->user_thread = true;
        thread->user_entry = entry;
        thread->user_stack = user_stack;

        thread->stack_size = THREAD_STACK_SIZE;
        thread->stack = kmalloc(thread->stack_size);

        if (thread->stack == NULL)
        {
            memset(thread, 0, sizeof(*thread));
            return NULL;
        }

        thread->context =
            kmalloc(sizeof(context_t));

        if (thread->context == NULL)
        {
            kfree(thread->stack);
            memset(thread, 0, sizeof(*thread));
            return NULL;
        }

        memset(
            thread->context,
            0,
            sizeof(context_t)
        );

        uintptr_t stack_top =
            (uintptr_t)thread->stack +
            thread->stack_size;

        stack_top &= ~((uintptr_t)0xFULL);
        stack_top -= sizeof(uintptr_t);

        *(uintptr_t *)stack_top = 0;

        thread->context->rsp = stack_top;
        thread->context->rbp = stack_top;

        /*
         * The scheduler enters the user bootstrap as a
         * normal kernel-mode thread first.
         */
        thread->context->rip =
            (uint64_t)user_thread_bootstrap;

        thread->context->rflags = 0x202;

        thread->wake_tick = 0;
        thread->scheduler_managed = false;
        thread->started = false;
        thread->exit_accounted = false;
        thread->next = NULL;
        thread->previous = NULL;

        owner->live_threads++;

        if (owner->main_thread == NULL)
            owner->main_thread = thread;

        return thread;
    }

    return NULL;
}

thread_t *thread_create(process_t *owner, void (*entry)(void), thread_priority_t priority)
{
    if (owner == NULL || entry == NULL) return NULL;

    for (uint32_t i = 0; i < THREAD_MAX_COUNT; ++i)
    {
        if (thread_table[i].tid != 0) continue;

        thread_t *thread = &thread_table[i];
        memset(thread, 0, sizeof(*thread));
        thread->tid = next_tid++;
        thread->owner = owner;
        thread->entry = entry;
        thread->state = THREAD_CREATED;
        thread->priority = priority;
        thread->user_thread = false;
        thread->user_entry = 0;
        thread->user_stack = 0;
        thread->stack_size = THREAD_STACK_SIZE;
        thread->stack = kmalloc(thread->stack_size);

        if (thread->stack == NULL)
        {
            memset(thread, 0, sizeof(*thread));
            return NULL;
        }

        thread->context = kmalloc(sizeof(context_t));
        if (thread->context == NULL)
        {
            kfree(thread->stack);
            memset(thread, 0, sizeof(*thread));
            return NULL;
        }

        memset(thread->context, 0, sizeof(context_t));

        uintptr_t stack_top = (uintptr_t)thread->stack + thread->stack_size;
        stack_top &= ~((uintptr_t)0xFULL);
        stack_top -= sizeof(uintptr_t);
        *(uintptr_t *)stack_top = 0;

        thread->context->rsp = stack_top;
        thread->context->rbp = stack_top;
        thread->context->rip = (uint64_t)thread_bootstrap;
        thread->context->rflags = 0x202;
        thread->wake_tick = 0;
        thread->scheduler_managed = false;
        thread->started = false;
        thread->exit_accounted = false;
        thread->next = NULL;
        thread->previous = NULL;

        owner->live_threads++;
        if (owner->main_thread == NULL)
            owner->main_thread = thread;

        return thread;
    }

    return NULL;
}

void thread_destroy(thread_t *thread)
{
    if (thread == NULL) return;

    if (thread == current_thread)
    {
        thread->state = THREAD_TERMINATED;
        return;
    }

    scheduler_remove_thread(thread);

    if (!thread->exit_accounted && thread->owner != NULL)
    {
        process_thread_exited(thread->owner);
        thread->exit_accounted = true;
    }

    thread->state = THREAD_TERMINATED;

    if (thread->owner != NULL && thread->owner->main_thread == thread)
        thread->owner->main_thread = NULL;

    if (thread->stack != NULL)
    {
        kfree(thread->stack);
        thread->stack = NULL;
    }

    if (thread->context != NULL)
    {
        kfree(thread->context);
        thread->context = NULL;
    }

    memset(thread, 0, sizeof(*thread));
}

void thread_destroy_process_threads(process_t *owner)
{
    if (owner == NULL) return;

    for (uint32_t i = 0; i < THREAD_MAX_COUNT; ++i)
    {
        thread_t *thread = &thread_table[i];
        if (thread->tid == 0 || thread->owner != owner) continue;

        if (thread == current_thread)
        {
            thread->state = THREAD_TERMINATED;
            continue;
        }

        thread_destroy(thread);
    }

    owner->main_thread = NULL;
}

thread_t *thread_current(void) { return current_thread; }
void thread_set_current(thread_t *thread) { current_thread = thread; }
