#include "foundation/time.h"

#include <stddef.h>

/* ============================================================
 * XyrisOS Time Manager
 * ============================================================
 * The PIT is configured in Hz, while the public millisecond API
 * must remain correct for arbitrary integer tick frequencies.
 * Fractional milliseconds are accumulated instead of truncating
 * every tick independently.
 */

static volatile uint64_t kernel_ticks = 0;
static volatile uint64_t kernel_milliseconds = 0;
static volatile uint64_t millisecond_remainder = 0;
static uint32_t kernel_frequency_hz = 100;
static uint64_t timer_id_counter = 1;

static XKTimer timers[XK_MAX_TIMERS];

void xk_time_init(void)
{
    kernel_ticks = 0;
    kernel_milliseconds = 0;
    millisecond_remainder = 0;
    kernel_frequency_hz = 100;
    timer_id_counter = 1;

    for (uint32_t i = 0; i < XK_MAX_TIMERS; i++)
    {
        timers[i].active = false;
        timers[i].id = 0;
        timers[i].start_tick = 0;
        timers[i].timeout_ticks = 0;
        timers[i].callback = NULL;
        timers[i].context = NULL;
    }
}

bool xk_time_set_frequency(uint32_t frequency_hz)
{
    if (frequency_hz == 0)
        return false;

    kernel_frequency_hz = frequency_hz;
    millisecond_remainder = 0;
    return true;
}

uint32_t xk_time_frequency(void)
{
    return kernel_frequency_hz;
}

void xk_time_tick(void)
{
    kernel_ticks++;

    /* Accumulate 1000 milliseconds per second without truncation. */
    millisecond_remainder += 1000ULL;
    if (millisecond_remainder >= kernel_frequency_hz)
    {
        kernel_milliseconds +=
            millisecond_remainder / kernel_frequency_hz;
        millisecond_remainder %= kernel_frequency_hz;
    }

    xk_timer_poll();
}

uint64_t xk_time_ticks(void)
{
    return kernel_ticks;
}

uint64_t xk_time_milliseconds(void)
{
    return kernel_milliseconds;
}

void xk_sleep(uint64_t milliseconds)
{
    if (milliseconds == 0)
        return;

    uint64_t target = kernel_milliseconds + milliseconds;

    while (kernel_milliseconds < target)
    {
        __asm__ volatile ("pause");
    }
}

XKTimer *xk_timer_create(
    uint64_t timeout_ticks,
    XKTimerCallback callback,
    void *context)
{
    if (callback == NULL)
        return NULL;

    for (uint32_t i = 0; i < XK_MAX_TIMERS; i++)
    {
        if (!timers[i].active)
        {
            timers[i].active = true;
            timers[i].id = timer_id_counter++;
            timers[i].start_tick = kernel_ticks;
            timers[i].timeout_ticks = timeout_ticks;
            timers[i].callback = callback;
            timers[i].context = context;
            return &timers[i];
        }
    }

    return NULL;
}

bool xk_timer_cancel(XKTimer *timer)
{
    if (timer == NULL || !timer->active)
        return false;

    timer->active = false;
    timer->callback = NULL;
    timer->context = NULL;
    return true;
}

void xk_timer_poll(void)
{
    for (uint32_t i = 0; i < XK_MAX_TIMERS; i++)
    {
        if (!timers[i].active)
            continue;

        uint64_t elapsed = kernel_ticks - timers[i].start_tick;
        if (elapsed >= timers[i].timeout_ticks)
        {
            XKTimerCallback callback = timers[i].callback;
            void *context = timers[i].context;

            timers[i].active = false;
            timers[i].callback = NULL;
            timers[i].context = NULL;

            if (callback != NULL)
                callback(context);
        }
    }
}
