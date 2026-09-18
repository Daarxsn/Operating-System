#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "../shell.h"
#include "../../services/registry/service_registry.h"

static int test_write(const char *text, size_t length, void *context)
{
    char *buffer = context;
    size_t used = strlen(buffer);

    if (used + length + 1u >= 512u)
        return -1;

    memcpy(buffer + used, text, length);
    buffer[used + length] = '\0';
    return 0;
}

int main(void)
{
    char output[512] = {0};

    xyris_service_registry_initialize();

    assert(xyris_service_registry_publish(
        "system-service", 1u, 7u, 9u) == 0);

    assert(xyris_shell_execute_line("echo hello world", test_write, output) == XYRIS_SHELL_OK);
    assert(strcmp(output, "hello world\n") == 0);

    output[0] = '\0';
    assert(xyris_shell_execute_line("services", test_write, output) == XYRIS_SHELL_OK);
    assert(strstr(output, "system-service v1\n") != NULL);

    output[0] = '\0';
    assert(xyris_shell_execute_line("service system-service", test_write, output) == XYRIS_SHELL_OK);
    assert(strcmp(output, "system-service v1\n") == 0);

    output[0] = '\0';
    assert(xyris_shell_execute_line("system", test_write, output) == XYRIS_SHELL_OK);
    assert(strstr(output, "system-service v1 registered\n") != NULL);

    assert(xyris_shell_execute_line("cat", test_write, output) == XYRIS_SHELL_INVALID_ARGUMENT);
    assert(xyris_shell_execute_line("pid extra", test_write, output) == XYRIS_SHELL_INVALID_ARGUMENT);
    assert(xyris_shell_execute_line("unknown", test_write, output) == XYRIS_SHELL_UNKNOWN_COMMAND);
    assert(xyris_shell_execute_line("quit", test_write, output) == XYRIS_SHELL_QUIT);
    assert(xyris_shell_execute_line("echo \"hello world\"", test_write, output) == XYRIS_SHELL_OK);

    return 0;
}
