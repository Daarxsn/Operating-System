#ifndef XYRIS_STRING_H
#define XYRIS_STRING_H

#include <stddef.h>

/*
 * Memory operations
 */

void* memset(
    void* ptr,
    int value,
    size_t num
);

void* memcpy(
    void* dest,
    const void* src,
    size_t num
);

int memcmp(
    const void* left,
    const void* right,
    size_t num
);

void* memmove(
    void* dest,
    const void* src,
    size_t num
);

/*
 * String operations
 */

size_t strlen(
    const char* str
);

int strcmp(
    const char* s1,
    const char* s2
);

int strncmp(
    const char* s1,
    const char* s2,
    size_t n
);

char* strcpy(
    char* dest,
    const char* src
);

char* strncpy(
    char* dest,
    const char* src,
    size_t n
);

char* strtok(
    char* str,
    const char* delimiters
);

/*
 * Integer to ASCII conversion
 */

char* itoa(
    unsigned long long value,
    char* buffer,
    int base
);

#endif