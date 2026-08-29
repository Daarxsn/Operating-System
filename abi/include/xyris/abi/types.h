#ifndef XYRIS_ABI_TYPES_H
#define XYRIS_ABI_TYPES_H

/*
 * Xyris System ABI v0.1 — fundamental public types.
 *
 * This header is part of the public application ABI. It must not include
 * private kernel headers or expose kernel object layouts.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Fixed-width scalar types used at ABI boundaries. */
typedef uint8_t   xyris_u8;
typedef uint16_t  xyris_u16;
typedef uint32_t  xyris_u32;
typedef uint64_t  xyris_u64;

typedef int8_t    xyris_i8;
typedef int16_t   xyris_i16;
typedef int32_t   xyris_i32;
typedef int64_t   xyris_i64;

/* Explicitly sized ABI quantities. */
typedef xyris_u64 xyris_size_t;
typedef xyris_i64 xyris_ssize_t;
typedef xyris_u64 xyris_addr_t;
typedef xyris_u64 xyris_user_ptr_t;
typedef xyris_u64 xyris_offset_t;

/* Kernel object identifiers. */
typedef xyris_u32 xyris_pid_t;
typedef xyris_u32 xyris_tid_t;
typedef xyris_u64 xyris_handle_t;
typedef xyris_u64 xyris_capability_t;
typedef xyris_u64 xyris_object_id_t;

/* File descriptors are signed to reserve negative values for errors. */
typedef xyris_i32 xyris_fd_t;

/* Time is represented in nanoseconds at the ABI boundary. */
typedef xyris_u64 xyris_time_ns_t;
typedef xyris_u64 xyris_duration_ns_t;

/* Generic ABI status/result value. */
typedef xyris_i64 xyris_status_t;

/* Fixed-width ABI boolean. */
typedef xyris_u8 xyris_bool_t;

#define XYRIS_TRUE  ((xyris_bool_t)1u)
#define XYRIS_FALSE ((xyris_bool_t)0u)

/* Reserved invalid/sentinel values. */
#define XYRIS_INVALID_HANDLE ((xyris_handle_t)0u)
#define XYRIS_INVALID_PID    ((xyris_pid_t)0u)
#define XYRIS_INVALID_TID    ((xyris_tid_t)0u)
#define XYRIS_INVALID_FD     ((xyris_fd_t)-1)
#define XYRIS_INVALID_CAP    ((xyris_capability_t)0u)
#define XYRIS_INVALID_OBJECT ((xyris_object_id_t)0u)

/*
 * Extensible structures crossing the ABI boundary begin with this header
 * when the individual structure specification says so.
 *
 * size    = caller-supplied structure size in bytes
 * version = structure revision
 * flags   = structure-specific flags
 */
typedef struct xyris_abi_header {
    xyris_u32 size;
    xyris_u16 version;
    xyris_u16 flags;
} xyris_abi_header_t;

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_ABI_TYPES_H */
