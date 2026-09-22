#include "drivers/block.h"

#include <stddef.h>

#define XK_BLOCK_MAX_DEVICES 32

static XKBlockDevice *block_devices[XK_BLOCK_MAX_DEVICES];
static uint32_t block_device_count;


/*
 * Initialize the block-device subsystem.
 */
bool xk_block_initialize(void)
{
    block_device_count = 0;

    for (uint32_t i = 0; i < XK_BLOCK_MAX_DEVICES; i++)
        block_devices[i] = NULL;

    return true;
}


/*
 * Shut down all registered block devices.
 */
void xk_block_shutdown(void)
{
    for (uint32_t i = 0; i < block_device_count; i++)
    {
        XKBlockDevice *device =
            block_devices[i];

        if (device == NULL)
            continue;

        if (device->initialized &&
            device->operations != NULL &&
            device->operations->shutdown != NULL)
        {
            device->operations->shutdown(device);
        }

        device->initialized = false;
        block_devices[i] = NULL;
    }

    block_device_count = 0;
}


/*
 * Register a block device.
 */
int xk_block_register(XKBlockDevice *device)
{
    if (device == NULL)
        return -1;

    if (device->operations == NULL)
        return -1;

    if (device->sector_size == 0)
        return -1;

    if (device->sector_count == 0)
        return -1;

    if (block_device_count >= XK_BLOCK_MAX_DEVICES)
        return -1;

    /*
     * Prevent the same device from being registered twice.
     */
    for (uint32_t i = 0; i < block_device_count; i++)
    {
        if (block_devices[i] == device)
            return -1;
    }

    if (device->operations->initialize != NULL)
    {
        if (device->operations->initialize(device) != 0)
            return -1;
    }

    device->initialized = true;
    block_devices[block_device_count++] = device;

    return 0;
}


/*
 * Unregister a block device.
 */
int xk_block_unregister(XKBlockDevice *device)
{
    if (device == NULL)
        return -1;

    for (uint32_t i = 0; i < block_device_count; i++)
    {
        if (block_devices[i] != device)
            continue;

        if (device->initialized &&
            device->operations != NULL &&
            device->operations->shutdown != NULL)
        {
            if (device->operations->shutdown(device) != 0)
                return -1;
        }

        device->initialized = false;

        block_devices[i] =
            block_devices[block_device_count - 1];

        block_devices[block_device_count - 1] = NULL;

        block_device_count--;

        return 0;
    }

    return -1;
}


/*
 * Return the number of registered block devices.
 */
uint32_t xk_block_device_count(void)
{
    return block_device_count;
}


/*
 * Return a registered block device.
 */
XKBlockDevice *xk_block_device_get(uint32_t index)
{
    if (index >= block_device_count)
        return NULL;

    return block_devices[index];
}


/*
 * Read sectors from a block device.
 */
int xk_block_read(
    XKBlockDevice *device,
    uint64_t sector,
    uint32_t count,
    void *buffer)
{
    if (device == NULL ||
        buffer == NULL ||
        count == 0 ||
        !device->initialized ||
        device->operations == NULL ||
        device->operations->read == NULL)
    {
        return -1;
    }

    if (sector >= device->sector_count)
        return -1;

    if ((uint64_t)count >
        device->sector_count - sector)
    {
        return -1;
    }

    return device->operations->read(
        device,
        sector,
        count,
        buffer
    );
}


/*
 * Write sectors to a block device.
 */
int xk_block_write(
    XKBlockDevice *device,
    uint64_t sector,
    uint32_t count,
    const void *buffer)
{
    if (device == NULL ||
        buffer == NULL ||
        count == 0 ||
        !device->initialized ||
        device->operations == NULL ||
        device->operations->write == NULL)
    {
        return -1;
    }

    if (device->read_only)
        return -1;

    if (sector >= device->sector_count)
        return -1;

    if ((uint64_t)count >
        device->sector_count - sector)
    {
        return -1;
    }

    if (device->operations->write == NULL)
        return -1;

    return device->operations->write(
        device,
        sector,
        count,
        buffer
    );
}