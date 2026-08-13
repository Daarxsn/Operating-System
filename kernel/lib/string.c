#include "string.h"

/*
 * ============================================================
 * XyrisOS Kernel String Library
 * ============================================================
 */

/*
 * ============================================================
 * Memory Operations
 * ============================================================
 */

void *memset(
    void *destination,
    int value,
    size_t size
)
{
    unsigned char *ptr =
        (unsigned char *)destination;

    while (size--)
    {
        *ptr++ = (unsigned char)value;
    }

    return destination;
}


void *memcpy(
    void *destination,
    const void *source,
    size_t size
)
{
    unsigned char *dst =
        (unsigned char *)destination;

    const unsigned char *src =
        (const unsigned char *)source;

    while (size--)
    {
        *dst++ = *src++;
    }

    return destination;
}


int memcmp(
    const void *left,
    const void *right,
    size_t size
)
{
    const unsigned char *a = left;
    const unsigned char *b = right;

    while (size--)
    {
        if (*a != *b)
        {
            return *a - *b;
        }

        a++;
        b++;
    }

    return 0;
}


void *memmove(
    void *destination,
    const void *source,
    size_t size
)
{
    unsigned char *dst =
        (unsigned char *)destination;

    const unsigned char *src =
        (const unsigned char *)source;

    if (dst < src)
    {
        while (size--)
        {
            *dst++ = *src++;
        }
    }
    else
    {
        dst += size;
        src += size;

        while (size--)
        {
            *--dst = *--src;
        }
    }

    return destination;
}


/*
 * ============================================================
 * String Operations
 * ============================================================
 */

size_t strlen(
    const char *string
)
{
    size_t length = 0;

    while (*string++)
    {
        length++;
    }

    return length;
}


char *strcpy(
    char *destination,
    const char *source
)
{
    char *start = destination;

    while ((*destination++ = *source++))
    {
    }

    return start;
}


char *strncpy(
    char *destination,
    const char *source,
    size_t size
)
{
    char *start = destination;

    while (size && *source)
    {
        *destination++ = *source++;
        size--;
    }

    while (size--)
    {
        *destination++ = '\0';
    }

    return start;
}



/*
 * ============================================================
 * Compare Strings
 * ============================================================
 */

int strcmp(
    const char *left,
    const char *right
)
{
    if (left == NULL || right == NULL)
    {
        if (left == right)
            return 0;

        return left == NULL ? -1 : 1;
    }

    while (*left && (*left == *right))
    {
        left++;
        right++;
    }

    return (unsigned char)*left -
           (unsigned char)*right;
}



int strncmp(
    const char *left,
    const char *right,
    size_t size
)
{
    if (size == 0)
        return 0;

    if (left == NULL || right == NULL)
    {
        if (left == right)
            return 0;

        return left == NULL ? -1 : 1;
    }

    while (size-- && *left && (*left == *right))
    {
        left++;
        right++;
    }

    if ((size_t)-1 == size)
        return 0;

    return (unsigned char)*left -
           (unsigned char)*right;
}

/*
 * ============================================================
 * Tokenize String
 * ============================================================
 *
 * Kernel-only, non-reentrant tokenizer used by the RAMFS
 * path parser. Callers must not use strtok concurrently.
 */

char *strtok(
    char *string,
    const char *delimiters
)
{
    static char *next = NULL;

    if (string != NULL)
        next = string;

    if (next == NULL || delimiters == NULL)
        return NULL;

    while (*next)
    {
        const char *delimiter = delimiters;

        while (*delimiter)
        {
            if (*next == *delimiter)
                break;

            delimiter++;
        }

        if (*delimiter == '\0')
            break;

        next++;
    }

    if (*next == '\0')
    {
        next = NULL;
        return NULL;
    }

    char *token = next;

    while (*next)
    {
        const char *delimiter = delimiters;

        while (*delimiter)
        {
            if (*next == *delimiter)
                break;

            delimiter++;
        }

        if (*delimiter != '\0')
        {
            *next = '\0';
            next++;
            return token;
        }

        next++;
    }

    next = NULL;
    return token;
}


/*
 * ============================================================
 * Integer Conversion
 * ============================================================
 *
 * Convert an unsigned integer to a string.
 *
 * Supported bases:
 *     2  - Binary
 *     8  - Octal
 *     10 - Decimal
 *     16 - Hexadecimal
 *
 * ============================================================
 */

char *itoa(
    unsigned long long value,
    char *buffer,
    int base
)
{
    if (buffer == NULL)
    {
        return NULL;
    }

    if (base < 2 || base > 16)
    {
        buffer[0] = '\0';
        return buffer;
    }

    static const char digits[] =
        "0123456789ABCDEF";

    char *start = buffer;
    char *ptr = buffer;

    do
    {
        *ptr++ =
            digits[value % (unsigned long long)base];

        value /=
            (unsigned long long)base;
    }
    while (value != 0);

    *ptr = '\0';

    ptr--;

    while (start < ptr)
    {
        char temp = *start;

        *start = *ptr;
        *ptr = temp;

        start++;
        ptr--;
    }

    return buffer;
}
