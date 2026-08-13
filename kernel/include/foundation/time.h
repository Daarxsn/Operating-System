#ifndef XK_FOUNDATION_TIME_H
#define XK_FOUNDATION_TIME_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 * XyrisOS Time Manager
 *
 * Responsible for kernel timekeeping and software timers.
 * ============================================================
 */

#define XK_MAX_TIMERS 64

typedef void (*XKTimerCallback)(void *context);

typedef struct XKTimer
{
    bool active;

    uint64_t id;

    uint64_t start_tick;

    uint64_t timeout_ticks;

    XKTimerCallback callback;

    void *context;

} XKTimer;

/* ------------------------------------------------------------
 * Time Manager
 * ------------------------------------------------------------ */

void xk_time_init(void);

void xk_time_tick(void);

uint64_t xk_time_ticks(void);

uint64_t xk_time_milliseconds(void);

void xk_sleep(uint64_t milliseconds);

/* ------------------------------------------------------------
 * Software Timers
 * ------------------------------------------------------------ */

XKTimer *xk_timer_create(
    uint64_t timeout_ticks,
    XKTimerCallback callback,
    void *context);

bool xk_timer_cancel(
    XKTimer *timer);

void xk_timer_poll(void);

#endif