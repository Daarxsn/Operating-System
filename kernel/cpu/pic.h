#ifndef XYRIS_PIC_H
#define XYRIS_PIC_H

#include <stdint.h>

void pic_initialize(void);

void pic_send_eoi(uint8_t irq);

void pic_mask_irq(uint8_t irq);

void pic_unmask_irq(uint8_t irq);

uint8_t pic_debug_get_master_mask(void);
uint8_t pic_debug_get_master_irr(void);

#endif