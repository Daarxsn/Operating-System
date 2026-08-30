#ifndef XYRIS_CORE_H
#define XYRIS_CORE_H

/*
 * Xyris SDK Core v0.1
 *
 * The core layer is the only SDK component that directly invokes the
 * Xyris System ABI. Higher-level SDK modules should build on this layer
 * instead of embedding syscall numbers or private kernel interfaces.
 */

#include <xyris/abi/xyris_abi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_SDK_VERSION_MAJOR 0u
#define XYRIS_SDK_VERSION_MINOR 1u
#define XYRIS_SDK_VERSION ((XYRIS_SDK_VERSION_MAJOR << 16) | XYRIS_SDK_VERSION_MINOR)

xyris_syscall_result_t xyris_syscall0(xyris_syscall_number_t number);

xyris_syscall_result_t xyris_syscall1(
    xyris_syscall_number_t number,
    xyris_syscall_arg_t arg1
);

xyris_syscall_result_t xyris_syscall2(
    xyris_syscall_number_t number,
    xyris_syscall_arg_t arg1,
    xyris_syscall_arg_t arg2
);

xyris_syscall_result_t xyris_syscall3(
    xyris_syscall_number_t number,
    xyris_syscall_arg_t arg1,
    xyris_syscall_arg_t arg2,
    xyris_syscall_arg_t arg3
);

xyris_syscall_result_t xyris_syscall4(
    xyris_syscall_number_t number,
    xyris_syscall_arg_t arg1,
    xyris_syscall_arg_t arg2,
    xyris_syscall_arg_t arg3,
    xyris_syscall_arg_t arg4
);

static inline xyris_bool_t xyris_failed(xyris_syscall_result_t result)
{
    return result < 0 ? XYRIS_TRUE : XYRIS_FALSE;
}

static inline xyris_bool_t xyris_succeeded(xyris_syscall_result_t result)
{
    return result >= 0 ? XYRIS_TRUE : XYRIS_FALSE;
}

static inline xyris_error_t xyris_error(xyris_syscall_result_t result)
{
    return result < 0 ? (xyris_error_t)result : XYRIS_OK;
}

xyris_syscall_result_t xyris_read(
    xyris_fd_t fd,
    void *buffer,
    xyris_size_t size
);

xyris_syscall_result_t xyris_write(
    xyris_fd_t fd,
    const void *buffer,
    xyris_size_t size
);

xyris_syscall_result_t xyris_open(const char *path);

xyris_syscall_result_t xyris_close(xyris_fd_t fd);

xyris_syscall_result_t xyris_exit(xyris_i32 code);

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_CORE_H */
