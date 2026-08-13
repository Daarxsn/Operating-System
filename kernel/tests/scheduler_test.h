#ifndef XYRIS_SCHEDULER_TEST_H
#define XYRIS_SCHEDULER_TEST_H

/*
 * ============================================================
 * XyrisOS Scheduler Test
 * ------------------------------------------------------------
 * Creates dummy kernel threads and validates scheduler
 * registration, state transitions and lifecycle bookkeeping.
 *
 * The real context-switch smoke test runs from kernel/core/kernel.c
 * after all initialization has completed.
 * ============================================================
 */

void scheduler_test_run(void);

#endif