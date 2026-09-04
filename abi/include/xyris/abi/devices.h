#ifndef XYRIS_ABI_DEVICES_H
#define XYRIS_ABI_DEVICES_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_DEVICE_CLASS_UNKNOWN 0u
#define XYRIS_DEVICE_CLASS_CHAR    1u
#define XYRIS_DEVICE_CLASS_BLOCK   2u
#define XYRIS_DEVICE_CLASS_NET     3u
#define XYRIS_DEVICE_CLASS_INPUT   4u
#define XYRIS_DEVICE_CLASS_DISPLAY 5u

#define XYRIS_DEVICE_FLAG_READABLE  (1u << 0)
#define XYRIS_DEVICE_FLAG_WRITABLE  (1u << 1)
#define XYRIS_DEVICE_FLAG_REMOVABLE (1u << 2)

#define XYRIS_DEVICE_NAME_MAX 32u

typedef struct xyris_device_info {
    xyris_abi_header_t header;
    xyris_handle_t handle;
    xyris_object_id_t object;
    xyris_u32 class_id;
    xyris_u32 flags;
    xyris_u32 vendor_id;
    xyris_u32 device_id;
    char name[XYRIS_DEVICE_NAME_MAX];
} xyris_device_info_t;

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_ABI_DEVICES_H */
