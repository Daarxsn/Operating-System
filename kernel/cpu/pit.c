#include "pit.h"
#include "io.h"

#include "../debug/print.h"

#include "../process/scheduler.h"
#include "../include/foundation/time.h"

static volatile uint64_t pit_ticks = 0;
static volatile uint64_t pit_debug_handler_count = 0;
static uint32_t pit_frequency = 100;

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43

void pit_initialize(uint32_t frequency)
{
    debug_print_line("PIT DEBUG: entered pit_initialize");
    debug_print_line("PIT DEBUG: before frequency check");

    if (frequency == 0)
    {
        frequency = 100;
    }

    debug_print_line("PIT DEBUG: frequency validated");

    pit_frequency = frequency;

    debug_print_line("PIT DEBUG: setting time frequency");

    (void)xk_time_set_frequency(frequency);

    debug_print_line("PIT DEBUG: time frequency set");

    uint16_t divisor =
        (uint16_t)(
            PIT_BASE_FREQUENCY /
            frequency
        );

    debug_print_line("PIT DEBUG: divisor calculated");

    outb(PIT_COMMAND, 0x36);

    debug_print_line("PIT DEBUG: command written");

    outb(
        PIT_CHANNEL0,
        divisor & 0xFF
    );

    debug_print_line("PIT DEBUG: low divisor written");

    outb(
        PIT_CHANNEL0,
        divisor >> 8
    );

    debug_print_line("PIT DEBUG: high divisor written");

    debug_print_line("PIT: hardware initialized");
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