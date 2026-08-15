#include "pit.h"
#include "io.h"

#include "../process/scheduler.h"
#include "../include/foundation/time.h"

static volatile uint64_t pit_ticks = 0;
static volatile uint64_t pit_debug_handler_count = 0;
static uint32_t pit_frequency = 100;

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43

void pit_initialize(uint32_t frequency)
{
    if (frequency == 0)
    {
        frequency = 100;
    }

    pit_frequency = frequency;

    uint16_t divisor =
        (uint16_t)(
            PIT_BASE_FREQUENCY /
            frequency
        );

    outb(PIT_COMMAND, 0x36);

    outb(
        PIT_CHANNEL0,
        divisor & 0xFF
    );

    outb(
        PIT_CHANNEL0,
        divisor >> 8
    );
}

void pit_handler(void)
{
    pit_debug_handler_count++;
    pit_ticks++;

    xk_time_tick();
    scheduler_tick();
}

uint64_t pit_get_ticks(void)
{
    return pit_ticks;
}

void pit_sleep(uint64_t milliseconds)
{
    uint64_t start = pit_ticks;

    uint64_t wait =
        (milliseconds * pit_frequency) /
        1000;

    while ((pit_ticks - start) < wait)
    {
        __asm__ volatile ("pause");
    }
}

uint64_t pit_debug_get_handler_count(void)
{
    return pit_debug_handler_count;
}