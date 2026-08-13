#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

enum
{
    SYS_READ = 0,
    SYS_WRITE,
    SYS_OPEN,
    SYS_CLOSE,
    SYS_EXIT,

    SYS_MAX
};

typedef uint64_t (*syscall_handler_t)(
    uint64_t,
    uint64_t,
    uint64_t,
    uint64_t
);

void syscall_init(void);

uint64_t syscall_dispatch(
    uint64_t number,
    uint64_t arg1,
    uint64_t arg2,
    uint64_t arg3,
    uint64_t arg4
);

#endif