#include "foundation/time.h"

#include <stddef.h>

/* ============================================================
 * XyrisOS Time Manager
 * ============================================================
 */

static volatile uint64_t kernel_ticks = 0;
static volatile uint64_t kernel_milliseconds = 0;
static uint64_t timer_id_counter = 1;

static XKTimer timers[XK_MAX_TIMERS];

/* ------------------------------------------------------------
 * Initialization
 * ------------------------------------------------------------ */

void xk_time_init(void)
{
    kernel_ticks = 0;
    kernel_milliseconds = 0;
    timer_id_counter = 1;

    for (uint32_t i = 0; i < XK_MAX_TIMERS; i++)
    {
        timers[i].active = false;
    }
}

/* ------------------------------------------------------------
 * Tick Handler
 * ------------------------------------------------------------ */

void xk_time_tick(void)
{
    kernel_ticks++;
    kernel_milliseconds++;

    xk_timer_poll();
}

/* ------------------------------------------------------------
 * Time Queries
 * ------------------------------------------------------------ */

uint64_t xk_time_ticks(void)
{
    return kernel_ticks;
}

uint64_t xk_time_milliseconds(void)
{
    return kernel_milliseconds;
}

/* ------------------------------------------------------------
 * Busy Wait (Temporary)
 * ------------------------------------------------------------ */

void xk_sleep(uint64_t milliseconds)
{
    uint64_t target = kernel_milliseconds + milliseconds;

    while (kernel_milliseconds < target)
    {
    }
}

/* ------------------------------------------------------------
 * Timer Creation
 * ------------------------------------------------------------ */

XKTimer *xk_timer_create(
    uint64_t timeout_ticks,
    XKTimerCallback callback,
    void *context)
{
    if (callback == NULL)
    {
        return NULL;
    }

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

/* ------------------------------------------------------------
 * Timer Cancel
 * ------------------------------------------------------------ */

bool xk_timer_cancel(XKTimer *timer)
{
    if (timer == NULL)
    {
        return false;
    }

    timer->active = false;

    return true;
}

/* ------------------------------------------------------------
 * Timer Poll
 * ------------------------------------------------------------ */

void xk_timer_poll(void)
{
    for (uint32_t i = 0; i < XK_MAX_TIMERS; i++)
    {
        if (!timers[i].active)
        {
            continue;
        }

        uint64_t elapsed = kernel_ticks - timers[i].start_tick;

        if (elapsed >= timers[i].timeout_ticks)
        {
            timers[i].active = false;

            timers[i].callback(timers[i].context);
        }
    }
}