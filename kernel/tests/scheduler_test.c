#include "scheduler_test.h"

#include <stddef.h>
#include <stdint.h>

#include "../boot/boot.h"
#include "../process/process.h"
#include "../process/thread.h"
#include "../process/scheduler.h"

static void scheduler_dummy_a(void) { }
static void scheduler_dummy_b(void) { }

static void scheduler_test_ok(const char *name, int condition)
{
    if (condition) boot_step_ok(name);
    else boot_step_fail(name);
}

void scheduler_test_run(void)
{
    process_t *process = process_create("scheduler-test", true);
    scheduler_test_ok("Scheduler Test: Create Process", process != NULL);
    if (process == NULL) return;

    scheduler_test_ok(
        "Scheduler Test: Parent Tracking",
        process->parent_pid != 0
    );

    thread_t *a = thread_create(process, scheduler_dummy_a, THREAD_PRIORITY_NORMAL);
    thread_t *b = thread_create(process, scheduler_dummy_b, THREAD_PRIORITY_NORMAL);

    scheduler_test_ok("Scheduler Test: Create Thread A", a != NULL);
    scheduler_test_ok("Scheduler Test: Create Thread B", b != NULL);

    if (a == NULL || b == NULL)
    {
        if (a != NULL) thread_destroy(a);
        if (b != NULL) thread_destroy(b);
        process_destroy(process);
        return;
    }

    scheduler_test_ok(
        "Scheduler Test: Live Thread Accounting",
        process->live_threads == 2
    );

    scheduler_add_thread(a);
    scheduler_add_thread(b);

    scheduler_test_ok("Scheduler Test: Thread A Ready", a->state == THREAD_READY);
    scheduler_test_ok("Scheduler Test: Thread B Ready", b->state == THREAD_READY);

    scheduler_add_thread(a);

    scheduler_remove_thread(a);
    scheduler_test_ok(
        "Scheduler Test: Remove Thread A",
        !a->scheduler_managed && a->state == THREAD_READY
    );

    scheduler_remove_thread(b);
    scheduler_test_ok(
        "Scheduler Test: Remove Thread B",
        !b->scheduler_managed && b->state == THREAD_READY
    );

    thread_destroy(a);
    thread_destroy(b);

    scheduler_test_ok(
        "Scheduler Test: Thread Cleanup",
        process->main_thread == NULL && process->live_threads == 0
    );

    process->exit_code = 42;
    process->exit_requested = true;
    process->state = PROCESS_TERMINATED;

    int32_t status = 0;
    int reap_result = process_reap(process->pid, &status);

    scheduler_test_ok(
        "Scheduler Test: Process Reap",
        reap_result == 0 && status == 42
    );

    scheduler_test_ok(
        "Scheduler Test: PCB Released",
        process->pid == 0
    );
}
