#include <stdio.h>

#include "../simulator/include/memory.h"

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

int test_memory(void)
{
    printf("\nRunning Memory Tests...\n");

    memory_ready = 0;

    ASSERT(memory_ready == 0,
           "Memory should not be ready before initialization");

    memory_init();

    ASSERT(memory_ready == 1,
           "Memory should be ready after initialization");

    /*
     * Simulate an unexpected memory state and verify
     * that reinitialization restores memory readiness.
     */
    memory_ready = 0;

    memory_init();

    ASSERT(memory_ready == 1,
           "Memory should recover after an unexpected state");

    /*
     * Verify that repeated initialization remains safe.
     */
    memory_init();

    ASSERT(memory_ready == 1,
           "Memory should remain ready after repeated initialization");

    printf("Memory tests passed!\n");

    return 0;
}