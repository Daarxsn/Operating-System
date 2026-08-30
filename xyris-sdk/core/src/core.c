#include "../include/xyris/core.h"

#include <stdint.h>

#if defined(__x86_64__)

static xyris_syscall_result_t xyris_syscall_invoke(
    xyris_syscall_number_t number,
    xyris_syscall_arg_t arg1,
    xyris_syscall_arg_t arg2,
    xyris_syscall_arg_t arg3,
    xyris_syscall_arg_t arg4)
{
    xyris_syscall_result_t result;

    /*
     * XyrisOS v0.1 uses IDT vector 0x80 for the userspace syscall entry.
     * The register mapping is kept in one place so higher-level SDK code
     * cannot drift from the public ABI.
     */
    __asm__ volatile (
        "int $0x80"
        : "=a"(result)
        : "a"(number),
          "D"(arg1),
          "S"(arg2),
          "d"(arg3),
          "c"(arg4)
        : "memory"
    );

    return result;
}

#else

#error "Xyris SDK core v0.1 currently supports x86-64 only"

#endif

xyris_syscall_result_t xyris_syscall0(xyris_syscall_number_t number)
{
    return xyris_syscall_invoke(number, 0, 0, 0, 0);
}

xyris_syscall_result_t xyris_syscall1(
    xyris_syscall_number_t number,
    xyris_syscall_arg_t arg1)
{
    return xyris_syscall_invoke(number, arg1, 0, 0, 0);
}

xyris_syscall_result_t xyris_syscall2(
    xyris_syscall_number_t number,
    xyris_syscall_arg_t arg1,
    xyris_syscall_arg_t arg2)
{
    return xyris_syscall_invoke(number, arg1, arg2, 0, 0);
}

xyris_syscall_result_t xyris_syscall3(
    xyris_syscall_number_t number,
    xyris_syscall_arg_t arg1,
    xyris_syscall_arg_t arg2,
    xyris_syscall_arg_t arg3)
{
    return xyris_syscall_invoke(number, arg1, arg2, arg3, 0);
}

xyris_syscall_result_t xyris_syscall4(
    xyris_syscall_number_t number,
    xyris_syscall_arg_t arg1,
    xyris_syscall_arg_t arg2,
    xyris_syscall_arg_t arg3,
    xyris_syscall_arg_t arg4)
{
    return xyris_syscall_invoke(number, arg1, arg2, arg3, arg4);
}

xyris_syscall_result_t xyris_read(
    xyris_fd_t fd,
    void *buffer,
    xyris_size_t size)
{
    return xyris_syscall3(
        XYRIS_SYS_READ,
        (xyris_syscall_arg_t)(xyris_i64)fd,
        (xyris_syscall_arg_t)(xyris_user_ptr_t)(uintptr_t)buffer,
        (xyris_syscall_arg_t)size
    );
}

xyris_syscall_result_t xyris_write(
    xyris_fd_t fd,
    const void *buffer,
    xyris_size_t size)
{
    return xyris_syscall3(
        XYRIS_SYS_WRITE,
        (xyris_syscall_arg_t)(xyris_i64)fd,
        (xyris_syscall_arg_t)(xyris_user_ptr_t)(uintptr_t)buffer,
        (xyris_syscall_arg_t)size
    );
}

xyris_syscall_result_t xyris_open(const char *path)
{
    return xyris_syscall1(
        XYRIS_SYS_OPEN,
        (xyris_syscall_arg_t)(xyris_user_ptr_t)(uintptr_t)path
    );
}

xyris_syscall_result_t xyris_close(xyris_fd_t fd)
{
    return xyris_syscall1(
        XYRIS_SYS_CLOSE,
        (xyris_syscall_arg_t)(xyris_i64)fd
    );
}

xyris_syscall_result_t xyris_exit(xyris_i32 code)
{
    return xyris_syscall1(
        XYRIS_SYS_EXIT,
        (xyris_syscall_arg_t)(xyris_i64)code
    );
}
