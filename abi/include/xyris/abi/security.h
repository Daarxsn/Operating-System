#ifndef XYRIS_ABI_SECURITY_H
#define XYRIS_ABI_SECURITY_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_SECURITY_RIGHT_READ     (1ull << 0)
#define XYRIS_SECURITY_RIGHT_WRITE    (1ull << 1)
#define XYRIS_SECURITY_RIGHT_EXECUTE  (1ull << 2)
#define XYRIS_SECURITY_RIGHT_SIGNAL   (1ull << 3)
#define XYRIS_SECURITY_RIGHT_TRANSFER (1ull << 4)
#define XYRIS_SECURITY_RIGHT_ADMIN    (1ull << 5)

typedef struct xyris_security_identity {
    xyris_abi_header_t header;
    xyris_u64 identity;
    xyris_u64 group;
    xyris_u64 flags;
} xyris_security_identity_t;

typedef struct xyris_security_policy {
    xyris_abi_header_t header;
    xyris_capability_t capability;
    xyris_object_id_t object;
    xyris_u64 allow;
    xyris_u64 deny;
} xyris_security_policy_t;

#ifdef __cplusplus
}
#endif

#endif /* XYRIS_ABI_SECURITY_H */
