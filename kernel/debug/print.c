#include "print.h"

#include <stddef.h>

#include "drivers/serial.h"

#include "../graphics/font.h"
#include "../graphics/framebuffer.h"

#define DEBUG_LEFT_MARGIN 20
#define DEBUG_TOP_MARGIN  20
#define DEBUG_LINE_HEIGHT 12
#define DEBUG_TEXT_COLOR  0xFFFFFF

static int cursor_x = DEBUG_LEFT_MARGIN;
static int cursor_y = DEBUG_TOP_MARGIN;

static void debug_scroll(void)
{
    uint32_t width = framebuffer_width();
    uint32_t height = framebuffer_height();
    uint32_t pitch = framebuffer_pitch();
    volatile uint32_t *fb = framebuffer_address();

    if (fb == NULL || width == 0 || height == 0 || pitch == 0)
        return;

    uint32_t rows = DEBUG_LINE_HEIGHT;

    if (rows >= height)
    {
        framebuffer_clear(0x00000000);
        cursor_y = DEBUG_TOP_MARGIN;
        return;
    }

    uint32_t pixels_per_row =
        pitch / sizeof(uint32_t);

    for (uint32_t y = 0; y < height - rows; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            fb[y * pixels_per_row + x] =
                fb[(y + rows) * pixels_per_row + x];
        }
    }

    for (uint32_t y = height - rows; y < height; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            fb[y * pixels_per_row + x] = 0x00000000;
        }
    }

    if (cursor_y >= (int)rows)
        cursor_y -= (int)rows;
    else
        cursor_y = DEBUG_TOP_MARGIN;
}

static void debug_ensure_line_visible(void)
{
    uint32_t height = framebuffer_height();

    if (height == 0)
        return;

    while (cursor_y + FONT_HEIGHT > (int)height)
        debug_scroll();
}

void debug_print_init(void)
{
    cursor_x = DEBUG_LEFT_MARGIN;
    cursor_y = DEBUG_TOP_MARGIN;
}

void debug_set_cursor(int x, int y)
{
    cursor_x = x;
    cursor_y = y;

    debug_ensure_line_visible();
}

void debug_print(const char *text)
{
    if (!text)
        return;

    /* Mirror the same diagnostic stream to COM1 once the serial
     * driver is initialized. Before initialization the serial API
     * safely ignores the write. */
    xk_serial_write_string(text);

    uint32_t width = framebuffer_width();

    while (*text)
    {
        if (*text == '\n')
        {
            cursor_x = DEBUG_LEFT_MARGIN;
            cursor_y += DEBUG_LINE_HEIGHT;

            debug_ensure_line_visible();

            text++;
            continue;
        }

        if (width != 0 &&
            cursor_x + FONT_WIDTH > (int)width)
        {
            cursor_x = DEBUG_LEFT_MARGIN;
            cursor_y += DEBUG_LINE_HEIGHT;

            debug_ensure_line_visible();
        }

        debug_ensure_line_visible();

        font_draw_char(
            cursor_x,
            cursor_y,
            *text,
            DEBUG_TEXT_COLOR
        );

        cursor_x += FONT_WIDTH;
        text++;
    }
}

void debug_print_line(const char *text)
{
    debug_print(text);

    cursor_x = DEBUG_LEFT_MARGIN;
    cursor_y += DEBUG_LINE_HEIGHT;

    debug_ensure_line_visible();
}