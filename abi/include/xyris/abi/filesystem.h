#ifndef XYRIS_ABI_FILESYSTEM_H
#define XYRIS_ABI_FILESYSTEM_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_FILE_TYPE_UNKNOWN   0u
#define XYRIS_FILE_TYPE_REGULAR   1u
#define XYRIS_FILE_TYPE_DIRECTORY 2u
#define XYRIS_FILE_TYPE_DEVICE    3u
#define XYRIS_FILE_TYPE_SYMLINK   4u

#define XYRIS_FILE_FLAG_READ     (1u << 0)
#define XYRIS_FILE_FLAG_WRITE    (1u << 1)
#define XYRIS_FILE_FLAG_CREATE   (1u << 2)
#define XYRIS_FILE_FLAG_TRUNCATE (1u << 3)

#define XYRIS_PATH_MAX 256u
#define XYRIS_DIRECTORY_NAME_MAX 256u

typedef struct xyris_file_info {
    xyris_abi_header_t header;
    xyris_fd_t fd;
    xyris_u32 type;
    xyris_u32 flags;
    xyris_u32 reserved0;
    xyris_size_t size;
    xyris_offset_t offset;
    xyris_object_id_t object;
} xyris_file_info_t;

typedef struct xyris_directory_entry {
    xyris_abi_header_t header;
    xyris_object_id_t object;
    xyris_u32 type;
    xyris_u32 reserved0;
    char name[XYRIS_DIRECTORY_NAME_MAX];
} xyris_directory_entry_t;

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_ABI_FILESYSTEM_H */
