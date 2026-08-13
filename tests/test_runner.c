#include <stdio.h>

int test_system(void);
int test_memory(void);
int test_cpu(void);
int test_simulator_commands(void);

int main(void)
{
    printf("=====================================\n");
    printf("       XyrisOS Test Runner\n");
    printf("=====================================\n");

    int result = 0;

    result |= test_system();
    result |= test_memory();
    result |= test_cpu();
    result |= test_simulator_commands();

    printf("\n=====================================\n");

    if (result == 0)
    {
        printf("ALL TESTS PASSED\n");
    }
    else
    {
        printf("SOME TESTS FAILED\n");
    }

    printf("=====================================\n");

    return result;
}