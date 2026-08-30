#ifndef XYRIS_TIMERS_H
#define XYRIS_TIMERS_H

/*
 * Xyris SDK Timers API v0.1.
 *
 * The public ABI v0.1 defines nanosecond time/duration types, but does not
 * assign timer syscalls or define a public timer object structure. This
 * header therefore exposes only validation of ABI-safe handles.
 */

#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline xyris_bool_t xyris_timer_handle_valid(xyris_handle_t handle)
{
    return handle != XYRIS_INVALID_HANDLE ? XYRIS_TRUE : XYRIS_FALSE;
}

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_TIMERS_H */
