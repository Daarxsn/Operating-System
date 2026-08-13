#ifndef XYRIS_QUEUE_H
#define XYRIS_QUEUE_H

#include <stdint.h>
#include <stdbool.h>

#include "thread.h"

/*
 * ============================================================
 * XyrisOS Thread Queue
 * ------------------------------------------------------------
 * Generic queue used by the scheduler for ready, blocked,
 * and sleeping thread queues.
 * ============================================================
 */

typedef struct thread_queue
{
    thread_t *head;
    thread_t *tail;

    uint32_t count;

} thread_queue_t;

/*
 * ------------------------------------------------------------
 * Queue API
 * ------------------------------------------------------------
 */

void queue_initialize(
    thread_queue_t *queue
);

bool queue_is_empty(
    const thread_queue_t *queue
);

bool queue_contains(
    const thread_queue_t *queue,
    const thread_t *thread
);

void queue_push(
    thread_queue_t *queue,
    thread_t *thread
);

thread_t *queue_pop(
    thread_queue_t *queue
);

bool queue_remove(
    thread_queue_t *queue,
    thread_t *thread
);

#endif