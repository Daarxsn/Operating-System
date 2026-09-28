#include "cpu.h"

static cpu_info_t g_cpu_info =
{
    .processor_count = 1
};

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

void cpu_set_processor_count(uint32_t count)
{
    if (count == 0)
        count = 1;

    g_cpu_info.processor_count = count;
}

cpu_info_t cpu_info(void)
{
    return g_cpu_info;
}