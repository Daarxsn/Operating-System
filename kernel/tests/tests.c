#include "tests.h"

#include "memory_tests.h"
#include "fs_tests.h"
#include "scheduler_test.h"

void run_kernel_tests(void)
{
    /*
     * Phase 2:
     * Physical memory, heap and virtual memory tests.
     */
    run_memory_tests();

    /*
     * Phase 3:
     * VFS, RAMFS, file API, syscall,
     * ELF and user-space preparation tests.
     */
    run_filesystem_tests();

    /*
     * Phase 4:
     * Process/thread registration and scheduler lifecycle tests.
     *
     * The actual context-switch smoke test runs later from
     * kernel/core/kernel.c because scheduler_start() does not
     * return to the boot path.
     */
    scheduler_test_run();
}
