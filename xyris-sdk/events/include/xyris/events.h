#ifndef XYRIS_EVENTS_H
#define XYRIS_EVENTS_H

/*
 * Xyris SDK Events API v0.1.
 *
 * The public ABI v0.1 does not currently assign event syscalls or define a
 * public event object/message layout. This header therefore exposes only
 * validation of ABI-safe handles that may identify future event objects.
 */

#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline xyris_bool_t xyris_event_handle_valid(xyris_handle_t handle)
{
    return handle != XYRIS_INVALID_HANDLE ? XYRIS_TRUE : XYRIS_FALSE;
}

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_EVENTS_H */
