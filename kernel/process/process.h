#ifndef XYRIS_PROCESS_H
#define XYRIS_PROCESS_H

#include <stdint.h>
#include <stdbool.h>

/*
 * ============================================================
 * XyrisOS Process Manager
 * ------------------------------------------------------------
 * Defines the Process Control Block (PCB) and public API for
 * process lifecycle management.
 * ============================================================
 */

/*
 * ------------------------------------------------------------
 * Configuration
 * ------------------------------------------------------------
 */

#define PROCESS_MAX_COUNT      128
#define PROCESS_NAME_LENGTH    64

/*
 * ------------------------------------------------------------
 * Forward Declaration
 * ------------------------------------------------------------
 */

struct thread;

typedef struct thread thread_t;

/*
 * ------------------------------------------------------------
 * Process Identifier
 * ------------------------------------------------------------
 */

typedef uint32_t process_id_t;

/*
 * ------------------------------------------------------------
 * Process State
 * ------------------------------------------------------------
 */

typedef enum
{
    PROCESS_CREATED = 0,

    PROCESS_READY,

    PROCESS_RUNNING,

    PROCESS_WAITING,

    PROCESS_TERMINATED

} process_state_t;

/*
 * ------------------------------------------------------------
 * Process Control Block
 * ------------------------------------------------------------
 */

typedef struct process
{
    /*
     * Identification
     */

    process_id_t pid;

    char name[PROCESS_NAME_LENGTH];

    /*
     * State
     */

    process_state_t state;

    /*
     * Process Type
     */

    bool kernel_process;

    /*
     * Main Thread
     */

    thread_t *main_thread;

    /*
     * Address Space
     */

    void *address_space;

    /*
     * Statistics
     */

    uint64_t cpu_time;

    /*
     * Process Flags
     */

    uint32_t flags;

} process_t;

/*
 * ------------------------------------------------------------
 * Process Manager API
 * ------------------------------------------------------------
 */

/*
 * Initialize process manager.
 */
void process_initialize(void);

/*
 * Create a process.
 */
process_t *process_create(
    const char *name,
    bool kernel_process
);

/*
 * Destroy a process.
 */
void process_destroy(
    process_t *process
);

/*
 * Get current process.
 */
process_t *process_current(void);

/*
 * Set current process.
 */
void process_set_current(
    process_t *process
);

#endif