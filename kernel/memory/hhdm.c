/*
 * hhdm.c
 * XyrisOS Kernel
 *
 * Higher Half Direct Map
 */

#include "hhdm.h"
#include "../boot/limine.h"
#include "vmm.h"
#include <stdint.h>
#include <stddef.h>

static uintptr_t hhdm_base = 0;

/* --------------------------------------------------
   Limine Request
-------------------------------------------------- */

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request =
{
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0
};

/* --------------------------------------------------
   Initialize
-------------------------------------------------- */

void hhdm_init(void)
{
    if (hhdm_request.response == NULL)
    {
        for (;;)
            __asm__ volatile ("hlt");
    }

    hhdm_base = hhdm_request.response->offset;
}

/* --------------------------------------------------
   Return HHDM Offset
-------------------------------------------------- */

uintptr_t hhdm_offset(void)
{
    return hhdm_base;
}

/* --------------------------------------------------
   Physical -> Virtual
-------------------------------------------------- */

void* phys_to_virt(uintptr_t physical)
{
    if (hhdm_base == 0)
    {
        for (;;)
            __asm__ volatile("hlt");
    }

    return (void *)(physical + hhdm_base);
}

/* --------------------------------------------------
   Virtual -> Physical
-------------------------------------------------- */

uintptr_t virt_to_phys(void *virtual_address)
{
    return (uintptr_t)virtual_address - hhdm_base;
}

bool hhdm_map_mmio(
    uintptr_t physical,
    size_t length
)
{
    if (length == 0)
        return false;

    const uintptr_t page_mask = 0xFFFULL;

    uintptr_t phys_start =
        physical & ~page_mask;

    uintptr_t phys_end =
        (physical + length + page_mask)
        & ~page_mask;

    if (phys_end < phys_start)
        return false;

    address_space_t *kernel =
        vmm_kernel_space();

    if (kernel == NULL)
        return false;

    for (uintptr_t phys = phys_start;
         phys < phys_end;
         phys += 0x1000)
    {
        uintptr_t virt =
            hhdm_base + phys;

        if (!vmm_map_page(
                kernel,
                virt,
                phys,
                VMM_WRITABLE |
                VMM_PWT |
                VMM_PCD |
                VMM_GLOBAL |
                VMM_NX))
        {
            /*
             * The page may already be mapped by
             * the bootloader. Verify that case.
             */
            phys_addr_t mapped =
                vmm_translate(
                    kernel,
                    virt
                );

            if ((mapped & ~page_mask) != phys)
                return false;
        }

        vmm_flush(virt);
    }

    return true;
}