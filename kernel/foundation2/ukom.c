#include "foundation/ukom.h"

/*
 * ============================================================
 * XyrisOS Universal Kernel Object Manager
 * ============================================================
 */

/* Global Object Registry */
static XKObject object_registry[XKOBJECT_MAX_OBJECTS];

/* Number of active objects */
static uint32_t object_count = 0;

/* Next unique object ID */
static uint64_t next_object_id = 1;


/* ------------------------------------------------------------
 * Initialize UKOM
 * ------------------------------------------------------------ */
void xkobject_init(void)
{
    object_count = 0;
    next_object_id = 1;

    for (uint32_t i = 0; i < XKOBJECT_MAX_OBJECTS; i++)
    {
        object_registry[i].id = 0;
        object_registry[i].type = XK_OBJECT_NONE;
        object_registry[i].state = XK_STATE_DESTROYED;
        object_registry[i].ref_count = 0;
        object_registry[i].data = 0;
    }
}


/* ------------------------------------------------------------
 * Create Object
 * ------------------------------------------------------------ */
XKObject *xkobject_create(XKObjectType type)
{
    if (object_count >= XKOBJECT_MAX_OBJECTS)
        return 0;

    for (uint32_t i = 0; i < XKOBJECT_MAX_OBJECTS; i++)
    {
        if (object_registry[i].id == 0)
        {
            object_registry[i].id = next_object_id++;
            object_registry[i].type = type;
            object_registry[i].state = XK_STATE_CREATED;
            object_registry[i].ref_count = 1;
            object_registry[i].data = 0;

            object_count++;

            return &object_registry[i];
        }
    }

    return 0;
}


/* ------------------------------------------------------------
 * Destroy Object
 * ------------------------------------------------------------ */
void xkobject_destroy(uint64_t id)
{
    for (uint32_t i = 0; i < XKOBJECT_MAX_OBJECTS; i++)
    {
        if (object_registry[i].id == id)
        {
            object_registry[i].id = 0;
            object_registry[i].type = XK_OBJECT_NONE;
            object_registry[i].state = XK_STATE_DESTROYED;
            object_registry[i].ref_count = 0;
            object_registry[i].data = 0;

            object_count--;

            return;
        }
    }
}


/* ------------------------------------------------------------
 * Find Object
 * ------------------------------------------------------------ */
XKObject *xkobject_find(uint64_t id)
{
    for (uint32_t i = 0; i < XKOBJECT_MAX_OBJECTS; i++)
    {
        if (object_registry[i].id == id)
        {
            return &object_registry[i];
        }
    }

    return 0;
}


/* ------------------------------------------------------------
 * Check Existence
 * ------------------------------------------------------------ */
bool xkobject_exists(uint64_t id)
{
    return (xkobject_find(id) != 0);
}


/* ------------------------------------------------------------
 * Increase Reference Count
 * ------------------------------------------------------------ */
void xkobject_retain(XKObject *object)
{
    if (object == 0)
        return;

    object->ref_count++;
}


/* ------------------------------------------------------------
 * Release Reference
 * ------------------------------------------------------------ */
void xkobject_release(XKObject *object)
{
    if (object == 0)
        return;

    if (object->ref_count > 0)
    {
        object->ref_count--;

        if (object->ref_count == 0)
        {
            xkobject_destroy(object->id);
        }
    }
}