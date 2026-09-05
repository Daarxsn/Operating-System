#ifndef SYSCALL_H
#define SYSCALL_H

#include "../../abi/include/xyris/abi/xyris_abi.h"
#include <stdbool.h>

struct process;

typedef xyris_syscall_result_t (*syscall_handler_t)(
    xyris_syscall_arg_t,
    xyris_syscall_arg_t,
    xyris_syscall_arg_t,
    xyris_syscall_arg_t
);

void syscall_init(void);

/* Called when a process is destroyed so SDK-owned resources are reclaimed. */
void syscall_process_cleanup(struct process *process);

/* Shared user-range validation for syscall service implementations. */
xyris_status_t syscall_validate_user_range(
    xyris_user_ptr_t address,
    xyris_size_t size,
    bool write_access);

xyris_syscall_result_t syscall_dispatch(
    xyris_syscall_number_t number,
    xyris_syscall_arg_t arg1,
    xyris_syscall_arg_t arg2,
    xyris_syscall_arg_t arg3,
    xyris_syscall_arg_t arg4
);

#endif
