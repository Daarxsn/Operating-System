#include "image.h"
#include "../graphics/graphics.h"
#include <stddef.h>

/* -------------------------------------------------
   Draw Image
------------------------------------------------- */

void draw_image(
    int x,
    int y,
    const Image *image)
{
    for (uint32_t yy = 0; yy < image->height; yy++)
    {
        for (uint32_t xx = 0; xx < image->width; xx++)
        {
            uint32_t pixel =
                image->pixels[yy * image->width + xx];

            uint8_t alpha = pixel >> 24;

            if (alpha == 0)
                continue;

            draw_pixel(
                x + xx,
                y + yy,
                pixel & 0x00FFFFFF
            );
        }
    }
}

/* -------------------------------------------------
   Draw Scaled Image
------------------------------------------------- */

void draw_image_scaled(
    int x,
    int y,
    const Image *image,
    int dst_width,
    int dst_height)
{
    if (image == NULL)
        return;

    if (dst_width <= 0 || dst_height <= 0)
        return;

    for (int dy = 0; dy < dst_height; dy++)
    {
        uint32_t src_y =
            (uint32_t)(((uint64_t)dy * image->height) / dst_height);

        if (src_y >= image->height)
            src_y = image->height - 1;

        for (int dx = 0; dx < dst_width; dx++)
        {
            uint32_t src_x =
                (uint32_t)(((uint64_t)dx * image->width) / dst_width);

            if (src_x >= image->width)
                src_x = image->width - 1;

            uint32_t pixel =
                image->pixels[src_y * image->width + src_x];

            uint8_t alpha = pixel >> 24;

            if (alpha == 0)
                continue;

            draw_pixel(
                x + dx,
                y + dy,
                pixel & 0x00FFFFFF
            );
        }
    }
}