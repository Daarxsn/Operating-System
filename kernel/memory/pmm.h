#ifndef XYRIS_PMM_H
#define XYRIS_PMM_H

#include <stdint.h>
#include <stddef.h>

/*
 * XyrisOS Physical Memory Manager
 *
 * All addresses handled here are physical addresses.
 */

#define PAGE_SIZE 4096

typedef uintptr_t phys_addr_t;


/* ================================================================
 * Statistics
 * ================================================================ */

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


/* ================================================================
 * Initialization
 * ================================================================ */

void pmm_init(void);


/* ================================================================
 * Allocation
 * ================================================================ */

phys_addr_t pmm_alloc_page(void);

phys_addr_t pmm_alloc_pages(
    size_t count
);


/* ================================================================
 * Reference counting
 *
 * Used by shared mappings and COW.
 * ================================================================ */

/*
 * Add one reference to an allocated physical page.
 */
void pmm_retain_page(
    phys_addr_t page
);


/*
 * Release one reference.
 *
 * The page is returned to the free pool when
 * its final reference disappears.
 */
void pmm_release_page(
    phys_addr_t page
);


/*
 * Get current reference count.
 */
uint32_t pmm_page_refcount(
    phys_addr_t page
);


/* ================================================================
 * Free
 * ================================================================ */

void pmm_free_page(
    phys_addr_t page
);

void pmm_free_pages(
    phys_addr_t page,
    size_t count
);


/* ================================================================
 * Reservation
 * ================================================================ */

void pmm_reserve(
    phys_addr_t address,
    size_t pages
);

void pmm_unreserve(
    phys_addr_t address,
    size_t pages
);


/* ================================================================
 * Statistics
 * ================================================================ */

pmm_stats_t pmm_get_stats(void);

#endif
