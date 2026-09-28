    #ifndef XYRIS_HARDWARE_API_H
#define XYRIS_HARDWARE_API_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware_inventory.h"
#include "pci_inventory.h"

/*
 * Initialize the Xyris hardware abstraction layer.
 *
 * This initializes the common hardware inventory used
 * by higher-level kernel and SDK services.
 */
void xk_hardware_init(void);

/*
 * Return a snapshot of detected hardware information.
 */
hardware_inventory_t xk_hardware_info(void);

/*
 * Return individual hardware information.
 */
uint32_t xk_hardware_cpu_count(void);

uint64_t xk_hardware_total_memory(void);
uint64_t xk_hardware_usable_memory(void);

uint32_t xk_hardware_display_width(void);
uint32_t xk_hardware_display_height(void);
uint32_t xk_hardware_display_pitch(void);

uint32_t xk_hardware_pci_device_count(void);

/*
 * Hardware availability status.
 *
 * Returns true when at least one device of the
 * corresponding category was detected.
 */
bool xk_hardware_usb_available(void);
bool xk_hardware_storage_available(void);
bool xk_hardware_network_available(void);
bool xk_hardware_audio_available(void);
uint32_t xk_hardware_audio_device_count(void);

/*
 * Return the number of registered storage devices.
 *
 * This reflects block devices successfully registered
 * with the kernel storage subsystem.
 */
uint32_t xk_hardware_storage_device_count(void);

/*
 * Return the number of discovered hardware devices.
 *
 * Currently backed by the PCI hardware inventory.
 */
uint32_t xk_hardware_device_count(void);

/*
 * Return information about one discovered hardware device.
 *
 * Returns 1 when the device exists and 0 when the
 * requested index is invalid or the output pointer is NULL.
 */
int xk_hardware_device_get(
    uint32_t index,
    pci_inventory_device_t *info
);

#endif