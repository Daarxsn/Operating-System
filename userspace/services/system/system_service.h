#ifndef XYRIS_USERSPACE_SYSTEM_SERVICE_H
#define XYRIS_USERSPACE_SYSTEM_SERVICE_H

#include <stdint.h>

#include <xyris/core.h>

#define XYRIS_SYSTEM_SERVICE_NAME "system-service"
#define XYRIS_SYSTEM_SERVICE_VERSION 1u

typedef struct
{
    uint32_t version;
    xyris_pid_t pid;
    xyris_tid_t tid;
    xyris_u32 device_count;
    xyris_u64 identity;
    xyris_u64 group;
    xyris_u64 security_flags;
} xyris_system_snapshot_t;

int xyris_system_service_start(void *context);
int xyris_system_service_stop(void *context);
int xyris_system_service_running(void);
int xyris_system_service_get_snapshot(xyris_system_snapshot_t *snapshot);

#endif /* XYRIS_USERSPACE_SYSTEM_SERVICE_H */
