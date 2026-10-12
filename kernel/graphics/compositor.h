#ifndef XYRIS_COMPOSITOR_H
#define XYRIS_COMPOSITOR_H

#include <stdint.h>
#include <stdbool.h>

#include "surface.h"

/*
 * XyrisOS Compositor
 *
 * Maintains GUI surfaces and composes them according to
 * visibility, position and Z-order.
 */

#define XYRIS_COMPOSITOR_MAX_SURFACES 64

typedef struct
{
    XyrisSurface *surface;

    int32_t x;
    int32_t y;

    int32_t z;

    bool visible;
} XyrisCompositorSurface;

/* -------------------------------------------------
   Lifecycle
------------------------------------------------- */

void compositor_init(void);

/* -------------------------------------------------
   Root/background
------------------------------------------------- */

void compositor_set_background(uint32_t color);

uint32_t compositor_background(void);

/* -------------------------------------------------
   Surface management
------------------------------------------------- */

int compositor_add_surface(
    XyrisSurface *surface,
    int32_t x,
    int32_t y,
    int32_t z
);

void compositor_remove_surface(XyrisSurface *surface);

void compositor_set_surface_position(
    XyrisSurface *surface,
    int32_t x,
    int32_t y
);

void compositor_set_surface_z(
    XyrisSurface *surface,
    int32_t z
);

void compositor_set_surface_visible(
    XyrisSurface *surface,
    bool visible
);

/* -------------------------------------------------
   Composition
------------------------------------------------- */

void compositor_compose(void);

/* -------------------------------------------------
   Display information
------------------------------------------------- */

uint32_t compositor_width(void);
uint32_t compositor_height(void);

#endif
