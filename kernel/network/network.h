#ifndef XYRIS_NETWORK_H
#define XYRIS_NETWORK_H

#include <stdint.h>

#include "../hardware/pci_inventory.h"

/*
 * Generic network device information.
 *
 * This abstraction separates higher-level networking
 * code from the underlying PCI hardware.
 */
typedef struct
{
    uint32_t index;

    uint8_t bus;
    uint8_t device;
    uint8_t function;

    uint16_t vendor_id;
    uint16_t device_id;

    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;

} xk_network_device_t;

/*
 * Initialize the network hardware abstraction.
 */
void xk_network_init(void);

/*
 * Return the number of detected network controllers.
 */
uint32_t xk_network_device_count(void);

/*
 * Return information about a network controller.
 *
 * Returns 1 on success and 0 when the index is invalid.
 */
int xk_network_device_get(
    uint32_t index,
    xk_network_device_t *info
);

/*
 * Return whether a network controller exists.
 */
int xk_network_available(void);

#endif