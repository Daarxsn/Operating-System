#ifndef XYRIS_PROCESS_H
#define XYRIS_PROCESS_H

#include <stdint.h>
#include <stdbool.h>

#define PROCESS_MAX_COUNT      128
#define PROCESS_NAME_LENGTH    64

struct thread;
typedef struct thread thread_t;
typedef uint32_t process_id_t;

typedef enum
{
    PROCESS_CREATED = 0,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_TERMINATED
} process_state_t;

typedef struct process
{
    process_id_t pid;
    process_id_t parent_pid;
    char name[PROCESS_NAME_LENGTH];
    process_state_t state;
    bool kernel_process;
    thread_t *main_thread;
    void *address_space;
    uint64_t cpu_time;
    uint64_t live_threads;
    int32_t exit_code;
    bool exit_requested;
    uint32_t flags;
} process_t;

void process_initialize(void);
process_t *process_create(const char *name, bool kernel_process);
void process_destroy(process_t *process);
process_t *process_current(void);
void process_set_current(process_t *process);

/* Thread/process lifecycle integration. */
void process_thread_exited(process_t *process);
void process_exit_current(int32_t exit_code);

/* Parent/child wait and zombie reaping. */
int process_wait(process_id_t pid, int32_t *exit_code);
int process_reap(process_id_t pid, int32_t *exit_code);

/* Address-space ownership is supplied by the memory subsystem. */
void process_set_address_space(process_t *process, void *address_space);

#endif
