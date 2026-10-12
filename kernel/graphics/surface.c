#include "surface.h"

#include <stddef.h>

#include "../memory/heap.h"
#include "../debug/print.h"

XyrisSurface *surface_create(
    uint32_t width,
    uint32_t height)
{
    if (width == 0 || height == 0)
        return NULL;

    XyrisSurface *surface =
        (XyrisSurface *)kmalloc(sizeof(XyrisSurface));

    if (surface == NULL)
    {
        
        return NULL;
    }


    uint64_t pixel_count =
        (uint64_t)width * (uint64_t)height;

    uint64_t byte_size =
        pixel_count * sizeof(uint32_t);

    surface->pixels =
        (uint32_t *)kmalloc((size_t)byte_size);

    if (surface->pixels == NULL)
    {
        
        kfree(surface);
        return NULL;
    }



    surface->width = width;
    surface->height = height;
    surface->pitch = width * sizeof(uint32_t);
    surface->owns_pixels = true;

    surface_clear(surface, 0x00000000);

    return surface;
}
void surface_destroy(
    XyrisSurface *surface)
{
    if (surface == NULL)
        return;

    if (surface->owns_pixels &&
        surface->pixels != NULL)
    {
        kfree(surface->pixels);
    }

    kfree(surface);
}

void surface_clear(
    XyrisSurface *surface,
    uint32_t color)
{
    if (surface == NULL ||
        surface->pixels == NULL)
        return;

    uint64_t count =
        (uint64_t)surface->width *
        (uint64_t)surface->height;

    for (uint64_t i = 0; i < count; i++)
    {
        surface->pixels[i] = color;
    }
}

void surface_put_pixel(
    XyrisSurface *surface,
    int x,
    int y,
    uint32_t color)
{
    if (surface == NULL ||
        surface->pixels == NULL)
        return;

    if (x < 0 || y < 0)
        return;

    if ((uint32_t)x >= surface->width ||
        (uint32_t)y >= surface->height)
        return;

    surface->pixels[
        ((uint32_t)y * surface->width) +
        (uint32_t)x
    ] = color;
}

void surface_fill_rect(
    XyrisSurface *surface,
    int x,
    int y,
    int width,
    int height,
    uint32_t color)
{
    if (surface == NULL ||
        surface->pixels == NULL)
        return;

    if (width <= 0 || height <= 0)
        return;

    for (int yy = y; yy < y + height; yy++)
    {
        for (int xx = x; xx < x + width; xx++)
        {
            surface_put_pixel(
                surface,
                xx,
                yy,
                color
            );
        }
    }
}

uint32_t surface_width(
    const XyrisSurface *surface)
{
    return surface ? surface->width : 0;
}

uint32_t surface_height(
    const XyrisSurface *surface)
{
    return surface ? surface->height : 0;
}