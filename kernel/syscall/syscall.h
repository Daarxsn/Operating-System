#ifndef SYSCALL_H
#define SYSCALL_H

#include "../../abi/include/xyris/abi/xyris_abi.h"

typedef xyris_syscall_result_t (*syscall_handler_t)(
    xyris_syscall_arg_t,
    xyris_syscall_arg_t,
    xyris_syscall_arg_t,
    xyris_syscall_arg_t
);

void syscall_init(void);

xyris_syscall_result_t syscall_dispatch(
    xyris_syscall_number_t number,
    xyris_syscall_arg_t arg1,
    xyris_syscall_arg_t arg2,
    xyris_syscall_arg_t arg3,
    xyris_syscall_arg_t arg4
);

#endif