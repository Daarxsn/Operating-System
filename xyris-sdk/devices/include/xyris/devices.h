#ifndef XYRIS_DEVICES_H
#define XYRIS_DEVICES_H

/*
 * Xyris SDK Devices API v0.1.
 *
 * ABI v0.1 does not yet assign device-management syscalls or publish a
 * device object structure. This header therefore exposes only validation
 * helpers for ABI-safe object/handle identifiers.
 */

#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline xyris_bool_t xyris_device_handle_valid(xyris_handle_t handle)
{
    return handle != XYRIS_INVALID_HANDLE ? XYRIS_TRUE : XYRIS_FALSE;
}

static inline xyris_bool_t xyris_device_object_valid(xyris_object_id_t object)
{
    return object != XYRIS_INVALID_OBJECT ? XYRIS_TRUE : XYRIS_FALSE;
}

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_DEVICES_H */
