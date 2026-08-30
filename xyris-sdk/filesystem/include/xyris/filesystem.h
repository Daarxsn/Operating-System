#ifndef XYRIS_FILESYSTEM_H
#define XYRIS_FILESYSTEM_H

/*
 * Xyris SDK Filesystem API v0.1.
 *
 * This module wraps only the filesystem-related operations assigned by the
 * public Xyris System ABI v0.1: open, close, read, and write.
 */

#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline xyris_bool_t xyris_fd_valid(xyris_fd_t fd)
{
    return fd >= 0 ? XYRIS_TRUE : XYRIS_FALSE;
}

static inline xyris_syscall_result_t xyris_file_open(const char *path)
{
    return xyris_open(path);
}

static inline xyris_syscall_result_t xyris_file_close(xyris_fd_t fd)
{
    return xyris_close(fd);
}

static inline xyris_syscall_result_t xyris_file_read(
    xyris_fd_t fd,
    void *buffer,
    xyris_size_t size)
{
    return xyris_read(fd, buffer, size);
}

static inline xyris_syscall_result_t xyris_file_write(
    xyris_fd_t fd,
    const void *buffer,
    xyris_size_t size)
{
    return xyris_write(fd, buffer, size);
}

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_FILESYSTEM_H */
