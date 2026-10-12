#include "compositor.h"

#include "renderer.h"
#include "framebuffer.h"

#include <stddef.h>

static XyrisCompositorSurface
    g_surfaces[XYRIS_COMPOSITOR_MAX_SURFACES];

static uint32_t g_surface_count = 0;

static uint32_t g_background_color = 0x00000000;

static bool g_initialized = false;

/* -------------------------------------------------
   Helpers
------------------------------------------------- */

static void clear_surface_entry(
    XyrisCompositorSurface *entry)
{
    if (entry == NULL)
        return;

    entry->surface = NULL;
    entry->x = 0;
    entry->y = 0;
    entry->z = 0;
    entry->visible = false;
}

static int find_surface(
    XyrisSurface *surface)
{
    if (surface == NULL)
        return -1;

    for (uint32_t i = 0;
         i < g_surface_count;
         i++)
    {
        if (g_surfaces[i].surface == surface)
            return (int)i;
    }

    return -1;
}

/*
 * Copy an opaque surface to the framebuffer.
 *
 * The surface is clipped against the display boundaries.
 */
static void composite_surface(
    const XyrisCompositorSurface *entry)
{
    if (entry == NULL ||
        entry->surface == NULL ||
        !entry->visible)
    {
        return;
    }

    XyrisSurface *surface = entry->surface;

    uint32_t width =
        surface_width(surface);

    uint32_t height =
        surface_height(surface);

    uint32_t screen_width =
        compositor_width();

    uint32_t screen_height =
        compositor_height();

    for (uint32_t sy = 0;
         sy < height;
         sy++)
    {
        int32_t screen_y =
            entry->y + (int32_t)sy;

        if (screen_y < 0 ||
            screen_y >= (int32_t)screen_height)
        {
            continue;
        }

        for (uint32_t sx = 0;
             sx < width;
             sx++)
        {
            int32_t screen_x =
                entry->x + (int32_t)sx;

            if (screen_x < 0 ||
                screen_x >= (int32_t)screen_width)
            {
                continue;
            }

            /*
             * M12-03 currently treats surfaces as opaque.
             */
            uint32_t color =
                surface->pixels[
                    sy * surface->width + sx
                ];

            renderer_draw_pixel(
                screen_x,
                screen_y,
                color
            );
        }
    }
}

/* -------------------------------------------------
   Lifecycle
------------------------------------------------- */

void compositor_init(void)
{
    if (g_initialized)
        return;

    for (uint32_t i = 0;
         i < XYRIS_COMPOSITOR_MAX_SURFACES;
         i++)
    {
        clear_surface_entry(
            &g_surfaces[i]
        );
    }

    g_surface_count = 0;
    g_background_color = 0x00000000;

    g_initialized = true;
}

/* -------------------------------------------------
   Root/background
------------------------------------------------- */

void compositor_set_background(uint32_t color)
{
    g_background_color = color;
}

uint32_t compositor_background(void)
{
    return g_background_color;
}

/* -------------------------------------------------
   Surface management
------------------------------------------------- */

int compositor_add_surface(
    XyrisSurface *surface,
    int32_t x,
    int32_t y,
    int32_t z)
{
    if (!g_initialized ||
        surface == NULL)
    {
        return -1;
    }

    if (find_surface(surface) >= 0)
        return -1;

    if (g_surface_count >=
        XYRIS_COMPOSITOR_MAX_SURFACES)
    {
        return -1;
    }

    uint32_t index = g_surface_count++;

    g_surfaces[index].surface = surface;
    g_surfaces[index].x = x;
    g_surfaces[index].y = y;
    g_surfaces[index].z = z;
    g_surfaces[index].visible = true;

    return (int)index;
}

void compositor_remove_surface(
    XyrisSurface *surface)
{
    int index = find_surface(surface);

    if (index < 0)
        return;

    uint32_t removed =
        (uint32_t)index;

    for (uint32_t i = removed;
         i + 1 < g_surface_count;
         i++)
    {
        g_surfaces[i] =
            g_surfaces[i + 1];
    }

    clear_surface_entry(
        &g_surfaces[g_surface_count - 1]
    );

    g_surface_count--;
}

void compositor_set_surface_position(
    XyrisSurface *surface,
    int32_t x,
    int32_t y)
{
    int index = find_surface(surface);

    if (index < 0)
        return;

    g_surfaces[index].x = x;
    g_surfaces[index].y = y;
}

void compositor_set_surface_z(
    XyrisSurface *surface,
    int32_t z)
{
    int index = find_surface(surface);

    if (index < 0)
        return;

    g_surfaces[index].z = z;
}

void compositor_set_surface_visible(
    XyrisSurface *surface,
    bool visible)
{
    int index = find_surface(surface);

    if (index < 0)
        return;

    g_surfaces[index].visible = visible;
}

/* -------------------------------------------------
   Composition
------------------------------------------------- */

void compositor_compose(void)
{
    if (!g_initialized)
        return;

    if (renderer_width() == 0 ||
        renderer_height() == 0)
    {
        return;
    }

    renderer_begin_frame();

    renderer_clear(
        g_background_color
    );

    /*
     * Compose visible surfaces from lowest Z
     * to highest Z.
     */
    bool composed[
        XYRIS_COMPOSITOR_MAX_SURFACES
    ];

    for (uint32_t i = 0;
         i < XYRIS_COMPOSITOR_MAX_SURFACES;
         i++)
    {
        composed[i] = false;
    }

    for (uint32_t layer = 0;
         layer < g_surface_count;
         layer++)
    {
        int selected = -1;

        for (uint32_t i = 0;
             i < g_surface_count;
             i++)
        {
            if (composed[i])
                continue;

            if (g_surfaces[i].surface == NULL ||
                !g_surfaces[i].visible)
            {
                composed[i] = true;
                continue;
            }

            if (selected < 0 ||
                g_surfaces[i].z <
                    g_surfaces[selected].z)
            {
                selected = (int)i;
            }
        }

        if (selected < 0)
            break;

        composite_surface(
            &g_surfaces[selected]
        );

        composed[selected] = true;
    }

    renderer_end_frame();
}

/* -------------------------------------------------
   Display information
------------------------------------------------- */

uint32_t compositor_width(void)
{
    return framebuffer_width();
}

uint32_t compositor_height(void)
{
    return framebuffer_height();
}
