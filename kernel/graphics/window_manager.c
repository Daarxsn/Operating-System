#include "window_manager.h"

#include <stddef.h>

#include "compositor.h" 

static XyrisWindow windows[
    XYRIS_WINDOW_MANAGER_MAX_WINDOWS
];

static uint32_t next_window_id = 1;
static int32_t next_z = 0;

static bool window_is_registered(
    const XyrisWindow *window)
{
    if (window == NULL)
        return false;

    for (uint32_t i = 0;
         i < XYRIS_WINDOW_MANAGER_MAX_WINDOWS;
         i++)
    {
        if (&windows[i] == window &&
            windows[i].surface != NULL)
        {
            return true;
        }
    }

    return false;
}

static XyrisWindow *find_free_window_slot(void)
{
    for (uint32_t i = 0;
         i < XYRIS_WINDOW_MANAGER_MAX_WINDOWS;
         i++)
    {
        if (windows[i].surface == NULL)
            return &windows[i];
    }

    return NULL;
}

void window_manager_init(void)
{
    for (uint32_t i = 0;
         i < XYRIS_WINDOW_MANAGER_MAX_WINDOWS;
         i++)
    {
        windows[i].id = 0;
        windows[i].surface = NULL;
        windows[i].x = 0;
        windows[i].y = 0;
        windows[i].z = 0;
        windows[i].visible = false;
        windows[i].focused = false;
        windows[i].movable = false;
    }

    next_window_id = 1;
    next_z = 0;
}

XyrisWindow *window_create(
    uint32_t width,
    uint32_t height,
    int32_t x,
    int32_t y)
{
    if (width == 0 || height == 0)
        return NULL;

    XyrisWindow *window =
        find_free_window_slot();

    if (window == NULL)
        return NULL;

    XyrisSurface *surface =
        surface_create(width, height);

    if (surface == NULL)
        return NULL;

    int32_t z = next_z++;

    if (compositor_add_surface(
            surface,
            x,
            y,
            z) < 0)
    {
        surface_destroy(surface);
        return NULL;
    }

    window->id = next_window_id++;
    window->surface = surface;
    window->x = x;
    window->y = y;
    window->z = z;
    window->visible = true;
    window->focused = false;
    window->movable = true;

    return window;
}

void window_destroy(
    XyrisWindow *window)
{
    if (!window_is_registered(window))
        return;

    compositor_remove_surface(
        window->surface
    );

    surface_destroy(
        window->surface
    );

    window->id = 0;
    window->surface = NULL;
    window->x = 0;
    window->y = 0;
    window->z = 0;
    window->visible = false;
    window->focused = false;
    window->movable = false;
}

void window_show(
    XyrisWindow *window)
{
    if (!window_is_registered(window))
        return;

    window->visible = true;

    compositor_set_surface_visible(
        window->surface,
        true
    );
}

void window_hide(
    XyrisWindow *window)
{
    if (!window_is_registered(window))
        return;

    window->visible = false;
    window->focused = false;

    compositor_set_surface_visible(
        window->surface,
        false
    );
}

void window_move(
    XyrisWindow *window,
    int32_t x,
    int32_t y)
{
    if (!window_is_registered(window))
        return;

    if (!window->movable)
        return;

    window->x = x;
    window->y = y;

    compositor_set_surface_position(
        window->surface,
        x,
        y
    );
}

void window_focus(
    XyrisWindow *window)
{
    if (!window_is_registered(window))
        return;

    if (!window->visible)
        return;

    for (uint32_t i = 0;
         i < XYRIS_WINDOW_MANAGER_MAX_WINDOWS;
         i++)
    {
        if (windows[i].surface != NULL)
            windows[i].focused = false;
    }

    window->focused = true;

    window->z = next_z++;

    compositor_set_surface_z(
        window->surface,
        window->z
    );
}

void window_unfocus(
    XyrisWindow *window)
{
    if (!window_is_registered(window))
        return;

    window->focused = false;
}

XyrisWindow *window_at(
    int32_t x,
    int32_t y)
{
    XyrisWindow *result = NULL;
    int32_t highest_z = INT32_MIN;

    for (uint32_t i = 0;
         i < XYRIS_WINDOW_MANAGER_MAX_WINDOWS;
         i++)
    {
        XyrisWindow *window = &windows[i];

        if (window->surface == NULL ||
            !window->visible)
        {
            continue;
        }

        int32_t width =
            (int32_t)surface_width(
                window->surface
            );

        int32_t height =
            (int32_t)surface_height(
                window->surface
            );

        if (x < window->x ||
            y < window->y ||
            x >= window->x + width ||
            y >= window->y + height)
        {
            continue;
        }

        if (window->z >= highest_z)
        {
            highest_z = window->z;
            result = window;
        }
    }

    return result;
}

XyrisWindow *window_focused(void)
{
    for (uint32_t i = 0;
         i < XYRIS_WINDOW_MANAGER_MAX_WINDOWS;
         i++)
    {
        if (windows[i].surface != NULL &&
            windows[i].focused)
        {
            return &windows[i];
        }
    }

    return NULL;
}

uint32_t window_id(
    const XyrisWindow *window)
{
    if (!window_is_registered(window))
        return 0;

    return window->id;
}
