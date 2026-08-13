#include <stdio.h>
#include <string.h>

#include "simulator.h"

int main(void)
{
    char command[100];

    printf("=====================================\n");
    printf("         XyrisSim v0.4\n");
    printf("=====================================\n\n");

    simulator_init();

    while (1)
    {
        printf("\nXyrisSim> ");

        if (fgets(command, sizeof(command), stdin) == NULL)
            break;

        command[strcspn(command, "\n")] = '\0';

        if (simulator_execute(command) == 1)
        {
            break;
        }
    }

    return 0;
}