#include <stdint.h>

#include "font.h"
#include "graphics.h"
#include "font8x8_basic.h"

/*
 * -------------------------------------------------------
 * Draw One Character
 * -------------------------------------------------------
 */

void font_draw_char(
    int x,
    int y,
    char c,
    uint32_t color)
{
    const unsigned char *glyph =
    (const unsigned char *)font8x8_basic[(unsigned char)c];

    for (int row = 0; row < FONT_HEIGHT; row++)
{
    uint8_t bits = glyph[row];

    for (int col = 0; col < FONT_WIDTH; col++)
    {
        if (bits & (1 << col))
        {
            draw_pixel(
                x + col,
                y + row,
                color
            );
        }
    }
}
}

/*
 * -------------------------------------------------------
 * Draw String
 * -------------------------------------------------------
 */

void font_draw_string(
    int x,
    int y,
    const char *text,
    uint32_t color)
{
    if (text == 0)
        return;

    int start_x = x;

    while (*text)
    {
        if (*text == '\n')
        {
            x = start_x;
            y += FONT_HEIGHT;
            text++;
            continue;
        }

        font_draw_char(
            x,
            y,
            *text,
            color
        );

        x += FONT_WIDTH;
        text++;
    }
}