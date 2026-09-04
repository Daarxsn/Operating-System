#ifndef XYRIS_ABI_PROCESS_H
#define XYRIS_ABI_PROCESS_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_PROCESS_STATE_UNKNOWN    0u
#define XYRIS_PROCESS_STATE_READY      1u
#define XYRIS_PROCESS_STATE_RUNNING    2u
#define XYRIS_PROCESS_STATE_BLOCKED    3u
#define XYRIS_PROCESS_STATE_EXITING    4u
#define XYRIS_PROCESS_STATE_TERMINATED 5u

#define XYRIS_PROCESS_FLAG_KERNEL     (1u << 0)
#define XYRIS_PROCESS_FLAG_USER       (1u << 1)
#define XYRIS_PROCESS_FLAG_DEBUGGABLE (1u << 2)

#define XYRIS_PROCESS_NAME_MAX 32u

typedef struct xyris_process_info {
    xyris_abi_header_t header;
    xyris_pid_t pid;
    xyris_pid_t parent_pid;
    xyris_u32 state;
    xyris_u32 flags;
    xyris_u32 thread_count;
    xyris_u32 reserved0;
    char name[XYRIS_PROCESS_NAME_MAX];
} xyris_process_info_t;

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_ABI_PROCESS_H */
