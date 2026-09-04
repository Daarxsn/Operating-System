#ifndef XYRIS_ABI_TIMERS_H
#define XYRIS_ABI_TIMERS_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_TIMER_ONESHOT  0u
#define XYRIS_TIMER_PERIODIC 1u

#define XYRIS_TIMER_FLAG_ABSOLUTE (1u << 0)
#define XYRIS_TIMER_FLAG_WAKE     (1u << 1)

#define XYRIS_TIMER_CLOCK_MONOTONIC 0u
#define XYRIS_TIMER_CLOCK_REALTIME  1u

typedef struct xyris_timer_spec {
    xyris_abi_header_t header;
    xyris_handle_t timer;
    xyris_u32 mode;
    xyris_u32 clock;
    xyris_u32 flags;
    xyris_u32 reserved0;
    xyris_time_ns_t deadline;
    xyris_duration_ns_t interval;
} xyris_timer_spec_t;

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_ABI_TIMERS_H */
