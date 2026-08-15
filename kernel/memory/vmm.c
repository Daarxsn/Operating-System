#include "vmm.h"

#include "pmm.h"
#include "heap.h"
#include "hhdm.h"

#include "../debug/print.h"
#include "../debug/hex.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* --------------------------------------------------
   x86-64 Paging Constants
-------------------------------------------------- */

#define ENTRIES_PER_TABLE 512

#define PAGE_SHIFT 12
#define PAGE_MASK 0x000FFFFFFFFFF000ULL

#define PML4_INDEX(x) (((x) >> 39) & 0x1FF)
#define PDPT_INDEX(x) (((x) >> 30) & 0x1FF)
#define PD_INDEX(x)   (((x) >> 21) & 0x1FF)
#define PT_INDEX(x)   (((x) >> 12) & 0x1FF)

/* --------------------------------------------------
   Internal Address Space
-------------------------------------------------- */

struct address_space
{
    uint64_t* pml4;
};

/* --------------------------------------------------
   Kernel Address Space
-------------------------------------------------- */

static struct address_space kernel_space;

/* --------------------------------------------------
   Helpers
-------------------------------------------------- */

static inline uintptr_t read_cr3(void)
{
    uintptr_t value;

    __asm__ volatile(
        "mov %%cr3, %0"
        : "=r"(value)
    );

    return value;
}

static inline void write_cr3(uintptr_t value)
{
    __asm__ volatile(
        "mov %0, %%cr3"
        :
        : "r"(value)
        : "memory"
    );
}

static inline uint64_t* current_pml4(void)
{
    uintptr_t cr3 = read_cr3();

    /* Remove CR3 flags */
    cr3 &= PAGE_MASK;

    return (uint64_t*)phys_to_virt(cr3);
}

static inline phys_addr_t entry_address(uint64_t entry)
{
    return (phys_addr_t)(entry & PAGE_MASK);
}

static uint64_t* page_table_from_entry(uint64_t entry)
{
    return (uint64_t*)phys_to_virt(entry_address(entry));
}

static inline bool entry_present(uint64_t entry)
{
    return (entry & VMM_PRESENT) != 0;
}

static inline bool entry_huge(uint64_t entry)
{
    return (entry & VMM_HUGE) != 0;
}

/* --------------------------------------------------
   Free Page Tables
-------------------------------------------------- */

static void free_page_tables(uint64_t* table, int level)
{
    if (table == NULL)
        return;

    /*
     * level:
     *
     * 3 = PML4
     * 2 = PDPT
     * 1 = PD
     * 0 = PT
     *
     * This function owns ONLY page-table pages.
     * It does NOT free physical frames mapped by PT entries.
     *
     * The owner of a mapped frame is responsible for freeing it.
     */

    if (level > 0)
    {
        for (size_t i = 0; i < ENTRIES_PER_TABLE; i++)
        {
            uint64_t entry = table[i];

            if (!(entry & VMM_PRESENT))
                continue;

            /*
             * Huge pages are leaf mappings at PD/PDPT level.
             * Their physical frames are owned by the caller,
             * so do not recurse into them.
             */
            if ((level == 2 || level == 1) &&
                (entry & VMM_HUGE))
            {
                continue;
            }

            free_page_tables(
                page_table_from_entry(entry),
                level - 1
            );
        }
    }

    /*
     * Free this page-table page itself.
     */
    pmm_free_page(
        (phys_addr_t)virt_to_phys(table)
    );
}
    
/* --------------------------------------------------
   Allocate Page Table
-------------------------------------------------- */

static uint64_t* allocate_table(void)
{
    phys_addr_t phys = pmm_alloc_page();

    if (phys == 0)
        return NULL;

    uint64_t* table =
        (uint64_t*)phys_to_virt(phys);

    memset(table, 0, PAGE_SIZE);

    return table;
}

/* --------------------------------------------------
   Page Table Walker
-------------------------------------------------- */

static uint64_t* walk_page_tables(
    address_space_t* space,
    uintptr_t virtual_addr,
    bool create,
    uint64_t flags)
{
    if (space == NULL || space->pml4 == NULL)
        return NULL;

    uint64_t* pml4 = space->pml4;

    /* --------------------------------------------------
       PML4
    -------------------------------------------------- */

    uint64_t* pdpt;

    size_t pml4_index =
        PML4_INDEX(virtual_addr);

    if (!(pml4[pml4_index] & VMM_PRESENT))
    {
        if (!create)
            return NULL;

        pdpt = allocate_table();

        if (pdpt == NULL)
            return NULL;

        pml4[pml4_index] =
            virt_to_phys(pdpt)
            | VMM_PRESENT
            | VMM_WRITABLE
            | (flags & VMM_USER);
    }
    else
    {
        pdpt = page_table_from_entry(
            pml4[pml4_index]
        );

        if (create && (flags & VMM_USER))
        {
            pml4[pml4_index] |= VMM_USER;
        }
    }

    if (pdpt == NULL)
        return NULL;

    /* --------------------------------------------------
       PDPT
    -------------------------------------------------- */

    uint64_t* pd;

    size_t pdpt_index =
        PDPT_INDEX(virtual_addr);

    if (!(pdpt[pdpt_index] & VMM_PRESENT))
    {
        if (!create)
            return NULL;

        pd = allocate_table();

        if (pd == NULL)
            return NULL;

        pdpt[pdpt_index] =
            virt_to_phys(pd)
            | VMM_PRESENT
            | VMM_WRITABLE
            | (flags & VMM_USER);
    }
    else
    {
        if (entry_huge(pdpt[pdpt_index]))
        {
            debug_print(
                "VMM: PDPT huge page detected\n"
            );

            return NULL;
        }

        pd = page_table_from_entry(
            pdpt[pdpt_index]
        );

        if (create && (flags & VMM_USER))
        {
            pdpt[pdpt_index] |= VMM_USER;
        }
    }

    if (pd == NULL)
        return NULL;

    /* --------------------------------------------------
       Page Directory
    -------------------------------------------------- */

    uint64_t* pt;

    size_t pd_index =
        PD_INDEX(virtual_addr);

    if (!(pd[pd_index] & VMM_PRESENT))
    {
        if (!create)
            return NULL;

        pt = allocate_table();

        if (pt == NULL)
            return NULL;

        pd[pd_index] =
            virt_to_phys(pt)
            | VMM_PRESENT
            | VMM_WRITABLE
            | (flags & VMM_USER);
    }
    else
    {
        if (pd[pd_index] & VMM_HUGE)
        {
            debug_print(
                "VMM: PD huge page detected\n"
            );

            return NULL;
        }

        pt = page_table_from_entry(
            pd[pd_index]
        );

        if (create && (flags & VMM_USER))
        {
            pd[pd_index] |= VMM_USER;
        }
    }

    if (pt == NULL)
        return NULL;

    return pt;
}

/* --------------------------------------------------
   Kernel Address Space
-------------------------------------------------- */

address_space_t* vmm_kernel_space(void)
{
    return &kernel_space;
}

/* --------------------------------------------------
   VMM Initialization
-------------------------------------------------- */

void vmm_init(void)
{
    kernel_space.pml4 = current_pml4();

    if (kernel_space.pml4 == NULL)
    {
        debug_print(
            "VMM: failed to get current PML4\n"
        );

        return;
    }

    debug_print(
        "VMM: initialized\n"
    );
}

/* --------------------------------------------------
   TLB Flush
-------------------------------------------------- */

void vmm_flush(uintptr_t virtual_addr)
{
    __asm__ volatile(
        "invlpg (%0)"
        :
        : "r"(virtual_addr)
        : "memory"
    );
}

/* --------------------------------------------------
   Switch Address Space
-------------------------------------------------- */

void vmm_switch_space(address_space_t* space)
{
    if (space == NULL || space->pml4 == NULL)
        return;

    write_cr3(
        virt_to_phys(space->pml4)
    );
}

/* --------------------------------------------------
   Map Page
-------------------------------------------------- */

bool vmm_map_page(
    address_space_t* space,
    uintptr_t virtual_addr,
    phys_addr_t physical_addr,
    uint64_t flags)
{
    if (space == NULL)
    {
        debug_print(
            "VMM: NULL address space\n"
        );

        return false;
    }

    if ((virtual_addr & (PAGE_SIZE - 1)) != 0)
    {
        debug_print(
            "VMM: virtual address not aligned\n"
        );

        return false;
    }

    if ((physical_addr & (PAGE_SIZE - 1)) != 0)
    {
        debug_print(
            "VMM: physical address not aligned\n"
        );

        return false;
    }

    uint64_t* pt =
        walk_page_tables(
            space,
            virtual_addr,
            true,
            flags
        );

    if (pt == NULL)
    {
        debug_print(
            "VMM: walk_page_tables failed\n"
        );

        return false;
    }

    size_t index =
        PT_INDEX(virtual_addr);

    if (pt[index] & VMM_PRESENT)
    {
        debug_print(
            "VMM: page already mapped\n"
        );

        return false;
    }

    flags &= (
        VMM_WRITABLE |
        VMM_USER |
        VMM_PWT |
        VMM_PCD |
        VMM_GLOBAL |
        VMM_COW |
        VMM_NX
    );

    pt[index] =
        (physical_addr & PAGE_MASK)
        | flags
        | VMM_PRESENT;

    vmm_flush(virtual_addr);

    debug_print(
        "VMM: page mapped successfully\n"
    );

    return true;
}

/* --------------------------------------------------
   Unmap Page
-------------------------------------------------- */

static bool table_empty(const uint64_t *table)
{
    if (table == NULL)
        return true;

    for (size_t i = 0; i < ENTRIES_PER_TABLE; i++)
    {
        if (table[i] & VMM_PRESENT)
            return false;
    }

    return true;
}

bool vmm_unmap_page(
    address_space_t* space,
    uintptr_t virtual_addr)
{
    if (space == NULL)
        return false;

    if ((virtual_addr & (PAGE_SIZE - 1)) != 0)
        return false;

    uint64_t *pml4 = space->pml4;

    if (pml4 == NULL)
        return false;

    size_t pml4_index = PML4_INDEX(virtual_addr);
    size_t pdpt_index = PDPT_INDEX(virtual_addr);
    size_t pd_index = PD_INDEX(virtual_addr);
    size_t pt_index = PT_INDEX(virtual_addr);

    uint64_t pml4_entry = pml4[pml4_index];

    if (!(pml4_entry & VMM_PRESENT))
        return false;

    uint64_t *pdpt = page_table_from_entry(pml4_entry);
    uint64_t pdpt_entry = pdpt[pdpt_index];

    if (!(pdpt_entry & VMM_PRESENT) ||
        (pdpt_entry & VMM_HUGE))
        return false;

    uint64_t *pd = page_table_from_entry(pdpt_entry);
    uint64_t pd_entry = pd[pd_index];

    if (!(pd_entry & VMM_PRESENT) ||
        (pd_entry & VMM_HUGE))
        return false;

    uint64_t *pt = page_table_from_entry(pd_entry);
    uint64_t pt_entry = pt[pt_index];

    if (!(pt_entry & VMM_PRESENT))
        return false;

    /* Unmapping removes the virtual mapping. The physical frame is
       owned by the caller and is therefore not freed here. */
    pt[pt_index] = 0;

    vmm_flush(virtual_addr);

    /* User address spaces own their lower-half page-table hierarchy.
       Reclaim empty intermediate tables so repeated map/unmap cycles
       do not leak page-table pages. The kernel page tables are bootloader
       owned/shared and are intentionally left intact. */
    if (space != &kernel_space && table_empty(pt))
    {
        pmm_free_page((phys_addr_t)virt_to_phys(pt));
        pd[pd_index] = 0;

        if (table_empty(pd))
        {
            pmm_free_page((phys_addr_t)virt_to_phys(pd));
            pdpt[pdpt_index] = 0;

            if (table_empty(pdpt))
            {
                pmm_free_page((phys_addr_t)virt_to_phys(pdpt));
                pml4[pml4_index] = 0;
            }
        }
    }

    return true;
}
/* --------------------------------------------------
   Protect Page
-------------------------------------------------- */

bool vmm_protect_page(
    address_space_t* space,
    uintptr_t virtual_addr,
    uint64_t flags)
{

    if (space == NULL)
    {
        return false;
    }

    if ((virtual_addr & (PAGE_SIZE - 1)) != 0)
    {
        return false;
    }

    uint64_t* pt =
        walk_page_tables(
            space,
            virtual_addr,
            false,
            0
        );

    if (pt == NULL)
    {
        return false;
    }

    size_t index =
        PT_INDEX(virtual_addr);

    uint64_t entry =
        pt[index];

    if (!(entry & VMM_PRESENT))
    {
        return false;
    }

    flags &= (
    VMM_WRITABLE |
    VMM_USER |
    VMM_PWT |
    VMM_PCD |
    VMM_GLOBAL |
    VMM_COW |
    VMM_NX
);

    uint64_t preserve =
    entry & ~(
        VMM_WRITABLE |
        VMM_USER |
        VMM_PWT |
        VMM_PCD |
        VMM_GLOBAL |
        VMM_COW |
        VMM_NX
    );

    pt[index] =
        preserve | flags;

    vmm_flush(virtual_addr);

    return true;
}

/* --------------------------------------------------
   Get Page Flags
-------------------------------------------------- */

uint64_t vmm_get_page_flags(
    address_space_t* space,
    uintptr_t virtual_addr)
{
    if (space == NULL)
        return 0;

    if ((virtual_addr & (PAGE_SIZE - 1)) != 0)
        return 0;

    uint64_t* pt =
        walk_page_tables(
            space,
            virtual_addr,
            false,
            0
        );

    if (pt == NULL)
        return 0;

    uint64_t entry =
        pt[PT_INDEX(virtual_addr)];

    if (!(entry & VMM_PRESENT))
        return 0;

    return entry & (
        VMM_PRESENT |
        VMM_WRITABLE |
        VMM_USER |
        VMM_PWT |
        VMM_PCD |
        VMM_ACCESSED |
        VMM_DIRTY |
        VMM_HUGE |
        VMM_GLOBAL |
        VMM_NX
    );
}

/* --------------------------------------------------
   Translate Virtual Address
-------------------------------------------------- */

phys_addr_t vmm_translate(
    address_space_t* space,
    uintptr_t virtual_addr)
{

    if (space == NULL)
    {
        return 0;
    }

    /*
     * Translation accepts an address inside a mapped page. The
     * page-table walk itself uses the page-aligned base address,
     * while the original page offset is preserved in the result.
     */

    uintptr_t page_addr =
        virtual_addr & ~(uintptr_t)(PAGE_SIZE - 1);

    uint64_t* pt =
        walk_page_tables(
            space,
            page_addr,
            false,
            0
        );

    if (pt == NULL)
    {
        return 0;
    }

    size_t index =
        PT_INDEX(virtual_addr);

    uint64_t entry =
        pt[index];

    /*
     * Do not print the entry yet.
     * Test the PRESENT bit directly.
     */

    if ((entry & VMM_PRESENT) == 0)
    {

        return 0;
    }

    phys_addr_t physical =
        entry_address(entry);

    return physical |
           (virtual_addr & (PAGE_SIZE - 1));
}
/* --------------------------------------------------
   Create Address Space
-------------------------------------------------- */

address_space_t* vmm_create_space(void)
{
    if (kernel_space.pml4 == NULL)
        return NULL;

    struct address_space* space =
        kmalloc(
            sizeof(struct address_space)
        );

    if (space == NULL)
        return NULL;

    uint64_t* pml4 =
        allocate_table();

    if (pml4 == NULL)
    {
        kfree(space);
        return NULL;
    }

    /*
     * Copy kernel mappings.
     *
     * The upper half of the address space is
     * shared with the kernel.
     */
    for (
        size_t i = 256;
        i < ENTRIES_PER_TABLE;
        i++
    )
    {
        pml4[i] =
            kernel_space.pml4[i];
    }

    space->pml4 =
        pml4;

    return space;
}

/* --------------------------------------------------
   Destroy Address Space
-------------------------------------------------- */

void vmm_destroy_space(
    address_space_t* space)
{
    if (space == NULL ||
        space == &kernel_space ||
        space->pml4 == NULL)
        return;

    /*
     * Free only user-space page tables.
     * Kernel mappings are shared.
     */
    for (size_t i = 0; i < 256; i++)
    {
        uint64_t entry =
            space->pml4[i];

        if (!(entry & VMM_PRESENT))
            continue;

        free_page_tables(
            page_table_from_entry(entry),
            2
        );
    }

    /*
     * Free the PML4.
     */
    pmm_free_page(
        (phys_addr_t)
        virt_to_phys(space->pml4)
    );

    /*
     * Free address-space structure.
     */
    kfree(space);
}
/* --------------------------------------------------
   Demand Paging
-------------------------------------------------- */

#define MAX_DEMAND_PAGES 64

struct demand_page
{
    address_space_t* space;
    uintptr_t virtual_addr;
    uint64_t flags;
    bool used;
};

static struct demand_page demand_pages[MAX_DEMAND_PAGES];

bool vmm_register_demand_page(
    address_space_t* space,
    uintptr_t virtual_addr,
    uint64_t flags)
{
    if (space == NULL)
        return false;

    if ((virtual_addr & (PAGE_SIZE - 1)) != 0)
        return false;

    if (vmm_translate(space, virtual_addr) != 0)
        return false;

    for (size_t i = 0; i < MAX_DEMAND_PAGES; i++)
    {
        if (!demand_pages[i].used)
            continue;

        if (demand_pages[i].space == space &&
            demand_pages[i].virtual_addr == virtual_addr)
        {
            return false;
        }
    }

    for (size_t i = 0; i < MAX_DEMAND_PAGES; i++)
    {
        if (demand_pages[i].used)
            continue;

        demand_pages[i].space = space;
        demand_pages[i].virtual_addr = virtual_addr;
        demand_pages[i].flags = flags;
        demand_pages[i].used = true;

        return true;
    }

    return false;
}

bool vmm_handle_page_fault(
    address_space_t* space,
    uintptr_t virtual_addr,
    uint64_t error_code)
{
    if (space == NULL)
        return false;

    uintptr_t page =
        virtual_addr & ~(PAGE_SIZE - 1);

    /*
     * ----------------------------------------------------------
     * COW WRITE FAULT
     *
     * Present + write fault on a page marked VMM_COW.
     * ----------------------------------------------------------
     */
    if ((error_code & 1ULL) &&
        (error_code & (1ULL << 1)))
    {
        uint64_t* pt =
            walk_page_tables(
                space,
                page,
                false,
                0
            );

        if (pt != NULL)
        {
            size_t index = PT_INDEX(page);
            uint64_t entry = pt[index];

            if ((entry & VMM_PRESENT) &&
                (entry & VMM_COW))
            {
                phys_addr_t old_physical =
                    entry_address(entry);

                uint32_t refcount =
                    pmm_page_refcount(old_physical);

                /*
                 * If this is the final reference, there is
                 * nothing to copy. Just make the page writable.
                 */
                if (refcount <= 1)
                {
                    uint64_t flags =
                        entry & ~PAGE_MASK;

                    flags |= VMM_WRITABLE;
                    flags &= ~VMM_COW;

                    pt[index] =
                        old_physical |
                        flags;

                    vmm_flush(page);

                    return true;
                }

                /*
                 * Shared page:
                 * allocate a private copy.
                 */
                phys_addr_t new_physical =
                    pmm_alloc_page();

                if (new_physical == 0)
                    return false;

                /*
                 * Copy the complete 4 KiB physical page
                 * through the HHDM.
                 */
                memcpy(
                    phys_to_virt(new_physical),
                    phys_to_virt(old_physical),
                    PAGE_SIZE
                );

                /*
                 * Preserve mapping attributes, but make
                 * the new page writable and no longer COW.
                 */
                uint64_t flags =
                    entry & ~PAGE_MASK;

                flags |= VMM_WRITABLE;
                flags &= ~VMM_COW;

                pt[index] =
                    new_physical |
                    flags;

                vmm_flush(page);

                /*
                 * Drop this address space's reference to
                 * the original shared page.
                 */
                pmm_release_page(old_physical);

                return true;
            }
        }

        return false;
    }

    /*
     * ----------------------------------------------------------
     * DEMAND PAGE FAULT
     *
     * Non-present registered page.
     * ----------------------------------------------------------
     */
    if (!(error_code & 1ULL))
    {
        for (size_t i = 0; i < MAX_DEMAND_PAGES; i++)
        {
            if (!demand_pages[i].used)
                continue;

            if (demand_pages[i].space != space ||
                demand_pages[i].virtual_addr != page)
                continue;

            phys_addr_t physical =
                pmm_alloc_page();

            if (physical == 0)
                return false;

            if (!vmm_map_page(
                    space,
                    page,
                    physical,
                    demand_pages[i].flags))
            {
                pmm_free_page(physical);
                return false;
            }

            demand_pages[i].used = false;

            return true;
        }
    }

    return false;
}
