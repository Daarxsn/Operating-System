#ifndef XYRIS_IPC_H
#define XYRIS_IPC_H

/*
 * Xyris SDK IPC API v0.1.
 *
 * The public ABI v0.1 defines generic handle, capability, and object
 * identifiers, but does not assign IPC syscalls or an IPC message layout.
 * This module therefore exposes only ABI-safe identifier validation.
 */

#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline xyris_bool_t xyris_ipc_handle_valid(xyris_handle_t handle)
{
    return handle != XYRIS_INVALID_HANDLE ? XYRIS_TRUE : XYRIS_FALSE;
}

static inline xyris_bool_t xyris_ipc_capability_valid(
    xyris_capability_t capability)
{
    return capability != XYRIS_INVALID_CAP ? XYRIS_TRUE : XYRIS_FALSE;
}

static inline xyris_bool_t xyris_ipc_object_valid(xyris_object_id_t object)
{
    return object != XYRIS_INVALID_OBJECT ? XYRIS_TRUE : XYRIS_FALSE;
}

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_IPC_H */
