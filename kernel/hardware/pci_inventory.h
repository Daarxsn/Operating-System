#ifndef XYRIS_PCI_INVENTORY_H
#define XYRIS_PCI_INVENTORY_H

#include <stdint.h>

typedef enum
{
    PCI_HARDWARE_OTHER = 0,
    PCI_HARDWARE_STORAGE,
    PCI_HARDWARE_NETWORK,
    PCI_HARDWARE_DISPLAY,
    PCI_HARDWARE_MULTIMEDIA,
    PCI_HARDWARE_USB,
    PCI_HARDWARE_BRIDGE,
    PCI_HARDWARE_INPUT
} pci_hardware_class_t;

typedef struct
{
    uint32_t total_devices;

    uint32_t storage_devices;
    uint32_t network_devices;
    uint32_t display_devices;
    uint32_t multimedia_devices;
    uint32_t usb_devices;
    uint32_t bridge_devices;
    uint32_t input_devices;
    uint32_t other_devices;
} pci_inventory_t;

/*
 * Information about one discovered PCI device.
 */
typedef struct
{
    uint8_t bus;
    uint8_t device;
    uint8_t function;

    uint16_t vendor_id;
    uint16_t device_id;

    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;

    pci_hardware_class_t hardware_class;
} pci_inventory_device_t;

/*
 * Initialize the PCI hardware inventory.
 *
 * Uses the existing PCI enumerator and classifies
 * discovered devices using their PCI class/subclass.
 */
void pci_inventory_init(void);

/*
 * Return the current PCI hardware inventory.
 */
pci_inventory_t pci_inventory_info(void);

/*
 * Print a PCI hardware inventory diagnostic.
 */
void pci_inventory_dump(void);

/*
 * Convert a PCI class/subclass pair into
 * an XyrisOS hardware category.
 */
pci_hardware_class_t pci_hardware_classify(
    uint8_t class_code,
    uint8_t subclass
);

/*
 * Return information about a discovered PCI device.
 *
 * Returns 1 when the device exists and 0 when
 * the requested index is outside the inventory
 * or the output pointer is invalid.
 */
int pci_inventory_device_get(
    uint32_t index,
    pci_inventory_device_t *info
);

#endif