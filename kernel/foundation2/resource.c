#include "foundation/resource.h"

#include <stddef.h>
#include "debug/print.h"
#include "debug/hex.h"

/* ============================================================
 * XyrisOS Resource Manager
 * ============================================================
 */

static XKResource resource_table[XK_RESOURCE_MAX_ENTRIES];
static uint64_t resource_counter = 1;
static uint32_t resource_count = 0;

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
    resource_count = 0;
}

/* ------------------------------------------------------------
 * Register Resource
 * ------------------------------------------------------------ */

uint64_t xk_resource_register(
    XKResourceType type,
    void *object,
    uint64_t owner)
{
    if (type == XK_RESOURCE_NONE)
        return 0;

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
    resource_count++;

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
    resource->state = XK_RESOURCE_DESTROYED;
    resource->object = NULL;
    resource->references = 0;
    if (resource_count > 0)
        resource_count--;

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

    if (resource == NULL ||
        state == XK_RESOURCE_UNUSED ||
        state == XK_RESOURCE_DESTROYED)
    {
        return false;
    }

    resource->state = state;
    return true;
}

bool xk_resource_retain(uint64_t id)
{
    XKResource *resource = find_resource(id);
    if (resource == NULL || resource->references == UINT32_MAX)
        return false;

    resource->references++;
    return true;
}

bool xk_resource_release(uint64_t id)
{
    XKResource *resource = find_resource(id);
    if (resource == NULL || resource->references == 0)
        return false;

    resource->references--;
    if (resource->references == 0)
    {
        resource->active = false;
        resource->state = XK_RESOURCE_DESTROYED;
        resource->object = NULL;
        if (resource_count > 0)
            resource_count--;
    }

    return true;
}

/* ------------------------------------------------------------
 * Debug Dump
 * ------------------------------------------------------------ */

void xk_resource_dump(void)
{
    for (uint32_t i = 0; i < XK_RESOURCE_MAX_ENTRIES; i++)
    {
        if (!resource_table[i].active)
            continue;
        debug_print("Resource ID: ");
        debug_print_hex64(resource_table[i].id);
        debug_print_line("");
    }
}

uint32_t xk_resource_count(void)
{
    return resource_count;
}

uint32_t xk_resource_count_owned_by(uint64_t owner)
{
    uint32_t count = 0;

    for (uint32_t i = 0; i < XK_RESOURCE_MAX_ENTRIES; i++)
    {
        if (resource_table[i].active &&
            resource_table[i].owner == owner)
        {
            count++;
        }
    }

    return count;
}
