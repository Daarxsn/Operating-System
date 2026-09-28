#ifndef XYRIS_FRAMEBUFFER_H
#define XYRIS_FRAMEBUFFER_H

#include <stdint.h>

#include "../boot/limine.h"

/*
 * Active framebuffer initialization
 */
void framebuffer_init(struct limine_framebuffer *fb);

/*
 * Active framebuffer geometry
 */
uint32_t framebuffer_width(void);
uint32_t framebuffer_height(void);
uint32_t framebuffer_pitch(void);

/*
 * Active framebuffer pixel format
 */
uint16_t framebuffer_bpp(void);
uint8_t framebuffer_memory_model(void);

uint8_t framebuffer_red_mask_size(void);
uint8_t framebuffer_red_mask_shift(void);

uint8_t framebuffer_green_mask_size(void);
uint8_t framebuffer_green_mask_shift(void);

uint8_t framebuffer_blue_mask_size(void);
uint8_t framebuffer_blue_mask_shift(void);

/*
 * Available video modes for the active framebuffer.
 */
uint64_t framebuffer_mode_count(void);

int framebuffer_mode_info(
    uint64_t index,
    uint32_t *width,
    uint32_t *height,
    uint16_t *bpp,
    uint64_t *pitch
);

/*
 * Limine framebuffer/display enumeration.
 *
 * These provide the foundation for multiple-display support.
 */
uint64_t framebuffer_count(void);

struct limine_framebuffer *framebuffer_get(uint64_t index);

int framebuffer_info(
    uint64_t index,
    uint32_t *width,
    uint32_t *height,
    uint32_t *pitch,
    uint16_t *bpp
);

/*
 * Active framebuffer memory access.
 */
volatile uint32_t *framebuffer_address(void);

/*
 * Basic drawing operations.
 */
void framebuffer_put_pixel(
    uint32_t x,
    uint32_t y,
    uint32_t color
);

void framebuffer_fill_rect(
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    uint32_t color
);

void framebuffer_clear(uint32_t color);

#endif