#include "sdk_services.h"

#include "../memory/pmm.h"
#include "../memory/hhdm.h"
#include "../memory/vmm.h"
#include "../process/scheduler.h"
#include "../process/thread.h"
#include "../include/foundation/time.h"
#include "../include/drivers/driver.h"
#include "../include/drivers/pci.h"
#include "../include/foundation/capability.h"
#include "../lib/string.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>

#define SDK_MAX_IPC_ENDPOINTS 64
#define SDK_IPC_QUEUE_DEPTH 16
#define SDK_MAX_EVENTS 64
#define SDK_EVENT_QUEUE_DEPTH 16
#define SDK_MAX_TIMERS 64
#define SDK_MAX_MEMORY_REGIONS 32
#define SDK_MAX_NET_SOCKETS 32
#define SDK_NET_QUEUE_DEPTH 16
#define SDK_NET_MESSAGE_MAX 4096

#define SDK_HANDLE_IPC   0x1000000000000000ULL
#define SDK_HANDLE_EVENT 0x2000000000000000ULL
#define SDK_HANDLE_TIMER 0x3000000000000000ULL
#define SDK_HANDLE_MASK  0xF000000000000000ULL

#define SDK_MEMORY_BASE 0x0000000010000000ULL
#define SDK_MEMORY_STRIDE 0x0000000002000000ULL
#define SDK_MEMORY_MAX 0x0000000001000000ULL
#define SDK_USER_LIMIT 0x0000800000000000ULL

#define SDK_NET_FD_BASE 1000

static xyris_handle_t next_handle = 1;

static xyris_handle_t new_handle(xyris_u64 kind)
{
    xyris_handle_t value = kind | (next_handle++ & 0x0FFFFFFFFFFFFFFFULL);
    if ((value & ~SDK_HANDLE_MASK) == 0)
        value |= 1;
    return value;
}

static process_t *current_process(void)
{
    return process_current();
}


static bool owner_matches(uint32_t owner_pid, process_t *process)
{
    return process != NULL && owner_pid == process->pid;
}

static xyris_status_t require_user_read(xyris_user_ptr_t address, xyris_size_t size)
{
    if (size == 0)
        return XYRIS_OK;
    if (address == 0 || address >= SDK_USER_LIMIT)
        return XYRIS_EFAULT;

    process_t *process = current_process();
    if (process == NULL || process->kernel_process)
        return XYRIS_OK;
    if (process->address_space == NULL)
        return XYRIS_EFAULT;

    if (size - 1 > SDK_USER_LIMIT - 1 - address)
        return XYRIS_EFAULT;

    uintptr_t page = (uintptr_t)address & ~(uintptr_t)(PAGE_SIZE - 1);
    uintptr_t last = (uintptr_t)(address + size - 1) & ~(uintptr_t)(PAGE_SIZE - 1);
    for (;;)
    {
        uint64_t flags = vmm_get_page_flags((address_space_t *)process->address_space, page);
        if ((flags & VMM_PRESENT) == 0 || (flags & VMM_USER) == 0)
            return XYRIS_EFAULT;
        if (page == last)
            break;
        page += PAGE_SIZE;
    }
    return XYRIS_OK;
}

static xyris_status_t require_user_write(xyris_user_ptr_t address, xyris_size_t size)
{
    xyris_status_t status = require_user_read(address, size);
    if (status != XYRIS_OK || size == 0)
        return status;

    process_t *process = current_process();
    if (process == NULL || process->kernel_process)
        return XYRIS_OK;

    uintptr_t page = (uintptr_t)address & ~(uintptr_t)(PAGE_SIZE - 1);
    uintptr_t last = (uintptr_t)(address + size - 1) & ~(uintptr_t)(PAGE_SIZE - 1);
    for (;;)
    {
        uint64_t flags = vmm_get_page_flags((address_space_t *)process->address_space, page);
        if ((flags & VMM_WRITABLE) == 0)
            return XYRIS_EFAULT;
        if (page == last)
            break;
        page += PAGE_SIZE;
    }
    return XYRIS_OK;
}

static xyris_status_t copy_from_user(void *dst, xyris_user_ptr_t src, xyris_size_t size)
{
    xyris_status_t status = require_user_read(src, size);
    if (status != XYRIS_OK)
        return status;
    if (size != 0)
        memcpy(dst, (const void *)(uintptr_t)src, (size_t)size);
    return XYRIS_OK;
}

static xyris_status_t copy_to_user(xyris_user_ptr_t dst, const void *src, xyris_size_t size)
{
    xyris_status_t status = require_user_write(dst, size);
    if (status != XYRIS_OK)
        return status;
    if (size != 0)
        memcpy((void *)(uintptr_t)dst, src, (size_t)size);
    return XYRIS_OK;
}

/* ------------------------------------------------------------
 * IPC
 * ------------------------------------------------------------ */
typedef struct {
    xyris_u32 length;
    xyris_u32 flags;
    xyris_u64 tag;
    uint32_t sender_pid;
    uint8_t data[XYRIS_IPC_MAX_MESSAGE];
} sdk_ipc_message_slot_t;

typedef struct {
    bool used;
    xyris_handle_t handle;
    uint32_t owner_pid;
    xyris_u32 flags;
    sdk_ipc_message_slot_t queue[SDK_IPC_QUEUE_DEPTH];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} sdk_ipc_endpoint_t;
static sdk_ipc_endpoint_t ipc_table[SDK_MAX_IPC_ENDPOINTS];

static sdk_ipc_endpoint_t *find_ipc(xyris_handle_t handle)
{
    for (size_t i = 0; i < SDK_MAX_IPC_ENDPOINTS; ++i)
        if (ipc_table[i].used && ipc_table[i].handle == handle)
            return &ipc_table[i];
    return NULL;
}

/* ------------------------------------------------------------
 * Events
 * ------------------------------------------------------------ */
typedef struct {
    bool used;
    xyris_handle_t handle;
    uint32_t owner_pid;
    xyris_u32 type;
    xyris_u32 flags;
    xyris_event_t queue[SDK_EVENT_QUEUE_DEPTH];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    uint64_t sequence;
} sdk_event_t;
static sdk_event_t event_table[SDK_MAX_EVENTS];

static sdk_event_t *find_event(xyris_handle_t handle)
{
    for (size_t i = 0; i < SDK_MAX_EVENTS; ++i)
        if (event_table[i].used && event_table[i].handle == handle)
            return &event_table[i];
    return NULL;
}

/* ------------------------------------------------------------
 * Timers
 * ------------------------------------------------------------ */
typedef struct {
    bool used;
    xyris_handle_t handle;
    uint32_t owner_pid;
    xyris_timer_spec_t spec;
    uint64_t deadline_ms;
} sdk_timer_t;
static sdk_timer_t timer_table[SDK_MAX_TIMERS];

static sdk_timer_t *find_timer(xyris_handle_t handle)
{
    for (size_t i = 0; i < SDK_MAX_TIMERS; ++i)
        if (timer_table[i].used && timer_table[i].handle == handle)
            return &timer_table[i];
    return NULL;
}

/* ------------------------------------------------------------
 * Memory mappings
 * ------------------------------------------------------------ */
typedef struct {
    bool used;
    uint32_t owner_pid;
    address_space_t *space;
    xyris_addr_t base;
    xyris_size_t size;
    xyris_u32 protection;
    xyris_u32 flags;
} sdk_memory_region_t;
static sdk_memory_region_t memory_table[SDK_MAX_MEMORY_REGIONS];

static sdk_memory_region_t *find_memory(process_t *process, xyris_addr_t base, xyris_size_t size)
{
    if (process == NULL)
        return NULL;
    for (size_t i = 0; i < SDK_MAX_MEMORY_REGIONS; ++i)
    {
        if (!memory_table[i].used || memory_table[i].owner_pid != process->pid || memory_table[i].space != (address_space_t *)process->address_space)
            continue;
        if (memory_table[i].base == base && memory_table[i].size == size)
            return &memory_table[i];
    }
    return NULL;
}

/* ------------------------------------------------------------
 * Loopback networking
 * ------------------------------------------------------------ */
typedef struct {
    xyris_u32 length;
    uint8_t data[SDK_NET_MESSAGE_MAX];
} sdk_net_packet_t;

typedef struct {
    bool used;
    xyris_fd_t fd;
    uint32_t owner_pid;
    xyris_u32 family;
    xyris_u32 protocol;
    bool bound;
    bool connected;
    uint16_t local_port;
    uint16_t remote_port;
    xyris_fd_t peer_fd;
    sdk_net_packet_t queue[SDK_NET_QUEUE_DEPTH];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} sdk_net_socket_t;
static sdk_net_socket_t net_table[SDK_MAX_NET_SOCKETS];
static uint16_t next_net_port = 40000;

static sdk_net_socket_t *find_net(xyris_fd_t fd)
{
    for (size_t i = 0; i < SDK_MAX_NET_SOCKETS; ++i)
        if (net_table[i].used && net_table[i].fd == fd)
            return &net_table[i];
    return NULL;
}

static bool net_endpoint_is_loopback(const xyris_net_endpoint_t *endpoint)
{
    if (endpoint == NULL)
        return false;
    if (endpoint->family == XYRIS_NET_FAMILY_UNSPEC)
        return true;
    if (endpoint->family != XYRIS_NET_FAMILY_IPV4)
        return false;
    if (endpoint->address_length == 0)
        return true;
    if (endpoint->address_length != 4)
        return false;
    return endpoint->address[0] == 127 && endpoint->address[1] == 0 &&
           endpoint->address[2] == 0 && endpoint->address[3] == 1;
}

static sdk_net_socket_t *find_bound_socket(uint16_t port)
{
    for (size_t i = 0; i < SDK_MAX_NET_SOCKETS; ++i)
        if (net_table[i].used && net_table[i].bound && net_table[i].local_port == port)
            return &net_table[i];
    return NULL;
}

/* ------------------------------------------------------------
 * Initialization / cleanup
 * ------------------------------------------------------------ */
void xyris_sdk_services_init(void)
{
    memset(ipc_table, 0, sizeof(ipc_table));
    memset(event_table, 0, sizeof(event_table));
    memset(timer_table, 0, sizeof(timer_table));
    memset(memory_table, 0, sizeof(memory_table));
    memset(net_table, 0, sizeof(net_table));
    next_handle = 1;
    next_net_port = 40000;
}

static void cleanup_memory(process_t *process)
{
    for (size_t i = 0; i < SDK_MAX_MEMORY_REGIONS; ++i)
    {
        sdk_memory_region_t *region = &memory_table[i];
        if (!region->used || region->owner_pid != process->pid || region->space != (address_space_t *)process->address_space)
            continue;

        for (xyris_size_t offset = 0; offset < region->size; offset += PAGE_SIZE)
        {
            uintptr_t va = (uintptr_t)(region->base + offset);
            phys_addr_t physical = vmm_translate(region->space, va);
            if (physical != 0)
            {
                vmm_unmap_page(region->space, va);
                pmm_free_page(physical & ~(phys_addr_t)(PAGE_SIZE - 1));
            }
        }
        memset(region, 0, sizeof(*region));
    }
}

void xyris_sdk_services_process_cleanup(process_t *process)
{
    if (process == NULL)
        return;

    cleanup_memory(process);

    for (size_t i = 0; i < SDK_MAX_IPC_ENDPOINTS; ++i)
        if (ipc_table[i].used && ipc_table[i].owner_pid == process->pid)
            memset(&ipc_table[i], 0, sizeof(ipc_table[i]));

    for (size_t i = 0; i < SDK_MAX_EVENTS; ++i)
        if (event_table[i].used && event_table[i].owner_pid == process->pid)
            memset(&event_table[i], 0, sizeof(event_table[i]));

    for (size_t i = 0; i < SDK_MAX_TIMERS; ++i)
        if (timer_table[i].used && timer_table[i].owner_pid == process->pid)
            memset(&timer_table[i], 0, sizeof(timer_table[i]));

    for (size_t i = 0; i < SDK_MAX_NET_SOCKETS; ++i)
    {
        if (net_table[i].used && net_table[i].owner_pid == process->pid)
        {
            xyris_fd_t peer = net_table[i].peer_fd;
            if (peer >= 0)
            {
                sdk_net_socket_t *other = find_net(peer);
                if (other != NULL)
                {
                    other->peer_fd = -1;
                    other->connected = false;
                }
            }
            memset(&net_table[i], 0, sizeof(net_table[i]));
        }
    }
}

/* ------------------------------------------------------------
 * Process / thread
 * ------------------------------------------------------------ */
xyris_syscall_result_t xyris_sdk_service_getpid(void)
{
    process_t *process = current_process();
    return process != NULL ? (xyris_syscall_result_t)process->pid : XYRIS_EBADSTATE;
}

xyris_syscall_result_t xyris_sdk_service_thread_self(void)
{
    thread_t *thread = thread_current();
    return thread != NULL ? (xyris_syscall_result_t)thread->tid : XYRIS_EBADSTATE;
}

xyris_syscall_result_t xyris_sdk_service_thread_yield(void)
{
    if (thread_current() == NULL)
        return XYRIS_EBADSTATE;
    scheduler_yield();
    return XYRIS_OK;
}

xyris_syscall_result_t xyris_sdk_service_thread_sleep(xyris_u64 milliseconds)
{
    if (thread_current() == NULL)
        return XYRIS_EBADSTATE;
    if (milliseconds == 0)
    {
        scheduler_yield();
        return XYRIS_OK;
    }

    uint64_t frequency = xk_time_frequency();
    uint64_t ticks = (milliseconds * frequency + 999ULL) / 1000ULL;
    if (ticks == 0) ticks = 1;
    scheduler_sleep_current(ticks);
    return XYRIS_OK;
}

xyris_syscall_result_t xyris_sdk_service_thread_info(xyris_tid_t tid, xyris_user_ptr_t out)
{
    thread_t *thread = thread_current();
    if (thread == NULL || tid != thread->tid)
        return XYRIS_ENOTFOUND;
    if (require_user_write(out, sizeof(xyris_thread_info_t)) != XYRIS_OK)
        return XYRIS_EFAULT;

    xyris_thread_info_t info;
    memset(&info, 0, sizeof(info));
    info.header.size = sizeof(info);
    info.header.version = 1;
    info.tid = thread->tid;
    info.pid = thread->owner != NULL ? thread->owner->pid : 0;
    switch (thread->state)
    {
        case THREAD_READY: info.state = XYRIS_THREAD_STATE_READY; break;
        case THREAD_RUNNING: info.state = XYRIS_THREAD_STATE_RUNNING; break;
        case THREAD_BLOCKED: info.state = XYRIS_THREAD_STATE_BLOCKED; break;
        case THREAD_TERMINATED: info.state = XYRIS_THREAD_STATE_TERMINATED; break;
        default: info.state = XYRIS_THREAD_STATE_UNKNOWN; break;
    }
    info.flags = thread->user_thread ? XYRIS_THREAD_FLAG_USER : XYRIS_THREAD_FLAG_KERNEL;
    info.priority = (xyris_u64)thread->priority;
    return copy_to_user(out, &info, sizeof(info));
}

/* ------------------------------------------------------------
 * Memory
 * ------------------------------------------------------------ */
static uint64_t memory_vmm_flags(xyris_u32 protection)
{
    uint64_t flags = VMM_USER;
    if (protection & XYRIS_MEMORY_PROT_WRITE) flags |= VMM_WRITABLE;
    if ((protection & XYRIS_MEMORY_PROT_EXEC) == 0) flags |= VMM_NX;
    return flags;
}

xyris_syscall_result_t xyris_sdk_service_memory_map(xyris_size_t size, xyris_u32 protection, xyris_u32 flags)
{
    process_t *process = current_process();
    if (process == NULL || process->kernel_process || process->address_space == NULL)
        return XYRIS_EPERM;
    if (size == 0 || size > SDK_MEMORY_MAX || (protection & ~(XYRIS_MEMORY_PROT_READ | XYRIS_MEMORY_PROT_WRITE | XYRIS_MEMORY_PROT_EXEC)) != 0 || protection == XYRIS_MEMORY_PROT_NONE)
        return XYRIS_EINVAL;
    if ((flags & ~(XYRIS_MEMORY_FLAG_PRIVATE | XYRIS_MEMORY_FLAG_SHARED | XYRIS_MEMORY_FLAG_GUARD | XYRIS_MEMORY_FLAG_RESERVED)) != 0)
        return XYRIS_EINVAL;
    if ((flags & XYRIS_MEMORY_FLAG_PRIVATE) && (flags & XYRIS_MEMORY_FLAG_SHARED))
        return XYRIS_EINVAL;

    xyris_size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    xyris_size_t mapped_size = pages * PAGE_SIZE;
    for (size_t i = 0; i < SDK_MAX_MEMORY_REGIONS; ++i)
    {
        if (memory_table[i].used)
            continue;
        xyris_addr_t base = SDK_MEMORY_BASE + (xyris_addr_t)i * SDK_MEMORY_STRIDE;
        bool conflict = false;
        for (size_t j = 0; j < SDK_MAX_MEMORY_REGIONS; ++j)
        {
            if (!memory_table[j].used || memory_table[j].owner_pid != process->pid)
                continue;
            xyris_addr_t end_a = base + mapped_size;
            xyris_addr_t end_b = memory_table[j].base + memory_table[j].size;
            if (base < end_b && memory_table[j].base < end_a) { conflict = true; break; }
        }
        if (conflict || base + mapped_size >= SDK_USER_LIMIT)
            continue;

        uint64_t vflags = memory_vmm_flags(protection);
        if ((flags & XYRIS_MEMORY_FLAG_GUARD) != 0)
            vflags |= VMM_NX;
        xyris_size_t done = 0;
        for (; done < mapped_size; done += PAGE_SIZE)
        {
            phys_addr_t page = pmm_alloc_page();
            if (page == 0 || !vmm_map_page((address_space_t *)process->address_space, (uintptr_t)(base + done), page, vflags))
            {
                if (page != 0) pmm_free_page(page);
                for (xyris_size_t rollback = 0; rollback < done; rollback += PAGE_SIZE)
                {
                    phys_addr_t old = vmm_translate((address_space_t *)process->address_space, (uintptr_t)(base + rollback));
                    if (old != 0) { vmm_unmap_page((address_space_t *)process->address_space, (uintptr_t)(base + rollback)); pmm_free_page(old & ~(phys_addr_t)(PAGE_SIZE - 1)); }
                }
                return XYRIS_ENOMEM;
            }
            memset((void *)(uintptr_t)(phys_to_virt(page)), 0, PAGE_SIZE);
        }
        memory_table[i].used = true;
        memory_table[i].owner_pid = process->pid;
        memory_table[i].space = (address_space_t *)process->address_space;
        memory_table[i].base = base;
        memory_table[i].size = mapped_size;
        memory_table[i].protection = protection;
        memory_table[i].flags = flags;
        return (xyris_syscall_result_t)base;
    }
    return XYRIS_ENOSPC;
}

xyris_syscall_result_t xyris_sdk_service_memory_unmap(xyris_addr_t base, xyris_size_t size)
{
    process_t *process = current_process();
    sdk_memory_region_t *region = find_memory(process, base, size);
    if (region == NULL)
        return XYRIS_ENOTFOUND;
    for (xyris_size_t offset = 0; offset < region->size; offset += PAGE_SIZE)
    {
        uintptr_t va = (uintptr_t)(region->base + offset);
        phys_addr_t physical = vmm_translate(region->space, va);
        if (physical != 0) { vmm_unmap_page(region->space, va); pmm_free_page(physical & ~(phys_addr_t)(PAGE_SIZE - 1)); }
    }
    memset(region, 0, sizeof(*region));
    return XYRIS_OK;
}

xyris_syscall_result_t xyris_sdk_service_memory_protect(xyris_addr_t base, xyris_size_t size, xyris_u32 protection)
{
    process_t *process = current_process();
    sdk_memory_region_t *region = find_memory(process, base, size);
    if (region == NULL)
        return XYRIS_ENOTFOUND;
    if (protection == XYRIS_MEMORY_PROT_NONE || (protection & ~(XYRIS_MEMORY_PROT_READ | XYRIS_MEMORY_PROT_WRITE | XYRIS_MEMORY_PROT_EXEC)) != 0)
        return XYRIS_EINVAL;
    uint64_t vflags = memory_vmm_flags(protection);
    for (xyris_size_t offset = 0; offset < region->size; offset += PAGE_SIZE)
        if (!vmm_protect_page(region->space, (uintptr_t)(region->base + offset), vflags))
            return XYRIS_EFAULT;
    region->protection = protection;
    return XYRIS_OK;
}

/* ------------------------------------------------------------
 * IPC
 * ------------------------------------------------------------ */
xyris_syscall_result_t xyris_sdk_service_ipc_create(xyris_u32 flags)
{
    if ((flags & ~(XYRIS_IPC_FLAG_NONBLOCK | XYRIS_IPC_FLAG_REPLY | XYRIS_IPC_FLAG_SIGNAL)) != 0)
        return XYRIS_EINVAL;
    process_t *process = current_process();
    if (process == NULL) return XYRIS_EBADSTATE;
    for (size_t i = 0; i < SDK_MAX_IPC_ENDPOINTS; ++i)
    {
        if (ipc_table[i].used) continue;
        sdk_ipc_endpoint_t *endpoint = &ipc_table[i];
        memset(endpoint, 0, sizeof(*endpoint));
        endpoint->used = true;
        endpoint->handle = new_handle(SDK_HANDLE_IPC);
        endpoint->owner_pid = process->pid;
        endpoint->flags = flags;
        (void)xk_capability_grant(endpoint->handle, XK_CAP_READ | XK_CAP_WRITE | XK_CAP_CREATE);
        return (xyris_syscall_result_t)endpoint->handle;
    }
    return XYRIS_ENOSPC;
}

xyris_syscall_result_t xyris_sdk_service_ipc_send(xyris_handle_t handle, xyris_capability_t capability, xyris_user_ptr_t message_ptr)
{
    sdk_ipc_endpoint_t *endpoint = find_ipc(handle);
    process_t *process = current_process();
    if (endpoint == NULL || process == NULL) return XYRIS_EBADHANDLE;
    if (capability != endpoint->handle || !xk_capability_check(endpoint->handle, XK_CAP_WRITE)) return XYRIS_ECAP;
    if (require_user_read(message_ptr, sizeof(xyris_ipc_message_t)) != XYRIS_OK) return XYRIS_EFAULT;
    xyris_ipc_message_t message_desc;
    if (copy_from_user(&message_desc, message_ptr, sizeof(message_desc)) != XYRIS_OK) return XYRIS_EFAULT;
    if (message_desc.endpoint != handle || message_desc.capability != capability || message_desc.header.version == 0u || message_desc.header.size < sizeof(message_desc) ||
        message_desc.length > XYRIS_IPC_MAX_MESSAGE || (message_desc.flags & ~(XYRIS_IPC_FLAG_NONBLOCK | XYRIS_IPC_FLAG_REPLY | XYRIS_IPC_FLAG_SIGNAL)) != 0)
        return XYRIS_EINVAL;
    if (message_desc.length != 0 && require_user_read(message_desc.data, message_desc.length) != XYRIS_OK) return XYRIS_EFAULT;
    if (endpoint->count >= SDK_IPC_QUEUE_DEPTH)
    {
        if ((message_desc.flags & XYRIS_IPC_FLAG_NONBLOCK) || (endpoint->flags & XYRIS_IPC_FLAG_NONBLOCK)) return XYRIS_EAGAIN;
        scheduler_yield();
        if (endpoint->count >= SDK_IPC_QUEUE_DEPTH) return XYRIS_EAGAIN;
    }
    sdk_ipc_message_slot_t *slot = &endpoint->queue[endpoint->tail];
    slot->length = message_desc.length;
    slot->flags = message_desc.flags;
    slot->tag = message_desc.tag;
    slot->sender_pid = process->pid;
    if (slot->length != 0) memcpy(slot->data, (const void *)(uintptr_t)message_desc.data, slot->length);
    endpoint->tail = (endpoint->tail + 1) % SDK_IPC_QUEUE_DEPTH;
    endpoint->count++;
    return (xyris_syscall_result_t)slot->length;
}

xyris_syscall_result_t xyris_sdk_service_ipc_recv(xyris_handle_t handle, xyris_user_ptr_t data, xyris_u32 capacity, xyris_user_ptr_t tag_out)
{
    sdk_ipc_endpoint_t *endpoint = find_ipc(handle);
    if (endpoint == NULL) return XYRIS_EBADHANDLE;
    if (!owner_matches(endpoint->owner_pid, current_process())) return XYRIS_EACCES;
    if (capacity != 0 && require_user_write(data, capacity) != XYRIS_OK) return XYRIS_EFAULT;
    if (tag_out != 0 && require_user_write(tag_out, sizeof(xyris_u64)) != XYRIS_OK) return XYRIS_EFAULT;
    if (endpoint->count == 0)
    {
        if (endpoint->flags & XYRIS_IPC_FLAG_NONBLOCK) return XYRIS_EAGAIN;
        scheduler_yield();
        if (endpoint->count == 0) return XYRIS_EAGAIN;
    }
    sdk_ipc_message_slot_t *message = &endpoint->queue[endpoint->head];
    if (message->length > capacity) return XYRIS_EOVERFLOW;
    if (message->length != 0) memcpy((void *)(uintptr_t)data, message->data, message->length);
    if (tag_out != 0) *(xyris_u64 *)(uintptr_t)tag_out = message->tag;
    xyris_u32 length = message->length;
    endpoint->head = (endpoint->head + 1) % SDK_IPC_QUEUE_DEPTH;
    endpoint->count--;
    return (xyris_syscall_result_t)length;
}

xyris_syscall_result_t xyris_sdk_service_ipc_close(xyris_handle_t handle)
{
    sdk_ipc_endpoint_t *endpoint = find_ipc(handle);
    if (endpoint == NULL) return XYRIS_EBADHANDLE;
    if (!owner_matches(endpoint->owner_pid, current_process())) return XYRIS_EACCES;
    xk_capability_clear(endpoint->handle);
    memset(endpoint, 0, sizeof(*endpoint));
    return XYRIS_OK;
}

/* ------------------------------------------------------------
 * Events
 * ------------------------------------------------------------ */
xyris_syscall_result_t xyris_sdk_service_event_create(xyris_u32 type, xyris_u32 flags)
{
    if (type < XYRIS_EVENT_SIGNAL || type > XYRIS_EVENT_IO || (flags & ~(XYRIS_EVENT_FLAG_EDGE | XYRIS_EVENT_FLAG_ONESHOT)) != 0)
        return XYRIS_EINVAL;
    process_t *process = current_process();
    if (process == NULL) return XYRIS_EBADSTATE;
    for (size_t i = 0; i < SDK_MAX_EVENTS; ++i)
    {
        if (event_table[i].used) continue;
        sdk_event_t *event = &event_table[i];
        memset(event, 0, sizeof(*event));
        event->used = true;
        event->handle = new_handle(SDK_HANDLE_EVENT);
        event->owner_pid = process->pid;
        event->type = type;
        event->flags = flags;
        event->sequence = 1;
        (void)xk_capability_grant(event->handle, XK_CAP_READ | XK_CAP_INTERRUPT);
        return (xyris_syscall_result_t)event->handle;
    }
    return XYRIS_ENOSPC;
}

xyris_syscall_result_t xyris_sdk_service_event_signal(xyris_handle_t handle, xyris_user_ptr_t payload)
{
    sdk_event_t *event = find_event(handle);
    if (event == NULL) return XYRIS_EBADHANDLE;
    if (!owner_matches(event->owner_pid, current_process())) return XYRIS_EACCES;
    if (payload != 0 && require_user_read(payload, XYRIS_EVENT_PAYLOAD_WORDS * sizeof(xyris_u64)) != XYRIS_OK) return XYRIS_EFAULT;
    if (event->count >= SDK_EVENT_QUEUE_DEPTH)
        return XYRIS_EAGAIN;
    xyris_event_t *out = &event->queue[event->tail];
    memset(out, 0, sizeof(*out));
    out->header.size = sizeof(*out);
    out->header.version = 1;
    out->source = handle;
    out->type = event->type;
    out->flags = event->flags;
    out->sequence = event->sequence++;
    if (payload != 0) memcpy(out->payload, (const void *)(uintptr_t)payload, sizeof(out->payload));
    event->tail = (event->tail + 1) % SDK_EVENT_QUEUE_DEPTH;
    event->count++;
    return XYRIS_OK;
}

xyris_syscall_result_t xyris_sdk_service_event_wait(xyris_handle_t handle, xyris_user_ptr_t out)
{
    sdk_event_t *event = find_event(handle);
    if (event == NULL) return XYRIS_EBADHANDLE;
    if (!owner_matches(event->owner_pid, current_process())) return XYRIS_EACCES;
    if (require_user_write(out, sizeof(xyris_event_t)) != XYRIS_OK) return XYRIS_EFAULT;
    if (event->count == 0)
    {
        scheduler_yield();
        if (event->count == 0) return XYRIS_EAGAIN;
    }
    xyris_event_t value = event->queue[event->head];
    event->head = (event->head + 1) % SDK_EVENT_QUEUE_DEPTH;
    event->count--;
    if ((event->flags & XYRIS_EVENT_FLAG_ONESHOT) != 0)
    {
        xk_capability_clear(event->handle);
        event->used = false;
    }
    return copy_to_user(out, &value, sizeof(value));
}

xyris_syscall_result_t xyris_sdk_service_event_close(xyris_handle_t handle)
{
    sdk_event_t *event = find_event(handle);
    if (event == NULL) return XYRIS_EBADHANDLE;
    if (!owner_matches(event->owner_pid, current_process())) return XYRIS_EACCES;
    xk_capability_clear(event->handle);
    memset(event, 0, sizeof(*event));
    return XYRIS_OK;
}

/* ------------------------------------------------------------
 * Timers
 * ------------------------------------------------------------ */
xyris_syscall_result_t xyris_sdk_service_timer_create(xyris_user_ptr_t spec_ptr)
{
    if (require_user_read(spec_ptr, sizeof(xyris_timer_spec_t)) != XYRIS_OK) return XYRIS_EFAULT;
    xyris_timer_spec_t spec;
    copy_from_user(&spec, spec_ptr, sizeof(spec));
    if (spec.header.size < sizeof(spec) || spec.header.version == 0 || spec.mode > XYRIS_TIMER_PERIODIC || spec.clock > XYRIS_TIMER_CLOCK_REALTIME || (spec.flags & ~(XYRIS_TIMER_FLAG_ABSOLUTE | XYRIS_TIMER_FLAG_WAKE)) != 0 || (spec.mode == XYRIS_TIMER_PERIODIC && spec.interval == 0))
        return XYRIS_EINVAL;
    process_t *process = current_process();
    if (process == NULL) return XYRIS_EBADSTATE;
    for (size_t i = 0; i < SDK_MAX_TIMERS; ++i)
    {
        if (timer_table[i].used) continue;
        sdk_timer_t *timer = &timer_table[i];
        memset(timer, 0, sizeof(*timer));
        timer->used = true;
        timer->handle = new_handle(SDK_HANDLE_TIMER);
        timer->owner_pid = process->pid;
        timer->spec = spec;
        timer->spec.timer = timer->handle;
        uint64_t now = xk_time_milliseconds();
        timer->deadline_ms = (spec.flags & XYRIS_TIMER_FLAG_ABSOLUTE) ? (spec.deadline + 999999ULL) / 1000000ULL : now + (spec.deadline + 999999ULL) / 1000000ULL;
        if (timer->deadline_ms < now && (spec.flags & XYRIS_TIMER_FLAG_ABSOLUTE)) timer->deadline_ms = now;
        (void)xk_capability_grant(timer->handle, XK_CAP_READ | XK_CAP_CREATE);
        return (xyris_syscall_result_t)timer->handle;
    }
    return XYRIS_ENOSPC;
}

xyris_syscall_result_t xyris_sdk_service_timer_cancel(xyris_handle_t handle)
{
    sdk_timer_t *timer = find_timer(handle);
    if (timer == NULL) return XYRIS_EBADHANDLE;
    if (!owner_matches(timer->owner_pid, current_process())) return XYRIS_EACCES;
    timer->used = false;
    xk_capability_clear(timer->handle);
    return XYRIS_OK;
}

xyris_syscall_result_t xyris_sdk_service_timer_wait(xyris_handle_t handle)
{
    sdk_timer_t *timer = find_timer(handle);
    if (timer == NULL) return XYRIS_EBADHANDLE;
    if (!owner_matches(timer->owner_pid, current_process())) return XYRIS_EACCES;
    for (;;)
    {
        uint64_t now = xk_time_milliseconds();
        if (now >= timer->deadline_ms)
        {
            if (timer->spec.mode == XYRIS_TIMER_PERIODIC)
            {
                uint64_t interval_ms = (timer->spec.interval + 999999ULL) / 1000000ULL;
                if (interval_ms == 0) interval_ms = 1;
                timer->deadline_ms += interval_ms;
            }
            return XYRIS_OK;
        }
        uint64_t remaining = timer->deadline_ms - now;
        uint64_t frequency = xk_time_frequency();
        uint64_t ticks = (remaining * frequency + 999ULL) / 1000ULL;
        if (ticks == 0) ticks = 1;
        scheduler_sleep_current(ticks);
    }
}

xyris_syscall_result_t xyris_sdk_service_timer_close(xyris_handle_t handle)
{
    return xyris_sdk_service_timer_cancel(handle);
}

/* ------------------------------------------------------------
 * Devices
 * ------------------------------------------------------------ */
static xyris_u32 device_class_from_driver(XKDriverType type)
{
    switch (type)
    {
        case XK_DRIVER_KEYBOARD: return XYRIS_DEVICE_CLASS_INPUT;
        case XK_DRIVER_MOUSE: return XYRIS_DEVICE_CLASS_INPUT;
        case XK_DRIVER_NETWORK: return XYRIS_DEVICE_CLASS_NET;
        case XK_DRIVER_STORAGE: return XYRIS_DEVICE_CLASS_BLOCK;
        case XK_DRIVER_GRAPHICS: return XYRIS_DEVICE_CLASS_DISPLAY;
        default: return XYRIS_DEVICE_CLASS_CHAR;
    }
}

xyris_syscall_result_t xyris_sdk_service_device_count(void)
{
    return (xyris_syscall_result_t)(xk_driver_count() + xk_pci_device_count());
}

xyris_syscall_result_t xyris_sdk_service_device_info(xyris_u32 index, xyris_user_ptr_t out)
{
    if (require_user_write(out, sizeof(xyris_device_info_t)) != XYRIS_OK) return XYRIS_EFAULT;
    xyris_u32 driver_count = xk_driver_count();
    xyris_device_info_t info;
    memset(&info, 0, sizeof(info));
    info.header.size = sizeof(info);
    info.header.version = 1;
    if (index < driver_count)
    {
        XKDriver *driver = xk_driver_get(index);
        if (driver == NULL) return XYRIS_ENOTFOUND;
        info.handle = SDK_HANDLE_IPC | (0x80000000ULL + index + 1ULL);
        info.object = (xyris_object_id_t)(index + 1U);
        info.class_id = device_class_from_driver(driver->type);
        info.flags = XYRIS_DEVICE_FLAG_READABLE | XYRIS_DEVICE_FLAG_WRITABLE;
        size_t n = 0;
        while (driver->name[n] != '\0' && n + 1 < XYRIS_DEVICE_NAME_MAX) { info.name[n] = driver->name[n]; ++n; }
        info.name[n] = '\0';
        return copy_to_user(out, &info, sizeof(info));
    }
    index -= driver_count;
    const XKPCIDevice *pci = xk_pci_device_get(index);
    if (pci == NULL) return XYRIS_ENOTFOUND;
    info.handle = SDK_HANDLE_IPC | (0x90000000ULL + index + 1ULL);
    info.object = (xyris_object_id_t)(0x10000ULL + index + 1ULL);
    if (pci->class_code == 0x02) info.class_id = XYRIS_DEVICE_CLASS_NET;
    else if (pci->class_code == 0x03) info.class_id = XYRIS_DEVICE_CLASS_DISPLAY;
    else if (pci->class_code == 0x01) info.class_id = XYRIS_DEVICE_CLASS_BLOCK;
    else info.class_id = XYRIS_DEVICE_CLASS_UNKNOWN;
    info.vendor_id = pci->vendor_id;
    info.device_id = pci->device_id;
    info.flags = XYRIS_DEVICE_FLAG_READABLE | XYRIS_DEVICE_FLAG_WRITABLE;
    info.name[0] = 'P'; info.name[1] = 'C'; info.name[2] = 'I'; info.name[3] = '\0';
    return copy_to_user(out, &info, sizeof(info));
}

/* ------------------------------------------------------------
 * Networking — deterministic loopback transport for v0.1.
 * ------------------------------------------------------------ */
xyris_syscall_result_t xyris_sdk_service_net_socket(xyris_u32 family, xyris_u32 protocol)
{
    if (!(family == XYRIS_NET_FAMILY_UNSPEC || family == XYRIS_NET_FAMILY_IPV4) || !(protocol == XYRIS_NET_PROTOCOL_UNSPEC || protocol == XYRIS_NET_PROTOCOL_TCP || protocol == XYRIS_NET_PROTOCOL_UDP))
        return XYRIS_EINVAL;
    process_t *process = current_process();
    if (process == NULL) return XYRIS_EBADSTATE;
    for (size_t i = 0; i < SDK_MAX_NET_SOCKETS; ++i)
    {
        if (net_table[i].used) continue;
        sdk_net_socket_t *socket = &net_table[i];
        memset(socket, 0, sizeof(*socket));
        socket->used = true;
        socket->fd = SDK_NET_FD_BASE + (xyris_fd_t)i;
        socket->owner_pid = process->pid;
        socket->family = family;
        socket->protocol = protocol;
        socket->peer_fd = XYRIS_INVALID_FD;
        return socket->fd;
    }
    return XYRIS_ENOSPC;
}

static xyris_status_t read_endpoint(xyris_user_ptr_t endpoint_ptr, xyris_net_endpoint_t *endpoint)
{
    if (require_user_read(endpoint_ptr, sizeof(*endpoint)) != XYRIS_OK) return XYRIS_EFAULT;
    if (copy_from_user(endpoint, endpoint_ptr, sizeof(*endpoint)) != XYRIS_OK) return XYRIS_EFAULT;
    if (endpoint->header.size < sizeof(*endpoint) || endpoint->header.version == 0 || endpoint->address_length > XYRIS_NET_ADDRESS_MAX || !net_endpoint_is_loopback(endpoint)) return XYRIS_EINVAL;
    return XYRIS_OK;
}

xyris_syscall_result_t xyris_sdk_service_net_bind(xyris_fd_t fd, xyris_user_ptr_t endpoint_ptr)
{
    sdk_net_socket_t *socket = find_net(fd);
    if (socket == NULL) return XYRIS_EBADHANDLE;
    if (!owner_matches(socket->owner_pid, current_process())) return XYRIS_EACCES;
    xyris_net_endpoint_t endpoint;
    xyris_status_t status = read_endpoint(endpoint_ptr, &endpoint);
    if (status != XYRIS_OK) return status;
    if (endpoint.port == 0) endpoint.port = next_net_port++;
    if (find_bound_socket(endpoint.port) != NULL) return XYRIS_EEXIST;
    socket->bound = true;
    socket->local_port = endpoint.port;
    endpoint.header.size = sizeof(endpoint);
    (void)copy_to_user(endpoint_ptr, &endpoint, sizeof(endpoint));
    return socket->local_port;
}

xyris_syscall_result_t xyris_sdk_service_net_connect(xyris_fd_t fd, xyris_user_ptr_t endpoint_ptr)
{
    sdk_net_socket_t *socket = find_net(fd);
    if (socket == NULL) return XYRIS_EBADHANDLE;
    if (!owner_matches(socket->owner_pid, current_process())) return XYRIS_EACCES;
    xyris_net_endpoint_t endpoint;
    xyris_status_t status = read_endpoint(endpoint_ptr, &endpoint);
    if (status != XYRIS_OK) return status;
    sdk_net_socket_t *peer = find_bound_socket(endpoint.port);
    if (peer == NULL) return XYRIS_ECONN;
    socket->connected = true;
    socket->peer_fd = peer->fd;
    socket->remote_port = endpoint.port;
    peer->connected = true;
    peer->peer_fd = socket->fd;
    if (!socket->bound) { socket->bound = true; socket->local_port = next_net_port++; }
    return XYRIS_OK;
}

xyris_syscall_result_t xyris_sdk_service_net_send(xyris_fd_t fd, xyris_user_ptr_t data, xyris_u32 length)
{
    sdk_net_socket_t *socket = find_net(fd);
    if (socket == NULL) return XYRIS_EBADHANDLE;
    if (!owner_matches(socket->owner_pid, current_process())) return XYRIS_EACCES;
    if (!socket->connected || socket->peer_fd < 0) return XYRIS_ECONN;
    if (length > SDK_NET_MESSAGE_MAX || (length != 0 && require_user_read(data, length) != XYRIS_OK)) return XYRIS_EFAULT;
    sdk_net_socket_t *peer = find_net(socket->peer_fd);
    if (peer == NULL) return XYRIS_ECONN;
    if (peer->count >= SDK_NET_QUEUE_DEPTH) return XYRIS_EAGAIN;
    sdk_net_packet_t *packet = &peer->queue[peer->tail];
    packet->length = length;
    if (length != 0) memcpy(packet->data, (const void *)(uintptr_t)data, length);
    peer->tail = (peer->tail + 1) % SDK_NET_QUEUE_DEPTH;
    peer->count++;
    return length;
}

xyris_syscall_result_t xyris_sdk_service_net_recv(xyris_fd_t fd, xyris_user_ptr_t data, xyris_u32 capacity)
{
    sdk_net_socket_t *socket = find_net(fd);
    if (socket == NULL) return XYRIS_EBADHANDLE;
    if (!owner_matches(socket->owner_pid, current_process())) return XYRIS_EACCES;
    if (capacity != 0 && require_user_write(data, capacity) != XYRIS_OK) return XYRIS_EFAULT;
    if (socket->count == 0) return XYRIS_EAGAIN;
    sdk_net_packet_t *packet = &socket->queue[socket->head];
    if (packet->length > capacity) return XYRIS_EOVERFLOW;
    if (packet->length != 0) memcpy((void *)(uintptr_t)data, packet->data, packet->length);
    xyris_u32 length = packet->length;
    socket->head = (socket->head + 1) % SDK_NET_QUEUE_DEPTH;
    socket->count--;
    return length;
}

xyris_syscall_result_t xyris_sdk_service_net_close(xyris_fd_t fd)
{
    sdk_net_socket_t *socket = find_net(fd);
    if (socket == NULL) return XYRIS_EBADHANDLE;
    if (!owner_matches(socket->owner_pid, current_process())) return XYRIS_EACCES;
    if (socket->peer_fd >= 0)
    {
        sdk_net_socket_t *peer = find_net(socket->peer_fd);
        if (peer != NULL) { peer->peer_fd = XYRIS_INVALID_FD; peer->connected = false; }
    }
    memset(socket, 0, sizeof(*socket));
    return XYRIS_OK;
}

/* ------------------------------------------------------------
 * Security
 * ------------------------------------------------------------ */
xyris_syscall_result_t xyris_sdk_service_security_identity(xyris_user_ptr_t out)
{
    if (require_user_write(out, sizeof(xyris_security_identity_t)) != XYRIS_OK) return XYRIS_EFAULT;
    process_t *process = current_process();
    if (process == NULL) return XYRIS_EBADSTATE;
    xyris_security_identity_t identity;
    memset(&identity, 0, sizeof(identity));
    identity.header.size = sizeof(identity);
    identity.header.version = 1;
    identity.identity = process->pid;
    identity.group = process->parent_pid;
    identity.flags = process->kernel_process ? 1ULL : 0ULL;
    return copy_to_user(out, &identity, sizeof(identity));
}

static XKCapability rights_to_capabilities(xyris_u64 rights)
{
    XKCapability caps = XK_CAP_NONE;
    if (rights & XYRIS_SECURITY_RIGHT_READ) caps |= XK_CAP_READ;
    if (rights & XYRIS_SECURITY_RIGHT_WRITE) caps |= XK_CAP_WRITE;
    if (rights & XYRIS_SECURITY_RIGHT_EXECUTE) caps |= XK_CAP_EXECUTE;
    if (rights & XYRIS_SECURITY_RIGHT_SIGNAL) caps |= XK_CAP_INTERRUPT;
    if (rights & XYRIS_SECURITY_RIGHT_TRANSFER) caps |= XK_CAP_MODIFY;
    if (rights & XYRIS_SECURITY_RIGHT_ADMIN) caps |= XK_CAP_KERNEL;
    return caps;
}

xyris_syscall_result_t xyris_sdk_service_security_check(xyris_capability_t capability, xyris_object_id_t object, xyris_u64 rights)
{
    if (capability == XYRIS_INVALID_CAP || object == XYRIS_INVALID_OBJECT || (rights & ~(XYRIS_SECURITY_RIGHT_READ | XYRIS_SECURITY_RIGHT_WRITE | XYRIS_SECURITY_RIGHT_EXECUTE | XYRIS_SECURITY_RIGHT_SIGNAL | XYRIS_SECURITY_RIGHT_TRANSFER | XYRIS_SECURITY_RIGHT_ADMIN)) != 0)
        return XYRIS_EINVAL;
    if (capability != object)
        return XYRIS_ECAP;
    if (rights == 0)
        return XYRIS_OK;
    XKCapability caps = rights_to_capabilities(rights);
    return xk_capability_check(object, caps) ? XYRIS_OK : XYRIS_EACCES;
}
