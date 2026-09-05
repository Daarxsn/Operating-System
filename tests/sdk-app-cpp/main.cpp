#include <xyris/sdk.h>

extern "C" void _start(void)
{
    static const char message[] = "XyrisOS 7.4 C++ SDK application OK\n";
    (void)xyris_write(1, message, sizeof(message) - 1);
    (void)xyris_exit(0);
    for (;;) { __asm__ volatile ("hlt"); }
}
