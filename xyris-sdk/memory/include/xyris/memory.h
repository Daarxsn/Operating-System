#ifndef XYRIS_MEMORY_H
#define XYRIS_MEMORY_H

/*
 * Xyris SDK Memory API v0.1.
 *
 * The public ABI v0.1 defines memory address and size types, but does not
 * assign memory-management syscalls. This module therefore exposes only
 * ABI-safe validation helpers and does not invent kernel operations.
 */

#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline xyris_bool_t xyris_memory_range_valid(
    xyris_addr_t address,
    xyris_size_t size
)
{
    if (size == 0u)
        return XYRIS_FALSE;

    return address <= UINT64_MAX - (size - 1u)
        ? XYRIS_TRUE
        : XYRIS_FALSE;
}

static inline xyris_bool_t xyris_memory_address_valid(xyris_addr_t address)
{
    return address != 0u ? XYRIS_TRUE : XYRIS_FALSE;
}

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_MEMORY_H */
