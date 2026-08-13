#ifndef XYRIS_PMM_H
#define XYRIS_PMM_H

#include <stdint.h>
#include <stddef.h>

/*
 * XyrisOS Physical Memory Manager
 *
 * All addresses handled here are
 * physical addresses.
 */

#define PAGE_SIZE 4096

typedef uintptr_t phys_addr_t;

/*
 * Physical memory statistics.
 */
typedef struct
{
    uint64_t total_memory;
    uint64_t usable_memory;
    uint64_t reserved_memory;

    size_t total_pages;
    size_t free_pages;
    size_t used_pages;
    size_t reserved_pages;

} pmm_stats_t;

/*
 * Initialize the PMM.
 *
 * Requires memory_map_init()
 * to have already completed.
 */
void pmm_init(void);

/*
 * Allocate one physical page.
 *
 * Returns 0 on failure.
 */
phys_addr_t
pmm_alloc_page(void);

/*
 * Allocate contiguous pages.
 *
 * Returns the first physical page.
 */
phys_addr_t
pmm_alloc_pages(size_t count);

/*
 * Free one page.
 */
void
pmm_free_page(phys_addr_t page);

/*
 * Free contiguous pages.
 */
void
pmm_free_pages(
    phys_addr_t page,
    size_t count
);

/*
 * Reserve physical pages.
 *
 * Used for:
 * - kernel image
 * - bootloader
 * - framebuffer
 * - ACPI
 */
void
pmm_reserve(
    phys_addr_t address,
    size_t pages
);

/*
 * Release reserved pages.
 */
void
pmm_unreserve(
    phys_addr_t address,
    size_t pages
);

/*
 * Statistics.
 */
pmm_stats_t
pmm_get_stats(void);

#endif
