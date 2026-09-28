#include "framebuffer.h"

#include <stddef.h>

static struct limine_framebuffer *g_framebuffer = NULL;

void framebuffer_init(struct limine_framebuffer *fb)
{
    g_framebuffer = fb;
}

/*
 * --------------------------------------------------------------------------
 * Active framebuffer geometry
 * --------------------------------------------------------------------------
 */

uint32_t framebuffer_width(void)
{
    return g_framebuffer
        ? (uint32_t)g_framebuffer->width
        : 0;
}

uint32_t framebuffer_height(void)
{
    return g_framebuffer
        ? (uint32_t)g_framebuffer->height
        : 0;
}

uint32_t framebuffer_pitch(void)
{
    return g_framebuffer
        ? (uint32_t)g_framebuffer->pitch
        : 0;
}

/*
 * --------------------------------------------------------------------------
 * Active framebuffer pixel format
 * --------------------------------------------------------------------------
 */

uint16_t framebuffer_bpp(void)
{
    return g_framebuffer
        ? g_framebuffer->bpp
        : 0;
}

uint8_t framebuffer_memory_model(void)
{
    return g_framebuffer
        ? g_framebuffer->memory_model
        : 0;
}

uint8_t framebuffer_red_mask_size(void)
{
    return g_framebuffer
        ? g_framebuffer->red_mask_size
        : 0;
}

uint8_t framebuffer_red_mask_shift(void)
{
    return g_framebuffer
        ? g_framebuffer->red_mask_shift
        : 0;
}

uint8_t framebuffer_green_mask_size(void)
{
    return g_framebuffer
        ? g_framebuffer->green_mask_size
        : 0;
}

uint8_t framebuffer_green_mask_shift(void)
{
    return g_framebuffer
        ? g_framebuffer->green_mask_shift
        : 0;
}

uint8_t framebuffer_blue_mask_size(void)
{
    return g_framebuffer
        ? g_framebuffer->blue_mask_size
        : 0;
}

uint8_t framebuffer_blue_mask_shift(void)
{
    return g_framebuffer
        ? g_framebuffer->blue_mask_shift
        : 0;
}

/*
 * --------------------------------------------------------------------------
 * Video mode enumeration
 * --------------------------------------------------------------------------
 */

uint64_t framebuffer_mode_count(void)
{
    if (!g_framebuffer)
    {
        return 0;
    }

    return g_framebuffer->mode_count;
}

int framebuffer_mode_info(
    uint64_t index,
    uint32_t *width,
    uint32_t *height,
    uint16_t *bpp,
    uint64_t *pitch)
{
    if (!g_framebuffer)
    {
        return -1;
    }

    if (!g_framebuffer->modes)
    {
        return -1;
    }

    if (index >= g_framebuffer->mode_count)
    {
        return -1;
    }

    struct limine_video_mode *mode =
        g_framebuffer->modes[index];

    if (!mode)
    {
        return -1;
    }

    if (width)
    {
        *width = (uint32_t)mode->width;
    }

    if (height)
    {
        *height = (uint32_t)mode->height;
    }

    if (bpp)
    {
        *bpp = mode->bpp;
    }

    if (pitch)
    {
        *pitch = mode->pitch;
    }

    return 0;
}

/*
 * --------------------------------------------------------------------------
 * Multiple framebuffer / display foundation
 * --------------------------------------------------------------------------
 *
 * Limine exposes all available framebuffers through the framebuffer
 * response. The kernel currently selects the first framebuffer as its
 * active display, but these functions expose the complete set so a
 * future display manager can support multiple displays.
 * --------------------------------------------------------------------------
 */

uint64_t framebuffer_count(void)
{
    if (!g_framebuffer)
    {
        return 0;
    }

    /*
     * The active framebuffer does not directly contain the total
     * framebuffer count. This function therefore reports one active
     * framebuffer when initialized.
     *
     * Full multi-display enumeration belongs at the Limine response
     * layer and should not be fabricated here.
     */
    return 1;
}

struct limine_framebuffer *framebuffer_get(uint64_t index)
{
    if (!g_framebuffer)
    {
        return NULL;
    }

    if (index != 0)
    {
        return NULL;
    }

    return g_framebuffer;
}

int framebuffer_info(
    uint64_t index,
    uint32_t *width,
    uint32_t *height,
    uint32_t *pitch,
    uint16_t *bpp)
{
    struct limine_framebuffer *fb =
        framebuffer_get(index);

    if (!fb)
    {
        return -1;
    }

    if (width)
    {
        *width = (uint32_t)fb->width;
    }

    if (height)
    {
        *height = (uint32_t)fb->height;
    }

    if (pitch)
    {
        *pitch = (uint32_t)fb->pitch;
    }

    if (bpp)
    {
        *bpp = fb->bpp;
    }

    return 0;
}

/*
 * --------------------------------------------------------------------------
 * Active framebuffer memory
 * --------------------------------------------------------------------------
 */

volatile uint32_t *framebuffer_address(void)
{
    if (!g_framebuffer)
    {
        return NULL;
    }

    return (volatile uint32_t *)g_framebuffer->address;
}

/*
 * --------------------------------------------------------------------------
 * Drawing operations
 * --------------------------------------------------------------------------
 */

void framebuffer_put_pixel(
    uint32_t x,
    uint32_t y,
    uint32_t color)
{
    if (!g_framebuffer)
    {
        return;
    }

    if (x >= framebuffer_width() ||
        y >= framebuffer_height())
    {
        return;
    }

    volatile uint32_t *fb =
        (volatile uint32_t *)g_framebuffer->address;

    uint32_t pixels_per_row =
        framebuffer_pitch() / sizeof(uint32_t);

    fb[y * pixels_per_row + x] = color;
}

void framebuffer_fill_rect(
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    uint32_t color)
{
    if (!g_framebuffer)
    {
        return;
    }

    uint32_t framebuffer_w = framebuffer_width();
    uint32_t framebuffer_h = framebuffer_height();

    /*
     * Completely outside the framebuffer.
     */
    if (x >= framebuffer_w ||
        y >= framebuffer_h)
    {
        return;
    }

    /*
     * Clip the rectangle to the framebuffer boundaries.
     */
    if (width > framebuffer_w - x)
    {
        width = framebuffer_w - x;
    }

    if (height > framebuffer_h - y)
    {
        height = framebuffer_h - y;
    }

    for (uint32_t py = y;
         py < y + height;
         ++py)
    {
        for (uint32_t px = x;
             px < x + width;
             ++px)
        {
            framebuffer_put_pixel(
                px,
                py,
                color
            );
        }
    }
}

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