#ifndef XYRIS_NETWORKING_H
#define XYRIS_NETWORKING_H

/*
 * Xyris SDK Networking API v0.1.
 *
 * ABI v0.1 does not yet publish networking syscalls or socket/address
 * structures. The public SDK therefore provides only ABI-safe descriptor
 * validation.
 */

#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline xyris_bool_t xyris_network_fd_valid(xyris_fd_t fd)
{
    return fd != XYRIS_INVALID_FD ? XYRIS_TRUE : XYRIS_FALSE;
}

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_NETWORKING_H */
