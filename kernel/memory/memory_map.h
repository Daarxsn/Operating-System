#ifndef XYRIS_MEMORY_MAP_H
#define XYRIS_MEMORY_MAP_H

#include <stdint.h>
#include <stddef.h>


/*
 * Physical memory region types.
 *
 * These are translated from the Limine
 * memory map into XyrisOS internal types.
 */
typedef enum
{
    MEMORY_USABLE,
    MEMORY_RESERVED,
    MEMORY_KERNEL,
    MEMORY_ACPI_RECLAIMABLE,
    MEMORY_ACPI_NVS,
    MEMORY_BAD_MEMORY,
    MEMORY_BOOTLOADER,
    MEMORY_FRAMEBUFFER,
    MEMORY_UNKNOWN

} memory_type_t;

/*
 * One physical memory region.
 */
typedef struct
{
    uint64_t base;
    uint64_t length;
    memory_type_t type;

} memory_region_t;

/*
 * Overall memory statistics.
 */
typedef struct
{
    uint64_t total_memory;
    uint64_t usable_memory;
    uint64_t reserved_memory;

    size_t region_count;

} memory_map_info_t;

/*
 * Initialize the memory map subsystem.
 *
 * Reads the Limine memory map and
 * stores it internally.
 */
void memory_map_init(void);

/*
 * Number of regions.
 */
size_t memory_map_region_count(void);

/*
 * Get one region.
 *
 * Returns NULL if index is invalid.
 */
const memory_region_t*
memory_map_region(size_t index);

/*
 * Overall statistics.
 */
memory_map_info_t
memory_map_info(void);

void memory_map_dump(void);

#endif