#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "display.h"

static void pause_ms(int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

void display_init(void)
{
    printf("[Display] Starting Display Engine...\n");
    pause_ms(400);

    printf("[Display] Loading Framebuffer...\n");
    pause_ms(400);

    printf("[Display] Initializing Terminal...\n");
    pause_ms(400);

    printf("[Display] Display Ready.\n");
}