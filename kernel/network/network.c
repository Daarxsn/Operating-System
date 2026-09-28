#include "network.h"

static uint32_t g_network_device_count;

void xk_network_init(void)
{
    pci_inventory_t inventory =
        pci_inventory_info();

    g_network_device_count =
        inventory.network_devices;
}

uint32_t xk_network_device_count(void)
{
    return g_network_device_count;
}

int xk_network_device_get(
    uint32_t index,
    xk_network_device_t *info)
{
    if (info == 0)
        return 0;

    uint32_t found = 0;

    for (uint32_t i = 0;
         i < pci_inventory_info().total_devices;
         i++)
    {
        pci_inventory_device_t pci_device;

        if (!pci_inventory_device_get(i, &pci_device))
            continue;

        if (pci_device.hardware_class !=
            PCI_HARDWARE_NETWORK)
        {
            continue;
        }

        if (found != index)
        {
            found++;
            continue;
        }

        info->index = index;

        info->bus =
            pci_device.bus;

        info->device =
            pci_device.device;

        info->function =
            pci_device.function;

        info->vendor_id =
            pci_device.vendor_id;

        info->device_id =
            pci_device.device_id;

        info->class_code =
            pci_device.class_code;

        info->subclass =
            pci_device.subclass;

        info->prog_if =
            pci_device.prog_if;

        return 1;
    }

    return 0;
}

int xk_network_available(void)
{
    return g_network_device_count != 0;
}