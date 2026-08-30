#ifndef XYRIS_ABI_SYSCALLS_H
#define XYRIS_ABI_SYSCALLS_H

#include "types.h"

/*
 * Xyris System ABI v0.1 syscall numbers.
 *
 * Numbers are part of the ABI and MUST NOT be reused for a different
 * operation once assigned. Unused values remain reserved so future
 * additions can be made without renumbering existing calls.
 */
enum {
    XYRIS_SYS_READ  = 0u,
    XYRIS_SYS_WRITE = 1u,
    XYRIS_SYS_OPEN  = 2u,
    XYRIS_SYS_CLOSE = 3u,
    XYRIS_SYS_EXIT  = 4u,

    XYRIS_SYS_MAX = 5u
};

/*
 * v0.1 syscall calling convention for the current x86-64 implementation:
 *
 *   RAX = syscall number
 *   RDI = argument 1
 *   RSI = argument 2
 *   RDX = argument 3
 *   RCX = argument 4
 *   RAX = signed 64-bit result on return
 *
 * A non-negative result indicates success. A negative result is an
 * xyris_error_t value. Positive values are operation-specific results
 * such as a file descriptor or byte count.
 */
typedef xyris_u64 xyris_syscall_number_t;
typedef xyris_u64 xyris_syscall_arg_t;
typedef xyris_status_t xyris_syscall_result_t;

#endif /* XYRIS_ABI_SYSCALLS_H */
