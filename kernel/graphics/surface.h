#ifndef XYRIS_SURFACE_H
#define XYRIS_SURFACE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint32_t width;
    uint32_t height;
    uint32_t pitch;

    uint32_t *pixels;

    bool owns_pixels;
} XyrisSurface;

/* Lifecycle */

XyrisSurface *surface_create(
    uint32_t width,
    uint32_t height
);

void surface_destroy(
    XyrisSurface *surface
);

/* Drawing */

void surface_clear(
    XyrisSurface *surface,
    uint32_t color
);

void surface_put_pixel(
    XyrisSurface *surface,
    int x,
    int y,
    uint32_t color
);

void surface_fill_rect(
    XyrisSurface *surface,
    int x,
    int y,
    int width,
    int height,
    uint32_t color
);

/* Information */

uint32_t surface_width(
    const XyrisSurface *surface
);

uint32_t surface_height(
    const XyrisSurface *surface
);

#endif