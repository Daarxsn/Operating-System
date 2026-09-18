#ifndef XYRIS_USERSPACE_SHELL_H
#define XYRIS_USERSPACE_SHELL_H

#include <stddef.h>

#include "../common/io.h"

#define XYRIS_SHELL_LINE_MAX 128u
#define XYRIS_SHELL_ARG_MAX  16u
#define XYRIS_SHELL_TOKEN_MAX 64u

typedef enum
{
    XYRIS_SHELL_OK = 0,
    XYRIS_SHELL_EMPTY = 1,
    XYRIS_SHELL_PARSE_ERROR = 2,
    XYRIS_SHELL_UNKNOWN_COMMAND = 3,
    XYRIS_SHELL_INVALID_ARGUMENT = 4,
    XYRIS_SHELL_OUTPUT_ERROR = 5,
    XYRIS_SHELL_QUIT = 6
} xyris_shell_status_t;

typedef xyris_user_write_fn xyris_shell_write_fn;

/* Execute one complete command line. No terminal or input device is assumed. */
xyris_shell_status_t xyris_shell_execute_line(
    const char *line,
    xyris_shell_write_fn write,
    void *context
);

#endif /* XYRIS_USERSPACE_SHELL_H */
