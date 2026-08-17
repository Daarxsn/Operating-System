#include "drivers/driver.h"

#include <stddef.h>
#include <string.h>
#include "debug/print.h"

static XKDriver *driver_table[XK_MAX_DRIVERS];

static bool driver_name_valid(const char *name)
{
    return name != NULL && name[0] != '\0';
}

void xk_driver_manager_init(void)
{
    for (uint32_t i = 0; i < XK_MAX_DRIVERS; i++)
        driver_table[i] = NULL;
}

bool xk_driver_register(XKDriver *driver)
{
    if (driver == NULL || !driver_name_valid(driver->name))
        return false;

    if (xk_driver_find(driver->name) != NULL)
        return false;

    for (uint32_t i = 0; i < XK_MAX_DRIVERS; i++)
    {
        if (driver_table[i] == NULL)
        {
            driver->state = XK_DRIVER_REGISTERED;
            driver_table[i] = driver;
            return true;
        }
    }

    return false;
}

XKDriver *xk_driver_find(const char *name)
{
    if (!driver_name_valid(name))
        return NULL;

    for (uint32_t i = 0; i < XK_MAX_DRIVERS; i++)
    {
        if (driver_table[i] != NULL &&
            strcmp(driver_table[i]->name, name) == 0)
            return driver_table[i];
    }

    return NULL;
}

void xk_driver_initialize_all(void)
{
    for (uint32_t i = 0; i < XK_MAX_DRIVERS; i++)
    {
        XKDriver *driver = driver_table[i];
        if (driver == NULL)
            continue;

        if (driver->state == XK_DRIVER_RUNNING)
            continue;

        if (driver->initialize == NULL)
        {
            /* A registration-only driver is valid and ready to use. */
            driver->state = XK_DRIVER_RUNNING;
            continue;
        }

        driver->state = XK_DRIVER_INITIALIZED;

        if (driver->initialize())
            driver->state = XK_DRIVER_RUNNING;
        else
            driver->state = XK_DRIVER_FAILED;
    }
}

void xk_driver_shutdown_all(void)
{
    for (uint32_t i = 0; i < XK_MAX_DRIVERS; i++)
    {
        XKDriver *driver = driver_table[i];
        if (driver == NULL)
            continue;

        if (driver->shutdown != NULL &&
            driver->state != XK_DRIVER_UNINITIALIZED &&
            driver->state != XK_DRIVER_REGISTERED)
        {
            driver->shutdown();
        }

        driver->state = XK_DRIVER_UNINITIALIZED;
    }
}

bool xk_driver_unregister(const char *name)
{
    if (!driver_name_valid(name))
        return false;

    for (uint32_t i = 0; i < XK_MAX_DRIVERS; i++)
    {
        XKDriver *driver = driver_table[i];
        if (driver == NULL || strcmp(driver->name, name) != 0)
            continue;

        if (driver->shutdown != NULL &&
            driver->state != XK_DRIVER_UNINITIALIZED &&
            driver->state != XK_DRIVER_REGISTERED)
        {
            driver->shutdown();
        }

        driver->state = XK_DRIVER_UNINITIALIZED;
        driver_table[i] = NULL;
        return true;
    }

    return false;
}

uint32_t xk_driver_count(void)
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < XK_MAX_DRIVERS; i++)
        if (driver_table[i] != NULL)
            count++;
    return count;
}

XKDriver *xk_driver_get(uint32_t index)
{
    if (index >= XK_MAX_DRIVERS)
        return NULL;
    return driver_table[index];
}

void xk_driver_dump(void)
{
    for (uint32_t i = 0; i < XK_MAX_DRIVERS; i++)
    {
        if (driver_table[i] == NULL)
            continue;
        debug_print("Driver: ");
        debug_print_line(driver_table[i]->name);
    }
}

uint32_t xk_driver_count_type(XKDriverType type)
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < XK_MAX_DRIVERS; i++)
        if (driver_table[i] != NULL && driver_table[i]->type == type)
            count++;
    return count;
}

uint32_t xk_driver_count_state(XKDriverState state)
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < XK_MAX_DRIVERS; i++)
        if (driver_table[i] != NULL && driver_table[i]->state == state)
            count++;
    return count;
}
