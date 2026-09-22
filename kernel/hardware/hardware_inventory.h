#ifndef XYRIS_HARDWARE_INVENTORY_H
#define XYRIS_HARDWARE_INVENTORY_H

#include <stdint.h>
#include <stddef.h>

typedef struct
{
    uint32_t processor_count;

    uint64_t total_memory;
    uint64_t usable_memory;
    uint64_t reserved_memory;
    size_t memory_region_count;

    uint32_t display_width;
    uint32_t display_height;
    uint32_t display_pitch;

    uint32_t pci_device_count;
} hardware_inventory_t;

/*
 * Initialize the hardware inventory.
 *
 * Collects information from the existing
 * CPU, memory, framebuffer and PCI subsystems.
 */
void hardware_inventory_init(void);

/*
 * Return the collected hardware information.
 */
hardware_inventory_t hardware_inventory_info(void);

/*
 * Print a hardware inventory diagnostic.
 */
void hardware_inventory_dump(void);

#endif