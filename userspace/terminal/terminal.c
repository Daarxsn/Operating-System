#include "terminal.h"

static int terminal_emit(
    xyris_shell_write_fn write,
    void *context,
    const char *text,
    size_t length)
{
    if (write == NULL)
        return 0;

    return write(text, length, context) == 0 ? 0 : -1;
}

static int terminal_emit_text(
    xyris_shell_write_fn write,
    void *context,
    const char *text)
{
    size_t length = 0;

    while (text[length] != '\0')
        ++length;

    return terminal_emit(write, context, text, length);
}

static int terminal_is_backspace(char character)
{
    return character == '\b' || character == 0x7FU;
}

static xyris_terminal_status_t terminal_discard_line(
    xyris_terminal_read_fn read,
    void *context)
{
    for (;;)
    {
        char character = '\0';
        int result = read(&character, context);

        if (result < 0)
            return XYRIS_TERMINAL_INPUT_ERROR;

        if (result == 0)
            return XYRIS_TERMINAL_EOF;

        if (character == '\n' || character == '\r')
            return XYRIS_TERMINAL_OK;
    }
}

xyris_terminal_status_t xyris_terminal_run_shell(
    xyris_terminal_read_fn read,
    xyris_shell_write_fn write,
    void *context)
{
    char line[XYRIS_TERMINAL_LINE_MAX];

    if (read == NULL)
        return XYRIS_TERMINAL_INPUT_ERROR;

    for (;;)
    {
        size_t length = 0;
        int saw_line_termination = 0;

        if (terminal_emit_text(write, context, "xyris> ") != 0)
            return XYRIS_TERMINAL_OUTPUT_ERROR;

        for (;;)
        {
            char character = '\0';
            int result = read(&character, context);

            if (result < 0)
                return XYRIS_TERMINAL_INPUT_ERROR;

            if (result == 0)
            {
                if (length == 0u)
                    return XYRIS_TERMINAL_EOF;

                break;
            }

            if (character == '\n' || character == '\r')
            {
                saw_line_termination = 1;
                if (terminal_emit(write, context, "\n", 1u) != 0)
                    return XYRIS_TERMINAL_OUTPUT_ERROR;
                break;
            }

            if (terminal_is_backspace(character))
            {
                if (length != 0u)
                {
                    --length;
                    if (terminal_emit(write, context, "\b \b", 3u) != 0)
                        return XYRIS_TERMINAL_OUTPUT_ERROR;
                }
                continue;
            }

            if (character < 0x20 || character == 0x7FU)
                continue;

            if (length + 1u >= XYRIS_TERMINAL_LINE_MAX)
            {
                if (terminal_emit_text(write, context, "\ninput line too long\n") != 0)
                    return XYRIS_TERMINAL_OUTPUT_ERROR;

                {
                    xyris_terminal_status_t discard_status =
                        terminal_discard_line(read, context);

                    if (discard_status != XYRIS_TERMINAL_OK)
                        return discard_status;
                }

                saw_line_termination = 0;
                break;
            }

            line[length++] = character;
            if (terminal_emit(write, context, &character, 1u) != 0)
                return XYRIS_TERMINAL_OUTPUT_ERROR;
        }

        if (!saw_line_termination)
            continue;

        line[length] = '\0';

        xyris_shell_status_t shell_status =
            xyris_shell_execute_line(line, write, context);

        if (shell_status == XYRIS_SHELL_QUIT)
            return XYRIS_TERMINAL_SHELL_QUIT;

        if (shell_status == XYRIS_SHELL_PARSE_ERROR ||
            shell_status == XYRIS_SHELL_INVALID_ARGUMENT)
        {
            if (terminal_emit_text(write, context, "shell: invalid input\n") != 0)
                return XYRIS_TERMINAL_OUTPUT_ERROR;
        }
        else if (shell_status == XYRIS_SHELL_UNKNOWN_COMMAND)
        {
            if (terminal_emit_text(write, context, "shell: unknown command\n") != 0)
                return XYRIS_TERMINAL_OUTPUT_ERROR;
        }
        else if (shell_status == XYRIS_SHELL_OUTPUT_ERROR)
        {
            return XYRIS_TERMINAL_OUTPUT_ERROR;
        }
    }
}
