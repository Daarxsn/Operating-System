#ifndef XYRIS_THREAD_H
#define XYRIS_THREAD_H

/*
 * Xyris SDK Thread API v0.1.
 *
 * The public ABI defines xyris_tid_t and thread-related sentinels, but v0.1
 * does not assign any thread syscalls yet. This header therefore exposes
 * ABI-safe thread identity helpers without inventing kernel operations.
 */

#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline xyris_bool_t xyris_thread_id_valid(xyris_tid_t tid)
{
    return tid != XYRIS_INVALID_TID ? XYRIS_TRUE : XYRIS_FALSE;
}

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_THREAD_H */
