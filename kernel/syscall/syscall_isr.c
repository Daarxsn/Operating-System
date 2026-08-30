#include "syscall.h"

#include "../cpu/isr.h"
#include "../debug/print.h"

#include <stdint.h>

void syscall_interrupt_handler(
    registers_t* regs
)
{
    debug_print("RING3 TEST: SYSCALL HANDLER ENTER\n");

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