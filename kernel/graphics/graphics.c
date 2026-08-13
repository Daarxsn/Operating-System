#include "graphics.h"
#include "framebuffer.h"

/* -------------------------------------------------
   Clear Screen
------------------------------------------------- */

void clear_screen(uint32_t color)
{
    volatile uint32_t *fb = framebuffer_address();

    uint32_t width  = framebuffer_width();
    uint32_t height = framebuffer_height();
    uint32_t pitch  = framebuffer_pitch() / 4;

    for (uint32_t y = 0; y < height; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            fb[y * pitch + x] = color;
        }
    }
}

/* -------------------------------------------------
   Draw Pixel
------------------------------------------------- */

void draw_pixel(
    uint32_t x,
    uint32_t y,
    uint32_t color)
{
    if (x >= framebuffer_width())
        return;

    if (y >= framebuffer_height())
        return;

    volatile uint32_t *fb = framebuffer_address();

    fb[y * (framebuffer_pitch() / 4) + x] = color;
}

/* -------------------------------------------------
   Fill Rectangle
------------------------------------------------- */

void fill_rect(
    int x,
    int y,
    int width,
    int height,
    uint32_t color)
{
    for (int yy = y; yy < y + height; yy++)
    {
        for (int xx = x; xx < x + width; xx++)
        {
            draw_pixel(xx, yy, color);
        }
    }
}

/* -------------------------------------------------
   Rectangle Border
------------------------------------------------- */

void draw_rect(
    int x,
    int y,
    int width,
    int height,
    uint32_t color)
{
    for (int i = 0; i < width; i++)
    {
        draw_pixel(x + i, y, color);
        draw_pixel(x + i, y + height - 1, color);
    }

    for (int i = 0; i < height; i++)
    {
        draw_pixel(x, y + i, color);
        draw_pixel(x + width - 1, y + i, color);
    }
}

/* -------------------------------------------------
   Bresenham Line Algorithm
------------------------------------------------- */

void draw_line(
    int x0,
    int y0,
    int x1,
    int y1,
    uint32_t color)
{
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int sx = (x0 < x1) ? 1 : -1;

    int dy = (y1 > y0) ? (y0 - y1) : (y1 - y0);
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx + dy;

    while (1)
    {
        draw_pixel(x0, y0, color);

        if (x0 == x1 && y0 == y1)
        {
            break;
        }

        int e2 = err * 2;

        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void draw_circle(
    int xc,
    int yc,
    int radius,
    uint32_t color)
{
    int x = 0;
    int y = radius;
    int d = 3 - (2 * radius);

    while (x <= y)
    {
        draw_pixel(xc + x, yc + y, color);
        draw_pixel(xc - x, yc + y, color);
        draw_pixel(xc + x, yc - y, color);
        draw_pixel(xc - x, yc - y, color);

        draw_pixel(xc + y, yc + x, color);
        draw_pixel(xc - y, yc + x, color);
        draw_pixel(xc + y, yc - x, color);
        draw_pixel(xc - y, yc - x, color);

        if (d < 0)
        {
            d += (4 * x) + 6;
        }
        else
        {
            d += 4 * (x - y) + 10;
            y--;
        }

        x++;
    }
}

void fill_circle(
    int xc,
    int yc,
    int radius,
    uint32_t color)
{
    for (int y = -radius; y <= radius; y++)
    {
        for (int x = -radius; x <= radius; x++)
        {
            if ((x * x) + (y * y) <= (radius * radius))
            {
                draw_pixel(
                    xc + x,
                    yc + y,
                    color
                );
            }
        }
    }
}