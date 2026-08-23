#include "elf.h"

#include "../memory/pmm.h"
#include "../memory/hhdm.h"
#include "../memory/heap.h"
#include "../lib/string.h"

static int range_ok(uint64_t offset, uint64_t length, uint64_t image_size)
{
    return offset <= image_size && length <= image_size - offset;
}

static void elf_load_info_reset(
    elf_load_info_t *load_info)
{
    if (load_info == NULL)
        return;

    load_info->pages = NULL;
    load_info->page_count = 0;
}

static int elf_record_page(
    elf_load_info_t *load_info,
    uintptr_t virtual_addr,
    phys_addr_t physical_addr)
{
    if (load_info == NULL)
        return -1;

    elf_page_owner_t *owner =
        (elf_page_owner_t *)kmalloc(
            sizeof(elf_page_owner_t)
        );

    if (owner == NULL)
        return -1;

    owner->virtual_addr = virtual_addr;
    owner->physical_addr = physical_addr;
    owner->next = load_info->pages;

    load_info->pages = owner;
    load_info->page_count++;

    return 0;
}

void elf_release_load(
    address_space_t *space,
    elf_load_info_t *load_info)
{
    if (load_info == NULL)
        return;

    elf_page_owner_t *owner =
        load_info->pages;

    while (owner != NULL)
    {
        elf_page_owner_t *next =
            owner->next;

        if (space != NULL)
        {
            /*
             * Remove the virtual mapping first.
             * vmm_unmap_page() does not free the
             * physical frame.
             */
            vmm_unmap_page(
                space,
                owner->virtual_addr
            );
        }

        /*
         * The physical frame belongs to this ELF load.
         */
        pmm_free_page(
            owner->physical_addr
        );

        kfree(owner);

        owner = next;
    }

    elf_load_info_reset(load_info);
}

int elf_validate(const void* image)
{
    if (image == NULL)
        return 0;

    const Elf64_Ehdr* elf =
        (const Elf64_Ehdr*)image;

    /*
     * ELF identification.
     */
    if (*(const uint32_t*)elf->e_ident != ELF_MAGIC)
        return 0;

    if (elf->e_ident[4] != ELFCLASS64 ||
        elf->e_ident[5] != ELFDATA2LSB)
    {
        return 0;
    }

    /*
     * Machine / ELF version.
     */
    if (elf->e_machine != EM_X86_64 ||
        elf->e_version != 1)
    {
        return 0;
    }

    /*
     * This loader only supports the ELF64 executable format
     * represented by these exact structures.
     */
    if (elf->e_ehsize != sizeof(Elf64_Ehdr) ||
        elf->e_phentsize != sizeof(Elf64_Phdr) ||
        elf->e_phnum == 0)
    {
        return 0;
    }

    /*
     * Program-header offset must not wrap when the table
     * address is calculated.
     *
     * elf_validate() does not know the image buffer size,
     * so complete buffer bounds checking is performed by
     * elf_load_into_space().
     */
    if (elf->e_phoff > UINT64_MAX -
        ((uint64_t)elf->e_phnum * sizeof(Elf64_Phdr)))
    {
        return 0;
    }

    return 1;
}

uint64_t elf_get_entry(const void* image)
{
    return elf_validate(image) ? ((const Elf64_Ehdr*)image)->e_entry : 0;
}

/*
 * The legacy entry point remains a validation-only framework API.  The
 * caller must provide an address space to actually load program segments.
 */
int elf_load(const void* image)
{
    return elf_validate(image) ? 0 : -1;
}

int elf_load_into_space(
        const void *image,
    size_t image_size,
    address_space_t *space,
    uint64_t *entry_out,
    elf_load_info_t *load_info)
{
    if (!elf_validate(image) ||
        !space ||
        !entry_out ||
        !load_info)
    {
        return -1;
    }

    elf_load_info_reset(load_info);

    if (image_size < sizeof(Elf64_Ehdr))
        return -1;

    const Elf64_Ehdr* elf =
        (const Elf64_Ehdr*)image;

    /*
     * ----------------------------------------------------
     * Validate program-header table bounds.
     * ----------------------------------------------------
     */

    uint64_t phdr_size =
        (uint64_t)elf->e_phnum *
        sizeof(Elf64_Phdr);

    if (!range_ok(
            elf->e_phoff,
            phdr_size,
            image_size))
    {
        return -1;
    }

    const Elf64_Phdr* phdr =
        (const Elf64_Phdr*)
        ((const uint8_t*)image + elf->e_phoff);

    /*
     * ----------------------------------------------------
     * Validate the entry point.
     *
     * It must belong to an executable PT_LOAD segment.
     * ----------------------------------------------------
     */

    bool entry_valid = false;

    /*
     * ----------------------------------------------------
     * Phase A:
     *
     * Validate every loadable segment completely before
     * allocating or modifying the address space.
     * ----------------------------------------------------
     */

    for (uint16_t i = 0;
         i < elf->e_phnum;
         ++i)
    {
        const Elf64_Phdr* seg =
            &phdr[i];

        if (seg->p_type != PT_LOAD)
            continue;

        /*
         * Filesz must never exceed Memsz.
         */
        if (seg->p_memsz < seg->p_filesz)
            return -1;

        /*
         * A zero-sized memory segment has nothing to map.
         */
        if (seg->p_memsz == 0)
            continue;

        /*
         * File-backed data must remain completely inside
         * the supplied image.
         */
        if (!range_ok(
                seg->p_offset,
                seg->p_filesz,
                image_size))
        {
            return -1;
        }

        /*
         * Reject virtual-address arithmetic overflow.
         */
        if (seg->p_vaddr >
            UINT64_MAX - seg->p_memsz)
        {
            return -1;
        }

        uint64_t end_value =
            seg->p_vaddr + seg->p_memsz;

        /*
         * User-space addresses must remain below the
         * canonical user-space limit used by XyrisOS.
         */
        if (seg->p_vaddr >=
                0x0000800000000000ULL ||
            end_value >
                0x0000800000000000ULL)
        {
            return -1;
        }

        /*
         * ELF PT_LOAD requires the file offset and virtual
         * address to have the same page offset.
         */
        if ((seg->p_offset & (PAGE_SIZE - 1)) !=
            (seg->p_vaddr & (PAGE_SIZE - 1)))
        {
            return -1;
        }

        /*
         * Page-end calculation must not overflow.
         */
        if (end_value >
            UINT64_MAX - (PAGE_SIZE - 1))
        {
            return -1;
        }

        uintptr_t start =
            (uintptr_t)(
                seg->p_vaddr &
                ~(uint64_t)(PAGE_SIZE - 1)
            );

        uintptr_t end =
            (uintptr_t)(
                (end_value + PAGE_SIZE - 1) &
                ~(uint64_t)(PAGE_SIZE - 1)
            );

        if (end <= start ||
            start >= 0x0000800000000000ULL ||
            end > 0x0000800000000000ULL)
        {
            return -1;
        }

        /*
         * Check whether the ELF entry point belongs to
         * this executable load segment.
         */
        if ((elf->e_entry >= seg->p_vaddr) &&
            (elf->e_entry < end_value) &&
            (seg->p_flags & PF_X))
        {
            entry_valid = true;
        }
    }

    /*
     * A valid executable image must have its entry point
     * inside an executable PT_LOAD segment.
     */
    if (!entry_valid)
        return -1;

    /*
     * ----------------------------------------------------
     * Phase B:
     *
     * All ELF metadata has passed validation.
     *
     * Allocate, map and populate the PT_LOAD segments.
     *
     * Full rollback/ownership tracking is the next Member 8
     * step; this phase deliberately does not change that
     * ownership model yet.
     * ----------------------------------------------------
     */

    for (uint16_t i = 0;
         i < elf->e_phnum;
         ++i)
    {
        const Elf64_Phdr* seg =
            &phdr[i];

        if (seg->p_type != PT_LOAD ||
            seg->p_memsz == 0)
        {
            continue;
        }

        uintptr_t start =
            (uintptr_t)(
                seg->p_vaddr &
                ~(uint64_t)(PAGE_SIZE - 1)
            );

        uint64_t end_value =
            seg->p_vaddr + seg->p_memsz;

        uintptr_t end =
            (uintptr_t)(
                (end_value + PAGE_SIZE - 1) &
                ~(uint64_t)(PAGE_SIZE - 1)
            );

        uint64_t flags =
            VMM_USER;

        if (seg->p_flags & PF_W)
            flags |= VMM_WRITABLE;

        if (!(seg->p_flags & PF_X))
            flags |= VMM_NX;

        /*
         * Allocate pages that do not already exist.
         */
        for (uintptr_t va = start;
             va < end;
             va += PAGE_SIZE)
        {
            if (vmm_translate(space, va) != 0)
                continue;

            phys_addr_t phys =
                pmm_alloc_page();

            if (phys == 0)
            {
                elf_release_load(
                    space,
                    load_info
                );

                return -1;
            }

            uint8_t* page =
                (uint8_t*)phys_to_virt(phys);

            memset(
                page,
                0,
                PAGE_SIZE
            );

            if (!vmm_map_page(
                    space,
                    va,
                    phys,
                    flags))
            {
                pmm_free_page(phys);

                elf_release_load(
                    space,
                    load_info
                );

                return -1;
            }

            if (elf_record_page(
                    load_info,
                    va,
                    phys) != 0)
            {
                /*
                * The page is mapped, so remove the mapping
                * before returning the physical frame.
                */
                vmm_unmap_page(
                    space,
                    va
                );

                pmm_free_page(phys);

                elf_release_load(
                    space,
                    load_info
                );

                return -1;
            }
        }

        /*
         * Copy the file-backed portion into the mapped
         * pages. The remaining p_memsz - p_filesz bytes
         * were already zeroed above, providing BSS.
         */
        const uint8_t* source =
            (const uint8_t*)image +
            seg->p_offset;

        uint64_t remaining =
            seg->p_filesz;

        uint64_t source_offset =
            seg->p_vaddr &
            (PAGE_SIZE - 1);

        uintptr_t va = start;

        while (remaining != 0)
        {
            uint64_t chunk =
                PAGE_SIZE - source_offset;

            if (chunk > remaining)
                chunk = remaining;

            phys_addr_t phys =
                vmm_translate(space, va) &
                ~(phys_addr_t)(PAGE_SIZE - 1);

            if (phys == 0)
            {
                elf_release_load(
                    space,
                    load_info
                );

                return -1;
            }

            uint8_t* destination =
                (uint8_t*)phys_to_virt(phys);

            memcpy(
                destination + source_offset,
                source,
                (size_t)chunk
            );

            source += chunk;
            remaining -= chunk;
            va += PAGE_SIZE;
            source_offset = 0;
        }
    }

    *entry_out = elf->e_entry;

    return 0;
}