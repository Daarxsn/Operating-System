#include "tests.h"

#include "memory_tests.h"
#include "fs_tests.h"
#include "scheduler_test.h"
#include "foundation_tests.h"
#include "sdk_services_tests.h"

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
     */
    scheduler_test_run();

    /*
     * Phase 5:
     * Foundation and driver manager tests.
     */
    run_foundation_tests();

    /* Phase 6: kernel-backed public SDK service boundary. */
    run_sdk_services_tests();
}