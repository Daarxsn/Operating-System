#ifndef XK_PCI_DRIVER_H
#define XK_PCI_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "drivers/driver.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC
#define XK_PCI_MAX_DEVICES 512
#define XK_PCI_MAX_BARS 6
#define XK_PCI_MAX_CAPABILITIES 48

typedef struct
{
    uint8_t id;
    uint8_t offset;
} XKPCICapability;

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
    uint16_t command;
    uint16_t status;
    uint8_t capability_pointer;
    uint32_t bars[XK_PCI_MAX_BARS];
    uint8_t bar_count;
    XKPCICapability capabilities[XK_PCI_MAX_CAPABILITIES];
    uint8_t capability_count;
} XKPCIDevice;

bool xk_pci_initialize(void);
void xk_pci_shutdown(void);
uint32_t xk_pci_read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint16_t xk_pci_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint8_t xk_pci_read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint16_t xk_pci_vendor_id(uint8_t bus, uint8_t device, uint8_t function);
void xk_pci_scan(void);
uint32_t xk_pci_device_count(void);
const XKPCIDevice *xk_pci_device_get(uint32_t index);
const XKPCICapability *xk_pci_find_capability(const XKPCIDevice *device, uint8_t capability_id);
extern XKDriver xk_pci_driver;

#endif
