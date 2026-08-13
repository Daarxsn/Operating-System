#ifndef XK_DRIVER_MANAGER_H
#define XK_DRIVER_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 * XyrisOS Driver Manager
 * ============================================================
 */

#define XK_MAX_DRIVERS 64

typedef enum
{
    XK_DRIVER_UNINITIALIZED = 0,
    XK_DRIVER_REGISTERED,
    XK_DRIVER_INITIALIZED,
    XK_DRIVER_RUNNING,
    XK_DRIVER_FAILED

} XKDriverState;

typedef enum
{
    XK_DRIVER_UNKNOWN = 0,

    XK_DRIVER_KEYBOARD,
    XK_DRIVER_MOUSE,
    XK_DRIVER_SERIAL,

    XK_DRIVER_STORAGE,
    XK_DRIVER_NETWORK,

    XK_DRIVER_GRAPHICS,

    XK_DRIVER_AUDIO,

    XK_DRIVER_CUSTOM

} XKDriverType;

typedef struct XKDriver
{
    const char *name;

    XKDriverType type;

    XKDriverState state;

    bool (*initialize)(void);

    void (*shutdown)(void);

} XKDriver;

/* Driver Manager */

void xk_driver_manager_init(void);

bool xk_driver_register(
    XKDriver *driver);

XKDriver *xk_driver_find(
    const char *name);

void xk_driver_initialize_all(void);

void xk_driver_shutdown_all(void);


/* ------------------------------------------------------------
 * Driver Registration API
 * ------------------------------------------------------------ */

bool xk_driver_unregister(
    const char *name);



uint32_t xk_driver_count(void);

XKDriver *xk_driver_get(
    uint32_t index);

void xk_driver_dump(void);

#endif