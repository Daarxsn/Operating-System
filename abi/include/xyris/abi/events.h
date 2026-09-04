#ifndef XYRIS_ABI_EVENTS_H
#define XYRIS_ABI_EVENTS_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_EVENT_NONE    0u
#define XYRIS_EVENT_SIGNAL  1u
#define XYRIS_EVENT_MESSAGE 2u
#define XYRIS_EVENT_TIMER   3u
#define XYRIS_EVENT_IO      4u

#define XYRIS_EVENT_FLAG_EDGE    (1u << 0)
#define XYRIS_EVENT_FLAG_ONESHOT (1u << 1)

#define XYRIS_EVENT_PAYLOAD_WORDS 4u

typedef struct xyris_event {
    xyris_abi_header_t header;
    xyris_handle_t source;
    xyris_u32 type;
    xyris_u32 flags;
    xyris_u64 sequence;
    xyris_u64 payload[XYRIS_EVENT_PAYLOAD_WORDS];
} xyris_event_t;

typedef struct xyris_event_subscription {
    xyris_abi_header_t header;
    xyris_handle_t event;
    xyris_handle_t target;
    xyris_u32 flags;
    xyris_u32 reserved0;
} xyris_event_subscription_t;

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_ABI_EVENTS_H */
