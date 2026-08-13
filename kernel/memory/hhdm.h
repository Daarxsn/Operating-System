#ifndef XYRIS_HHDM_H
#define XYRIS_HHDM_H

#include <stdint.h>

/*
 * Higher Half Direct Map
 *
 * Converts physical addresses into
 * kernel virtual addresses.
 */

void hhdm_init(void);

/*
 * HHDM offset supplied by Limine.
 */
uintptr_t hhdm_offset(void);

/*
 * Physical -> Virtual
 */
void* phys_to_virt(uintptr_t physical);

/*
 * Virtual -> Physical
 */
uintptr_t virt_to_phys(void* virtual_address);

#endif