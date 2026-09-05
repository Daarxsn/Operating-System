#ifndef XYRIS_IPC_H
#define XYRIS_IPC_H

#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_SDK_IPC_VERSION_MAJOR 0u
#define XYRIS_SDK_IPC_VERSION_MINOR 1u

static inline xyris_bool_t xyris_ipc_handle_valid(xyris_handle_t handle)
{ return handle != XYRIS_INVALID_HANDLE ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_ipc_capability_valid(xyris_capability_t capability)
{ return capability != XYRIS_INVALID_CAP ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_ipc_object_valid(xyris_object_id_t object)
{ return object != XYRIS_INVALID_OBJECT ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_ipc_flags_valid(xyris_u32 flags)
{ const xyris_u32 known = XYRIS_IPC_FLAG_NONBLOCK | XYRIS_IPC_FLAG_REPLY | XYRIS_IPC_FLAG_SIGNAL; return (flags & ~known) == 0u ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_ipc_length_valid(xyris_u32 length)
{ return length <= XYRIS_IPC_MAX_MESSAGE ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_ipc_message_valid(const xyris_ipc_message_t *message)
{
    if (!message || message->header.size < sizeof(xyris_ipc_message_t) || message->header.version == 0u) return XYRIS_FALSE;
    if (!xyris_ipc_handle_valid(message->endpoint) || !xyris_ipc_capability_valid(message->capability)) return XYRIS_FALSE;
    if (!xyris_ipc_flags_valid(message->flags) || !xyris_ipc_length_valid(message->length)) return XYRIS_FALSE;
    return message->length == 0u || message->data != 0u ? XYRIS_TRUE : XYRIS_FALSE;
}
static inline void xyris_ipc_message_init(xyris_ipc_message_t *message, xyris_handle_t endpoint, xyris_capability_t capability, xyris_u32 flags, const void *data, xyris_u32 length, xyris_u64 tag)
{
    if (!message) return;
    message->header.size = sizeof(*message); message->header.version = 1u; message->header.flags = 0u;
    message->endpoint = endpoint; message->capability = capability; message->flags = flags; message->length = length;
    message->data = (xyris_user_ptr_t)(uintptr_t)data; message->tag = tag;
}

static inline xyris_handle_t xyris_ipc_create(xyris_u32 flags)
{ return (xyris_handle_t)xyris_syscall1(XYRIS_SYS_IPC_CREATE, flags); }
static inline xyris_status_t xyris_ipc_send(xyris_handle_t endpoint, xyris_capability_t capability, const void *data, xyris_u32 length, xyris_u32 flags, xyris_u64 tag)
{
    /* The ABI has four argument registers; flags/tag are carried in a message descriptor. */
    xyris_ipc_message_t message; xyris_ipc_message_init(&message, endpoint, capability, flags, data, length, tag);
    return xyris_syscall4(XYRIS_SYS_IPC_SEND, endpoint, capability, (xyris_user_ptr_t)(uintptr_t)data, ((xyris_u64)flags << 32) | length);
}
static inline xyris_status_t xyris_ipc_receive(xyris_handle_t endpoint, void *buffer, xyris_u32 capacity, xyris_u64 *tag_out)
{ return xyris_syscall4(XYRIS_SYS_IPC_RECV, endpoint, (xyris_user_ptr_t)(uintptr_t)buffer, capacity, (xyris_user_ptr_t)(uintptr_t)tag_out); }
static inline xyris_status_t xyris_ipc_close(xyris_handle_t endpoint)
{ return xyris_syscall1(XYRIS_SYS_IPC_CLOSE, endpoint); }

#ifdef __cplusplus
}
#endif
#endif
