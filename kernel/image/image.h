#ifndef XYRIS_IMAGE_H
#define XYRIS_IMAGE_H

#include <stdint.h>

typedef struct
{
    uint32_t width;
    uint32_t height;
    const uint32_t *pixels;
} Image;

/* Draw image at original size */
void draw_image(
    int x,
    int y,
    const Image *image
);

/* Draw image scaled */
void draw_image_scaled(
    int x,
    int y,
    const Image *image,
    int dst_width,
    int dst_height
);

#endif