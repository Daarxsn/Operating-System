#ifndef XYRISOS_HHDM_H
#define XYRISOS_HHDM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

bool hhdm_map_mmio(uintptr_t physical, size_t length);

void hhdm_init(void);

uintptr_t hhdm_offset(void);

void *phys_to_virt(uintptr_t physical);

uintptr_t virt_to_phys(void *virtual_address);

#endif