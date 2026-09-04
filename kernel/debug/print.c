#include "print.h"

#include <stddef.h>
#include <stdbool.h>

#include "drivers/serial.h"

#include "../graphics/font.h"
#include "../graphics/framebuffer.h"

#define DEBUG_LEFT_MARGIN 20
#define DEBUG_TOP_MARGIN  20
#define DEBUG_LINE_HEIGHT 12
#define DEBUG_TEXT_COLOR  0xFFFFFF

static int cursor_x = DEBUG_LEFT_MARGIN;
static int cursor_y = DEBUG_TOP_MARGIN;

/*
 * Controls whether diagnostic text is rendered to the framebuffer.
 *
 * When false:
 *   - Serial output continues normally.
 *   - Framebuffer output is completely suppressed.
 *
 * This is used during the graphical XyrisOS boot splash.
 */
static bool framebuffer_enabled = true;


/* -------------------------------------------------
   Framebuffer Scrolling
------------------------------------------------- */

static void debug_scroll(void)
{
    uint32_t width = framebuffer_width();
    uint32_t height = framebuffer_height();
    uint32_t pitch = framebuffer_pitch();

    volatile uint32_t *fb =
        framebuffer_address();

    if (fb == NULL ||
        width == 0 ||
        height == 0 ||
        pitch == 0)
    {
        return;
    }

    uint32_t rows =
        DEBUG_LINE_HEIGHT;

    if (rows >= height)
    {
        framebuffer_clear(0x00000000);

        cursor_y =
            DEBUG_TOP_MARGIN;

        return;
    }

    uint32_t pixels_per_row =
        pitch / sizeof(uint32_t);

    /*
     * Move the framebuffer contents upward.
     */
    for (uint32_t y = 0;
         y < height - rows;
         y++)
    {
        for (uint32_t x = 0;
             x < width;
             x++)
        {
            fb[
                y * pixels_per_row + x
            ] =
                fb[
                    (y + rows) *
                    pixels_per_row + x
                ];
        }
    }

    /*
     * Clear the newly exposed bottom area.
     */
    for (uint32_t y = height - rows;
         y < height;
         y++)
    {
        for (uint32_t x = 0;
             x < width;
             x++)
        {
            fb[
                y * pixels_per_row + x
            ] = 0x00000000;
        }
    }

    /*
     * Keep the diagnostic cursor aligned with
     * the scrolled framebuffer.
     */
    if (cursor_y >= (int)rows)
    {
        cursor_y -= (int)rows;
    }
    else
    {
        cursor_y =
            DEBUG_TOP_MARGIN;
    }
}


/* -------------------------------------------------
   Line Visibility
------------------------------------------------- */

static void debug_ensure_line_visible(void)
{
    uint32_t height =
        framebuffer_height();

    if (height == 0)
        return;

    while (
        cursor_y + FONT_HEIGHT >
        (int)height
    )
    {
        debug_scroll();
    }
}


/* -------------------------------------------------
   Framebuffer Output Control
------------------------------------------------- */

void debug_set_framebuffer_enabled(
    bool enabled
)
{
    framebuffer_enabled = enabled;
}


/* -------------------------------------------------
   Debug Initialization
------------------------------------------------- */

void debug_print_init(void)
{
    cursor_x =
        DEBUG_LEFT_MARGIN;

    cursor_y =
        DEBUG_TOP_MARGIN;
}


/* -------------------------------------------------
   Cursor Control
------------------------------------------------- */

void debug_set_cursor(
    int x,
    int y
)
{
    cursor_x = x;
    cursor_y = y;

    /*
     * Only perform framebuffer visibility handling
     * when framebuffer output is enabled.
     */
    if (framebuffer_enabled)
    {
        debug_ensure_line_visible();
    }
}


/* -------------------------------------------------
   Debug Print
------------------------------------------------- */

void debug_print(
    const char *text
)
{
    if (!text)
        return;

    /*
     * Serial output is ALWAYS enabled.
     *
     * This means kernel diagnostics continue to appear
     * on COM1 even when framebuffer output is disabled.
     */
    xk_serial_write_string(text);

    /*
     * During the graphical splash, framebuffer output
     * is disabled completely.
     */
    if (!framebuffer_enabled)
        return;

    uint32_t width =
        framebuffer_width();

    while (*text)
    {
        /*
         * Explicit newline.
         */
        if (*text == '\n')
        {
            cursor_x =
                DEBUG_LEFT_MARGIN;

            cursor_y +=
                DEBUG_LINE_HEIGHT;

            debug_ensure_line_visible();

            text++;

            continue;
        }

        /*
         * Automatic line wrapping.
         */
        if (
            width != 0 &&
            cursor_x + FONT_WIDTH >
                (int)width
        )
        {
            cursor_x =
                DEBUG_LEFT_MARGIN;

            cursor_y +=
                DEBUG_LINE_HEIGHT;

            debug_ensure_line_visible();
        }

        /*
         * Make sure the current line is visible.
         */
        debug_ensure_line_visible();

        /*
         * Render the character.
         */
        font_draw_char(
            cursor_x,
            cursor_y,
            *text,
            DEBUG_TEXT_COLOR
        );

        cursor_x +=
            FONT_WIDTH;

        text++;
    }
}


/* -------------------------------------------------
   Debug Print Line
------------------------------------------------- */

void debug_print_line(
    const char *text
)
{
    if (!text)
        return;

    /*
     * debug_print() handles serial output and,
     * when enabled, framebuffer rendering.
     */
    debug_print(text);

    /*
     * Keep the framebuffer cursor synchronized only
     * when framebuffer rendering is active.
     */
    if (!framebuffer_enabled)
        return;

    cursor_x =
        DEBUG_LEFT_MARGIN;

    cursor_y +=
        DEBUG_LINE_HEIGHT;

    debug_ensure_line_visible();
}