#ifndef XYRIS_VMM_H
#define XYRIS_VMM_H

#include <stdint.h>
#include <stdbool.h>

#include "pmm.h"

/* --------------------------------------------------
   x86-64 Page Flags
-------------------------------------------------- */

typedef enum
{
    VMM_PRESENT  = 1ULL << 0,
    VMM_WRITABLE = 1ULL << 1,
    VMM_USER     = 1ULL << 2,
    VMM_PWT      = 1ULL << 3,
    VMM_PCD      = 1ULL << 4,
    VMM_ACCESSED = 1ULL << 5,
    VMM_DIRTY    = 1ULL << 6,
    VMM_HUGE     = 1ULL << 7,
    VMM_GLOBAL   = 1ULL << 8,
    VMM_NX       = 1ULL << 63

} vmm_flags_t;

/* --------------------------------------------------
   Address Space
-------------------------------------------------- */

struct address_space;
typedef struct address_space address_space_t;

/* --------------------------------------------------
   Initialization
-------------------------------------------------- */

void vmm_init(void);

/* --------------------------------------------------
   Kernel Address Space
-------------------------------------------------- */

address_space_t* vmm_kernel_space(void);

/* --------------------------------------------------
   Mapping
-------------------------------------------------- */

bool vmm_map_page(
    address_space_t* space,
    uintptr_t virtual_addr,
    phys_addr_t physical_addr,
    uint64_t flags);

bool vmm_unmap_page(
    address_space_t* space,
    uintptr_t virtual_addr);

    bool vmm_protect_page(
    address_space_t* space,
    uintptr_t virtual_addr,
    uint64_t flags);

    uint64_t vmm_get_page_flags(
    address_space_t* space,
    uintptr_t virtual_addr);

phys_addr_t vmm_translate(
    address_space_t* space,
    uintptr_t virtual_addr);

/* --------------------------------------------------
   Address Spaces
-------------------------------------------------- */

address_space_t* vmm_create_space(void);

void vmm_destroy_space(address_space_t* space);

void vmm_switch_space(address_space_t* space);

/* --------------------------------------------------
   TLB
-------------------------------------------------- */

void vmm_flush(uintptr_t virtual_addr);

#endif