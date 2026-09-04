#ifndef XYRIS_ABI_MEMORY_H
#define XYRIS_ABI_MEMORY_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_MEMORY_PROT_NONE  0u
#define XYRIS_MEMORY_PROT_READ  (1u << 0)
#define XYRIS_MEMORY_PROT_WRITE (1u << 1)
#define XYRIS_MEMORY_PROT_EXEC  (1u << 2)

#define XYRIS_MEMORY_FLAG_PRIVATE  (1u << 0)
#define XYRIS_MEMORY_FLAG_SHARED   (1u << 1)
#define XYRIS_MEMORY_FLAG_GUARD    (1u << 2)
#define XYRIS_MEMORY_FLAG_RESERVED (1u << 3)

typedef struct xyris_memory_region {
    xyris_abi_header_t header;
    xyris_addr_t base;
    xyris_size_t size;
    xyris_u32 protection;
    xyris_u32 flags;
    xyris_handle_t object;
    xyris_u64 offset;
} xyris_memory_region_t;

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_ABI_MEMORY_H */
