#ifndef XYRIS_PIT_H
#define XYRIS_PIT_H

#include <stdint.h>

/* PIT runs from a fixed input clock */
#define PIT_BASE_FREQUENCY 1193182

/* Initialize the PIT to the requested frequency (Hz) */
void pit_initialize(uint32_t frequency);

/* Called from IRQ0 */
void pit_handler(void);

/* Tick counter */
uint64_t pit_get_ticks(void);

/* Millisecond delay (busy wait) */
void pit_sleep(uint64_t milliseconds);

#endif