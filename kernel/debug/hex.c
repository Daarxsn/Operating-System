#include "hex.h"
#include "print.h"

static const char hex_digits[] = "0123456789ABCDEF";

void uint64_to_hex(uint64_t value, char *buffer)
{
    buffer[0] = '0';
    buffer[1] = 'x';

    for (int i = 0; i < 16; i++)
    {
        int shift = (15 - i) * 4;

        buffer[2 + i] =
            hex_digits[(value >> shift) & 0xF];
    }

    buffer[18] = '\0';
}

void debug_print_hex64(uint64_t value)
{
    char buffer[HEX64_STRING_SIZE];

    uint64_to_hex(value, buffer);

    debug_print_line(buffer);
}