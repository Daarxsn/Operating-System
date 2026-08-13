#ifndef XYRIS_THREAD_H
#define XYRIS_THREAD_H

#include <stdint.h>
#include <stdbool.h>

#include "process.h"

/*
 * ============================================================
 * XyrisOS Thread Manager
 * ------------------------------------------------------------
 * Defines the Thread Control Block (TCB) and thread lifecycle.
 * ============================================================
 */

/*
 * ------------------------------------------------------------
 * Configuration
 * ------------------------------------------------------------
 */

#define THREAD_MAX_COUNT      256
#define THREAD_STACK_SIZE     (16 * 1024)

/*
 * ------------------------------------------------------------
 * Forward Declaration
 * ------------------------------------------------------------
 */

struct context;

typedef struct context context_t;

/*
 * ------------------------------------------------------------
 * Thread Identifier
 * ------------------------------------------------------------
 */

typedef uint32_t thread_id_t;

/*
 * ------------------------------------------------------------
 * Thread State
 * ------------------------------------------------------------
 */

typedef enum
{
    THREAD_CREATED = 0,

    THREAD_READY,

    THREAD_RUNNING,

    THREAD_BLOCKED,

    THREAD_SLEEPING,

    THREAD_TERMINATED

} thread_state_t;

/*
 * ------------------------------------------------------------
 * Thread Priority
 * ------------------------------------------------------------
 */

typedef enum
{
    THREAD_PRIORITY_IDLE = 0,

    THREAD_PRIORITY_LOW,

    THREAD_PRIORITY_NORMAL,

    THREAD_PRIORITY_HIGH,

    THREAD_PRIORITY_REALTIME

} thread_priority_t;

/*
 * ------------------------------------------------------------
 * Thread Control Block
 * ------------------------------------------------------------
 */

typedef struct thread
{
    /*
     * Identification
     */

    thread_id_t tid;

    /*
     * Owner Process
     */

    process_t *owner;

    /*
     * Entry Point
     */

    void (*entry)(void);

    /*
     * Current State
     */

    thread_state_t state;

    /*
     * Scheduling Priority
     */

    thread_priority_t priority;

    /*
     * CPU Context
     */

    context_t *context;

    /*
     * Kernel Stack
     */

    void *stack;

    uint64_t stack_size;

    /*
     * Runtime Statistics
     */

    uint64_t cpu_time;

    uint64_t time_slice;

    /*
     * Scheduler wake-up tick for sleeping threads.
     */
    uint64_t wake_tick;

    /*
     * Whether this thread is registered with the scheduler.
     */
    bool scheduler_managed;

    /*
     * Queue Links
     */

    struct thread *next;

    struct thread *previous;

} thread_t;

/*
 * ------------------------------------------------------------
 * Thread Manager API
 * ------------------------------------------------------------
 */

void thread_initialize(void);

thread_t *thread_create(
    process_t *owner,
    void (*entry)(void),
    thread_priority_t priority
);

void thread_destroy(
    thread_t *thread
);

void thread_destroy_process_threads(
    process_t *owner
);

thread_t *thread_current(void);

void thread_set_current(
    thread_t *thread
);

#endif