#ifndef XYRIS_FOUNDATION_UKOM_H
#define XYRIS_FOUNDATION_UKOM_H

/*
 * ============================================================
 * XyrisOS Kernel
 * Universal Kernel Object Manager (UKOM)
 * ============================================================
 */

#include <stdint.h>
#include <stdbool.h>

#define XKOBJECT_MAX_OBJECTS 256

/*
 * ============================================================
 * Object Types
 * ============================================================
 */

typedef enum
{
    XK_OBJECT_NONE = 0,

    XK_OBJECT_PROCESS,
    XK_OBJECT_THREAD,

    XK_OBJECT_DRIVER,
    XK_OBJECT_DEVICE,

    XK_OBJECT_TIMER,
    XK_OBJECT_EVENT,

    XK_OBJECT_RESOURCE,

    XK_OBJECT_CUSTOM

} XKObjectType;

/*
 * ============================================================
 * Object States
 * ============================================================
 */

typedef enum
{
    XK_STATE_CREATED = 0,

    XK_STATE_ACTIVE,

    XK_STATE_SUSPENDED,

    XK_STATE_DESTROYED

} XKObjectState;

/*
 * ============================================================
 * Universal Kernel Object
 * ============================================================
 */

typedef struct XKObject
{
    uint64_t id;

    XKObjectType type;

    XKObjectState state;

    uint32_t ref_count;

    void *data;

} XKObject;

/*
 * ============================================================
 * API
 * ============================================================
 */

void xkobject_init(void);

XKObject *xkobject_create(XKObjectType type);

void xkobject_destroy(uint64_t id);

XKObject *xkobject_find(uint64_t id);

bool xkobject_exists(uint64_t id);

void xkobject_retain(XKObject *object);

void xkobject_release(XKObject *object);

#endif