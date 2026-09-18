#include "userspace_utilities.h"

#include <stddef.h>

#include <xyris/filesystem.h>
#include <xyris/process.h>

#include "../services/registry/service_registry.h"
#include "../services/system/system_service.h"

static size_t utility_strlen(const char *text)
{
    size_t length = 0;

    if (text == NULL)
        return 0;

    while (text[length] != '\0')
        ++length;

    return length;
}

static size_t utility_u64_to_text(
    unsigned long long value,
    char *buffer,
    size_t capacity)
{
    char reverse[24];
    size_t count = 0;
    size_t index;

    if (buffer == NULL || capacity < 2u)
        return 0;

    if (value == 0u)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return 1u;
    }

    while (value != 0u && count < sizeof(reverse))
    {
        reverse[count++] = (char)('0' + (value % 10u));
        value /= 10u;
    }

    if (count + 1u > capacity)
        return 0;

    for (index = 0; index < count; ++index)
        buffer[index] = reverse[count - index - 1u];

    buffer[count] = '\0';
    return count;
}

static int utility_emit(
    xyris_user_write_fn write,
    void *context,
    const char *data,
    size_t length)
{
    if (data == NULL && length != 0u)
        return XYRIS_UTILITY_INVALID_ARGUMENT;

    /* A NULL writer is accepted by kernel bootstrap validation. */
    if (write == NULL)
        return 0;

    return write(data, length, context) == 0
        ? 0
        : XYRIS_UTILITY_IO_ERROR;
}

static int utility_emit_text(
    xyris_user_write_fn write,
    void *context,
    const char *text)
{
    return utility_emit(write, context, text, utility_strlen(text));
}

int xyris_utility_echo(
    size_t argc,
    const char *const *argv,
    xyris_user_write_fn write,
    void *context)
{
    size_t index;

    if (argv == NULL || argc == 0u)
        return XYRIS_UTILITY_INVALID_ARGUMENT;

    for (index = 1u; index < argc; ++index)
    {
        if (index > 1u && utility_emit_text(write, context, " ") != 0)
            return XYRIS_UTILITY_IO_ERROR;

        if (utility_emit_text(write, context, argv[index]) != 0)
            return XYRIS_UTILITY_IO_ERROR;
    }

    return utility_emit_text(write, context, "\n");
}

int xyris_utility_services(
    xyris_user_write_fn write,
    void *context)
{
    size_t index;
    size_t count = xyris_service_registry_count();
    char line[96];

    for (index = 0; index < count; ++index)
    {
        xyris_service_descriptor_t descriptor;
        char version[24];
        size_t pos = 0;
        size_t name_len;
        size_t version_len;

        if (xyris_service_registry_get(index, &descriptor) != 0)
            return XYRIS_UTILITY_NOT_FOUND;

        if (utility_u64_to_text(
                descriptor.version,
                version,
                sizeof(version)) == 0)
            return XYRIS_UTILITY_IO_ERROR;

        name_len = utility_strlen(descriptor.name);
        version_len = utility_strlen(version);

        if (name_len + version_len + 5u >= sizeof(line))
            return XYRIS_UTILITY_IO_ERROR;

        line[pos++] = '-';
        line[pos++] = ' ';

        for (size_t i = 0; i < name_len; ++i)
            line[pos++] = descriptor.name[i];

        line[pos++] = ' ';
        line[pos++] = 'v';
        for (size_t i = 0; i < version_len; ++i)
            line[pos++] = version[i];
        line[pos++] = '\n';
        line[pos] = '\0';

        if (utility_emit(write, context, line, pos) != 0)
            return XYRIS_UTILITY_IO_ERROR;
    }

    if (count == 0u)
        return utility_emit_text(write, context, "No services registered.\n");

    return 0;
}

int xyris_utility_service(
    const char *name,
    xyris_user_write_fn write,
    void *context)
{
    xyris_service_descriptor_t descriptor;
    char version[24];
    char line[128];
    size_t name_len;
    size_t version_len;
    size_t pos = 0;

    if (name == NULL || name[0] == '\0')
        return XYRIS_UTILITY_INVALID_ARGUMENT;

    if (xyris_service_registry_lookup(name, &descriptor) != 0)
        return XYRIS_UTILITY_NOT_FOUND;

    if (utility_u64_to_text(
            descriptor.version,
            version,
            sizeof(version)) == 0)
        return XYRIS_UTILITY_IO_ERROR;

    name_len = utility_strlen(descriptor.name);
    version_len = utility_strlen(version);

    if (name_len + version_len + 4u >= sizeof(line))
        return XYRIS_UTILITY_IO_ERROR;

    for (size_t i = 0; i < name_len; ++i)
        line[pos++] = descriptor.name[i];

    line[pos++] = ' ';
    line[pos++] = 'v';

    for (size_t i = 0; i < version_len; ++i)
        line[pos++] = version[i];

    line[pos++] = '\n';
    line[pos] = '\0';

    return utility_emit(write, context, line, pos);
}

int xyris_utility_system(
    xyris_user_write_fn write,
    void *context)
{
    xyris_service_descriptor_t descriptor;
    char version[24];
    char line[128];
    size_t pos = 0;

    if (xyris_service_registry_lookup(
            XYRIS_SYSTEM_SERVICE_NAME,
            &descriptor) != 0)
        return XYRIS_UTILITY_NOT_FOUND;

    if (utility_u64_to_text(
            descriptor.version,
            version,
            sizeof(version)) == 0)
        return XYRIS_UTILITY_IO_ERROR;

    if (utility_strlen(XYRIS_SYSTEM_SERVICE_NAME) +
        utility_strlen(version) + 12u >= sizeof(line))
        return XYRIS_UTILITY_IO_ERROR;

    for (size_t i = 0; XYRIS_SYSTEM_SERVICE_NAME[i] != '\0'; ++i)
        line[pos++] = XYRIS_SYSTEM_SERVICE_NAME[i];

    line[pos++] = ' ';
    line[pos++] = 'v';

    for (size_t i = 0; version[i] != '\0'; ++i)
        line[pos++] = version[i];

    line[pos++] = ' ';
    line[pos++] = 'r';
    line[pos++] = 'e';
    line[pos++] = 'g';
    line[pos++] = 'i';
    line[pos++] = 's';
    line[pos++] = 't';
    line[pos++] = 'e';
    line[pos++] = 'r';
    line[pos++] = 'e';
    line[pos++] = 'd';
    line[pos++] = '\n';
    line[pos] = '\0';

    return utility_emit(write, context, line, pos);
}

int xyris_utility_pid(
    xyris_user_write_fn write,
    void *context)
{
    char pid[24];
    xyris_pid_t value = xyris_process_get_pid();

    if (value == XYRIS_INVALID_PID)
        return XYRIS_UTILITY_IO_ERROR;

    if (utility_u64_to_text(value, pid, sizeof(pid)) == 0)
        return XYRIS_UTILITY_IO_ERROR;

    if (utility_emit_text(write, context, pid) != 0)
        return XYRIS_UTILITY_IO_ERROR;

    return utility_emit_text(write, context, "\n");
}

int xyris_utility_cat(
    const char *path,
    xyris_user_write_fn write,
    void *context)
{
    char buffer[256];
    xyris_fd_t fd;

    if (path == NULL || path[0] == '\0')
        return XYRIS_UTILITY_INVALID_ARGUMENT;

    fd = (xyris_fd_t)xyris_file_open(path);
    if (!xyris_fd_valid(fd))
        return XYRIS_UTILITY_NOT_FOUND;

    for (;;)
    {
        xyris_syscall_result_t result =
            xyris_file_read(fd, buffer, sizeof(buffer));

        if (result < 0)
        {
            (void)xyris_file_close(fd);
            return XYRIS_UTILITY_IO_ERROR;
        }

        if (result == 0)
            break;

        if (utility_emit(write, context, buffer, (size_t)result) != 0)
        {
            (void)xyris_file_close(fd);
            return XYRIS_UTILITY_IO_ERROR;
        }
    }

    if (xyris_file_close(fd) != XYRIS_OK)
        return XYRIS_UTILITY_IO_ERROR;

    return XYRIS_UTILITY_OUTPUT_OK;
}
