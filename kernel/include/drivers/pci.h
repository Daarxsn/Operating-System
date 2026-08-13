#ifndef XK_PCI_DRIVER_H
#define XK_PCI_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#include "drivers/driver.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

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
    uint8_t header_type;

} XKPCIDevice;

/* Driver */

bool xk_pci_initialize(void);
void xk_pci_shutdown(void);

/* Configuration Space */

uint32_t xk_pci_read32(
    uint8_t bus,
    uint8_t device,
    uint8_t function,
    uint8_t offset);

uint16_t xk_pci_vendor_id(
    uint8_t bus,
    uint8_t device,
    uint8_t function);

void xk_pci_scan(void);

uint32_t xk_pci_device_count(void);
const XKPCIDevice *xk_pci_device_get(uint32_t index);

extern XKDriver xk_pci_driver;

#endif