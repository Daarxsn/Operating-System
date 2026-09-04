#ifndef XYRIS_ABI_NETWORKING_H
#define XYRIS_ABI_NETWORKING_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_NET_FAMILY_UNSPEC 0u
#define XYRIS_NET_FAMILY_IPV4  2u
#define XYRIS_NET_FAMILY_IPV6 10u

#define XYRIS_NET_PROTOCOL_UNSPEC 0u
#define XYRIS_NET_PROTOCOL_TCP    6u
#define XYRIS_NET_PROTOCOL_UDP   17u

#define XYRIS_NET_ADDRESS_MAX 16u

typedef struct xyris_net_endpoint {
    xyris_abi_header_t header;
    xyris_u32 family;
    xyris_u32 protocol;
    xyris_u16 port;
    xyris_u16 flags;
    xyris_u8 address[XYRIS_NET_ADDRESS_MAX];
    xyris_u32 address_length;
    xyris_u32 reserved0;
} xyris_net_endpoint_t;

typedef struct xyris_net_socket_info {
    xyris_abi_header_t header;
    xyris_fd_t fd;
    xyris_u32 state;
    xyris_net_endpoint_t local;
    xyris_net_endpoint_t remote;
} xyris_net_socket_info_t;

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_ABI_NETWORKING_H */
