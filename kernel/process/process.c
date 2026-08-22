#include "process.h"
#include "thread.h"
#include "scheduler.h"
#include "user.h"

#include "../loader/elf.h"
#include "../memory/vmm.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static process_t process_table[PROCESS_MAX_COUNT];
static process_t *current_process = NULL;
static process_id_t next_pid = 1;

static process_t *process_find(process_id_t pid)
{
    if (pid == 0) return NULL;
    for (uint32_t i = 0; i < PROCESS_MAX_COUNT; ++i)
        if (process_table[i].pid == pid) return &process_table[i];
    return NULL;
}

static bool process_is_child_of(const process_t *child, const process_t *parent)
{
    return child != NULL && parent != NULL && child->parent_pid == parent->pid;
}

void process_initialize(void)
{
    memset(process_table, 0, sizeof(process_table));
    current_process = NULL;
    next_pid = 1;
}

process_t *process_create(const char *name, bool kernel_process)
{
    if (name == NULL) return NULL;

    for (uint32_t i = 0; i < PROCESS_MAX_COUNT; ++i)
    {
        if (process_table[i].pid != 0) continue;

        process_t *process = &process_table[i];
        memset(process, 0, sizeof(*process));
        process->pid = next_pid++;
        if (process->pid == 0) process->pid = next_pid++;

        strncpy(process->name, name, PROCESS_NAME_LENGTH - 1);
        process->name[PROCESS_NAME_LENGTH - 1] = '\0';
        process->parent_pid = current_process != NULL ? current_process->pid : 0;
        process->state = PROCESS_CREATED;
        process->kernel_process = kernel_process;
        process->main_thread = NULL;
        process->address_space = NULL;
        process->cpu_time = 0;
        process->live_threads = 0;
        process->exit_code = 0;
        process->exit_requested = false;
        process->flags = 0;
        return process;
    }

    return NULL;
}

process_t *process_create_user(
    const char *name,
    const void *image,
    size_t image_size)
{
    if (name == NULL || image == NULL || image_size == 0)
        return NULL;

    process_t *process = process_create(name, false);
    if (process == NULL)
        return NULL;

    address_space_t *space = vmm_create_space();
    if (space == NULL)
    {
        process_destroy(process);
        return NULL;
    }

    uint64_t entry = 0;

    if (elf_load_into_space(
            image,
            image_size,
            space,
            &entry) != 0)
    {
        vmm_destroy_space(space);
        process_destroy(process);
        return NULL;
    }

    user_process_t user = {0};

    if (user_prepare_in_space(
            &user,
            space,
            entry) != 0)
    {
        vmm_destroy_space(space);
        process_destroy(process);
        return NULL;
    }

    process->address_space = space;

    thread_t *thread = thread_create_user(
        process,
        user.entry,
        user.stack,
        THREAD_PRIORITY_NORMAL
    );

    if (thread == NULL)
    {
        /*
         * user_prepare_in_space() does not own
         * the address space.
         */
        user.owns_address_space = true;

        user_destroy(&user);

        process->address_space = NULL;
        process_destroy(process);
        return NULL;
    }

    process->state = PROCESS_READY;
    scheduler_add_thread(thread);

    return process;
}

void process_set_address_space(process_t *process, void *address_space)
{
    if (process != NULL)
        process->address_space = address_space;
}

void process_thread_exited(process_t *process)
{
    if (process == NULL || process->live_threads == 0)
        return;

    process->live_threads--;

    if (process->live_threads == 0)
    {
        process->state = PROCESS_TERMINATED;
        process->exit_requested = true;
    }
}

void process_exit_current(int32_t exit_code)
{
    process_t *process = current_process;
    if (process == NULL) return;

    process->exit_code = exit_code;
    process->exit_requested = true;
    scheduler_exit_current();
}

void process_destroy(process_t *process)
{
    if (process == NULL) return;

    if (current_process == process)
    {
        process->exit_requested = true;
        process->state = PROCESS_TERMINATED;
        return;
    }

    thread_destroy_process_threads(process);

    if (process->address_space != NULL)
    {
        vmm_destroy_space((address_space_t *)process->address_space);
        process->address_space = NULL;
    }

    memset(process, 0, sizeof(*process));
}

int process_reap(process_id_t pid, int32_t *exit_code)
{
    process_t *parent = current_process;
    process_t *child = process_find(pid);

    if (parent == NULL || child == NULL ||
        !process_is_child_of(child, parent) ||
        child->state != PROCESS_TERMINATED)
        return -1;

    if (exit_code != NULL)
        *exit_code = child->exit_code;

    process_destroy(child);
    return 0;
}

int process_wait(process_id_t pid, int32_t *exit_code)
{
    process_t *parent = current_process;
    if (parent == NULL) return -1;

    for (;;)
    {
        bool found_child = false;

        for (uint32_t i = 0; i < PROCESS_MAX_COUNT; ++i)
        {
            process_t *child = &process_table[i];
            if (!process_is_child_of(child, parent)) continue;
            if (pid != 0 && child->pid != pid) continue;

            found_child = true;
            if (child->state == PROCESS_TERMINATED)
                return process_reap(child->pid, exit_code);
        }

        if (!found_child) return -1;
        parent->state = PROCESS_WAITING;
        scheduler_yield();
        if (current_process == parent && parent->state == PROCESS_WAITING)
            parent->state = PROCESS_RUNNING;
    }
}

process_t *process_current(void)
{
    return current_process;
}

void process_set_current(process_t *process)
{
    current_process = process;
}
