#include "hardware_inventory.h"
#include "drivers/block.h"

#include "../cpu/cpu.h"
#include "../memory/memory_map.h"
#include "../graphics/framebuffer.h"
#include "../include/drivers/pci.h"
#include "../debug/print.h"
#include "../debug/hex.h"

static hardware_inventory_t g_hardware_inventory;


/*
 * Initialize hardware inventory.
 */
void hardware_inventory_init(void)
{
    cpu_info_t cpu =
        cpu_info();

    memory_map_info_t memory =
        memory_map_info();

    g_hardware_inventory.processor_count =
        cpu.processor_count;

    g_hardware_inventory.total_memory =
        memory.total_memory;

    g_hardware_inventory.usable_memory =
        memory.usable_memory;

    g_hardware_inventory.reserved_memory =
        memory.reserved_memory;

    g_hardware_inventory.memory_region_count =
        memory.region_count;

    g_hardware_inventory.display_width =
        framebuffer_width();

    g_hardware_inventory.display_height =
        framebuffer_height();

    g_hardware_inventory.display_pitch =
        framebuffer_pitch();

    g_hardware_inventory.pci_device_count =
        xk_pci_device_count();

    g_hardware_inventory.usb_controller_count = 0;
    g_hardware_inventory.storage_device_count = 0;
    g_hardware_inventory.network_device_count = 0;
    g_hardware_inventory.audio_device_count = 0;

    /*
     * Classify discovered PCI devices.
     *
     * PCI class codes:
     * 0x01 = Mass Storage
     * 0x02 = Network
     * 0x03 = Display
     * 0x04 = Multimedia
     * 0x0C = Serial Bus
     *        subclass 0x03 = USB
     */
    for (uint32_t i = 0;
         i < g_hardware_inventory.pci_device_count;
         ++i)
    {
        const XKPCIDevice *device =
            xk_pci_device_get(i);

        if (device == NULL)
            continue;

        switch (device->class_code)
        {
            case 0x01:
                g_hardware_inventory.storage_device_count++;
                break;

            case 0x02:
                g_hardware_inventory.network_device_count++;
                break;

            case 0x04:
                g_hardware_inventory.audio_device_count++;
                break;

            case 0x0C:
                if (device->subclass == 0x03)
                    g_hardware_inventory.usb_controller_count++;
                break;

            default:
                break;
        }
    }
}


/*
 * Return hardware inventory.
 */
hardware_inventory_t hardware_inventory_info(void)
{
    return g_hardware_inventory;
}


/*
 * Print hardware inventory diagnostics.
 */
void hardware_inventory_dump(void)
{
    debug_print_line(
        "========== HARDWARE INVENTORY =========="
    );

    debug_print("Processors: ");
    debug_print_hex64(
        (uint64_t)g_hardware_inventory.processor_count
    );
    debug_print_line("");

    debug_print("Total Memory: ");
    debug_print_hex64(
        g_hardware_inventory.total_memory
    );
    debug_print_line("");

    debug_print("Usable Memory: ");
    debug_print_hex64(
        g_hardware_inventory.usable_memory
    );
    debug_print_line("");

    debug_print("Reserved Memory: ");
    debug_print_hex64(
        g_hardware_inventory.reserved_memory
    );
    debug_print_line("");

    debug_print("Memory Regions: ");
    debug_print_hex64(
        (uint64_t)g_hardware_inventory.memory_region_count
    );
    debug_print_line("");

    debug_print("Display Width: ");
    debug_print_hex64(
        (uint64_t)g_hardware_inventory.display_width
    );
    debug_print_line("");

    debug_print("Display Height: ");
    debug_print_hex64(
        (uint64_t)g_hardware_inventory.display_height
    );
    debug_print_line("");

    debug_print("Display Pitch: ");
    debug_print_hex64(
        (uint64_t)g_hardware_inventory.display_pitch
    );
    debug_print_line("");

    debug_print("PCI Devices: ");
    debug_print_hex64(
        (uint64_t)g_hardware_inventory.pci_device_count
    );
    debug_print_line("");

    debug_print("USB Controllers: ");
    debug_print_hex64(
        (uint64_t)g_hardware_inventory.usb_controller_count
    );
    debug_print_line("");

    debug_print("Storage Devices: ");
    debug_print_hex64(
        (uint64_t)g_hardware_inventory.storage_device_count
    );
    debug_print_line("");

    debug_print("Network Devices: ");
    debug_print_hex64(
        (uint64_t)g_hardware_inventory.network_device_count
    );
    debug_print_line("");

    debug_print("Audio Devices: ");
    debug_print_hex64(
        (uint64_t)g_hardware_inventory.audio_device_count
    );
    debug_print_line("");

    debug_print_line("Hardware Device Status:");

    debug_print("  USB: ");
    debug_print_line(
        g_hardware_inventory.usb_controller_count > 0
            ? "Detected"
            : "Not Detected"
    );

    debug_print("  Storage: ");
    debug_print_line(
        g_hardware_inventory.storage_device_count > 0
            ? "Detected"
            : "Not Detected"
    );

    debug_print("  Network: ");
    debug_print_line(
        g_hardware_inventory.network_device_count > 0
            ? "Detected"
            : "Not Detected"
    );

    debug_print("  Audio: ");
    debug_print_line(
        g_hardware_inventory.audio_device_count > 0
            ? "Detected"
            : "Not Detected"
    );

    debug_print_line(
        "========================================"
    );
}