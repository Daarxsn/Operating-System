/*
 * bitmap.c
 * XyrisOS Kernel
 *
 * Generic bitmap implementation used by the Physical Memory Manager.
 */

#include "bitmap.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define INVALID_PAGE_INDEX ((size_t)-1)

void bitmap_init(bitmap_t* bitmap, uint8_t* buffer, size_t bits)
{
    if (bitmap == NULL || buffer == NULL)
        return;

    bitmap->data = buffer;
    bitmap->bits = bits;

    /* Mark all bits as free */
    size_t bytes = (bits + 7) / 8;

    for (size_t i = 0; i < bytes; i++)
    {
        bitmap->data[i] = 0;
    }
}

void bitmap_set(bitmap_t* bitmap, size_t bit)
{
    if (bitmap == NULL || bit >= bitmap->bits)
        return;

    bitmap->data[bit / 8] |= (1 << (bit % 8));
}

void bitmap_clear(bitmap_t* bitmap, size_t bit)
{
    if (bitmap == NULL || bit >= bitmap->bits)
        return;

    bitmap->data[bit / 8] &= ~(1 << (bit % 8));
}

void bitmap_clear_all(bitmap_t* bitmap)
{
    if (bitmap == NULL || bitmap->data == NULL)
        return;

    size_t bytes = (bitmap->bits + 7) / 8;

    for (size_t i = 0; i < bytes; i++)
    {
        bitmap->data[i] = 0;
    }
}

bool bitmap_test(const bitmap_t* bitmap, size_t bit)
{
    if (bitmap == NULL || bit >= bitmap->bits)
        return false;

    return (bitmap->data[bit / 8] & (1 << (bit % 8))) != 0;
}

size_t bitmap_find_free(const bitmap_t* bitmap)
{
    if (bitmap == NULL)
        return INVALID_PAGE_INDEX;

    for (size_t bit = 0; bit < bitmap->bits; bit++)
    {
        if (!bitmap_test(bitmap, bit))
            return bit;
    }

    return INVALID_PAGE_INDEX;
}