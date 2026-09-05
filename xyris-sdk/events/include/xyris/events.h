#ifndef XYRIS_EVENTS_H
#define XYRIS_EVENTS_H

#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_SDK_EVENTS_VERSION_MAJOR 0u
#define XYRIS_SDK_EVENTS_VERSION_MINOR 1u

static inline xyris_bool_t xyris_event_handle_valid(xyris_handle_t handle)
{ return handle != XYRIS_INVALID_HANDLE ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_event_type_valid(xyris_u32 type)
{ return type >= XYRIS_EVENT_SIGNAL && type <= XYRIS_EVENT_IO ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_event_flags_valid(xyris_u32 flags)
{ const xyris_u32 known = XYRIS_EVENT_FLAG_EDGE | XYRIS_EVENT_FLAG_ONESHOT; return (flags & ~known) == 0u ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_event_valid(const xyris_event_t *event)
{ return event && event->header.size >= sizeof(*event) && event->header.version != 0u && xyris_event_handle_valid(event->source) && xyris_event_type_valid(event->type) && xyris_event_flags_valid(event->flags) ? XYRIS_TRUE : XYRIS_FALSE; }
static inline void xyris_event_init(xyris_event_t *event, xyris_handle_t source, xyris_u32 type, xyris_u32 flags, xyris_u64 sequence)
{ if (!event) return; event->header.size = sizeof(*event); event->header.version = 1u; event->header.flags = 0u; event->source = source; event->type = type; event->flags = flags; event->sequence = sequence; for (xyris_u32 i = 0; i < XYRIS_EVENT_PAYLOAD_WORDS; ++i) event->payload[i] = 0u; }
static inline xyris_bool_t xyris_event_subscription_valid(const xyris_event_subscription_t *subscription)
{ return subscription && subscription->header.size >= sizeof(*subscription) && subscription->header.version != 0u && xyris_event_handle_valid(subscription->event) && xyris_event_handle_valid(subscription->target) ? XYRIS_TRUE : XYRIS_FALSE; }

static inline xyris_handle_t xyris_event_create(xyris_u32 type, xyris_u32 flags)
{ return (xyris_handle_t)xyris_syscall2(XYRIS_SYS_EVENT_CREATE, type, flags); }
static inline xyris_status_t xyris_event_signal(xyris_handle_t event, const xyris_u64 payload[XYRIS_EVENT_PAYLOAD_WORDS])
{ return xyris_syscall2(XYRIS_SYS_EVENT_SIGNAL, event, (xyris_user_ptr_t)(uintptr_t)payload); }
static inline xyris_status_t xyris_event_wait(xyris_handle_t event, xyris_event_t *out)
{ return xyris_syscall2(XYRIS_SYS_EVENT_WAIT, event, (xyris_user_ptr_t)(uintptr_t)out); }
static inline xyris_status_t xyris_event_close(xyris_handle_t event)
{ return xyris_syscall1(XYRIS_SYS_EVENT_CLOSE, event); }

#ifdef __cplusplus
}
#endif
#endif
