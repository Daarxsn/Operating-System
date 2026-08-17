#include "drivers/pci.h"
#include "cpu/io.h"
#include <stddef.h>

static bool pci_initialized = false;
static XKPCIDevice pci_devices[XK_PCI_MAX_DEVICES];
static uint32_t pci_devices_found = 0;

uint32_t xk_pci_read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset)
{
    uint32_t address = (1U << 31) |
        ((uint32_t)bus << 16) |
        ((uint32_t)device << 11) |
        ((uint32_t)function << 8) |
        (offset & 0xFCU);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

uint16_t xk_pci_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset)
{
    uint32_t value = xk_pci_read32(bus, device, function, offset);
    return (uint16_t)(value >> ((offset & 2U) * 8U));
}

uint8_t xk_pci_read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset)
{
    uint32_t value = xk_pci_read32(bus, device, function, offset);
    return (uint8_t)(value >> ((offset & 3U) * 8U));
}

uint16_t xk_pci_vendor_id(uint8_t bus, uint8_t device, uint8_t function)
{
    return xk_pci_read16(bus, device, function, 0);
}

static bool pci_add_function(uint8_t bus, uint8_t device, uint8_t function)
{
    if (pci_devices_found >= XK_PCI_MAX_DEVICES)
        return false;

    uint16_t vendor = xk_pci_vendor_id(bus, device, function);
    if (vendor == 0xFFFFU)
        return true;

    uint32_t id = xk_pci_read32(bus, device, function, 0);
    uint32_t class_info = xk_pci_read32(bus, device, function, 8);
    uint32_t header_info = xk_pci_read32(bus, device, function, 0x0C);

    XKPCIDevice *entry = &pci_devices[pci_devices_found++];
    entry->bus = bus;
    entry->device = device;
    entry->function = function;
    entry->vendor_id = (uint16_t)(id & 0xFFFFU);
    entry->device_id = (uint16_t)(id >> 16);
    entry->class_code = (uint8_t)(class_info >> 24);
    entry->subclass = (uint8_t)(class_info >> 16);
    entry->prog_if = (uint8_t)(class_info >> 8);
    entry->header_type = (uint8_t)(header_info >> 16);
    entry->command = xk_pci_read16(bus, device, function, 0x04);
    entry->status = xk_pci_read16(bus, device, function, 0x06);
    entry->capability_pointer = 0;
    entry->bar_count = 0;
    entry->capability_count = 0;

    uint8_t header_type = entry->header_type & 0x7FU;
    uint8_t max_bars = (header_type == 0x00U) ? 6U : ((header_type == 0x01U) ? 2U : 0U);
    for (uint8_t i = 0; i < max_bars; i++)
    {
        entry->bars[i] = xk_pci_read32(bus, device, function, (uint8_t)(0x10U + i * 4U));
        entry->bar_count++;
        /* A 64-bit memory BAR consumes the following slot. */
        if ((entry->bars[i] & 0x7U) == 0x4U && i + 1U < max_bars)
            i++;
    }

    if ((entry->status & (1U << 4)) != 0)
    {
        uint8_t cap = xk_pci_read8(bus, device, function, 0x34);
        entry->capability_pointer = cap;
        for (uint8_t guard = 0; cap != 0 && guard < XK_PCI_MAX_CAPABILITIES; guard++)
        {
            cap &= 0xFCU;
            if (cap < 0x40U || cap >= 0xFCU)
                break;
            uint8_t id_cap = xk_pci_read8(bus, device, function, cap);
            uint8_t next = xk_pci_read8(bus, device, function, (uint8_t)(cap + 1U));
            entry->capabilities[entry->capability_count].id = id_cap;
            entry->capabilities[entry->capability_count].offset = cap;
            entry->capability_count++;
            cap = next;
        }
    }

    return true;
}

void xk_pci_scan(void)
{
    pci_devices_found = 0;
    for (uint32_t i = 0; i < XK_PCI_MAX_DEVICES; i++)
    {
        pci_devices[i].bar_count = 0;
        pci_devices[i].capability_count = 0;
    }

    for (uint16_t bus = 0; bus < 256U; bus++)
    {
        for (uint8_t device = 0; device < 32U; device++)
        {
            uint16_t vendor = xk_pci_vendor_id((uint8_t)bus, device, 0);
            if (vendor == 0xFFFFU)
                continue;

            (void)pci_add_function((uint8_t)bus, device, 0);

            uint8_t header = xk_pci_read8((uint8_t)bus, device, 0, 0x0E);
            if ((header & 0x80U) != 0)
            {
                for (uint8_t function = 1; function < 8U; function++)
                    (void)pci_add_function((uint8_t)bus, device, function);
            }
        }
    }
}

uint32_t xk_pci_device_count(void) { return pci_devices_found; }

const XKPCIDevice *xk_pci_device_get(uint32_t index)
{
    if (index >= pci_devices_found) return NULL;
    return &pci_devices[index];
}

const XKPCICapability *xk_pci_find_capability(const XKPCIDevice *device, uint8_t capability_id)
{
    if (device == NULL) return NULL;
    for (uint8_t i = 0; i < device->capability_count; i++)
        if (device->capabilities[i].id == capability_id)
            return &device->capabilities[i];
    return NULL;
}

bool xk_pci_initialize(void)
{
    xk_pci_scan();
    pci_initialized = true;
    return true;
}

void xk_pci_shutdown(void)
{
    pci_initialized = false;
    pci_devices_found = 0;
}

XKDriver xk_pci_driver =
{
    .name = "PCI Enumerator",
    .type = XK_DRIVER_CUSTOM,
    .state = XK_DRIVER_UNINITIALIZED,
    .initialize = xk_pci_initialize,
    .shutdown = xk_pci_shutdown
};
