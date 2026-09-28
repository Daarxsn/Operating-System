#include "hardware_api.h"
#include "pci_inventory.h"
#include "drivers/block.h"

void xk_hardware_init(void)
{
    hardware_inventory_init();
}

hardware_inventory_t xk_hardware_info(void)
{
    return hardware_inventory_info();
}

uint32_t xk_hardware_cpu_count(void)
{
    return hardware_inventory_info().processor_count;
}

uint64_t xk_hardware_total_memory(void)
{
    return hardware_inventory_info().total_memory;
}

uint64_t xk_hardware_usable_memory(void)
{
    return hardware_inventory_info().usable_memory;
}

uint32_t xk_hardware_display_width(void)
{
    return hardware_inventory_info().display_width;
}

uint32_t xk_hardware_display_height(void)
{
    return hardware_inventory_info().display_height;
}

uint32_t xk_hardware_display_pitch(void)
{
    return hardware_inventory_info().display_pitch;
}

uint32_t xk_hardware_pci_device_count(void)
{
    return hardware_inventory_info().pci_device_count;
}

bool xk_hardware_usb_available(void)
{
    return hardware_inventory_info().usb_controller_count > 0;
}

bool xk_hardware_storage_available(void)
{
    return hardware_inventory_info().storage_device_count > 0;
}

bool xk_hardware_network_available(void)
{
    return hardware_inventory_info().network_device_count > 0;
}

bool xk_hardware_audio_available(void)
{
    return hardware_inventory_info().audio_device_count > 0;
}

uint32_t xk_hardware_audio_device_count(void)
{
    return hardware_inventory_info().audio_device_count;
}

uint32_t xk_hardware_device_count(void)
{
    return pci_inventory_info().total_devices;
}

int xk_hardware_device_get(
    uint32_t index,
    pci_inventory_device_t *info)
{
    return pci_inventory_device_get(index, info);
}

uint32_t xk_hardware_storage_device_count(void)
{
    return xk_block_device_count();
}
