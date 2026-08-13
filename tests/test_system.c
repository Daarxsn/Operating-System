#include <stdio.h>

#include "../simulator/include/system.h"

#define ASSERT(condition, message)        \
    do                                    \
    {                                     \
        if (!(condition))                 \
        {                                 \
            printf("[FAIL] %s\n", message); \
            return 1;                     \
        }                                 \
        printf("[PASS] %s\n", message);   \
    } while (0)

int test_system(void)
{
    printf("\nRunning System Tests...\n");

    /*
     * Simulate an invalid/corrupted system state
     * before initialization.
     */
    system_state.cpu_online = 0;
    system_state.memory_online = 0;
    system_state.display_online = 0;
    system_state.kernel_loaded = 1;
    system_state.ram_mb = 0;

    system_init();

    ASSERT(system_state.cpu_online == 1,
           "CPU should be online after system initialization");

    ASSERT(system_state.memory_online == 1,
           "Memory should be online after system initialization");

    ASSERT(system_state.display_online == 1,
           "Display should be online after system initialization");

    ASSERT(system_state.kernel_loaded == 0,
           "Kernel should not be loaded initially");

    ASSERT(system_state.ram_mb == 64,
           "System should initialize with 64 MB RAM");

    /*
     * Verify that system_init() can safely restore
     * the system state after it has been modified.
     */
    system_state.cpu_online = 0;
    system_state.memory_online = 0;
    system_state.display_online = 0;
    system_state.kernel_loaded = 1;
    system_state.ram_mb = 128;

    system_init();

    ASSERT(system_state.cpu_online == 1,
           "CPU should recover after system state corruption");

    ASSERT(system_state.memory_online == 1,
           "Memory should recover after system state corruption");

    ASSERT(system_state.display_online == 1,
           "Display should recover after system state corruption");

    ASSERT(system_state.kernel_loaded == 0,
           "Kernel state should reset after system state corruption");

    ASSERT(system_state.ram_mb == 64,
           "RAM configuration should reset after system state corruption");

    printf("System tests passed!\n");

    return 0;
}