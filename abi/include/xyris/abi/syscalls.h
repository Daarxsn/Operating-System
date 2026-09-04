#ifndef XYRIS_ABI_SYSCALLS_H
#define XYRIS_ABI_SYSCALLS_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Xyris System ABI v0.1 syscall contract.
 *
 * Assigned syscall numbers are permanent within ABI major version 0. New
 * operations are appended or allocated from an explicitly reserved range;
 * existing numbers are never reused for another operation.
 */
#define XYRIS_SYSCALL_ABI_VERSION 1u
#define XYRIS_SYSCALL_MAX_ARGS 4u
#define XYRIS_SYSCALL_VECTOR 0x80u

#define XYRIS_SYS_READ  0u
#define XYRIS_SYS_WRITE 1u
#define XYRIS_SYS_OPEN  2u
#define XYRIS_SYS_CLOSE 3u
#define XYRIS_SYS_EXIT  4u

#define XYRIS_SYS_MAX 5u
#define XYRIS_SYS_RESERVED_FIRST XYRIS_SYS_MAX

/* x86-64 register mapping for the v0.1 syscall entry. */
#define XYRIS_SYSCALL_REG_NUMBER "RAX"
#define XYRIS_SYSCALL_REG_ARG1   "RDI"
#define XYRIS_SYSCALL_REG_ARG2   "RSI"
#define XYRIS_SYSCALL_REG_ARG3   "RDX"
#define XYRIS_SYSCALL_REG_ARG4   "RCX"
#define XYRIS_SYSCALL_REG_RESULT "RAX"

typedef xyris_u64 xyris_syscall_number_t;
typedef xyris_u64 xyris_syscall_arg_t;
typedef xyris_status_t xyris_syscall_result_t;

/* Canonical ABI descriptions of the currently assigned syscall arguments. */
typedef struct xyris_sys_read_args {
    xyris_fd_t fd;
    xyris_user_ptr_t buffer;
    xyris_size_t size;
} xyris_sys_read_args_t;

typedef struct xyris_sys_write_args {
    xyris_fd_t fd;
    xyris_user_ptr_t buffer;
    xyris_size_t size;
} xyris_sys_write_args_t;

typedef struct xyris_sys_open_args {
    xyris_user_ptr_t path;
} xyris_sys_open_args_t;

typedef struct xyris_sys_close_args {
    xyris_fd_t fd;
} xyris_sys_close_args_t;

typedef struct xyris_sys_exit_args {
    xyris_i32 code;
} xyris_sys_exit_args_t;

/* Stable result semantics: non-negative values are operation results. */
#define XYRIS_SYSCALL_SUCCESS(result) ((result) >= 0)
#define XYRIS_SYSCALL_FAILED(result)  ((result) < 0)

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_ABI_SYSCALLS_H */
