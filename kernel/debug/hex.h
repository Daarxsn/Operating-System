#ifndef XYRIS_HEX_H
#define XYRIS_HEX_H

#include <stdint.h>

#define HEX64_STRING_SIZE 19

void uint64_to_hex(uint64_t value, char *buffer);

/* Debug helper */
void debug_print_hex64(uint64_t value);

#endif