#ifndef XYRIS_NETWORKING_H
#define XYRIS_NETWORKING_H

#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_SDK_NETWORKING_VERSION_MAJOR 0u
#define XYRIS_SDK_NETWORKING_VERSION_MINOR 1u

static inline xyris_bool_t xyris_network_fd_valid(xyris_fd_t fd)
{ return fd != XYRIS_INVALID_FD ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_network_family_valid(xyris_u32 family)
{ return family == XYRIS_NET_FAMILY_UNSPEC || family == XYRIS_NET_FAMILY_IPV4 || family == XYRIS_NET_FAMILY_IPV6 ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_network_protocol_valid(xyris_u32 protocol)
{ return protocol == XYRIS_NET_PROTOCOL_UNSPEC || protocol == XYRIS_NET_PROTOCOL_TCP || protocol == XYRIS_NET_PROTOCOL_UDP ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_network_endpoint_valid(const xyris_net_endpoint_t *endpoint)
{ return endpoint && endpoint->header.size >= sizeof(*endpoint) && endpoint->header.version != 0u && xyris_network_family_valid(endpoint->family) && xyris_network_protocol_valid(endpoint->protocol) && endpoint->address_length <= XYRIS_NET_ADDRESS_MAX ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_network_socket_info_valid(const xyris_net_socket_info_t *info)
{ return info && info->header.size >= sizeof(*info) && info->header.version != 0u && xyris_network_fd_valid(info->fd) && xyris_network_endpoint_valid(&info->local) && xyris_network_endpoint_valid(&info->remote) ? XYRIS_TRUE : XYRIS_FALSE; }

/* v0.1 provides a deterministic kernel loopback transport. */
static inline xyris_fd_t xyris_net_socket(xyris_u32 family, xyris_u32 protocol)
{ return (xyris_fd_t)xyris_syscall2(XYRIS_SYS_NET_SOCKET, family, protocol); }
static inline xyris_status_t xyris_net_bind(xyris_fd_t fd, xyris_net_endpoint_t *endpoint)
{ return xyris_syscall2(XYRIS_SYS_NET_BIND, (xyris_syscall_arg_t)(xyris_i64)fd, (xyris_user_ptr_t)(uintptr_t)endpoint); }
static inline xyris_status_t xyris_net_connect(xyris_fd_t fd, const xyris_net_endpoint_t *endpoint)
{ return xyris_syscall2(XYRIS_SYS_NET_CONNECT, (xyris_syscall_arg_t)(xyris_i64)fd, (xyris_user_ptr_t)(uintptr_t)endpoint); }
static inline xyris_status_t xyris_net_send(xyris_fd_t fd, const void *data, xyris_u32 length)
{ return xyris_syscall3(XYRIS_SYS_NET_SEND, (xyris_syscall_arg_t)(xyris_i64)fd, (xyris_user_ptr_t)(uintptr_t)data, length); }
static inline xyris_status_t xyris_net_recv(xyris_fd_t fd, void *data, xyris_u32 capacity)
{ return xyris_syscall3(XYRIS_SYS_NET_RECV, (xyris_syscall_arg_t)(xyris_i64)fd, (xyris_user_ptr_t)(uintptr_t)data, capacity); }
static inline xyris_status_t xyris_net_close(xyris_fd_t fd)
{ return xyris_syscall1(XYRIS_SYS_NET_CLOSE, (xyris_syscall_arg_t)(xyris_i64)fd); }

#ifdef __cplusplus
}
#endif
#endif
