#ifndef XYRIS_SECURITY_H
#define XYRIS_SECURITY_H

/*
 * Xyris SDK Security API v0.1.
 *
 * ABI v0.1 exposes capability identifiers but does not yet publish
 * capability-management syscalls or a permission structure.
 */

#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline xyris_bool_t xyris_capability_valid(xyris_capability_t capability)
{
    return capability != XYRIS_INVALID_CAP ? XYRIS_TRUE : XYRIS_FALSE;
}

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_SECURITY_H */
