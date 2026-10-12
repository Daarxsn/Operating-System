#ifndef XYRIS_RENDERER_H
#define XYRIS_RENDERER_H

#include <stdint.h>

/*
 * XyrisOS Renderer
 *
 * Hardware-independent drawing interface for the GUI subsystem.
 *
 * The renderer sits between the compositor / UI system
 * and the underlying framebuffer / graphics implementation.
 */

/* -------------------------------------------------
   Renderer lifecycle
------------------------------------------------- */

void renderer_init(void);

void renderer_begin_frame(void);

void renderer_end_frame(void);

/* -------------------------------------------------
   Basic drawing
------------------------------------------------- */

void renderer_clear(uint32_t color);

void renderer_draw_pixel(
    int x,
    int y,
    uint32_t color
);

void renderer_draw_rect(
    int x,
    int y,
    int width,
    int height,
    uint32_t color
);

void renderer_fill_rect(
    int x,
    int y,
    int width,
    int height,
    uint32_t color
);

void renderer_draw_line(
    int x0,
    int y0,
    int x1,
    int y1,
    uint32_t color
);

void renderer_draw_circle(
    int center_x,
    int center_y,
    int radius,
    uint32_t color
);

void renderer_fill_circle(
    int center_x,
    int center_y,
    int radius,
    uint32_t color
);

/* -------------------------------------------------
   Display information
------------------------------------------------- */

uint32_t renderer_width(void);

uint32_t renderer_height(void);

#endif