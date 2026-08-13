#ifndef XYRIS_PANIC_H
#define XYRIS_PANIC_H

#include "../cpu/isr.h"

void kernel_panic(registers_t *regs, const char *reason);

#endif