#include <xyris/process.h>
#include <xyris/thread.h>

#include "service_manager.h"
#include "../services/system/system_service.h"
#include "../services/registry/service_registry.h"
#include "../shell/shell.h"
#include "../utilities/userspace_utilities.h"

static int init_service_start(void *context)
{
    (void)context;
    return 0;
}

static int init_service_stop(void *context)
{
    (void)context;
    return 0;
}

void _start(void)
{
    xyris_pid_t pid = xyris_process_get_pid();
    int manager_status;

    if (pid == 0 || pid == XYRIS_INVALID_PID)
    {
        (void)xyris_process_exit(1);
        for (;;)
            xyris_thread_sleep(1000);
    }

    xyris_service_manager_initialize();
    xyris_service_registry_initialize();

    /*
     * The init service represents this process itself. It establishes the
     * lifecycle anchor for later independent service processes.
     */
    if (xyris_service_register(
            "userspace-init",
            NULL,
            0,
            init_service_start,
            init_service_stop,
            NULL) != 0)
    {
        (void)xyris_process_exit(2);
        for (;;)
            xyris_thread_sleep(1000);
    }

    {
        const char *system_dependencies[] = { "userspace-init" };

        if (xyris_service_register(
                XYRIS_SYSTEM_SERVICE_NAME,
                system_dependencies,
                1,
                xyris_system_service_start,
                xyris_system_service_stop,
                NULL) != 0)
        {
            (void)xyris_process_exit(3);
            for (;;)
                xyris_thread_sleep(1000);
        }
    }

    manager_status = xyris_service_start_all();
    if (manager_status != 0)
    {
        (void)xyris_process_exit(3);
        for (;;)
            xyris_thread_sleep(1000);
    }

    if (xyris_service_registry_publish(
            XYRIS_SYSTEM_SERVICE_NAME,
            XYRIS_SYSTEM_SERVICE_VERSION,
            XYRIS_INVALID_HANDLE,
            XYRIS_INVALID_CAP) != 0)
    {
        (void)xyris_process_exit(5);
        for (;;)
            xyris_thread_sleep(1000);
    }

    {
        xyris_service_descriptor_t descriptor;

        if (xyris_service_registry_lookup(
                XYRIS_SYSTEM_SERVICE_NAME, &descriptor) != 0 ||
            descriptor.version != XYRIS_SYSTEM_SERVICE_VERSION ||
            descriptor.endpoint != XYRIS_INVALID_HANDLE ||
            descriptor.capability != XYRIS_INVALID_CAP)
        {
            (void)xyris_process_exit(6);
            for (;;)
                xyris_thread_sleep(1000);
        }
    }

    {
        xyris_system_snapshot_t snapshot;

        if (xyris_system_service_get_snapshot(&snapshot) != 0 ||
            snapshot.version != XYRIS_SYSTEM_SERVICE_VERSION ||
            snapshot.pid != pid ||
            snapshot.tid == XYRIS_INVALID_TID)
        {
            (void)xyris_process_exit(4);
            for (;;)
                xyris_thread_sleep(1000);
        }
    }

    /* M9.5 validates the shell command core without assuming a terminal device. */
    if (xyris_shell_execute_line("help", NULL, NULL) != XYRIS_SHELL_OK ||
        xyris_shell_execute_line("services", NULL, NULL) != XYRIS_SHELL_OK ||
        xyris_shell_execute_line("system", NULL, NULL) != XYRIS_SHELL_OK ||
        xyris_shell_execute_line("pid", NULL, NULL) != XYRIS_SHELL_OK)
    {
        (void)xyris_process_exit(7);
        for (;;)
            xyris_thread_sleep(1000);
    }

    /* M9.6 validates a real userspace filesystem utility against the public ABI. */
    if (xyris_utility_cat("/etc/xyris.txt", NULL, NULL) != XYRIS_UTILITY_OUTPUT_OK)
    {
        (void)xyris_process_exit(8);
        for (;;)
            xyris_thread_sleep(1000);
    }

    /* M9.5/M9.6 keep init alive. A later terminal adapter will supply interactive I/O. */
    for (;;)
        xyris_thread_sleep(1000);
}
