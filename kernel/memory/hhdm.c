/*
 * hhdm.c
 * XyrisOS Kernel
 *
 * Higher Half Direct Map
 */

#include "hhdm.h"
#include "../boot/limine.h"

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