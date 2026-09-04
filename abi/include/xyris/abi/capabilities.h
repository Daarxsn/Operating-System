#ifndef XYRIS_ABI_CAPABILITIES_H
#define XYRIS_ABI_CAPABILITIES_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_CAP_READ      (1ull << 0)
#define XYRIS_CAP_WRITE     (1ull << 1)
#define XYRIS_CAP_EXECUTE   (1ull << 2)
#define XYRIS_CAP_DUPLICATE (1ull << 3)
#define XYRIS_CAP_TRANSFER  (1ull << 4)
#define XYRIS_CAP_CONTROL   (1ull << 5)
#define XYRIS_CAP_ADMIN     (1ull << 6)

typedef struct xyris_capability_info {
    xyris_abi_header_t header;
    xyris_capability_t capability;
    xyris_object_id_t object;
    xyris_u64 rights;
    xyris_u64 flags;
} xyris_capability_info_t;

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_ABI_CAPABILITIES_H */
