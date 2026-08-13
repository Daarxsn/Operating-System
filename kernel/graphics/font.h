#ifndef XYRIS_FONT_H
#define XYRIS_FONT_H

#include <stdint.h>

#define FONT_WIDTH  8
#define FONT_HEIGHT 8

/*
 * Draw a single character.
 */
void font_draw_char(
    int x,
    int y,
    char c,
    uint32_t color
);

/*
 * Draw a string.
 * Supports '\n' for new lines.
 */
void font_draw_string(
    int x,
    int y,
    const char *text,
    uint32_t color
);

#endif