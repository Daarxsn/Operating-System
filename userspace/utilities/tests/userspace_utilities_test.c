#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "userspace_utilities.h"
#include "../services/registry/service_registry.h"

static int test_write(const char *data, size_t length, void *context)
{
    char *buffer = context;
    size_t used = strlen(buffer);

    if (used + length + 1u >= 1024u)
        return -1;

    memcpy(buffer + used, data, length);
    buffer[used + length] = '\0';
    return 0;
}

int main(void)
{
    char output[1024] = {0};
    const char *echo_argv[] = { "echo", "hello", "world" };

    xyris_service_registry_initialize();
    assert(xyris_service_registry_publish(
        "system-service", 1u, 10u, 20u) == 0);

    assert(xyris_utility_echo(
        3u,
        echo_argv,
        test_write,
        output) == 0);
    assert(strcmp(output, "hello world\n") == 0);

    output[0] = '\0';
    assert(xyris_utility_services(test_write, output) == 0);
    assert(strstr(output, "system-service v1\n") != NULL);

    output[0] = '\0';
    assert(xyris_utility_service(
        "system-service",
        test_write,
        output) == 0);
    assert(strcmp(output, "system-service v1\n") == 0);

    output[0] = '\0';
    assert(xyris_utility_system(test_write, output) == 0);
    assert(strstr(output, "system-service v1 registered\n") != NULL);

    output[0] = '\0';
    assert(xyris_utility_service(
        "missing-service",
        test_write,
        output) == XYRIS_UTILITY_NOT_FOUND);

    assert(xyris_utility_echo(0u, echo_argv, test_write, output) ==
        XYRIS_UTILITY_INVALID_ARGUMENT);

    return 0;
}
