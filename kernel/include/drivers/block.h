#ifndef XK_BLOCK_DRIVER_H
#define XK_BLOCK_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#define XK_BLOCK_DEVICE_NAME_MAX 32

typedef struct XKBlockDevice XKBlockDevice;

/*
 * Block-device operations.
 *
 * Storage drivers such as SATA, NVMe and USB storage
 * implement these operations behind this common interface.
 */
typedef struct
{
    int (*initialize)(XKBlockDevice *device);
    int (*shutdown)(XKBlockDevice *device);

    int (*read)(
        XKBlockDevice *device,
        uint64_t sector,
        uint32_t count,
        void *buffer
    );

    int (*write)(
        XKBlockDevice *device,
        uint64_t sector,
        uint32_t count,
        const void *buffer
    );
} XKBlockOperations;

/*
 * Generic block device.
 */
struct XKBlockDevice
{
    char name[XK_BLOCK_DEVICE_NAME_MAX];

    uint32_t sector_size;
    uint64_t sector_count;

    bool read_only;
    bool initialized;

    const XKBlockOperations *operations;
    void *private_data;
};

/*
 * Initialize the block-device subsystem.
 */
bool xk_block_initialize(void);

/*
 * Shut down all registered block devices.
 */
void xk_block_shutdown(void);

/*
 * Register a block device.
 */
int xk_block_register(XKBlockDevice *device);

/*
 * Unregister a block device.
 */
int xk_block_unregister(XKBlockDevice *device);

/*
 * Return the number of registered block devices.
 */
uint32_t xk_block_device_count(void);

/*
 * Return a registered block device by index.
 */
XKBlockDevice *xk_block_device_get(uint32_t index);

/*
 * Read sectors from a block device.
 */
int xk_block_read(
    XKBlockDevice *device,
    uint64_t sector,
    uint32_t count,
    void *buffer
);

/*
 * Write sectors to a block device.
 */
int xk_block_write(
    XKBlockDevice *device,
    uint64_t sector,
    uint32_t count,
    const void *buffer
);

#endif