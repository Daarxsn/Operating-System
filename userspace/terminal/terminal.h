#ifndef XYRIS_USERSPACE_TERMINAL_H
#define XYRIS_USERSPACE_TERMINAL_H

#include <stddef.h>

#include "../common/io.h"
#include "../shell/shell.h"

#define XYRIS_TERMINAL_LINE_MAX XYRIS_SHELL_LINE_MAX

/*
 * Userspace terminal adapters provide bytes from the platform terminal.
 * Return >0 for one byte, 0 for end-of-input, and <0 for an input error.
 */
typedef int (*xyris_terminal_read_fn)(char *character, void *context);

typedef enum
{
    XYRIS_TERMINAL_OK = 0,
    XYRIS_TERMINAL_EOF = 1,
    XYRIS_TERMINAL_INPUT_ERROR = 2,
    XYRIS_TERMINAL_OUTPUT_ERROR = 3,
    XYRIS_TERMINAL_SHELL_QUIT = 4,
    XYRIS_TERMINAL_LINE_ERROR = 5
} xyris_terminal_status_t;

/* Run one interactive shell session using injected terminal I/O. */
xyris_terminal_status_t xyris_terminal_run_shell(
    xyris_terminal_read_fn read,
    xyris_shell_write_fn write,
    void *context
);

#endif /* XYRIS_USERSPACE_TERMINAL_H */
