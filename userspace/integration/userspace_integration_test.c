#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "../services/registry/service_registry.h"
#include "../shell/shell.h"
#include "../terminal/terminal.h"
#include "../utilities/userspace_utilities.h"

#define OUTPUT_CAPACITY 4096u
#define INPUT_CAPACITY 512u

typedef struct
{
    const char *input;
    size_t input_offset;
    int input_error;
    char output[OUTPUT_CAPACITY];
    size_t output_length;
    int output_error;
} integration_io_t;

static int integration_read(char *character, void *context)
{
    integration_io_t *io = context;

    if (io == NULL || character == NULL)
        return -1;

    if (io->input_error)
        return -1;

    if (io->input[io->input_offset] == '\0')
        return 0;

    *character = io->input[io->input_offset++];
    return 1;
}

static int integration_write(
    const char *data,
    size_t length,
    void *context)
{
    integration_io_t *io = context;

    if (io == NULL || (data == NULL && length != 0u))
        return -1;

    if (io->output_error)
        return -1;

    if (io->output_length + length + 1u > OUTPUT_CAPACITY)
        return -1;

    if (length != 0u)
        memcpy(io->output + io->output_length, data, length);

    io->output_length += length;
    io->output[io->output_length] = '\0';
    return 0;
}

static void integration_reset_output(integration_io_t *io)
{
    io->output_length = 0u;
    io->output[0] = '\0';
    io->output_error = 0;
}

static void test_normal_session(void)
{
    integration_io_t io = {
        .input = "echo hello world\nservices\nservice system-service\nquit\n",
        .input_offset = 0u,
        .input_error = 0,
        .output = {0},
        .output_length = 0u,
        .output_error = 0
    };

    assert(xyris_service_registry_publish(
        "system-service", 1u, 10u, 20u) == 0);

    assert(xyris_terminal_run_shell(
        integration_read, integration_write, &io) ==
        XYRIS_TERMINAL_SHELL_QUIT);

    assert(strstr(io.output, "hello world\n") != NULL);
    assert(strstr(io.output, "system-service v1\n") != NULL);
    assert(strstr(io.output, "xyris> quit") != NULL);
}

static void test_line_editing(void)
{
    integration_io_t io = {
        .input = "echo abc\b\bX\rquit\n",
        .input_offset = 0u,
        .input_error = 0,
        .output = {0},
        .output_length = 0u,
        .output_error = 0
    };

    integration_reset_output(&io);

    assert(xyris_terminal_run_shell(
        integration_read, integration_write, &io) ==
        XYRIS_TERMINAL_SHELL_QUIT);
    assert(strstr(io.output, "aX\n") != NULL);
}

static void test_overlong_line_is_discarded(void)
{
    static char input[INPUT_CAPACITY];
    integration_io_t io = {
        .input = input,
        .input_offset = 0u,
        .input_error = 0,
        .output = {0},
        .output_length = 0u,
        .output_error = 0
    };
    size_t index;

    for (index = 0; index < 150u; ++index)
        input[index] = 'a';
    input[150] = '\n';
    strcpy(input + 151u, "echo recovered\nquit\n");

    assert(xyris_terminal_run_shell(
        integration_read, integration_write, &io) ==
        XYRIS_TERMINAL_SHELL_QUIT);
    assert(strstr(io.output, "input line too long\n") != NULL);
    assert(strstr(io.output, "recovered\n") != NULL);
}

static void test_input_and_output_errors(void)
{
    integration_io_t input_error_io = {
        .input = "echo hello\n",
        .input_offset = 0u,
        .input_error = 1,
        .output = {0},
        .output_length = 0u,
        .output_error = 0
    };
    integration_io_t output_error_io = {
        .input = "quit\n",
        .input_offset = 0u,
        .input_error = 0,
        .output = {0},
        .output_length = 0u,
        .output_error = 1
    };

    assert(xyris_terminal_run_shell(
        integration_read, integration_write, &input_error_io) ==
        XYRIS_TERMINAL_INPUT_ERROR);
    assert(xyris_terminal_run_shell(
        integration_read, integration_write, &output_error_io) ==
        XYRIS_TERMINAL_OUTPUT_ERROR);
}

static void test_registry_shell_dispatch(void)
{
    char output[512] = {0};

    assert(xyris_shell_execute_line(
        "service system-service", integration_write, NULL) ==
        XYRIS_SHELL_OUTPUT_ERROR);

    {
        integration_io_t io = {
            .input = "",
            .input_offset = 0u,
            .input_error = 0,
            .output = {0},
            .output_length = 0u,
            .output_error = 0
        };

        (void)output;
        assert(xyris_shell_execute_line(
            "service system-service",
            integration_write,
            &io) == XYRIS_SHELL_OK);
        assert(strcmp(io.output, "system-service v1\n") == 0);
    }
}

int main(void)
{
    xyris_service_registry_initialize();
    test_normal_session();

    xyris_service_registry_initialize();
    test_line_editing();

    xyris_service_registry_initialize();
    test_overlong_line_is_discarded();

    xyris_service_registry_initialize();
    test_input_and_output_errors();

    xyris_service_registry_initialize();
    assert(xyris_service_registry_publish(
        "system-service", 1u, 10u, 20u) == 0);
    test_registry_shell_dispatch();

    return 0;
}
