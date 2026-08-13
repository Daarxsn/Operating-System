#ifndef XYRIS_FRAMEBUFFER_H
#define XYRIS_FRAMEBUFFER_H

#include <stdint.h>
#include "../boot/limine.h"

void framebuffer_init(struct limine_framebuffer *fb);

uint32_t framebuffer_width(void);
uint32_t framebuffer_height(void);
uint32_t framebuffer_pitch(void);

volatile uint32_t *framebuffer_address(void);

/* Graphics primitives */

void framebuffer_put_pixel(uint32_t x,
                           uint32_t y,
                           uint32_t color);

void framebuffer_fill_rect(uint32_t x,
                           uint32_t y,
                           uint32_t width,
                           uint32_t height,
                           uint32_t color);

void framebuffer_clear(uint32_t color);

#endif