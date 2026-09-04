#ifndef XYRIS_ABI_THREAD_H
#define XYRIS_ABI_THREAD_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_THREAD_STATE_UNKNOWN    0u
#define XYRIS_THREAD_STATE_READY      1u
#define XYRIS_THREAD_STATE_RUNNING    2u
#define XYRIS_THREAD_STATE_BLOCKED    3u
#define XYRIS_THREAD_STATE_TERMINATED 4u

#define XYRIS_THREAD_FLAG_USER   (1u << 0)
#define XYRIS_THREAD_FLAG_KERNEL (1u << 1)

#define XYRIS_THREAD_NAME_MAX 32u

typedef struct xyris_thread_info {
    xyris_abi_header_t header;
    xyris_tid_t tid;
    xyris_pid_t pid;
    xyris_u32 state;
    xyris_u32 flags;
    xyris_u64 priority;
    xyris_u64 reserved0;
    char name[XYRIS_THREAD_NAME_MAX];
} xyris_thread_info_t;

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_ABI_THREAD_H */
