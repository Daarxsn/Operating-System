#ifndef XYRIS_USERSPACE_UTILITIES_H
#define XYRIS_USERSPACE_UTILITIES_H

#include <stddef.h>

#include <xyris/core.h>

#include "../common/io.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_UTILITY_OUTPUT_OK 0
#define XYRIS_UTILITY_INVALID_ARGUMENT (-1)
#define XYRIS_UTILITY_NOT_FOUND (-2)
#define XYRIS_UTILITY_IO_ERROR (-3)

int xyris_utility_echo(
    size_t argc,
    const char *const *argv,
    xyris_user_write_fn write,
    void *context
);

int xyris_utility_services(
    xyris_user_write_fn write,
    void *context
);

int xyris_utility_service(
    const char *name,
    xyris_user_write_fn write,
    void *context
);

int xyris_utility_system(
    xyris_user_write_fn write,
    void *context
);

int xyris_utility_pid(
    xyris_user_write_fn write,
    void *context
);

int xyris_utility_cat(
    const char *path,
    xyris_user_write_fn write,
    void *context
);

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_USERSPACE_UTILITIES_H */
