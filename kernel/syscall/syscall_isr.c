#include "syscall.h"

#include "../cpu/isr.h"
#include "../debug/print.h"

#include <stdint.h>

void syscall_interrupt_handler(registers_t *regs)
{
<<<<<<< HEAD
    debug_print("RING3 TEST: SYSCALL HANDLER ENTER\n");

    uint64_t result =
=======
    xyris_syscall_result_t result =
>>>>>>> 9601b4775b8e7e0e97a2cb6d4ae7396e41de1c0c
        syscall_dispatch(
            (xyris_syscall_number_t)regs->rax,
            (xyris_syscall_arg_t)regs->rdi,
            (xyris_syscall_arg_t)regs->rsi,
            (xyris_syscall_arg_t)regs->rdx,
            (xyris_syscall_arg_t)regs->rcx
        );

    regs->rax = (uint64_t)result;
}
