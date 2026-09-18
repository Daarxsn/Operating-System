#include "system_service.h"

#include <stddef.h>

#include <xyris/devices.h>
#include <xyris/process.h>
#include <xyris/security.h>
#include <xyris/thread.h>

static xyris_system_snapshot_t system_snapshot;
static int system_service_is_running;

static void snapshot_clear(void)
{
    system_snapshot.version = 0u;
    system_snapshot.pid = XYRIS_INVALID_PID;
    system_snapshot.tid = XYRIS_INVALID_TID;
    system_snapshot.device_count = 0u;
    system_snapshot.identity = 0u;
    system_snapshot.group = 0u;
    system_snapshot.security_flags = 0u;
}

int xyris_system_service_start(void *context)
{
    xyris_security_identity_t identity;
    xyris_status_t status;
    xyris_pid_t pid;
    xyris_tid_t tid;
    xyris_u32 device_count;

    (void)context;

    if (system_service_is_running)
        return 0;

    pid = xyris_process_get_pid();
    tid = xyris_thread_self();
    if (pid == XYRIS_INVALID_PID || tid == XYRIS_INVALID_TID)
        return -1;

    /* The v0.1 device-count SDK helper returns the count as an unsigned value. */
    device_count = xyris_device_count();

    identity.header.size = sizeof(identity);
    identity.header.version = 1u;
    identity.header.flags = 0u;
    identity.identity = 0u;
    identity.group = 0u;
    identity.flags = 0u;

    status = xyris_security_get_identity(&identity);
    if (status != XYRIS_OK || !xyris_security_identity_valid(&identity))
        return -1;

    system_snapshot.version = XYRIS_SYSTEM_SERVICE_VERSION;
    system_snapshot.pid = pid;
    system_snapshot.tid = tid;
    system_snapshot.device_count = device_count;
    system_snapshot.identity = identity.identity;
    system_snapshot.group = identity.group;
    system_snapshot.security_flags = identity.flags;
    system_service_is_running = 1;

    return 0;
}

int xyris_system_service_stop(void *context)
{
    (void)context;

    if (!system_service_is_running)
        return 0;

    snapshot_clear();
    system_service_is_running = 0;
    return 0;
}

int xyris_system_service_running(void)
{
    return system_service_is_running;
}

int xyris_system_service_get_snapshot(xyris_system_snapshot_t *snapshot)
{
    if (snapshot == NULL || !system_service_is_running)
        return -1;

    *snapshot = system_snapshot;
    return 0;
}
