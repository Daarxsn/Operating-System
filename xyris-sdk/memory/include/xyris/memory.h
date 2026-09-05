#ifndef XYRIS_MEMORY_H
#define XYRIS_MEMORY_H

/* Xyris SDK Memory API v0.1 — kernel-backed virtual memory management. */
#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline xyris_bool_t xyris_memory_range_valid(xyris_addr_t address, xyris_size_t size)
{
    if (size == 0u) return XYRIS_FALSE;
    return address <= UINT64_MAX - (size - 1u) ? XYRIS_TRUE : XYRIS_FALSE;
}

static inline xyris_bool_t xyris_memory_address_valid(xyris_addr_t address)
{ return address != 0u ? XYRIS_TRUE : XYRIS_FALSE; }

static inline xyris_addr_t xyris_memory_map(xyris_size_t size, xyris_u32 protection, xyris_u32 flags)
{ return (xyris_addr_t)xyris_syscall3(XYRIS_SYS_MEMORY_MAP, size, protection, flags); }

static inline xyris_status_t xyris_memory_unmap(xyris_addr_t base, xyris_size_t size)
{ return xyris_syscall2(XYRIS_SYS_MEMORY_UNMAP, base, size); }

static inline xyris_status_t xyris_memory_protect(xyris_addr_t base, xyris_size_t size, xyris_u32 protection)
{ return xyris_syscall3(XYRIS_SYS_MEMORY_PROTECT, base, size, protection); }

#ifdef __cplusplus
}
#endif
#endif
