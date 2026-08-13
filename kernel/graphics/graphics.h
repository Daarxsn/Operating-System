#ifndef XYRIS_GRAPHICS_H
#define XYRIS_GRAPHICS_H

#include <stdint.h>

/* -------------------------------------------------
   Screen
------------------------------------------------- */

void clear_screen(uint32_t color);

/* -------------------------------------------------
   Pixel
------------------------------------------------- */

void draw_pixel(
    uint32_t x,
    uint32_t y,
    uint32_t color
);

/* -------------------------------------------------
   Line
------------------------------------------------- */

void draw_line(
    int x0,
    int y0,
    int x1,
    int y1,
    uint32_t color
);

/* -------------------------------------------------
   Rectangle
------------------------------------------------- */

void draw_rect(
    int x,
    int y,
    int width,
    int height,
    uint32_t color
);

void fill_rect(
    int x,
    int y,
    int width,
    int height,
    uint32_t color
);

void draw_circle(
    int xc,
    int yc,
    int radius,
    uint32_t color
);

void fill_circle(
    int xc,
    int yc,
    int radius,
    uint32_t color
);

#endif