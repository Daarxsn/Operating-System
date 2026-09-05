#include "syscall.h"
#include "sdk_services.h"

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

xyris_status_t syscall_validate_user_range(
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

    if (process == NULL || process->kernel_process)
        return XYRIS_EPERM;

    process->exit_code = (int32_t)code;
    process->exit_requested = true;
    scheduler_exit_current();

    /* Defensive fallback if the scheduler unexpectedly returns. */
    return XYRIS_EBADSTATE;
}

static xyris_syscall_result_t sys_getpid(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a1; (void)a2; (void)a3; (void)a4; return xyris_sdk_service_getpid(); }
static xyris_syscall_result_t sys_thread_self(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a1; (void)a2; (void)a3; (void)a4; return xyris_sdk_service_thread_self(); }
static xyris_syscall_result_t sys_thread_yield(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a1; (void)a2; (void)a3; (void)a4; return xyris_sdk_service_thread_yield(); }
static xyris_syscall_result_t sys_thread_sleep(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a2; (void)a3; (void)a4; return xyris_sdk_service_thread_sleep(a1); }
static xyris_syscall_result_t sys_thread_info(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a3; (void)a4; return xyris_sdk_service_thread_info((xyris_tid_t)a1, a2); }
static xyris_syscall_result_t sys_memory_map(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a4; return xyris_sdk_service_memory_map(a1, (xyris_u32)a2, (xyris_u32)a3); }
static xyris_syscall_result_t sys_memory_unmap(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a3; (void)a4; return xyris_sdk_service_memory_unmap(a1, a2); }
static xyris_syscall_result_t sys_memory_protect(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a4; return xyris_sdk_service_memory_protect(a1, a2, (xyris_u32)a3); }
static xyris_syscall_result_t sys_ipc_create(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a2; (void)a3; (void)a4; return xyris_sdk_service_ipc_create((xyris_u32)a1); }
static xyris_syscall_result_t sys_ipc_send(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a4; return xyris_sdk_service_ipc_send(a1, a2, a3); }
static xyris_syscall_result_t sys_ipc_recv(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { return xyris_sdk_service_ipc_recv(a1, a2, (xyris_u32)a3, a4); }
static xyris_syscall_result_t sys_ipc_close(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a2; (void)a3; (void)a4; return xyris_sdk_service_ipc_close(a1); }
static xyris_syscall_result_t sys_event_create(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a3; (void)a4; return xyris_sdk_service_event_create((xyris_u32)a1, (xyris_u32)a2); }
static xyris_syscall_result_t sys_event_signal(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a3; (void)a4; return xyris_sdk_service_event_signal(a1, a2); }
static xyris_syscall_result_t sys_event_wait(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a3; (void)a4; return xyris_sdk_service_event_wait(a1, a2); }
static xyris_syscall_result_t sys_event_close(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a2; (void)a3; (void)a4; return xyris_sdk_service_event_close(a1); }
static xyris_syscall_result_t sys_timer_create(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a2; (void)a3; (void)a4; return xyris_sdk_service_timer_create(a1); }
static xyris_syscall_result_t sys_timer_cancel(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a2; (void)a3; (void)a4; return xyris_sdk_service_timer_cancel(a1); }
static xyris_syscall_result_t sys_timer_wait(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a2; (void)a3; (void)a4; return xyris_sdk_service_timer_wait(a1); }
static xyris_syscall_result_t sys_timer_close(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a2; (void)a3; (void)a4; return xyris_sdk_service_timer_close(a1); }
static xyris_syscall_result_t sys_device_count(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a1; (void)a2; (void)a3; (void)a4; return xyris_sdk_service_device_count(); }
static xyris_syscall_result_t sys_device_info(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a3; (void)a4; return xyris_sdk_service_device_info((xyris_u32)a1, a2); }
static xyris_syscall_result_t sys_net_socket(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a3; (void)a4; return xyris_sdk_service_net_socket((xyris_u32)a1, (xyris_u32)a2); }
static xyris_syscall_result_t sys_net_bind(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a3; (void)a4; return xyris_sdk_service_net_bind((xyris_fd_t)a1, a2); }
static xyris_syscall_result_t sys_net_connect(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a3; (void)a4; return xyris_sdk_service_net_connect((xyris_fd_t)a1, a2); }
static xyris_syscall_result_t sys_net_send(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a4; return xyris_sdk_service_net_send((xyris_fd_t)a1, a2, (xyris_u32)a3); }
static xyris_syscall_result_t sys_net_recv(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a4; return xyris_sdk_service_net_recv((xyris_fd_t)a1, a2, (xyris_u32)a3); }
static xyris_syscall_result_t sys_net_close(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a2; (void)a3; (void)a4; return xyris_sdk_service_net_close((xyris_fd_t)a1); }
static xyris_syscall_result_t sys_security_identity(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a2; (void)a3; (void)a4; return xyris_sdk_service_security_identity(a1); }
static xyris_syscall_result_t sys_security_check(xyris_syscall_arg_t a1, xyris_syscall_arg_t a2, xyris_syscall_arg_t a3, xyris_syscall_arg_t a4) { (void)a4; return xyris_sdk_service_security_check(a1, a2, a3); }

void syscall_init(void)
{
    for (xyris_size_t i = 0; i < XYRIS_SYS_MAX; ++i)
        syscall_table[i] = NULL;

    syscall_table[XYRIS_SYS_READ] = sys_read;
    syscall_table[XYRIS_SYS_WRITE] = sys_write;
    syscall_table[XYRIS_SYS_OPEN] = sys_open;
    syscall_table[XYRIS_SYS_CLOSE] = sys_close;
    syscall_table[XYRIS_SYS_EXIT] = sys_exit;
    syscall_table[XYRIS_SYS_GETPID] = sys_getpid;
    syscall_table[XYRIS_SYS_THREAD_SELF] = sys_thread_self;
    syscall_table[XYRIS_SYS_THREAD_YIELD] = sys_thread_yield;
    syscall_table[XYRIS_SYS_THREAD_SLEEP] = sys_thread_sleep;
    syscall_table[XYRIS_SYS_THREAD_INFO] = sys_thread_info;
    syscall_table[XYRIS_SYS_MEMORY_MAP] = sys_memory_map;
    syscall_table[XYRIS_SYS_MEMORY_UNMAP] = sys_memory_unmap;
    syscall_table[XYRIS_SYS_MEMORY_PROTECT] = sys_memory_protect;
    syscall_table[XYRIS_SYS_IPC_CREATE] = sys_ipc_create;
    syscall_table[XYRIS_SYS_IPC_SEND] = sys_ipc_send;
    syscall_table[XYRIS_SYS_IPC_RECV] = sys_ipc_recv;
    syscall_table[XYRIS_SYS_IPC_CLOSE] = sys_ipc_close;
    syscall_table[XYRIS_SYS_EVENT_CREATE] = sys_event_create;
    syscall_table[XYRIS_SYS_EVENT_SIGNAL] = sys_event_signal;
    syscall_table[XYRIS_SYS_EVENT_WAIT] = sys_event_wait;
    syscall_table[XYRIS_SYS_EVENT_CLOSE] = sys_event_close;
    syscall_table[XYRIS_SYS_TIMER_CREATE] = sys_timer_create;
    syscall_table[XYRIS_SYS_TIMER_CANCEL] = sys_timer_cancel;
    syscall_table[XYRIS_SYS_TIMER_WAIT] = sys_timer_wait;
    syscall_table[XYRIS_SYS_TIMER_CLOSE] = sys_timer_close;
    syscall_table[XYRIS_SYS_DEVICE_COUNT] = sys_device_count;
    syscall_table[XYRIS_SYS_DEVICE_INFO] = sys_device_info;
    syscall_table[XYRIS_SYS_NET_SOCKET] = sys_net_socket;
    syscall_table[XYRIS_SYS_NET_BIND] = sys_net_bind;
    syscall_table[XYRIS_SYS_NET_CONNECT] = sys_net_connect;
    syscall_table[XYRIS_SYS_NET_SEND] = sys_net_send;
    syscall_table[XYRIS_SYS_NET_RECV] = sys_net_recv;
    syscall_table[XYRIS_SYS_NET_CLOSE] = sys_net_close;
    syscall_table[XYRIS_SYS_SECURITY_IDENTITY] = sys_security_identity;
    syscall_table[XYRIS_SYS_SECURITY_CHECK] = sys_security_check;

    xyris_sdk_services_init();
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

void syscall_process_cleanup(struct process *process)
{
    xyris_sdk_services_process_cleanup((process_t *)process);
}
