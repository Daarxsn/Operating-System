#include <xyris/abi/xyris_abi.h>
#include <cstddef>

static_assert(sizeof(xyris_u8) == 1, "xyris_u8 width changed");
static_assert(sizeof(xyris_u16) == 2, "xyris_u16 width changed");
static_assert(sizeof(xyris_u32) == 4, "xyris_u32 width changed");
static_assert(sizeof(xyris_u64) == 8, "xyris_u64 width changed");
static_assert(sizeof(xyris_pid_t) == 4, "xyris_pid_t width changed");
static_assert(sizeof(xyris_tid_t) == 4, "xyris_tid_t width changed");
static_assert(sizeof(xyris_handle_t) == 8, "xyris_handle_t width changed");
static_assert(sizeof(xyris_capability_t) == 8, "xyris_capability_t width changed");
static_assert(sizeof(xyris_fd_t) == 4, "xyris_fd_t width changed");
static_assert(sizeof(xyris_status_t) == 8, "xyris_status_t width changed");
static_assert(sizeof(xyris_abi_header_t) == 8, "ABI header layout changed");
static_assert(offsetof(xyris_abi_header_t, size) == 0, "ABI header size offset changed");
static_assert(offsetof(xyris_abi_header_t, version) == 4, "ABI header version offset changed");
static_assert(offsetof(xyris_abi_header_t, flags) == 6, "ABI header flags offset changed");

static_assert(offsetof(xyris_process_info_t, header) == 0, "process header missing");
static_assert(offsetof(xyris_thread_info_t, header) == 0, "thread header missing");
static_assert(offsetof(xyris_memory_region_t, header) == 0, "memory header missing");
static_assert(offsetof(xyris_file_info_t, header) == 0, "file header missing");
static_assert(offsetof(xyris_directory_entry_t, header) == 0, "directory header missing");
static_assert(offsetof(xyris_ipc_message_t, header) == 0, "IPC header missing");
static_assert(offsetof(xyris_event_t, header) == 0, "event header missing");
static_assert(offsetof(xyris_timer_spec_t, header) == 0, "timer header missing");
static_assert(offsetof(xyris_device_info_t, header) == 0, "device header missing");
static_assert(offsetof(xyris_net_endpoint_t, header) == 0, "network header missing");
static_assert(offsetof(xyris_security_identity_t, header) == 0, "security header missing");
static_assert(offsetof(xyris_capability_info_t, header) == 0, "capability header missing");

static_assert(sizeof(xyris_sys_read_args_t) == 24, "READ ABI arguments changed");
static_assert(sizeof(xyris_sys_write_args_t) == 24, "WRITE ABI arguments changed");
static_assert(sizeof(xyris_sys_open_args_t) == 8, "OPEN ABI arguments changed");
static_assert(sizeof(xyris_sys_close_args_t) == 4, "CLOSE ABI arguments changed");
static_assert(sizeof(xyris_sys_exit_args_t) == 4, "EXIT ABI arguments changed");

int main()
{
    return (XYRIS_ABI_MAJOR == 0u &&
            XYRIS_ABI_MINOR == 1u &&
            XYRIS_SYSCALL_ABI_VERSION == 1u &&
            XYRIS_SYSCALL_VECTOR == 0x80u &&
            XYRIS_SYS_MAX == 5u &&
            XYRIS_SYS_RESERVED_FIRST == XYRIS_SYS_MAX) ? 0 : 1;
}
