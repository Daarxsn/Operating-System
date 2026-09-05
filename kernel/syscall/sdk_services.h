#ifndef XYRIS_SDK_SERVICES_H
#define XYRIS_SDK_SERVICES_H

#include "../process/process.h"
#include "../../abi/include/xyris/abi/xyris_abi.h"

/* Kernel-backed implementation of the public Xyris SDK service modules. */
void xyris_sdk_services_init(void);
void xyris_sdk_services_process_cleanup(process_t *process);

xyris_syscall_result_t xyris_sdk_service_getpid(void);
xyris_syscall_result_t xyris_sdk_service_thread_self(void);
xyris_syscall_result_t xyris_sdk_service_thread_yield(void);
xyris_syscall_result_t xyris_sdk_service_thread_sleep(xyris_u64 milliseconds);
xyris_syscall_result_t xyris_sdk_service_thread_info(xyris_tid_t tid, xyris_user_ptr_t out);

xyris_syscall_result_t xyris_sdk_service_memory_map(xyris_size_t size, xyris_u32 protection, xyris_u32 flags);
xyris_syscall_result_t xyris_sdk_service_memory_unmap(xyris_addr_t base, xyris_size_t size);
xyris_syscall_result_t xyris_sdk_service_memory_protect(xyris_addr_t base, xyris_size_t size, xyris_u32 protection);

xyris_syscall_result_t xyris_sdk_service_ipc_create(xyris_u32 flags);
xyris_syscall_result_t xyris_sdk_service_ipc_send(xyris_handle_t endpoint, xyris_capability_t capability, xyris_user_ptr_t message);
xyris_syscall_result_t xyris_sdk_service_ipc_recv(xyris_handle_t endpoint, xyris_user_ptr_t data, xyris_u32 capacity, xyris_user_ptr_t tag_out);
xyris_syscall_result_t xyris_sdk_service_ipc_close(xyris_handle_t endpoint);

xyris_syscall_result_t xyris_sdk_service_event_create(xyris_u32 type, xyris_u32 flags);
xyris_syscall_result_t xyris_sdk_service_event_signal(xyris_handle_t event, xyris_user_ptr_t payload);
xyris_syscall_result_t xyris_sdk_service_event_wait(xyris_handle_t event, xyris_user_ptr_t out);
xyris_syscall_result_t xyris_sdk_service_event_close(xyris_handle_t event);

xyris_syscall_result_t xyris_sdk_service_timer_create(xyris_user_ptr_t spec);
xyris_syscall_result_t xyris_sdk_service_timer_cancel(xyris_handle_t timer);
xyris_syscall_result_t xyris_sdk_service_timer_wait(xyris_handle_t timer);
xyris_syscall_result_t xyris_sdk_service_timer_close(xyris_handle_t timer);

xyris_syscall_result_t xyris_sdk_service_device_count(void);
xyris_syscall_result_t xyris_sdk_service_device_info(xyris_u32 index, xyris_user_ptr_t out);

xyris_syscall_result_t xyris_sdk_service_net_socket(xyris_u32 family, xyris_u32 protocol);
xyris_syscall_result_t xyris_sdk_service_net_bind(xyris_fd_t fd, xyris_user_ptr_t endpoint);
xyris_syscall_result_t xyris_sdk_service_net_connect(xyris_fd_t fd, xyris_user_ptr_t endpoint);
xyris_syscall_result_t xyris_sdk_service_net_send(xyris_fd_t fd, xyris_user_ptr_t data, xyris_u32 length);
xyris_syscall_result_t xyris_sdk_service_net_recv(xyris_fd_t fd, xyris_user_ptr_t data, xyris_u32 capacity);
xyris_syscall_result_t xyris_sdk_service_net_close(xyris_fd_t fd);

xyris_syscall_result_t xyris_sdk_service_security_identity(xyris_user_ptr_t out);
xyris_syscall_result_t xyris_sdk_service_security_check(xyris_capability_t capability, xyris_object_id_t object, xyris_u64 rights);

#endif
