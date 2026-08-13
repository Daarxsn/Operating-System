#include "process.h"

#include <stddef.h>
#include "../lib/string.h"
#include "thread.h"
#include "../memory/vmm.h"

/*
 * ============================================================
 * XyrisOS Process Manager
 * ============================================================
 */

/*
 * ------------------------------------------------------------
 * Process Table
 * ------------------------------------------------------------
 */

static process_t process_table[PROCESS_MAX_COUNT];

static process_t *current_process = NULL;

static process_id_t next_pid = 1;

/*
 * ------------------------------------------------------------
 * Initialize Process Manager
 * ------------------------------------------------------------
 */

void process_initialize(void)
{
    memset(process_table, 0, sizeof(process_table));

    current_process = NULL;

    next_pid = 1;
}

/*
 * ------------------------------------------------------------
 * Create Process
 * ------------------------------------------------------------
 */

process_t *process_create(
    const char *name,
    bool kernel_process)
{
    if (name == NULL)
    {
        return NULL;
    }

    for (uint32_t i = 0; i < PROCESS_MAX_COUNT; i++)
    {
        if (process_table[i].pid == 0)
        {
            process_t *process = &process_table[i];

            memset(process, 0, sizeof(process_t));

            process->pid = next_pid++;

            strncpy(
                process->name,
                name,
                PROCESS_NAME_LENGTH - 1
            );

            process->name[PROCESS_NAME_LENGTH - 1] = '\0';

            process->state = PROCESS_CREATED;

            process->kernel_process = kernel_process;

            process->main_thread = NULL;

            process->address_space = NULL;

            process->cpu_time = 0;

            process->flags = 0;

            return process;
        }
    }

    return NULL;
}

/*
 * ------------------------------------------------------------
 * Destroy Process
 * ------------------------------------------------------------
 */

void process_destroy(process_t *process)
{
    if (process == NULL)
    {
        return;
    }

    /*
     * A running process cannot safely free its active thread
     * stack or clear its own PCB. Terminate it and defer the final
     * PCB reclamation, but safely clean up every non-current thread.
     */
    if (current_process == process)
    {
        process->state =
            PROCESS_TERMINATED;

        thread_destroy_process_threads(process);
        return;
    }

    thread_destroy_process_threads(process);

    if (process->address_space != NULL)
    {
        vmm_destroy_space(
            (address_space_t *)process->address_space
        );

        process->address_space = NULL;
    }

    memset(process, 0, sizeof(process_t));
}

/*
 * ------------------------------------------------------------
 * Current Process
 * ------------------------------------------------------------
 */

process_t *process_current(void)
{
    return current_process;
}

/*
 * ------------------------------------------------------------
 * Set Current Process
 * ------------------------------------------------------------
 */

void process_set_current(process_t *process)
{
    current_process = process;
}