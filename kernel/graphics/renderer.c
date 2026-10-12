#include "renderer.h"

#include "graphics.h"
#include "framebuffer.h"

/*
 * XyrisOS Renderer
 *
 * The renderer provides the stable drawing API used by
 * the future compositor and GUI layers.
 *
 * For the first implementation it delegates to the
 * existing framebuffer/graphics subsystem.
 *
 * Later this layer can evolve without forcing the
 * compositor or desktop shell to know how pixels reach
 * the display.
 */

static int renderer_initialized = 0;

/* -------------------------------------------------
   Renderer lifecycle
------------------------------------------------- */

void renderer_init(void)
{
    renderer_initialized = 1;
}

void renderer_begin_frame(void)
{
    /*
     * Reserved for future frame preparation,
     * double buffering, damage tracking, etc.
     */
}

void renderer_end_frame(void)
{
    /*
     * Reserved for future buffer presentation,
     * synchronization and compositing.
     */
}

/* -------------------------------------------------
   Basic drawing
------------------------------------------------- */

void renderer_clear(uint32_t color)
{
    if (!renderer_initialized)
        return;

    clear_screen(color);
}

void renderer_draw_pixel(
    int x,
    int y,
    uint32_t color)
{
    if (!renderer_initialized)
        return;

    if (x < 0 || y < 0)
        return;

    draw_pixel(
        (uint32_t)x,
        (uint32_t)y,
        color
    );
}

void renderer_draw_rect(
    int x,
    int y,
    int width,
    int height,
    uint32_t color)
{
    if (!renderer_initialized)
        return;

    if (width <= 0 || height <= 0)
        return;

    draw_rect(
        x,
        y,
        width,
        height,
        color
    );
}

void renderer_fill_rect(
    int x,
    int y,
    int width,
    int height,
    uint32_t color)
{
    if (!renderer_initialized)
        return;

    if (width <= 0 || height <= 0)
        return;

    fill_rect(
        x,
        y,
        width,
        height,
        color
    );
}

void renderer_draw_line(
    int x0,
    int y0,
    int x1,
    int y1,
    uint32_t color)
{
    if (!renderer_initialized)
        return;

    draw_line(
        x0,
        y0,
        x1,
        y1,
        color
    );
}

void renderer_draw_circle(
    int center_x,
    int center_y,
    int radius,
    uint32_t color)
{
    if (!renderer_initialized)
        return;

    if (radius <= 0)
        return;

    draw_circle(
        center_x,
        center_y,
        radius,
        color
    );
}

void renderer_fill_circle(
    int center_x,
    int center_y,
    int radius,
    uint32_t color)
{
    if (!renderer_initialized)
        return;

    if (radius <= 0)
        return;

    fill_circle(
        center_x,
        center_y,
        radius,
        color
    );
}

/* -------------------------------------------------
   Display information
------------------------------------------------- */

uint32_t renderer_width(void)
{
    return framebuffer_width();
}

uint32_t renderer_height(void)
{
    return framebuffer_height();
}