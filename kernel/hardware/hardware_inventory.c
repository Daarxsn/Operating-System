#include "hardware_inventory.h"

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

    debug_print_line(
        "========================================"
    );
}