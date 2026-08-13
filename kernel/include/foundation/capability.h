#ifndef XYRIS_FOUNDATION_CAPABILITY_H
#define XYRIS_FOUNDATION_CAPABILITY_H

#include <stdint.h>
#include <stdbool.h>

/*
 * ============================================================
 * XyrisOS Capability Manager
 * ------------------------------------------------------------
 * Defines kernel object capabilities and permission management.
 * Every capability is represented as a bit inside a 64-bit mask.
 * ============================================================
 */

/* Maximum capability entries */
#define XK_CAPABILITY_MAX_ENTRIES 256

/*
 * Capability Flags
 * ------------------------------------------------------------
 * Multiple capabilities can be combined using bitwise OR.
 */

typedef enum
{
    XK_CAP_NONE            = 0,

    XK_CAP_READ            = (1ULL << 0),
    XK_CAP_WRITE           = (1ULL << 1),
    XK_CAP_EXECUTE         = (1ULL << 2),

    XK_CAP_CREATE          = (1ULL << 3),
    XK_CAP_DELETE          = (1ULL << 4),
    XK_CAP_MODIFY          = (1ULL << 5),

    XK_CAP_MEMORY          = (1ULL << 6),
    XK_CAP_PROCESS         = (1ULL << 7),
    XK_CAP_THREAD          = (1ULL << 8),

    XK_CAP_DRIVER          = (1ULL << 9),
    XK_CAP_DEVICE          = (1ULL << 10),
    XK_CAP_FILESYSTEM      = (1ULL << 11),

    XK_CAP_NETWORK         = (1ULL << 12),
    XK_CAP_INTERRUPT       = (1ULL << 13),
    XK_CAP_KERNEL          = (1ULL << 14)

} XKCapability;


/*
 * Capability Entry
 * ------------------------------------------------------------
 * Stores permissions assigned to a kernel object.
 */

typedef struct
{
    uint64_t object_id;
    uint64_t permissions;
    bool active;

} XKCapabilityEntry;


/*
 * ============================================================
 * Public API
 * ============================================================
 */

void xk_capability_init(void);

bool xk_capability_grant(
    uint64_t object_id,
    XKCapability capability);

bool xk_capability_revoke(
    uint64_t object_id,
    XKCapability capability);

bool xk_capability_check(
    uint64_t object_id,
    XKCapability capability);

void xk_capability_clear(
    uint64_t object_id);

#endif /* XYRIS_FOUNDATION_CAPABILITY_H */