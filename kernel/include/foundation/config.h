#ifndef XK_FOUNDATION_CONFIG_H
#define XK_FOUNDATION_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 * XyrisOS Configuration Manager
 *
 * Responsible for storing kernel configuration values.
 * ============================================================
 */

#define XK_CONFIG_MAX_ENTRIES 64
#define XK_CONFIG_KEY_LENGTH 32

typedef struct
{
    bool active;

    char key[XK_CONFIG_KEY_LENGTH];

    uint64_t value;

} XKConfigEntry;

/* ------------------------------------------------------------
 * Configuration Manager
 * ------------------------------------------------------------ */

void xk_config_init(void);

bool xk_config_set(
    const char *key,
    uint64_t value);

bool xk_config_get(
    const char *key,
    uint64_t *value);

#endif