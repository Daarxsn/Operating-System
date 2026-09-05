#ifndef XYRIS_TIMERS_H
#define XYRIS_TIMERS_H

#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_SDK_TIMERS_VERSION_MAJOR 0u
#define XYRIS_SDK_TIMERS_VERSION_MINOR 1u

static inline xyris_bool_t xyris_timer_handle_valid(xyris_handle_t handle)
{ return handle != XYRIS_INVALID_HANDLE ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_timer_mode_valid(xyris_u32 mode)
{ return mode == XYRIS_TIMER_ONESHOT || mode == XYRIS_TIMER_PERIODIC ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_timer_clock_valid(xyris_u32 clock)
{ return clock == XYRIS_TIMER_CLOCK_MONOTONIC || clock == XYRIS_TIMER_CLOCK_REALTIME ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_timer_flags_valid(xyris_u32 flags)
{ const xyris_u32 known = XYRIS_TIMER_FLAG_ABSOLUTE | XYRIS_TIMER_FLAG_WAKE; return (flags & ~known) == 0u ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_timer_spec_valid(const xyris_timer_spec_t *spec)
{ return spec && spec->header.size >= sizeof(*spec) && spec->header.version != 0u && xyris_timer_handle_valid(spec->timer) && xyris_timer_mode_valid(spec->mode) && xyris_timer_clock_valid(spec->clock) && xyris_timer_flags_valid(spec->flags) && (spec->mode != XYRIS_TIMER_PERIODIC || spec->interval != 0u) ? XYRIS_TRUE : XYRIS_FALSE; }
static inline void xyris_timer_spec_init(xyris_timer_spec_t *spec, xyris_handle_t timer, xyris_u32 mode, xyris_u32 clock, xyris_u32 flags, xyris_time_ns_t deadline, xyris_duration_ns_t interval)
{ if (!spec) return; spec->header.size = sizeof(*spec); spec->header.version = 1u; spec->header.flags = 0u; spec->timer = timer; spec->mode = mode; spec->clock = clock; spec->flags = flags; spec->reserved0 = 0u; spec->deadline = deadline; spec->interval = interval; }

static inline xyris_handle_t xyris_timer_create(xyris_timer_spec_t *spec)
{ return (xyris_handle_t)xyris_syscall1(XYRIS_SYS_TIMER_CREATE, (xyris_user_ptr_t)(uintptr_t)spec); }
static inline xyris_status_t xyris_timer_cancel(xyris_handle_t timer)
{ return xyris_syscall1(XYRIS_SYS_TIMER_CANCEL, timer); }
static inline xyris_status_t xyris_timer_wait(xyris_handle_t timer)
{ return xyris_syscall1(XYRIS_SYS_TIMER_WAIT, timer); }
static inline xyris_status_t xyris_timer_close(xyris_handle_t timer)
{ return xyris_syscall1(XYRIS_SYS_TIMER_CLOSE, timer); }

#ifdef __cplusplus
}
#endif
#endif
