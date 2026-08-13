#ifndef XYRIS_THREAD_H
#define XYRIS_THREAD_H

#include <stdint.h>
#include <stddef.h>
#include "context.h"
#include "../memory/pmm.h"

#define THREAD_STACK_PAGES 4
#define THREAD_STACK_SIZE (THREAD_STACK_PAGES * PAGE_SIZE)

typedef enum
{
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_TERMINATED,
    THREAD_SLEEPING

} thread_state_t;

typedef struct thread
{
    uint64_t id;
    void (*entry)(void*);
    void* argument;

    uintptr_t stack_base;
    uintptr_t stack_top;
    size_t stack_size;

    context_t context;

    thread_state_t state;

    struct thread* next_all;
    struct thread* next_ready;

} thread_t;

void thread_init(void);

thread_t* thread_create(
    void (*entry)(void*),
    void* argument);

void thread_destroy(thread_t* thread);

#endif