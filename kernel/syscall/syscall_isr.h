#ifndef XYRIS_SYSCALL_ISR_H
#define XYRIS_SYSCALL_ISR_H

#include "../cpu/isr.h"

void syscall_interrupt_handler(
    registers_t* regs
);

#endif