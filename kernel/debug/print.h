#ifndef XYRIS_PRINT_H
#define XYRIS_PRINT_H

#include <stdint.h>
#include <stdbool.h>

/* Initialize cursor position */
void debug_print_init(void);

/* Cursor control */
void debug_set_cursor(int x, int y);

/* Print functions */
void debug_print(const char *text);
void debug_print_line(const char *text);

/*
 * Enable or disable diagnostic output to the framebuffer.
 *
 * Serial output is unaffected.
 */
void debug_set_framebuffer_enabled(bool enabled);

#endif