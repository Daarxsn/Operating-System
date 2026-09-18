#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "../terminal.h"
#include "../../services/registry/service_registry.h"

#define INPUT_CAPACITY 256u
#define OUTPUT_CAPACITY 1024u

typedef struct
{
    const char *input;
    size_t offset;
    char output[OUTPUT_CAPACITY];
    size_t output_length;
    int commands_seen;
} terminal_test_context_t;

static int test_read(char *character, void *context)
{
    terminal_test_context_t *state = context;

    if (state == NULL || character == NULL)
        return -1;

    if (state->input[state->offset] == '\0')
        return 0;

    *character = state->input[state->offset++];
    return 1;
}

static int test_write(const char *text, size_t length, void *context)
{
    terminal_test_context_t *state = context;

    if (state == NULL || (text == NULL && length != 0u))
        return -1;

    if (state->output_length + length + 1u > OUTPUT_CAPACITY)
        return -1;

    if (length != 0u)
        memcpy(state->output + state->output_length, text, length);
    state->output_length += length;
    state->output[state->output_length] = '\0';
    return 0;
}

int main(void)
{
    terminal_test_context_t state = {
        .input = "echo hello\nquit\n",
        .offset = 0u,
        .output = {0},
        .output_length = 0u,
        .commands_seen = 0
    };
    xyris_terminal_status_t status;

    xyris_service_registry_initialize();
    assert(xyris_service_registry_publish(
        "system-service", 1u,
        XYRIS_INVALID_HANDLE, XYRIS_INVALID_CAP) == 0);

    status = xyris_terminal_run_shell(test_read, test_write, &state);

    assert(status == XYRIS_TERMINAL_SHELL_QUIT);
    assert(strstr(state.output, "xyris> echo hello") != NULL);
    assert(strstr(state.output, "hello\n") != NULL);
    assert(strstr(state.output, "xyris> quit") != NULL);

    state.input = "echo abc\b\bX\nquit\n";
    state.offset = 0u;
    state.output_length = 0u;
    state.output[0] = '\0';

    status = xyris_terminal_run_shell(test_read, test_write, &state);
    assert(status == XYRIS_TERMINAL_SHELL_QUIT);
    assert(strstr(state.output, "aX\n") != NULL);
    assert(strstr(state.output, "\\b \\b") == NULL);

    return 0;
}
