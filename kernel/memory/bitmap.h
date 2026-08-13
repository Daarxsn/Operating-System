#ifndef XYRIS_BITMAP_H
#define XYRIS_BITMAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    uint8_t* data;
    size_t bits;
} bitmap_t;

void bitmap_init(bitmap_t* bitmap, uint8_t* buffer, size_t bits);

void bitmap_set(bitmap_t* bitmap, size_t bit);

void bitmap_clear(bitmap_t* bitmap, size_t bit);

void bitmap_clear_all(bitmap_t* bitmap);

bool bitmap_test(const bitmap_t* bitmap, size_t bit);

size_t bitmap_find_free(const bitmap_t* bitmap);

#endif