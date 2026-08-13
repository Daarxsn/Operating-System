#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "cpu.h"

int cpu_ready = 0;

static void pause_ms(int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

void cpu_init(void)
{
    cpu_ready = 0;

    printf("[CPU] Detecting Processor...\n");
    pause_ms(350);

    printf("[CPU] Initializing Registers...\n");
    pause_ms(350);

    cpu_ready = 1;

    printf("[CPU] CPU Ready.\n");
}