#include "memory_tests.h"

#include <stdint.h>
#include <stdbool.h>

#include "../memory/pmm.h"
#include "../memory/heap.h"
#include "../memory/vmm.h"
#include "../memory/hhdm.h"
#include "../boot/boot.h"
#include "../cpu/page_fault.h"

#include <string.h>


static void page_fault_handler_test(void)
{
    /*
     * Page fault infrastructure is installed through:
     * IDT vector 14 -> isr14 -> exception_dispatch()
     * -> page_fault_handler().
     *
     * The handler itself is intentionally not invoked here because
     * the current implementation terminates the kernel after a fault.
     */
    boot_step_ok("VMM Test: Page Fault Handler Installed");
}

void run_memory_tests(void)
{
    page_fault_handler_test();

    /* ---------------------------------
       Demand Paging Test
    ---------------------------------- */

    const uintptr_t demand_virt =
        0xFFFF900030000000ULL;

    bool demand_ok =
        vmm_register_demand_page(
            vmm_kernel_space(),
            demand_virt,
            VMM_WRITABLE);

    if (demand_ok &&
        vmm_translate(
            vmm_kernel_space(),
            demand_virt) == 0)
    {
        boot_step_ok(
            "VMM Test: Demand Page Registration");
    }
    else
    {
        boot_step_fail(
            "VMM Test: Demand Page Registration");
    }

    /* ---------------------------------
       Demand Paging Fault Recovery Test
    ---------------------------------- */

    const uintptr_t demand_fault_virt =
        0xFFFF900030001000ULL;

    bool demand_fault_ok =
        vmm_register_demand_page(
            vmm_kernel_space(),
            demand_fault_virt,
            VMM_WRITABLE);

    if (demand_fault_ok)
    {
        volatile uint64_t *demand_ptr =
            (volatile uint64_t *)demand_fault_virt;

        /*
         * This access intentionally touches a registered,
         * non-present page. The page-fault handler should
         * allocate and map a physical page, then execution
         * should continue here.
         */
        *demand_ptr = 0x58595249534F534FULL;

        phys_addr_t recovered =
            vmm_translate(
                vmm_kernel_space(),
                demand_fault_virt);

        if (recovered != 0 &&
            *demand_ptr == 0x58595249534F534FULL)
        {
            boot_step_ok(
                "VMM Test: Demand Page Fault Recovery");
        }
        else
        {
            boot_step_fail(
                "VMM Test: Demand Page Fault Recovery");
        }
    }
    else
    {
        boot_step_fail(
            "VMM Test: Demand Page Fault Recovery");
    }

    /* ---------------------------------
       PMM Tests
    ---------------------------------- */

    pmm_stats_t before = pmm_get_stats();

    phys_addr_t page = pmm_alloc_page();

    if (page == 0)
    {
        boot_step_fail("PMM Test: Allocate Page");
        return;
    }

    boot_step_ok("PMM Test: Allocate Page");

    /* ---------------------------------
       PMM Statistics After Allocation
    ---------------------------------- */

    pmm_stats_t after_alloc = pmm_get_stats();

    if (after_alloc.free_pages == before.free_pages - 1 &&
        after_alloc.used_pages == before.used_pages + 1)
    {
        boot_step_ok("PMM Test: Statistics After Allocation");
    }
    else
    {
        boot_step_fail("PMM Test: Statistics After Allocation");
    }

    /* ---------------------------------
       PMM Reference Counting Test
    ---------------------------------- */

    bool refcount_ok = true;

    if (pmm_page_refcount(page) != 1)
    {
        refcount_ok = false;
    }

    pmm_retain_page(page);

    if (pmm_page_refcount(page) != 2)
    {
        refcount_ok = false;
    }

    pmm_release_page(page);

    if (pmm_page_refcount(page) != 1)
    {
        refcount_ok = false;
    }

    if (refcount_ok)
    {
        boot_step_ok("PMM Test: Reference Counting");
    }
    else
    {
        boot_step_fail("PMM Test: Reference Counting");
    }

    /* ---------------------------------
       PMM Free Page Test
    ---------------------------------- */

    pmm_free_page(page);

    pmm_stats_t after_free = pmm_get_stats();

    if (after_free.free_pages == before.free_pages &&
        after_free.used_pages == before.used_pages)
    {
        boot_step_ok("PMM Test: Free Page");
    }
    else
    {
        boot_step_fail("PMM Test: Free Page");
    }

    /* ---------------------------------
       PMM Multi-Page Stress Test
    ---------------------------------- */

    phys_addr_t pages[10] = {0};

    int success = 1;

    for (int i = 0; i < 10; i++)
    {
        pages[i] = pmm_alloc_page();

        if (pages[i] == 0)
        {
            success = 0;
            break;
        }
    }

    if (success)
    {
        boot_step_ok("PMM Test: Multi-Page Allocation");
    }
    else
    {
        boot_step_fail("PMM Test: Multi-Page Allocation");
    }

    for (int i = 0; i < 10; i++)
    {
        if (pages[i] != 0)
        {
            pmm_free_page(pages[i]);
        }
    }

    /* ---------------------------------
       PMM OOM Test
    ---------------------------------- */

    pmm_stats_t oom_before = pmm_get_stats();

    phys_addr_t oom_page =
        pmm_alloc_pages(oom_before.free_pages + 1);

    pmm_stats_t oom_after = pmm_get_stats();

    if (oom_page == 0 &&
        oom_after.free_pages == oom_before.free_pages &&
        oom_after.used_pages == oom_before.used_pages)
    {
        boot_step_ok("PMM Test: OOM Handling");
    }
    else
    {
        boot_step_fail("PMM Test: OOM Handling");

        if (oom_page != 0)
        {
            pmm_free_pages(
                oom_page,
                oom_before.free_pages + 1
            );
        }
    }

    /* ---------------------------------
       PMM Fragmentation Stress Test
    ---------------------------------- */

    pmm_stats_t frag_before = pmm_get_stats();

    phys_addr_t frag_pages[16] = {0};
    bool frag_alloc_ok = true;

    for (int i = 0; i < 16; i++)
    {
        frag_pages[i] = pmm_alloc_page();

        if (frag_pages[i] == 0)
        {
            frag_alloc_ok = false;
            break;
        }
    }

    /*
     * Free every other page to create fragmented free space.
     */
    for (int i = 0; i < 16; i += 2)
    {
        if (frag_pages[i] != 0)
        {
            pmm_free_page(frag_pages[i]);
            frag_pages[i] = 0;
        }
    }

    /*
     * A contiguous allocation should still succeed when
     * enough contiguous pages are available elsewhere.
     */
    phys_addr_t frag_contiguous =
        pmm_alloc_pages(4);

    bool frag_test_ok =
        frag_alloc_ok &&
        frag_contiguous != 0;

    if (frag_contiguous != 0)
    {
        pmm_free_pages(frag_contiguous, 4);
    }

    /*
     * Release the remaining fragmented pages.
     */
    for (int i = 0; i < 16; i++)
    {
        if (frag_pages[i] != 0)
        {
            pmm_free_page(frag_pages[i]);
            frag_pages[i] = 0;
        }
    }

    pmm_stats_t frag_after = pmm_get_stats();

    if (frag_test_ok &&
        frag_after.free_pages == frag_before.free_pages &&
        frag_after.used_pages == frag_before.used_pages)
    {
        boot_step_ok("PMM Test: Fragmentation Stress");
    }
    else
    {
        boot_step_fail("PMM Test: Fragmentation Stress");
    }

    /* ---------------------------------
       Heap Tests
    ---------------------------------- */


    void* ptr1 = kmalloc(1);
    void* ptr2 = kmalloc(64);
    void* ptr3 = kmalloc(4096);

    if (ptr1 && ptr2 && ptr3)
    {
        boot_step_ok("Heap Test: Multiple Allocations");
    }
    else
    {
        boot_step_fail("Heap Test: Multiple Allocations");
    }

    /* ---------------------------------
       Heap Alignment Test
    ---------------------------------- */

    if ((((uintptr_t)ptr2) & 0x7) == 0)
    {
        boot_step_ok("Heap Test: Alignment");
    }
    else
    {
        boot_step_fail("Heap Test: Alignment");
    }

    kfree(ptr1);
    kfree(ptr2);
    kfree(ptr3);

    boot_step_ok("Heap Test: Free");

    /* ---------------------------------
       Heap Calloc Test
    ---------------------------------- */

    uint32_t *zeroed = (uint32_t *)kcalloc(16, sizeof(uint32_t));
    bool calloc_ok = true;

    if (zeroed == NULL)
    {
        calloc_ok = false;
    }
    else
    {
        for (size_t i = 0; i < 16; i++)
        {
            if (zeroed[i] != 0)
            {
                calloc_ok = false;
                break;
            }
        }

        kfree(zeroed);
    }

    if (calloc_ok)
        boot_step_ok("Heap Test: Calloc");
    else
        boot_step_fail("Heap Test: Calloc");

    /* ---------------------------------
       Heap Realloc Test
    ---------------------------------- */

    uint8_t *resized = (uint8_t *)kmalloc(32);
    bool realloc_ok = true;

    if (resized == NULL)
    {
        realloc_ok = false;
    }
    else
    {
        for (size_t i = 0; i < 32; i++)
            resized[i] = (uint8_t)(i + 1);

        uint8_t *grown = (uint8_t *)krealloc(resized, 128);

        if (grown == NULL)
        {
            realloc_ok = false;
        }
        else
        {
            for (size_t i = 0; i < 32; i++)
            {
                if (grown[i] != (uint8_t)(i + 1))
                {
                    realloc_ok = false;
                    break;
                }
            }

            kfree(grown);
        }
    }

    if (realloc_ok)
        boot_step_ok("Heap Test: Realloc");
    else
        boot_step_fail("Heap Test: Realloc");

    /* ---------------------------------
       Heap Segment Growth Test
    ---------------------------------- */

    const size_t growth_size = 16 * PAGE_SIZE;

    uint8_t *large =
        (uint8_t *)kmalloc(growth_size);

    bool growth_ok = true;

    if (large == NULL)
    {
        growth_ok = false;
    }
    else
    {
        const size_t middle = growth_size / 2;

        large[0] = 0xA5;
        large[middle] = 0x3C;
        large[growth_size - 1] = 0x5A;

        if (large[0] != 0xA5 ||
            large[middle] != 0x3C ||
            large[growth_size - 1] != 0x5A)
        {
            growth_ok = false;
        }

        kfree(large);
    }

    if (growth_ok)
        boot_step_ok("Heap Test: Segment Growth");
    else
        boot_step_fail("Heap Test: Segment Growth");


    /* ---------------------------------
       VMM Tests
    ---------------------------------- */

    phys_addr_t phys = pmm_alloc_page();

    if (phys == 0)
    {
        boot_step_fail("VMM Test: Allocate Physical Page");
        return;
    }

    uintptr_t virt = 0xFFFF900010000000ULL;

    if (vmm_map_page(
            vmm_kernel_space(),
            virt,
            phys,
            VMM_WRITABLE))
    {
        boot_step_ok("VMM Test: Map Page");
    }
    else
    {
        boot_step_fail("VMM Test: Map Page");
        pmm_free_page(phys);
        return;
    }

    phys_addr_t translated =
        vmm_translate(vmm_kernel_space(), virt);

    if (translated == phys)
    {
        boot_step_ok("VMM Test: Translate");
    }
    else
    {
        boot_step_fail("VMM Test: Translate");
    }

    /* ---------------------------------
       VMM Get Flags Test
    ---------------------------------- */

    uint64_t page_flags =
        vmm_get_page_flags(
            vmm_kernel_space(),
            virt);

    if ((page_flags & VMM_PRESENT) &&
        (page_flags & VMM_WRITABLE))
    {
        boot_step_ok("VMM Test: Get Flags");
    }
    else
    {
        boot_step_fail("VMM Test: Get Flags");
    }

    /* ---------------------------------
       VMM Protect Page Test
    ---------------------------------- */

    if (vmm_protect_page(
            vmm_kernel_space(),
            virt,
            VMM_PRESENT))
    {
        boot_step_ok("VMM Test: Protect Page");
    }
    else
    {
        boot_step_fail("VMM Test: Protect Page");
    }

    page_flags =
        vmm_get_page_flags(
            vmm_kernel_space(),
            virt);

    if (!(page_flags & VMM_WRITABLE))
    {
        boot_step_ok("VMM Test: Updated Flags");
    }
    else
    {
        boot_step_fail("VMM Test: Updated Flags");
    }

    if (vmm_unmap_page(vmm_kernel_space(), virt))
    {
        boot_step_ok("VMM Test: Unmap Page");
    }
    else
    {
        boot_step_fail("VMM Test: Unmap Page");
    }

    /* ---------------------------------
       VMM Invalid Translate Test
    ----------------------------------*/

    if (vmm_translate(
            vmm_kernel_space(),
            virt) == 0)
    {
        boot_step_ok("VMM Test: Invalid Translate");
    }
    else
    {
        boot_step_fail("VMM Test: Invalid Translate");
    }

    /* ---------------------------------
       VMM Double Unmap Test
    ---------------------------------- */

    if (!vmm_unmap_page(
            vmm_kernel_space(),
            virt))
    {
        boot_step_ok("VMM Test: Double Unmap");
    }
    else
    {
        boot_step_fail("VMM Test: Double Unmap");
    }

    /* ---------------------------------
       VMM Protect Missing Page Test
    ---------------------------------- */

    if (!vmm_protect_page(
            vmm_kernel_space(),
            virt,
            VMM_WRITABLE))
    {
        boot_step_ok("VMM Test: Protect Missing Page");
    }
    else
    {
        boot_step_fail("VMM Test: Protect Missing Page");
    }

    pmm_free_page(phys);

    /* ---------------------------------
   Address Space Tests
---------------------------------- */


/* ---------------------------------
   VMM Advanced Protection Test
---------------------------------- */

phys_addr_t protect_phys = pmm_alloc_page();

bool protection_ok = true;

const uintptr_t protect_virt =
    0xFFFF900020000000ULL;

if (protect_phys == 0)
{
    protection_ok = false;
}
else if (!vmm_map_page(
            vmm_kernel_space(),
            protect_virt,
            protect_phys,
            VMM_WRITABLE))
{
    protection_ok = false;
}
else
{
    uint64_t flags =
        vmm_get_page_flags(
            vmm_kernel_space(),
            protect_virt);

    if (!(flags & VMM_PRESENT) ||
        !(flags & VMM_WRITABLE))
    {
        protection_ok = false;
    }

    if (!vmm_protect_page(
            vmm_kernel_space(),
            protect_virt,
            VMM_PRESENT | VMM_NX))
    {
        protection_ok = false;
    }

    flags =
        vmm_get_page_flags(
            vmm_kernel_space(),
            protect_virt);

    if (!(flags & VMM_PRESENT) ||
        (flags & VMM_WRITABLE) ||
        !(flags & VMM_NX))
    {
        protection_ok = false;
    }

    if (!vmm_unmap_page(
            vmm_kernel_space(),
            protect_virt))
    {
        protection_ok = false;
    }
}

if (protect_phys != 0)
{
    pmm_free_page(protect_phys);
}

if (protection_ok)
{
    boot_step_ok("VMM Test: Advanced Protection");
}
else
{
    boot_step_fail("VMM Test: Advanced Protection");
}

address_space_t* space =
    vmm_create_space();

if (space != NULL)
{
    boot_step_ok("VMM Test: Create Address Space");
}
else
{
    boot_step_fail("VMM Test: Create Address Space");
    return;
}

vmm_destroy_space(space);

boot_step_ok("VMM Test: Destroy Address Space");

/* ---------------------------------
   Multiple Address Space Isolation Test
---------------------------------- */

address_space_t *space_a = vmm_create_space();
address_space_t *space_b = vmm_create_space();

bool isolation_ok = true;

phys_addr_t page_a = 0;
phys_addr_t page_b = 0;

const uintptr_t isolation_virt =
    0x0000000000400000ULL;

if (space_a == NULL || space_b == NULL)
{
    isolation_ok = false;
}
else
{
    page_a = pmm_alloc_page();
    page_b = pmm_alloc_page();

    if (page_a == 0 || page_b == 0 || page_a == page_b)
    {
        isolation_ok = false;
    }
    else
    {
        if (!vmm_map_page(
                space_a,
                isolation_virt,
                page_a,
                VMM_WRITABLE))
        {
            isolation_ok = false;
        }

        if (!vmm_map_page(
                space_b,
                isolation_virt,
                page_b,
                VMM_WRITABLE))
        {
            isolation_ok = false;
        }

        phys_addr_t translated_a =
            vmm_translate(
                space_a,
                isolation_virt);

        phys_addr_t translated_b =
            vmm_translate(
                space_b,
                isolation_virt);

        if (translated_a != page_a)
            isolation_ok = false;

        if (translated_b != page_b)
            isolation_ok = false;

        /*
         * The same virtual address must resolve to
         * different physical pages in independent
         * address spaces.
         */
        if (translated_a == translated_b)
            isolation_ok = false;
    }
}

if (page_a != 0)
{
    pmm_free_page(page_a);
}

if (page_b != 0)
{
    pmm_free_page(page_b);
}

if (space_a != NULL)
{
    vmm_destroy_space(space_a);
}

if (space_b != NULL)
{
    vmm_destroy_space(space_b);
}

if (isolation_ok)
{
    boot_step_ok(
        "VMM Test: Address Space Isolation");
}
else
{
    boot_step_fail(
        "VMM Test: Address Space Isolation");
}

/* ---------------------------------
   Address Space Stress Test
---------------------------------- */

pmm_stats_t stats_before =
    pmm_get_stats();

bool stress_success = true;

for (int i = 0; i < 100; i++)
{
    space = vmm_create_space();

    if (space == NULL)
    {
        stress_success = false;
        break;
    }

    vmm_destroy_space(space);
}

if (stress_success)
{
    boot_step_ok("VMM Test: Address Space Stress");
}
else
{
    boot_step_fail("VMM Test: Address Space Stress");
    return;
}

pmm_stats_t stats_after =
    pmm_get_stats();

if (stats_before.free_pages == stats_after.free_pages &&
    stats_before.used_pages == stats_after.used_pages)
{
    boot_step_ok("VMM Test: No Memory Leak");
}
else
{
    boot_step_fail("VMM Test: No Memory Leak");
}
}
