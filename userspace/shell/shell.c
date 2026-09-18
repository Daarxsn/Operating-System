#include "shell.h"

#include <stddef.h>

#include "../utilities/userspace_utilities.h"

static size_t shell_strlen(const char *text)
{
    size_t length = 0;

    if (text == NULL)
        return 0;

    while (text[length] != '\0')
        ++length;

    return length;
}

static int shell_streq(const char *left, const char *right)
{
    size_t index = 0;

    if (left == NULL || right == NULL)
        return 0;

    while (left[index] != '\0' && right[index] != '\0')
    {
        if (left[index] != right[index])
            return 0;
        ++index;
    }

    return left[index] == right[index];
}

static int shell_parse(
    const char *line,
    char storage[XYRIS_SHELL_ARG_MAX][XYRIS_SHELL_TOKEN_MAX],
    const char *argv[XYRIS_SHELL_ARG_MAX],
    size_t *argc_out
)
{
    const char *cursor = line;
    size_t argc = 0;

    if (line == NULL || storage == NULL || argv == NULL || argc_out == NULL)
        return -1;

    while (*cursor != '\0')
    {
        size_t length = 0;
        char quote = '\0';

        while (*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n')
            ++cursor;

        if (*cursor == '\0')
            break;

        if (argc >= XYRIS_SHELL_ARG_MAX)
            return -1;

        if (*cursor == '\'' || *cursor == '"')
            quote = *cursor++;

        while (*cursor != '\0')
        {
            if (quote != '\0')
            {
                if (*cursor == quote)
                {
                    ++cursor;
                    quote = '\0';
                    break;
                }
            }
            else if (*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n')
            {
                break;
            }

            if (length + 1u >= XYRIS_SHELL_TOKEN_MAX)
                return -1;

            storage[argc][length++] = *cursor++;
        }

        if (quote != '\0')
            return -1;

        storage[argc][length] = '\0';
        argv[argc] = storage[argc];
        ++argc;

        while (*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n')
            ++cursor;
    }

    *argc_out = argc;
    return 0;
}

static int shell_command_help(xyris_shell_write_fn write, void *context)
{
    static const char help[] =
        "help      show commands\n"
        "echo      print arguments\n"
        "cat       print a file\n"
        "pid       print current process ID\n"
        "services  list registered services\n"
        "service   show one service\n"
        "system    show system service\n"
        "quit      request shell termination\n";

    if (write == NULL)
        return 0;

    return write(help, sizeof(help) - 1u, context) == 0 ? 0 : -1;
}

static int shell_command_cat(
    size_t argc,
    const char *const *argv,
    xyris_shell_write_fn write,
    void *context)
{
    if (argc != 2u)
        return -2;

    return xyris_utility_cat(argv[1], write, context);
}

xyris_shell_status_t xyris_shell_execute_line(
    const char *line,
    xyris_shell_write_fn write,
    void *context)
{
    char storage[XYRIS_SHELL_ARG_MAX][XYRIS_SHELL_TOKEN_MAX];
    const char *argv[XYRIS_SHELL_ARG_MAX];
    size_t argc;
    int command_status;

    if (line == NULL || shell_strlen(line) >= XYRIS_SHELL_LINE_MAX)
        return XYRIS_SHELL_PARSE_ERROR;

    if (shell_parse(line, storage, argv, &argc) != 0)
        return XYRIS_SHELL_PARSE_ERROR;

    if (argc == 0u)
        return XYRIS_SHELL_EMPTY;

    if (shell_streq(argv[0], "help"))
    {
        command_status = shell_command_help(write, context);
    }
    else if (shell_streq(argv[0], "echo"))
    {
        command_status = xyris_utility_echo(argc, argv, write, context);
    }
    else if (shell_streq(argv[0], "cat"))
    {
        command_status = shell_command_cat(argc, argv, write, context);
        if (command_status == -2)
            return XYRIS_SHELL_INVALID_ARGUMENT;
    }
    else if (shell_streq(argv[0], "pid"))
    {
        if (argc != 1u)
            return XYRIS_SHELL_INVALID_ARGUMENT;
        command_status = xyris_utility_pid(write, context);
    }
    else if (shell_streq(argv[0], "services"))
    {
        if (argc != 1u)
            return XYRIS_SHELL_INVALID_ARGUMENT;
        command_status = xyris_utility_services(write, context);
    }
    else if (shell_streq(argv[0], "service"))
    {
        if (argc != 2u)
            return XYRIS_SHELL_INVALID_ARGUMENT;
        command_status = xyris_utility_service(argv[1], write, context);
    }
    else if (shell_streq(argv[0], "system"))
    {
        if (argc != 1u)
            return XYRIS_SHELL_INVALID_ARGUMENT;
        command_status = xyris_utility_system(write, context);
    }
    else if (shell_streq(argv[0], "quit"))
    {
        if (argc != 1u)
            return XYRIS_SHELL_INVALID_ARGUMENT;
        return XYRIS_SHELL_QUIT;
    }
    else
    {
        return XYRIS_SHELL_UNKNOWN_COMMAND;
    }

    return command_status == 0 ? XYRIS_SHELL_OK : XYRIS_SHELL_OUTPUT_ERROR;
}
