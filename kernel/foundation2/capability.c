#include "foundation/capability.h"

#include <stddef.h>

/*
 * ============================================================
 * Capability Registry
 * ============================================================
 */

static XKCapabilityEntry capability_table[XK_CAPABILITY_MAX_ENTRIES];


/*
 * ============================================================
 * Internal Helpers
 * ============================================================
 */

static XKCapabilityEntry *find_entry(uint64_t object_id)
{
    for (size_t i = 0; i < XK_CAPABILITY_MAX_ENTRIES; i++)
    {
        if (capability_table[i].active &&
            capability_table[i].object_id == object_id)
        {
            return &capability_table[i];
        }
    }

    return NULL;
}

static XKCapabilityEntry *allocate_entry(uint64_t object_id)
{
    for (size_t i = 0; i < XK_CAPABILITY_MAX_ENTRIES; i++)
    {
        if (!capability_table[i].active)
        {
            capability_table[i].active = true;
            capability_table[i].object_id = object_id;
            capability_table[i].permissions = XK_CAP_NONE;

            return &capability_table[i];
        }
    }

    return NULL;
}


/*
 * ============================================================
 * Public API
 * ============================================================
 */

void xk_capability_init(void)
{
    for (size_t i = 0; i < XK_CAPABILITY_MAX_ENTRIES; i++)
    {
        capability_table[i].active = false;
        capability_table[i].object_id = 0;
        capability_table[i].permissions = XK_CAP_NONE;
    }
}

bool xk_capability_grant(
    uint64_t object_id,
    XKCapability capability)
{
    XKCapabilityEntry *entry = find_entry(object_id);

    if (entry == NULL)
    {
        entry = allocate_entry(object_id);

        if (entry == NULL)
        {
            return false;
        }
    }

    entry->permissions |= capability;

    return true;
}

bool xk_capability_revoke(
    uint64_t object_id,
    XKCapability capability)
{
    XKCapabilityEntry *entry = find_entry(object_id);

    if (entry == NULL)
    {
        return false;
    }

    entry->permissions &= ~capability;

    return true;
}

bool xk_capability_check(
    uint64_t object_id,
    XKCapability capability)
{
    XKCapabilityEntry *entry = find_entry(object_id);

    if (entry == NULL)
    {
        return false;
    }

    return (entry->permissions & capability) == capability;
}

void xk_capability_clear(uint64_t object_id)
{
    XKCapabilityEntry *entry = find_entry(object_id);

    if (entry == NULL)
    {
        return;
    }

    entry->permissions = XK_CAP_NONE;
    entry->active = false;
    entry->object_id = 0;
}