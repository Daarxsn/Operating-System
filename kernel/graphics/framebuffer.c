#include "framebuffer.h"
#include <stddef.h>

static struct limine_framebuffer *g_framebuffer = NULL;

/*
 * Initialize the framebuffer provided by Limine.
 */
void framebuffer_init(struct limine_framebuffer *fb)
{
    g_framebuffer = fb;
}

/*
 * Return framebuffer width in pixels.
 */
uint32_t framebuffer_width(void)
{
    return g_framebuffer ? g_framebuffer->width : 0;
}

/*
 * Return framebuffer height in pixels.
 */
uint32_t framebuffer_height(void)
{
    return g_framebuffer ? g_framebuffer->height : 0;
}

/*
 * Return framebuffer pitch in bytes.
 */
uint32_t framebuffer_pitch(void)
{
    return g_framebuffer ? g_framebuffer->pitch : 0;
}

/*
 * Return framebuffer base address.
 */
volatile uint32_t *framebuffer_address(void)
{
    if (!g_framebuffer)
        return NULL;

    return (volatile uint32_t *)g_framebuffer->address;
}

/*
 * Draw a single pixel.
 */
void framebuffer_put_pixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (!g_framebuffer)
        return;

    if (x >= g_framebuffer->width || y >= g_framebuffer->height)
        return;

    volatile uint32_t *fb =
        (volatile uint32_t *)g_framebuffer->address;

    uint32_t pixels_per_row = g_framebuffer->pitch / sizeof(uint32_t);

    fb[y * pixels_per_row + x] = color;
}

/*
 * Fill a rectangle with a solid color.
 */
void framebuffer_fill_rect(uint32_t x,
                           uint32_t y,
                           uint32_t width,
                           uint32_t height,
                           uint32_t color)
{
    if (!g_framebuffer)
        return;

    for (uint32_t yy = 0; yy < height; yy++)
    {
        for (uint32_t xx = 0; xx < width; xx++)
        {
            framebuffer_put_pixel(
                x + xx,
                y + yy,
                color
            );
        }
    }
}

/*
 * Clear the entire screen with a solid color.
 */
void framebuffer_clear(uint32_t color)
{
    framebuffer_fill_rect(
        0,
        0,
        framebuffer_width(),
        framebuffer_height(),
        color
    );
}