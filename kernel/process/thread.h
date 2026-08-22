#ifndef XYRIS_THREAD_H
#define XYRIS_THREAD_H

#include <stdint.h>
#include <stdbool.h>
#include "process.h"

#define THREAD_MAX_COUNT      256
#define THREAD_STACK_SIZE     (16 * 1024)

struct context;
typedef struct context context_t;
typedef uint32_t thread_id_t;

typedef enum
{
    THREAD_CREATED = 0,
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_SLEEPING,
    THREAD_TERMINATED
} thread_state_t;

typedef enum
{
    THREAD_PRIORITY_IDLE = 0,
    THREAD_PRIORITY_LOW,
    THREAD_PRIORITY_NORMAL,
    THREAD_PRIORITY_HIGH,
    THREAD_PRIORITY_REALTIME
} thread_priority_t;

typedef struct thread
{
    thread_id_t tid;
    process_t *owner;
    void (*entry)(void);
    thread_state_t state;
    thread_priority_t priority;

    context_t *context;

    void *stack;
    uint64_t stack_size;

    /*
     * --------------------------------------------------------
     * User-mode execution
     * --------------------------------------------------------
     *
     * Kernel threads leave these fields unused.
     *
     * A user thread stores the Ring-3 entry point and initial
     * user stack here. The actual iret frame will be prepared
     * when we implement the user-mode bootstrap.
     */
    bool user_thread;
    uint64_t user_entry;
    uint64_t user_stack;

    uint64_t cpu_time;
    uint64_t time_slice;
    uint64_t wake_tick;

    bool scheduler_managed;
    bool started;
    bool exit_accounted;

    struct thread *next;
    struct thread *previous;

} thread_t;

void thread_initialize(void);

thread_t *thread_create(
    process_t *owner,
    void (*entry)(void),
    thread_priority_t priority
);

thread_t *thread_create_user(
    process_t *owner,
    uint64_t entry,
    uint64_t user_stack,
    thread_priority_t priority
);

void thread_destroy(thread_t *thread);
void thread_destroy_process_threads(process_t *owner);

thread_t *thread_current(void);
void thread_set_current(thread_t *thread);

#endif