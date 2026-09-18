#ifndef XYRIS_USERSPACE_IO_H
#define XYRIS_USERSPACE_IO_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*xyris_user_write_fn)(
    const char *data,
    size_t length,
    void *context
);

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_USERSPACE_IO_H */
