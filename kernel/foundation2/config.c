#include "foundation/config.h"

#include <stddef.h>
#include <string.h>

/* ============================================================
 * XyrisOS Configuration Manager
 * ============================================================
 */

static XKConfigEntry config_table[XK_CONFIG_MAX_ENTRIES];

/* ------------------------------------------------------------
 * Initialize
 * ------------------------------------------------------------ */

void xk_config_init(void)
{
    for (uint32_t i = 0; i < XK_CONFIG_MAX_ENTRIES; i++)
    {
        config_table[i].active = false;
    }
}

/* ------------------------------------------------------------
 * Set Configuration Value
 * ------------------------------------------------------------ */

bool xk_config_set(
    const char *key,
    uint64_t value)
{
    if (key == NULL || key[0] == '\0')
    {
        return false;
    }

    /* Reject keys that cannot be represented without truncation. */
    uint32_t length = 0;
    while (key[length] != '\0')
    {
        length++;
        if (length >= XK_CONFIG_KEY_LENGTH)
            return false;
    }

    /* Update existing key */

    for (uint32_t i = 0; i < XK_CONFIG_MAX_ENTRIES; i++)
    {
        if (config_table[i].active &&
            strcmp(config_table[i].key, key) == 0)
        {
            config_table[i].value = value;
            return true;
        }
    }

    /* Create new key */

    for (uint32_t i = 0; i < XK_CONFIG_MAX_ENTRIES; i++)
    {
        if (!config_table[i].active)
        {
            config_table[i].active = true;

            strncpy(
                config_table[i].key,
                key,
                XK_CONFIG_KEY_LENGTH - 1);

            config_table[i].key[XK_CONFIG_KEY_LENGTH - 1] = '\0';

            config_table[i].value = value;

            return true;
        }
    }

    return false;
}

/* ------------------------------------------------------------
 * Get Configuration Value
 * ------------------------------------------------------------ */

bool xk_config_get(
    const char *key,
    uint64_t *value)
{
    if (key == NULL || value == NULL)
    {
        return false;
    }

    for (uint32_t i = 0; i < XK_CONFIG_MAX_ENTRIES; i++)
    {
        if (config_table[i].active &&
            strcmp(config_table[i].key, key) == 0)
        {
            *value = config_table[i].value;
            return true;
        }
    }

    return false;
}

bool xk_config_remove(const char *key)
{
    if (key == NULL)
        return false;

    for (uint32_t i = 0; i < XK_CONFIG_MAX_ENTRIES; i++)
    {
        if (config_table[i].active &&
            strcmp(config_table[i].key, key) == 0)
        {
            config_table[i].active = false;
            config_table[i].key[0] = '\0';
            config_table[i].value = 0;
            return true;
        }
    }
    return false;
}

uint32_t xk_config_count(void)
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < XK_CONFIG_MAX_ENTRIES; i++)
        if (config_table[i].active)
            count++;
    return count;
}
