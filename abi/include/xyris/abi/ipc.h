#ifndef XYRIS_ABI_IPC_H
#define XYRIS_ABI_IPC_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_IPC_FLAG_NONBLOCK (1u << 0)
#define XYRIS_IPC_FLAG_REPLY    (1u << 1)
#define XYRIS_IPC_FLAG_SIGNAL   (1u << 2)

#define XYRIS_IPC_MAX_MESSAGE 4096u

typedef struct xyris_ipc_message {
    xyris_abi_header_t header;
    xyris_handle_t endpoint;
    xyris_capability_t capability;
    xyris_u32 flags;
    xyris_u32 length;
    xyris_user_ptr_t data;
    xyris_u64 tag;
} xyris_ipc_message_t;

typedef struct xyris_ipc_endpoint_info {
    xyris_abi_header_t header;
    xyris_handle_t handle;
    xyris_object_id_t object;
    xyris_u32 flags;
    xyris_u32 queue_depth;
} xyris_ipc_endpoint_info_t;

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_ABI_IPC_H */
