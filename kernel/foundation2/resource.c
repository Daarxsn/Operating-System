#include "foundation/resource.h"

#include <stddef.h>

/* ============================================================
 * XyrisOS Resource Manager
 * ============================================================
 */

static XKResource resource_table[XK_RESOURCE_MAX_ENTRIES];
static uint64_t resource_counter = 1;

/* ------------------------------------------------------------
 * Internal Helpers
 * ------------------------------------------------------------ */

static XKResource *find_resource(uint64_t id)
{
    for (uint32_t i = 0; i < XK_RESOURCE_MAX_ENTRIES; i++)
    {
        if (resource_table[i].active &&
            resource_table[i].id == id)
        {
            return &resource_table[i];
        }
    }

    return NULL;
}

static XKResource *allocate_resource(void)
{
    for (uint32_t i = 0; i < XK_RESOURCE_MAX_ENTRIES; i++)
    {
        if (!resource_table[i].active)
        {
            return &resource_table[i];
        }
    }

    return NULL;
}

static uint64_t next_resource_id(void)
{
    return resource_counter++;
}

/* ------------------------------------------------------------
 * Initialization
 * ------------------------------------------------------------ */

void xk_resource_init(void)
{
    for (uint32_t i = 0; i < XK_RESOURCE_MAX_ENTRIES; i++)
    {
        resource_table[i].active = false;
    }

    resource_counter = 1;
}

/* ------------------------------------------------------------
 * Register Resource
 * ------------------------------------------------------------ */

uint64_t xk_resource_register(
    XKResourceType type,
    void *object,
    uint64_t owner)
{
    XKResource *resource = allocate_resource();

    if (resource == NULL)
    {
        return 0;
    }

    resource->id = next_resource_id();
    resource->type = type;
    resource->state = XK_RESOURCE_CREATED;
    resource->owner = owner;
    resource->flags = 0;
    resource->references = 1;
    resource->object = object;
    resource->active = true;

    return resource->id;
}

/* ------------------------------------------------------------
 * Unregister Resource
 * ------------------------------------------------------------ */

bool xk_resource_unregister(uint64_t id)
{
    XKResource *resource = find_resource(id);

    if (resource == NULL)
    {
        return false;
    }

    resource->active = false;

    return true;
}

/* ------------------------------------------------------------
 * Get Resource
 * ------------------------------------------------------------ */

XKResource *xk_resource_get(uint64_t id)
{
    return find_resource(id);
}

/* ------------------------------------------------------------
 * Set Resource State
 * ------------------------------------------------------------ */

bool xk_resource_set_state(
    uint64_t id,
    XKResourceState state)
{
    XKResource *resource = find_resource(id);

    if (resource == NULL)
    {
        return false;
    }

    resource->state = state;

    return true;
}

/* ------------------------------------------------------------
 * Debug Dump
 * ------------------------------------------------------------ */

void xk_resource_dump(void)
{
    /*
     * Future implementation:
     * Iterate through the resource table and
     * print active resources to the kernel logger.
     */
}
