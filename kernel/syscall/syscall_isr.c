#include "syscall.h"

#include "../cpu/isr.h"
#include "../debug/print.h"
#include "../debug/hex.h"

#include <stdint.h>

void syscall_interrupt_handler(registers_t *regs)
{
    debug_print("RING3 TEST: SYSCALL HANDLER ENTER\n");
    debug_print("RING3 TEST: SYSCALL NUMBER = ");
    debug_print_hex64((uint64_t)regs->rax);
    debug_print("\n");

    xyris_syscall_result_t result =
        syscall_dispatch(
            (xyris_syscall_number_t)regs->rax,
            (xyris_syscall_arg_t)regs->rdi,
            (xyris_syscall_arg_t)regs->rsi,
            (xyris_syscall_arg_t)regs->rdx,
            (xyris_syscall_arg_t)regs->rcx
        );

    regs->rax = (uint64_t)result;
}