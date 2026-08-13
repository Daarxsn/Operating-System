#include "drivers/pci.h"

#include "cpu/io.h"
#include <stddef.h>

static bool pci_initialized = false;

#define XK_PCI_MAX_DEVICES 256
static XKPCIDevice pci_devices[XK_PCI_MAX_DEVICES];
static uint32_t pci_device_count = 0;

/* ------------------------------------------------------------
 * PCI Configuration Read
 * ------------------------------------------------------------ */

uint32_t xk_pci_read32(
    uint8_t bus,
    uint8_t device,
    uint8_t function,
    uint8_t offset)
{
    uint32_t address =
        (1U << 31) |
        ((uint32_t)bus << 16) |
        ((uint32_t)device << 11) |
        ((uint32_t)function << 8) |
        (offset & 0xFC);

    outl(PCI_CONFIG_ADDRESS, address);

    return inl(PCI_CONFIG_DATA);
}

/* ------------------------------------------------------------
 * Vendor ID
 * ------------------------------------------------------------ */

uint16_t xk_pci_vendor_id(
    uint8_t bus,
    uint8_t device,
    uint8_t function)
{
    return (uint16_t)
        (xk_pci_read32(bus, device, function, 0) & 0xFFFF);
}

/* ------------------------------------------------------------
 * Scan
 * ------------------------------------------------------------ */

void xk_pci_scan(void)
{
    pci_device_count = 0;

    for (uint16_t bus = 0; bus < 256; bus++)
    {
        for (uint8_t device = 0; device < 32; device++)
        {
            uint16_t vendor =
                xk_pci_vendor_id(bus, device, 0);

            if (vendor == 0xFFFF)
            {
                continue;
            }

            if (pci_device_count >= XK_PCI_MAX_DEVICES)
            {
                return;
            }

            uint32_t id =
                xk_pci_read32(bus, device, 0, 0);

            uint32_t class_info =
                xk_pci_read32(bus, device, 0, 8);

            XKPCIDevice *entry =
                &pci_devices[pci_device_count++];

            entry->bus = (uint8_t)bus;
            entry->device = device;
            entry->function = 0;
            entry->vendor_id = (uint16_t)(id & 0xFFFF);
            entry->device_id = (uint16_t)(id >> 16);
            entry->class_code = (uint8_t)(class_info >> 24);
            entry->subclass = (uint8_t)(class_info >> 16);
            entry->prog_if = (uint8_t)(class_info >> 8);
            entry->header_type = (uint8_t)(
                xk_pci_read32(bus, device, 0, 0x0C) >> 16
            );
        }
    }
}

uint32_t xk_pci_device_count(void)
{
    return pci_device_count;
}

const XKPCIDevice *xk_pci_device_get(uint32_t index)
{
    if (index >= pci_device_count)
    {
        return NULL;
    }

    return &pci_devices[index];
}

/* ------------------------------------------------------------
 * Initialize
 * ------------------------------------------------------------ */

bool xk_pci_initialize(void)
{
    pci_initialized = true;

    xk_pci_scan();

    return true;
}

/* ------------------------------------------------------------
 * Shutdown
 * ------------------------------------------------------------ */

void xk_pci_shutdown(void)
{
    pci_initialized = false;
}

/* ------------------------------------------------------------
 * Driver Object
 * ------------------------------------------------------------ */

XKDriver xk_pci_driver =
{
    .name = "PCI Enumerator",
    .type = XK_DRIVER_CUSTOM,
    .state = XK_DRIVER_UNINITIALIZED,
    .initialize = xk_pci_initialize,
    .shutdown = xk_pci_shutdown
};