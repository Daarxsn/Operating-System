#ifndef XYRIS_ABI_SYSCALLS_H
#define XYRIS_ABI_SYSCALLS_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Xyris System ABI v0.1 syscall contract.
 *
 * Syscall numbers are append-only within this ABI revision. Existing numbers
 * are never reused. The service set below is the complete v0.1 SDK boundary.
 */
#define XYRIS_SYSCALL_ABI_VERSION 1u
#define XYRIS_SYSCALL_MAX_ARGS 4u
#define XYRIS_SYSCALL_VECTOR 0x80u

#define XYRIS_SYS_READ            0u
#define XYRIS_SYS_WRITE           1u
#define XYRIS_SYS_OPEN            2u
#define XYRIS_SYS_CLOSE           3u
#define XYRIS_SYS_EXIT            4u
#define XYRIS_SYS_GETPID          5u
#define XYRIS_SYS_THREAD_SELF     6u
#define XYRIS_SYS_THREAD_YIELD    7u
#define XYRIS_SYS_THREAD_SLEEP    8u
#define XYRIS_SYS_THREAD_INFO     9u
#define XYRIS_SYS_MEMORY_MAP     10u
#define XYRIS_SYS_MEMORY_UNMAP   11u
#define XYRIS_SYS_MEMORY_PROTECT 12u
#define XYRIS_SYS_IPC_CREATE     13u
#define XYRIS_SYS_IPC_SEND       14u
#define XYRIS_SYS_IPC_RECV       15u
#define XYRIS_SYS_IPC_CLOSE      16u
#define XYRIS_SYS_EVENT_CREATE   17u
#define XYRIS_SYS_EVENT_SIGNAL   18u
#define XYRIS_SYS_EVENT_WAIT     19u
#define XYRIS_SYS_EVENT_CLOSE    20u
#define XYRIS_SYS_TIMER_CREATE   21u
#define XYRIS_SYS_TIMER_CANCEL   22u
#define XYRIS_SYS_TIMER_WAIT     23u
#define XYRIS_SYS_TIMER_CLOSE    24u
#define XYRIS_SYS_DEVICE_COUNT   25u
#define XYRIS_SYS_DEVICE_INFO    26u
#define XYRIS_SYS_NET_SOCKET     27u
#define XYRIS_SYS_NET_BIND       28u
#define XYRIS_SYS_NET_CONNECT    29u
#define XYRIS_SYS_NET_SEND       30u
#define XYRIS_SYS_NET_RECV       31u
#define XYRIS_SYS_NET_CLOSE      32u
#define XYRIS_SYS_SECURITY_IDENTITY 33u
#define XYRIS_SYS_SECURITY_CHECK    34u

#define XYRIS_SYS_MAX 35u
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

/* Canonical ABI descriptions of syscall arguments. */
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

typedef struct xyris_sys_memory_map_args {
    xyris_size_t size;
    xyris_u32 protection;
    xyris_u32 flags;
} xyris_sys_memory_map_args_t;

typedef struct xyris_sys_memory_unmap_args {
    xyris_addr_t base;
    xyris_size_t size;
} xyris_sys_memory_unmap_args_t;

typedef struct xyris_sys_memory_protect_args {
    xyris_addr_t base;
    xyris_size_t size;
    xyris_u32 protection;
} xyris_sys_memory_protect_args_t;

/* Stable result semantics: non-negative values are operation results. */
#define XYRIS_SYSCALL_SUCCESS(result) ((result) >= 0)
#define XYRIS_SYSCALL_FAILED(result)  ((result) < 0)

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_ABI_SYSCALLS_H */
