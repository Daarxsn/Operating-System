#include "syscall.h"

#include "../fs/file.h"
#include "../process/process.h"
#include "../process/scheduler.h"
#include "../memory/vmm.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define USER_ADDRESS_LIMIT 0x0000800000000000ULL
#define SYSCALL_MAX_PATH   64

static syscall_handler_t syscall_table[XYRIS_SYS_MAX];

static xyris_status_t syscall_validate_user_range(
    xyris_user_ptr_t address,
    xyris_size_t size,
    bool write_access)
{
    if (size == 0)
        return XYRIS_OK;

    process_t *process = process_current();

    /* Trusted kernel callers are permitted to use kernel addresses. */
    if (process == NULL || process->kernel_process)
        return XYRIS_OK;

    if (address == 0 || address >= USER_ADDRESS_LIMIT)
        return XYRIS_EFAULT;

    if (size - 1 > USER_ADDRESS_LIMIT - 1 - address)
        return XYRIS_EFAULT;

    uint64_t end = address + size - 1;
    if (process->address_space == NULL)
        return XYRIS_EFAULT;

    uint64_t page = address & ~(uint64_t)(PAGE_SIZE - 1);
    uint64_t last_page = end & ~(uint64_t)(PAGE_SIZE - 1);

    while (1)
    {
        uint64_t flags = vmm_get_page_flags(
            (address_space_t *)process->address_space,
            page
        );

        if ((flags & VMM_PRESENT) == 0 ||
            (flags & VMM_USER) == 0)
            return XYRIS_EFAULT;

        if (write_access && (flags & VMM_WRITABLE) == 0)
            return XYRIS_EFAULT;

        if (page == last_page)
            break;

        page += PAGE_SIZE;
    }

    return XYRIS_OK;
}

static xyris_status_t syscall_copy_user_string(
    xyris_user_ptr_t user_address,
    char *destination,
    xyris_size_t destination_size)
{
    if (!destination || destination_size == 0)
        return XYRIS_EINVAL;

    for (xyris_size_t i = 0; i < destination_size; ++i)
    {
        xyris_user_ptr_t address = user_address + i;

        xyris_status_t status =
            syscall_validate_user_range(address, 1, false);

        if (status != XYRIS_OK)
            return status;

        char c = *(const char *)(uintptr_t)address;
        destination[i] = c;

        if (c == '\0')
            return XYRIS_OK;
    }

    destination[destination_size - 1] = '\0';
    return XYRIS_EOVERFLOW;
}

static xyris_syscall_result_t sys_open(
    xyris_syscall_arg_t path,
    xyris_syscall_arg_t unused1,
    xyris_syscall_arg_t unused2,
    xyris_syscall_arg_t unused3)
{
    (void)unused1;
    (void)unused2;
    (void)unused3;

    char kernel_path[SYSCALL_MAX_PATH];

    xyris_status_t status =
        syscall_copy_user_string(path, kernel_path, sizeof(kernel_path));

    if (status != XYRIS_OK)
        return status;

    int fd = open(kernel_path);
    if (fd < 0)
        return XYRIS_ENOTFOUND;

    return (xyris_syscall_result_t)fd;
}

static xyris_syscall_result_t sys_close(
    xyris_syscall_arg_t fd,
    xyris_syscall_arg_t unused1,
    xyris_syscall_arg_t unused2,
    xyris_syscall_arg_t unused3)
{
    (void)unused1;
    (void)unused2;
    (void)unused3;

    if (fd > INT32_MAX)
        return XYRIS_EBADHANDLE;

    return close((int)fd) == 0
        ? XYRIS_OK
        : XYRIS_EBADHANDLE;
}

static xyris_syscall_result_t sys_read(
    xyris_syscall_arg_t fd,
    xyris_syscall_arg_t buffer,
    xyris_syscall_arg_t size,
    xyris_syscall_arg_t unused)
{
    (void)unused;

    xyris_status_t status =
        syscall_validate_user_range(buffer, size, true);

    if (status != XYRIS_OK)
        return status;

    if (fd > INT32_MAX || size > SIZE_MAX)
        return XYRIS_EINVAL;

    int result = read(
        (int)fd,
        (void *)(uintptr_t)buffer,
        (size_t)size
    );

    return result < 0
        ? XYRIS_EBADHANDLE
        : (xyris_syscall_result_t)result;
}

static xyris_syscall_result_t sys_write(
    xyris_syscall_arg_t fd,
    xyris_syscall_arg_t buffer,
    xyris_syscall_arg_t size,
    xyris_syscall_arg_t unused)
{
    (void)unused;

    xyris_status_t status =
        syscall_validate_user_range(buffer, size, false);

    if (status != XYRIS_OK)
        return status;

    if (fd > INT32_MAX || size > SIZE_MAX)
        return XYRIS_EINVAL;

    int result = write(
        (int)fd,
        (const void *)(uintptr_t)buffer,
        (size_t)size
    );

    return result < 0
        ? XYRIS_EBADHANDLE
        : (xyris_syscall_result_t)result;
}

static xyris_syscall_result_t sys_exit(
    xyris_syscall_arg_t code,
    xyris_syscall_arg_t unused1,
    xyris_syscall_arg_t unused2,
    xyris_syscall_arg_t unused3)
{
    (void)unused1;
    (void)unused2;
    (void)unused3;

    process_t *process = process_current();

<<<<<<< HEAD
    /*
     * Kernel code must not accidentally terminate the kernel
     * scheduler through a syscall test.
     */
    if (process != NULL)
        process->exit_code = (int)code;
=======
    if (process == NULL || process->kernel_process)
        return XYRIS_EPERM;
>>>>>>> 9601b4775b8e7e0e97a2cb6d4ae7396e41de1c0c

    process->exit_code = (int32_t)code;
    process->exit_requested = true;
    scheduler_exit_current();

<<<<<<< HEAD
    /*
    * scheduler_exit_current() switches away from the current
    * thread.
    * This is only a defensive fallback.
    */
    return (uint64_t)-1;
=======
    /* Defensive fallback if the scheduler unexpectedly returns. */
    return XYRIS_EBADSTATE;
>>>>>>> 9601b4775b8e7e0e97a2cb6d4ae7396e41de1c0c
}

void syscall_init(void)
{
    for (xyris_size_t i = 0; i < XYRIS_SYS_MAX; ++i)
        syscall_table[i] = NULL;

    syscall_table[XYRIS_SYS_READ] = sys_read;
    syscall_table[XYRIS_SYS_WRITE] = sys_write;
    syscall_table[XYRIS_SYS_OPEN] = sys_open;
    syscall_table[XYRIS_SYS_CLOSE] = sys_close;
    syscall_table[XYRIS_SYS_EXIT] = sys_exit;
}

xyris_syscall_result_t syscall_dispatch(
    xyris_syscall_number_t number,
    xyris_syscall_arg_t arg1,
    xyris_syscall_arg_t arg2,
    xyris_syscall_arg_t arg3,
    xyris_syscall_arg_t arg4)
{
    if (number >= XYRIS_SYS_MAX)
        return XYRIS_ENOSYS;

    syscall_handler_t handler = syscall_table[number];

    if (handler == NULL)
        return XYRIS_ENOSYS;

    return handler(arg1, arg2, arg3, arg4);
}
