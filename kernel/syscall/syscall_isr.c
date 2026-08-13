#include "syscall.h"

#include "../cpu/isr.h"

#include <stdint.h>

void syscall_interrupt_handler(
    registers_t* regs
)
{
    uint64_t result =
        syscall_dispatch(
            regs->rax,
            regs->rdi,
            regs->rsi,
            regs->rdx,
            regs->rcx
        );

    regs->rax = result;
}