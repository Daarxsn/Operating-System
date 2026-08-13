#ifndef XK_FOUNDATION_RESOURCE_H
#define XK_FOUNDATION_RESOURCE_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 * XyrisOS Resource Manager
 *
 * Responsible for registering, tracking and managing kernel
 * resources.
 * ============================================================
 */

#define XK_RESOURCE_MAX_ENTRIES 256

/* ------------------------------------------------------------
 * Resource Types
 * ------------------------------------------------------------ */

typedef enum
{
    XK_RESOURCE_NONE = 0,

    XK_RESOURCE_MEMORY,
    XK_RESOURCE_HEAP,

    XK_RESOURCE_PROCESS,
    XK_RESOURCE_THREAD,

    XK_RESOURCE_DRIVER,
    XK_RESOURCE_DEVICE,

    XK_RESOURCE_FILESYSTEM,

    XK_RESOURCE_NETWORK,

    XK_RESOURCE_TIMER,

    XK_RESOURCE_EVENT,

    XK_RESOURCE_IPC,

    XK_RESOURCE_FRAMEBUFFER,

    XK_RESOURCE_CUSTOM

} XKResourceType;

/* ------------------------------------------------------------
 * Resource States
 * ------------------------------------------------------------ */

typedef enum
{
    XK_RESOURCE_UNUSED = 0,

    XK_RESOURCE_CREATED,

    XK_RESOURCE_READY,

    XK_RESOURCE_RUNNING,

    XK_RESOURCE_WAITING,

    XK_RESOURCE_STOPPED,

    XK_RESOURCE_DESTROYED

} XKResourceState;

/* ------------------------------------------------------------
 * Resource Entry
 * ------------------------------------------------------------ */

typedef struct
{
    uint64_t id;

    XKResourceType type;

    XKResourceState state;

    uint64_t owner;

    uint32_t flags;

    uint32_t references;

    void *object;

    bool active;

} XKResource;

/* ------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------ */

void xk_resource_init(void);

uint64_t xk_resource_register(
    XKResourceType type,
    void *object,
    uint64_t owner);

bool xk_resource_unregister(
    uint64_t id);

XKResource *xk_resource_get(
    uint64_t id);

bool xk_resource_set_state(
    uint64_t id,
    XKResourceState state);

void xk_resource_dump(void);

#endif
