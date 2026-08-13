#ifndef XYRIS_CPU_H
#define XYRIS_CPU_H

#include <stdint.h>

void cpu_halt(void);

_Noreturn void cpu_halt_forever(void);

/*
 * Read CR2.
 *
 * CR2 contains the virtual address that
 * caused the most recent page fault.
 */
uintptr_t cpu_read_cr2(void);

#endif