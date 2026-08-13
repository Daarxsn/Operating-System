#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "memory.h"

int memory_ready = 0;

static void pause_ms(int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

void memory_init(void)
{
    memory_ready = 0;

    printf("[Memory] Checking RAM...\n");
    pause_ms(350);

    printf("[Memory] Mapping Memory...\n");
    pause_ms(350);

    memory_ready = 1;

    printf("[Memory] Memory Ready.\n");
}