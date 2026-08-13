#include "cpu.h"

uintptr_t cpu_read_cr2(void)
{
    uintptr_t value;

    __asm__ volatile(
        "mov %%cr2, %0"
        : "=r"(value));

    return value;
}

void cpu_halt(void)
{
    __asm__ volatile("hlt");
}

void cpu_halt_forever(void)
{
    __asm__ volatile("cli");

    while (1)
    {
        __asm__ volatile("hlt");
    }
}