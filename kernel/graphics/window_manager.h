#ifndef XYRIS_WINDOW_MANAGER_H
#define XYRIS_WINDOW_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#include "surface.h"

#define XYRIS_WINDOW_MANAGER_MAX_WINDOWS 32

typedef struct
{
    uint32_t id;

    XyrisSurface *surface;

    int32_t x;
    int32_t y;

    int32_t z;

    bool visible;
    bool focused;
    bool movable;
} XyrisWindow;

/* Window manager lifecycle */

void window_manager_init(void);

/* Window lifecycle */

XyrisWindow *window_create(
    uint32_t width,
    uint32_t height,
    int32_t x,
    int32_t y
);

void window_destroy(
    XyrisWindow *window
);

/* Window state */

void window_show(
    XyrisWindow *window
);

void window_hide(
    XyrisWindow *window
);

void window_move(
    XyrisWindow *window,
    int32_t x,
    int32_t y
);

void window_focus(
    XyrisWindow *window
);

void window_unfocus(
    XyrisWindow *window
);

/* Queries */

XyrisWindow *window_at(
    int32_t x,
    int32_t y
);

XyrisWindow *window_focused(void);

uint32_t window_id(
    const XyrisWindow *window
);

#endif
