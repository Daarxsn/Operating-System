/*
 * pmm.c
 *
 * XyrisOS Kernel
 *
 * Physical Memory Manager
 *
 * Responsibilities:
 *   - Discover usable physical memory
 *   - Maintain allocation/reservation bitmaps
 *   - Allocate/free physical pages
 *   - Allocate/free contiguous physical pages
 *   - Reserve/unreserve physical ranges
 *   - Maintain PMM statistics
 *   - Maintain physical page reference counts
 */

#include "pmm.h"
#include "bitmap.h"
#include "memory_map.h"
#include "hhdm.h"

#include "../debug/print.h"
#include "../debug/hex.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PMM_MAX_USABLE_REGIONS 64
#define INVALID_PAGE_INDEX ((size_t)-1)

#ifndef XYRIS_PMM_TRACE
#define XYRIS_PMM_TRACE 0
#endif

static bitmap_t allocation_bitmap;
static bitmap_t reserved_bitmap;

static uint8_t *bitmap_buffer = NULL;

/*
 * One reference count per physical page.
 *
 * 0 = no active references
 * 1 = exclusively referenced
 * 2+ = shared page, suitable for COW
 */
static uint32_t *page_refcounts = NULL;

static size_t bitmap_page_count = 0;

static const memory_region_t *
usable_regions[PMM_MAX_USABLE_REGIONS];

static size_t usable_region_count = 0;

static pmm_stats_t stats;


/* ================================================================
 * Debug tracing
 * ================================================================ */

static void pmm_trace_alloc(
    phys_addr_t page,
    size_t count)
{
#if XYRIS_PMM_TRACE
    debug_print("PMM ALLOC page=");
    debug_print_hex64((uint64_t)page);
    debug_print(" count=");
    debug_print_hex64((uint64_t)count);
    debug_print(" used=");
    debug_print_hex64((uint64_t)stats.used_pages);
    debug_print(" free=");
    debug_print_hex64((uint64_t)stats.free_pages);
    debug_print("\n");
#else
    (void)page;
    (void)count;
#endif
}


static void pmm_trace_free(
    phys_addr_t page)
{
#if XYRIS_PMM_TRACE
    debug_print("PMM FREE page=");
    debug_print_hex64((uint64_t)page);
    debug_print(" used=");
    debug_print_hex64((uint64_t)stats.used_pages);
    debug_print(" free=");
    debug_print_hex64((uint64_t)stats.free_pages);
    debug_print("\n");
#else
    (void)page;
#endif
}


static void pmm_trace_free_rejected(
    phys_addr_t page,
    const char *reason)
{
    debug_print("PMM FREE REJECT page=");
    debug_print_hex64((uint64_t)page);
    debug_print(" reason=");
    debug_print(reason);
    debug_print("\n");
}


/* ================================================================
 * Basic helpers
 * ================================================================ */

static size_t page_index(
    phys_addr_t address)
{
    return (size_t)(address / PAGE_SIZE);
}


static phys_addr_t page_address(
    size_t page)
{
    return (phys_addr_t)page * PAGE_SIZE;
}


static uint64_t align_up_u64(
    uint64_t value,
    uint64_t alignment)
{
    if (alignment == 0)
        return value;

    uint64_t remainder =
        value % alignment;

    if (remainder == 0)
        return value;

    return value +
           (alignment - remainder);
}


static uint64_t align_down_u64(
    uint64_t value,
    uint64_t alignment)
{
    if (alignment == 0)
        return value;

    return value -
           (value % alignment);
}


/* ================================================================
 * Discover usable memory
 * ================================================================ */

static void discover_usable_regions(void)
{
    usable_region_count = 0;

    size_t count =
        memory_map_region_count();

    for (size_t i = 0;
         i < count;
         i++)
    {
        const memory_region_t *region =
            memory_map_region(i);

        if (region == NULL)
            continue;

        if (region->type != MEMORY_USABLE)
            continue;

        if (region->length == 0)
            continue;

        if (usable_region_count >=
            PMM_MAX_USABLE_REGIONS)
            break;

        usable_regions[
            usable_region_count++
        ] = region;
    }
}


/* ================================================================
 * Highest physical address
 * ================================================================ */

static uint64_t highest_physical_end(void)
{
    uint64_t highest = 0;

    size_t count =
        memory_map_region_count();

    for (size_t i = 0;
         i < count;
         i++)
    {
        const memory_region_t *region =
            memory_map_region(i);

        if (region == NULL)
            continue;

        /*
         * Only usable memory needs to be represented
         * by the PMM allocation bitmap.
         *
         * Reserved/firmware/MMIO regions can exist at
         * very high physical addresses and should not
         * inflate the PMM metadata size.
         */
        if (region->type != MEMORY_USABLE)
            continue;

        uint64_t end =
            region->base +
            region->length;

        if (end > highest)
            highest = end;
    }

    return highest;
}


/* ================================================================
 * Bitmap / metadata sizing
 * ================================================================ */

static size_t bitmap_bytes(void)
{
    return (
        bitmap_page_count + 7
    ) / 8;
}


static size_t bitmap_storage_pages(void)
{
    /*
     * PMM metadata contains:
     *
     *   allocation bitmap
     *   reservation bitmap
     *   uint32_t reference count per physical page
     */

    uint64_t bitmap_total =
        (uint64_t)bitmap_bytes() * 2ULL;

    uint64_t refcount_total =
        (uint64_t)bitmap_page_count *
        sizeof(uint32_t);

    uint64_t total =
        bitmap_total +
        refcount_total;

    return (size_t)(
        (total + PAGE_SIZE - 1) /
        PAGE_SIZE
    );
}


/* ================================================================
 * Find metadata storage region
 * ================================================================ */

static const memory_region_t *
find_bitmap_region(void)
{
    size_t required_pages =
        bitmap_storage_pages();

    const memory_region_t *best =
        NULL;

    uint64_t best_length = 0;

    for (size_t i = 0;
         i < usable_region_count;
         i++)
    {
        const memory_region_t *region =
            usable_regions[i];

        uint64_t start =
            align_up_u64(
                region->base,
                PAGE_SIZE
            );

        uint64_t end =
            align_down_u64(
                region->base +
                region->length,
                PAGE_SIZE
            );

        if (end <= start)
            continue;

        uint64_t pages =
            (end - start) /
            PAGE_SIZE;

        if (pages < required_pages)
            continue;

        if (region->length > best_length)
        {
            best = region;
            best_length =
                region->length;
        }
    }

    return best;
}


/* ================================================================
 * Initialize PMM metadata
 * ================================================================ */

static void initialize_bitmaps(
    const memory_region_t *region)
{
    if (region == NULL)
        return;

    size_t bytes =
        bitmap_bytes();

    uint64_t start =
        align_up_u64(
            region->base,
            PAGE_SIZE
        );

    bitmap_buffer =
        (uint8_t *)phys_to_virt(
            (uintptr_t)start
        );

    uint8_t *allocation_data =
        bitmap_buffer;

    uint8_t *reserved_data =
        bitmap_buffer + bytes;

    uint8_t *refcount_data =
        reserved_data + bytes;


    bitmap_init(
        &allocation_bitmap,
        allocation_data,
        bitmap_page_count
    );

    bitmap_init(
        &reserved_bitmap,
        reserved_data,
        bitmap_page_count
    );


    page_refcounts =
        (uint32_t *)refcount_data;

    for (size_t i = 0;
         i < bitmap_page_count;
         i++)
    {
        page_refcounts[i] = 0;
    }


    /*
     * Initially everything is unavailable.
     */
    for (size_t i = 0;
         i < bitmap_page_count;
         i++)
    {
        bitmap_set(
            &reserved_bitmap,
            i
        );
    }


    bitmap_clear_all(
        &allocation_bitmap
    );
}


/* ================================================================
 * Check physical memory-map membership
 * ================================================================ */

static bool page_in_memory_map(
    size_t index)
{
    uint64_t address =
        (uint64_t)page_address(index);

    size_t count =
        memory_map_region_count();

    for (size_t i = 0;
         i < count;
         i++)
    {
        const memory_region_t *region =
            memory_map_region(i);

        if (region == NULL)
            continue;

        uint64_t start =
            align_down_u64(
                region->base,
                PAGE_SIZE
            );

        uint64_t end =
            align_up_u64(
                region->base +
                region->length,
                PAGE_SIZE
            );

        if (address >= start &&
            address < end)
        {
            return true;
        }
    }

    return false;
}


/* ================================================================
 * Release usable regions
 * ================================================================ */

static void release_usable_regions(void)
{
    stats.free_pages = 0;

    stats.reserved_pages =
        stats.total_pages;

    for (size_t i = 0;
         i < usable_region_count;
         i++)
    {
        const memory_region_t *region =
            usable_regions[i];

        uint64_t start =
            align_up_u64(
                region->base,
                PAGE_SIZE
            );

        uint64_t end =
            align_down_u64(
                region->base +
                region->length,
                PAGE_SIZE
            );

        for (uint64_t address = start;
             address < end;
             address += PAGE_SIZE)
        {
            size_t index =
                page_index(
                    (phys_addr_t)address
                );

            if (index >= bitmap_page_count)
                break;

            if (bitmap_test(
                    &reserved_bitmap,
                    index))
            {
                bitmap_clear(
                    &reserved_bitmap,
                    index
                );

                stats.free_pages++;

                if (stats.reserved_pages > 0)
                    stats.reserved_pages--;
            }
        }
    }
}


/* ================================================================
 * Initialization
 * ================================================================ */

void pmm_init(void)
{
    memory_map_info_t map =
        memory_map_info();


    stats.total_memory =
        map.total_memory;

    stats.usable_memory =
        map.usable_memory;

    stats.reserved_memory =
        map.reserved_memory;


    stats.total_pages =
        (size_t)(
            (map.total_memory +
             PAGE_SIZE - 1) /
            PAGE_SIZE
        );

    stats.free_pages = 0;
    stats.used_pages = 0;
    stats.reserved_pages = 0;


    uint64_t physical_end =
        highest_physical_end();

    bitmap_page_count =
        (size_t)(
            (physical_end +
             PAGE_SIZE - 1) /
            PAGE_SIZE
        );


    if (stats.total_pages == 0 ||
        bitmap_page_count == 0)
    {
        debug_print_line(
            "PMM: No physical memory"
        );

        for (;;)
            __asm__ volatile("hlt");
    }


    discover_usable_regions();

    if (usable_region_count == 0)
    {
        debug_print_line(
            "PMM: No usable memory regions"
        );

        for (;;)
            __asm__ volatile("hlt");
    }


    const memory_region_t *bitmap_region =
        find_bitmap_region();

    if (bitmap_region == NULL)
    {
        debug_print_line(
            "PMM: Unable to place bitmap"
        );

        for (;;)
            __asm__ volatile("hlt");
    }


    initialize_bitmaps(
        bitmap_region
    );

    release_usable_regions();


    /*
     * Never allocate physical page zero.
     */
    pmm_reserve(
        0,
        1
    );


    /*
     * Reserve PMM metadata.
     */
    uint64_t bitmap_start =
        align_up_u64(
            bitmap_region->base,
            PAGE_SIZE
        );

    pmm_reserve(
        (phys_addr_t)bitmap_start,
        bitmap_storage_pages()
    );


    debug_print_line(
        "PMM: Initialized"
    );
}


/* ================================================================
 * Page state
 * ================================================================ */

static bool page_is_free(
    size_t index)
{
    if (index >= bitmap_page_count)
        return false;

    return
        !bitmap_test(
            &allocation_bitmap,
            index
        ) &&
        !bitmap_test(
            &reserved_bitmap,
            index
        );
}


/* ================================================================
 * Find one free page
 * ================================================================ */

static size_t find_free_page(void)
{
    for (size_t r = 0;
         r < usable_region_count;
         r++)
    {
        const memory_region_t *region =
            usable_regions[r];

        uint64_t start =
            align_up_u64(
                region->base,
                PAGE_SIZE
            );

        uint64_t end =
            align_down_u64(
                region->base +
                region->length,
                PAGE_SIZE
            );

        for (uint64_t address = start;
             address < end;
             address += PAGE_SIZE)
        {
            size_t index =
                page_index(
                    (phys_addr_t)address
                );

            if (index >= bitmap_page_count)
                break;

            if (page_is_free(index))
                return index;
        }
    }

    return INVALID_PAGE_INDEX;
}


/* ================================================================
 * Allocate one page
 * ================================================================ */

phys_addr_t pmm_alloc_page(void)
{
    size_t page =
        find_free_page();

    if (page == INVALID_PAGE_INDEX)
        return 0;

    bitmap_set(
        &allocation_bitmap,
        page
    );

    page_refcounts[page] = 1;

    if (stats.free_pages > 0)
        stats.free_pages--;

    stats.used_pages++;

    pmm_trace_alloc(
        page_address(page),
        1
    );

    return page_address(page);
}


/* ================================================================
 * Find contiguous pages
 * ================================================================ */

static size_t find_free_pages(
    size_t count)
{
    if (count == 0)
        return INVALID_PAGE_INDEX;

    for (size_t r = 0;
         r < usable_region_count;
         r++)
    {
        const memory_region_t *region =
            usable_regions[r];

        uint64_t start =
            align_up_u64(
                region->base,
                PAGE_SIZE
            );

        uint64_t end =
            align_down_u64(
                region->base +
                region->length,
                PAGE_SIZE
            );

        size_t consecutive = 0;
        size_t first =
            INVALID_PAGE_INDEX;

        for (uint64_t address = start;
             address < end;
             address += PAGE_SIZE)
        {
            size_t index =
                page_index(
                    (phys_addr_t)address
                );

            if (index >= bitmap_page_count)
                break;

            if (page_is_free(index))
            {
                if (consecutive == 0)
                    first = index;

                consecutive++;

                if (consecutive == count)
                    return first;
            }
            else
            {
                consecutive = 0;
                first = INVALID_PAGE_INDEX;
            }
        }
    }

    return INVALID_PAGE_INDEX;
}


/* ================================================================
 * Allocate contiguous pages
 * ================================================================ */

phys_addr_t pmm_alloc_pages(
    size_t count)
{
    if (count == 0)
        return 0;

    if (count > stats.free_pages)
        return 0;

    size_t first =
        find_free_pages(count);

    if (first == INVALID_PAGE_INDEX)
        return 0;

    for (size_t i = 0;
         i < count;
         i++)
    {
        size_t index =
            first + i;

        bitmap_set(
            &allocation_bitmap,
            index
        );

        page_refcounts[index] = 1;
    }

    stats.used_pages += count;
    stats.free_pages -= count;

    pmm_trace_alloc(
        page_address(first),
        count
    );

    return page_address(first);
}


/* ================================================================
 * Reference counting
 * ================================================================ */

void pmm_retain_page(
    phys_addr_t page)
{
    if ((page & (PAGE_SIZE - 1)) != 0)
        return;

    size_t index =
        page_index(page);

    if (index == 0 ||
        index >= bitmap_page_count)
        return;

    if (!bitmap_test(
            &allocation_bitmap,
            index))
        return;

    if (page_refcounts[index] ==
        UINT32_MAX)
        return;

    page_refcounts[index]++;
}


uint32_t pmm_page_refcount(
    phys_addr_t page)
{
    if ((page & (PAGE_SIZE - 1)) != 0)
        return 0;

    size_t index =
        page_index(page);

    if (index >= bitmap_page_count)
        return 0;

    return page_refcounts[index];
}


void pmm_release_page(
    phys_addr_t page)
{
    if ((page & (PAGE_SIZE - 1)) != 0)
        return;

    size_t index =
        page_index(page);

    if (index == 0 ||
        index >= bitmap_page_count)
        return;

    if (!bitmap_test(
            &allocation_bitmap,
            index))
        return;

    if (page_refcounts[index] == 0)
        return;

    page_refcounts[index]--;

    /*
     * Only the final reference actually
     * returns the page to the free pool.
     */
    if (page_refcounts[index] == 0)
    {
        pmm_free_page(page);
    }
}


/* ================================================================
 * Free one page
 * ================================================================ */

void pmm_free_page(
    phys_addr_t page)
{
    if ((page & (PAGE_SIZE - 1)) != 0)
    {
        pmm_trace_free_rejected(
            page,
            "unaligned"
        );
        return;
    }

    size_t index =
        page_index(page);

    if (index == 0)
    {
        pmm_trace_free_rejected(
            page,
            "page-zero"
        );
        return;
    }

    if (index >= bitmap_page_count)
    {
        pmm_trace_free_rejected(
            page,
            "out-of-bitmap"
        );
        return;
    }

    if (!bitmap_test(
            &allocation_bitmap,
            index))
    {
        pmm_trace_free_rejected(
            page,
            "not-allocated"
        );
        return;
    }

    if (bitmap_test(
            &reserved_bitmap,
            index))
    {
        pmm_trace_free_rejected(
            page,
            "reserved"
        );
        return;
    }

    /*
     * Direct pmm_free_page() is only valid for an
     * exclusively owned page.
     *
     * Shared pages must use pmm_release_page().
     */
    if (page_refcounts[index] > 1)
    {
        pmm_trace_free_rejected(
            page,
            "shared-page"
        );
        return;
    }

    bitmap_clear(
        &allocation_bitmap,
        index
    );

    page_refcounts[index] = 0;

    if (stats.used_pages > 0)
        stats.used_pages--;

    stats.free_pages++;

    pmm_trace_free(page);
}


/* ================================================================
 * Free contiguous pages
 * ================================================================ */

void pmm_free_pages(
    phys_addr_t page,
    size_t count)
{
    if (count == 0)
        return;

    if ((page & (PAGE_SIZE - 1)) != 0)
        return;

    size_t first =
        page_index(page);

    if (first >= bitmap_page_count)
        return;

    for (size_t i = 0;
         i < count;
         i++)
    {
        size_t index =
            first + i;

        if (index >= bitmap_page_count)
            break;

        pmm_release_page(
            page_address(index)
        );
    }
}


/* ================================================================
 * Reserve physical pages
 * ================================================================ */

void pmm_reserve(
    phys_addr_t address,
    size_t pages)
{
    if (pages == 0)
        return;

    if ((address & (PAGE_SIZE - 1)) != 0)
        return;

    size_t first =
        page_index(address);

    if (first >= bitmap_page_count)
        return;

    for (size_t i = 0;
         i < pages;
         i++)
    {
        size_t index =
            first + i;

        if (index >= bitmap_page_count)
            break;

        if (bitmap_test(
                &allocation_bitmap,
                index))
        {
            continue;
        }

        if (bitmap_test(
                &reserved_bitmap,
                index))
        {
            continue;
        }

        bitmap_set(
            &reserved_bitmap,
            index
        );

        if (stats.free_pages > 0)
            stats.free_pages--;

        stats.reserved_pages++;
    }
}


/* ================================================================
 * Unreserve physical pages
 * ================================================================ */

void pmm_unreserve(
    phys_addr_t address,
    size_t pages)
{
    if (pages == 0)
        return;

    if ((address & (PAGE_SIZE - 1)) != 0)
        return;

    size_t first =
        page_index(address);

    if (first >= bitmap_page_count)
        return;

    for (size_t i = 0;
         i < pages;
         i++)
    {
        size_t index =
            first + i;

        if (index == 0)
            continue;

        if (index >= bitmap_page_count)
            break;

        if (bitmap_test(
                &allocation_bitmap,
                index))
        {
            continue;
        }

        if (!page_in_memory_map(index))
            continue;

        if (!bitmap_test(
                &reserved_bitmap,
                index))
        {
            continue;
        }

        bitmap_clear(
            &reserved_bitmap,
            index
        );

        if (stats.reserved_pages > 0)
            stats.reserved_pages--;

        stats.free_pages++;
    }
}


/* ================================================================
 * Statistics
 * ================================================================ */

pmm_stats_t pmm_get_stats(void)
{
    return stats;
}
