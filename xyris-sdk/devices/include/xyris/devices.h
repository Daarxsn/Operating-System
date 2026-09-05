#ifndef XYRIS_DEVICES_H
#define XYRIS_DEVICES_H

#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_SDK_DEVICES_VERSION_MAJOR 0u
#define XYRIS_SDK_DEVICES_VERSION_MINOR 1u

static inline xyris_bool_t xyris_device_handle_valid(xyris_handle_t handle)
{ return handle != XYRIS_INVALID_HANDLE ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_device_object_valid(xyris_object_id_t object)
{ return object != XYRIS_INVALID_OBJECT ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_device_class_valid(xyris_u32 class_id)
{ return class_id <= XYRIS_DEVICE_CLASS_DISPLAY ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_device_flags_valid(xyris_u32 flags)
{ const xyris_u32 known = XYRIS_DEVICE_FLAG_READABLE | XYRIS_DEVICE_FLAG_WRITABLE | XYRIS_DEVICE_FLAG_REMOVABLE; return (flags & ~known) == 0u ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_device_info_valid(const xyris_device_info_t *info)
{ return info && info->header.size >= sizeof(*info) && info->header.version != 0u && xyris_device_handle_valid(info->handle) && xyris_device_object_valid(info->object) && xyris_device_class_valid(info->class_id) && xyris_device_flags_valid(info->flags) ? XYRIS_TRUE : XYRIS_FALSE; }

static inline xyris_u32 xyris_device_count(void)
{ return (xyris_u32)xyris_syscall0(XYRIS_SYS_DEVICE_COUNT); }
static inline xyris_status_t xyris_device_get_info(xyris_u32 index, xyris_device_info_t *out)
{ return xyris_syscall2(XYRIS_SYS_DEVICE_INFO, index, (xyris_user_ptr_t)(uintptr_t)out); }

#ifdef __cplusplus
}
#endif
#endif
