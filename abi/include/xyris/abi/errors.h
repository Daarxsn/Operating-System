#ifndef XYRIS_ABI_ERRORS_H
#define XYRIS_ABI_ERRORS_H

#include "types.h"

/*
 * Xyris ABI v0.1 error namespace.
 *
 * Zero means success. Negative values identify failures. The numeric values
 * are Xyris-owned and are not intended to mirror another operating system's
 * errno namespace.
 */
typedef enum xyris_error {
    XYRIS_OK         = 0,
    XYRIS_EINVAL     = -1,
    XYRIS_EBADHANDLE = -2,
    XYRIS_ENOTFOUND  = -3,
    XYRIS_EPERM      = -4,
    XYRIS_EACCES     = -5,
    XYRIS_ENOMEM     = -6,
    XYRIS_EBUSY      = -7,
    XYRIS_EEXIST     = -8,
    XYRIS_ENOTSUP    = -9,
    XYRIS_EIO        = -10,
    XYRIS_EINTR      = -11,
    XYRIS_EAGAIN     = -12,
    XYRIS_EFAULT     = -13,
    XYRIS_EBADARG    = -14,
    XYRIS_EOVERFLOW  = -15,
    XYRIS_EBADSTATE  = -16,
    XYRIS_ENOSPC     = -17,
    XYRIS_ETIMEDOUT  = -18,
    XYRIS_ECONN      = -19,
    XYRIS_ECAP       = -20,
    XYRIS_ENOSYS     = -21
} xyris_error_t;

#endif /* XYRIS_ABI_ERRORS_H */
