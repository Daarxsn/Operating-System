#ifndef SYSCALL_H
#define SYSCALL_H

#include "../../abi/include/xyris/abi/xyris_abi.h"

/*
 * Legacy kernel-test aliases.
 *
 * Existing kernel tests historically referenced SYS_* names. Keep these
 * aliases inside the kernel-facing header so internal tests remain source
 * compatible while the public ABI uses the XYRIS_SYS_* namespace.
 */
#define SYS_READ  XYRIS_SYS_READ
#define SYS_WRITE XYRIS_SYS_WRITE
#define SYS_OPEN  XYRIS_SYS_OPEN
#define SYS_CLOSE XYRIS_SYS_CLOSE
#define SYS_EXIT  XYRIS_SYS_EXIT

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
