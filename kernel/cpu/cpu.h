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

/*
 * CPU hardware information.
 *
 * The boot layer supplies the processor count
 * discovered through the Limine MP response.
 */
typedef struct
{
    uint32_t processor_count;
} cpu_info_t;

/*
 * Set CPU information discovered during boot.
 */
void cpu_set_processor_count(uint32_t count);

/*
 * Get CPU hardware information.
 */
cpu_info_t cpu_info(void);

#endif