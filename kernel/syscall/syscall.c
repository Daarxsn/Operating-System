#include "syscall.h"

#include "../fs/file.h"
#include "../process/process.h"
#include "../process/thread.h"
#include "../process/scheduler.h"
#include "../memory/vmm.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define USER_ADDRESS_LIMIT 0x0000800000000000ULL
#define SYSCALL_MAX_PATH   64

/*
 * ------------------------------------------------------------
 * System Call Table
 * ------------------------------------------------------------
 */

static syscall_handler_t syscall_table[SYS_MAX];


/*
 * ------------------------------------------------------------
 * User Pointer Validation
 * ------------------------------------------------------------
 *
 * Kernel-side tests execute while the kernel process is current.
 * Kernel callers are trusted.
 *
 * User-mode callers must have a current non-kernel process and
 * every page in the requested range must be present and marked
 * VMM_USER.
 * ------------------------------------------------------------
 */

static int syscall_validate_user_range(
    uint64_t address,
    size_t size,
    bool write_access)
{
    if (size == 0)
        return 1;

    process_t *process =
        process_current();

    /*
     * Kernel-mode syscall dispatch is trusted. Kernel pointers may
     * legitimately live in the higher-half address space, so this
     * check MUST happen before applying the user virtual-address
     * limit.
     *
     * Real user processes continue through the strict range and page
     * permission validation below.
     */
    if (process == NULL || process->kernel_process)
        return 1;

    if (address == 0 ||
        address >= USER_ADDRESS_LIMIT)
    {
        return 0;
    }

    if (size - 1 > USER_ADDRESS_LIMIT - 1 - address)
        return 0;

    uint64_t end =
        address + (uint64_t)size - 1;

    if (process->address_space == NULL)
        return 0;

    uint64_t page =
        address & ~(uint64_t)(PAGE_SIZE - 1);

    uint64_t last_page =
        end & ~(uint64_t)(PAGE_SIZE - 1);

    while (1)
    {
        uint64_t flags =
            vmm_get_page_flags(
                (address_space_t *)process->address_space,
                page
            );

        if ((flags & VMM_PRESENT) == 0 ||
            (flags & VMM_USER) == 0)
        {
            return 0;
        }

        /*
         * A read syscall writes into the caller's buffer.
         * A write syscall reads from the caller's buffer.
         *
         * The current VMM interface exposes writability as the
         * page permission we need for kernel writes.
         */
        if (write_access &&
            (flags & VMM_WRITABLE) == 0)
        {
            return 0;
        }

        if (page == last_page)
            break;

        page += PAGE_SIZE;
    }

    return 1;
}


/*
 * ------------------------------------------------------------
 * Copy User String
 * ------------------------------------------------------------
 */

static int syscall_copy_user_string(
    uint64_t user_address,
    char *destination,
    size_t destination_size)
{
    if (!destination ||
        destination_size == 0 ||
        !syscall_validate_user_range(
            user_address,
            1,
            false))
    {
        return -1;
    }

    for (size_t i = 0; i < destination_size; ++i)
    {
        uint64_t address =
            user_address + i;

        if (!syscall_validate_user_range(
                address,
                1,
                false))
        {
            return -1;
        }

        char c =
            *(const char *)(uintptr_t)address;

        destination[i] = c;

        if (c == '\0')
            return 0;
    }

    destination[destination_size - 1] = '\0';

    return -1;
}


/*
 * ------------------------------------------------------------
 * File System Syscalls
 * ------------------------------------------------------------
 */

static uint64_t sys_open(
    uint64_t path,
    uint64_t unused1,
    uint64_t unused2,
    uint64_t unused3)
{
    (void)unused1;
    (void)unused2;
    (void)unused3;

    char kernel_path[SYSCALL_MAX_PATH];

    if (syscall_copy_user_string(
            path,
            kernel_path,
            sizeof(kernel_path)) != 0)
    {
        return (uint64_t)-1;
    }

    return (uint64_t)open(kernel_path);
}


static uint64_t sys_close(
    uint64_t fd,
    uint64_t unused1,
    uint64_t unused2,
    uint64_t unused3)
{
    (void)unused1;
    (void)unused2;
    (void)unused3;

    return (uint64_t)close((int)fd);
}


static uint64_t sys_read(
    uint64_t fd,
    uint64_t buffer,
    uint64_t size,
    uint64_t unused)
{
    (void)unused;

    if (size > (uint64_t)(size_t)-1)
        return (uint64_t)-1;

    if (!syscall_validate_user_range(
            buffer,
            (size_t)size,
            true))
    {
        return (uint64_t)-1;
    }

    return (uint64_t)read(
        (int)fd,
        (void *)(uintptr_t)buffer,
        (size_t)size
    );
}


static uint64_t sys_write(
    uint64_t fd,
    uint64_t buffer,
    uint64_t size,
    uint64_t unused)
{
    (void)unused;

    if (!syscall_validate_user_range(
            buffer,
            (size_t)size,
            false))
    {
        return (uint64_t)-1;
    }

    return (uint64_t)write(
        (int)fd,
        (const void *)(uintptr_t)buffer,
        (size_t)size
    );
}


/*
 * ------------------------------------------------------------
 * Process Syscalls
 * ------------------------------------------------------------
 */

static uint64_t sys_exit(
    uint64_t code,
    uint64_t unused1,
    uint64_t unused2,
    uint64_t unused3)
{
    (void)code;
    (void)unused1;
    (void)unused2;
    (void)unused3;

    process_t *process =
        process_current();

    /*
     * Kernel code must not accidentally terminate the kernel
     * scheduler through a syscall test.
     */
    if (process == NULL ||
        process->kernel_process)
    {
        return (uint64_t)-1;
    }

    process->state =
        PROCESS_TERMINATED;

    scheduler_exit_current();

    /*
     * scheduler_exit_current() switches away from the current
     * thread. This is only a defensive fallback.
     */
    return (uint64_t)-1;
}


/*
 * ------------------------------------------------------------
 * Initialization
 * ------------------------------------------------------------
 */

void syscall_init(void)
{
    for (size_t i = 0; i < SYS_MAX; ++i)
        syscall_table[i] = NULL;

    syscall_table[SYS_READ]  = sys_read;
    syscall_table[SYS_WRITE] = sys_write;
    syscall_table[SYS_OPEN]  = sys_open;
    syscall_table[SYS_CLOSE] = sys_close;
    syscall_table[SYS_EXIT]  = sys_exit;
}


/*
 * ------------------------------------------------------------
 * Dispatcher
 * ------------------------------------------------------------
 */

uint64_t syscall_dispatch(
    uint64_t number,
    uint64_t arg1,
    uint64_t arg2,
    uint64_t arg3,
    uint64_t arg4)
{
    if (number >= SYS_MAX)
        return (uint64_t)-1;

    syscall_handler_t handler =
        syscall_table[number];

    if (handler == NULL)
        return (uint64_t)-1;

    return handler(
        arg1,
        arg2,
        arg3,
        arg4
    );
}
