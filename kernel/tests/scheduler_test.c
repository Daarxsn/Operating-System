#include "scheduler_test.h"

#include <stddef.h>

#include "../boot/boot.h"
#include "../process/process.h"
#include "../process/thread.h"
#include "../process/scheduler.h"

static void scheduler_dummy_a(void)
{
    /*
     * This function is intentionally never executed by this
     * pre-scheduler structural test.
     */
}

static void scheduler_dummy_b(void)
{
    /*
     * This function is intentionally never executed by this
     * pre-scheduler structural test.
     */
}

static void scheduler_test_ok(
    const char *name,
    int condition)
{
    if (condition)
        boot_step_ok(name);
    else
        boot_step_fail(name);
}

void scheduler_test_run(void)
{
    /*
     * This test runs before scheduler_start(), so it validates
     * registration/state/lifecycle behavior without performing
     * a context switch from the boot path.
     */

    process_t *process =
        process_create(
            "scheduler-test",
            true
        );

    scheduler_test_ok(
        "Scheduler Test: Create Process",
        process != NULL
    );

    if (process == NULL)
        return;

    thread_t *a =
        thread_create(
            process,
            scheduler_dummy_a,
            THREAD_PRIORITY_NORMAL
        );

    thread_t *b =
        thread_create(
            process,
            scheduler_dummy_b,
            THREAD_PRIORITY_NORMAL
        );

    scheduler_test_ok(
        "Scheduler Test: Create Thread A",
        a != NULL
    );

    scheduler_test_ok(
        "Scheduler Test: Create Thread B",
        b != NULL
    );

    if (a == NULL || b == NULL)
    {
        if (a != NULL)
            thread_destroy(a);

        if (b != NULL)
            thread_destroy(b);

        process_destroy(process);
        return;
    }

    scheduler_add_thread(a);
    scheduler_add_thread(b);

    scheduler_test_ok(
        "Scheduler Test: Thread A Ready",
        a->state == THREAD_READY
    );

    scheduler_test_ok(
        "Scheduler Test: Thread B Ready",
        b->state == THREAD_READY
    );

    /*
     * Duplicate registration must not change a READY thread
     * into multiple queue entries.
     */
    scheduler_add_thread(a);

    scheduler_remove_thread(a);

    scheduler_test_ok(
        "Scheduler Test: Remove Thread A",
        !a->scheduler_managed &&
        a->state == THREAD_READY
    );

    scheduler_remove_thread(b);

    scheduler_test_ok(
        "Scheduler Test: Remove Thread B",
        !b->scheduler_managed &&
        b->state == THREAD_READY
    );

    thread_destroy(a);
    thread_destroy(b);

    scheduler_test_ok(
        "Scheduler Test: Thread Cleanup",
        process->main_thread == NULL
    );

    process_destroy(process);
}
