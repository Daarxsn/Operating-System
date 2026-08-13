#include "elf.h"

#include "../memory/pmm.h"
#include "../memory/hhdm.h"
#include "../lib/string.h"

static int range_ok(uint64_t offset, uint64_t length, uint64_t image_size)
{
    return offset <= image_size && length <= image_size - offset;
}

int elf_validate(const void* image)
{
    if (!image)
        return 0;

    const Elf64_Ehdr* elf = image;

    if (*(const uint32_t*)elf->e_ident != ELF_MAGIC)
        return 0;

    if (elf->e_ident[4] != ELFCLASS64 ||
        elf->e_ident[5] != ELFDATA2LSB ||
        elf->e_machine != EM_X86_64 ||
        elf->e_version != 1 ||
        elf->e_ehsize != sizeof(Elf64_Ehdr) ||
        elf->e_phentsize != sizeof(Elf64_Phdr) ||
        elf->e_phnum == 0)
        return 0;

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

int elf_load_into_space(const void* image, size_t image_size,
                        address_space_t* space, uint64_t* entry_out)
{
    if (!elf_validate(image) || !space || !entry_out)
        return -1;

    const Elf64_Ehdr* elf = image;

    if (image_size < sizeof(Elf64_Ehdr) ||
        !range_ok(elf->e_phoff, (uint64_t)elf->e_phnum * sizeof(Elf64_Phdr), image_size))
        return -1;

    const Elf64_Phdr* phdr =
        (const Elf64_Phdr*)((const uint8_t*)image + elf->e_phoff);

    /* This API receives an in-memory image; validate the program-header
       table and each segment's internal arithmetic before touching memory. */
    uint64_t loaded_entry = elf->e_entry;

    for (uint16_t i = 0; i < elf->e_phnum; ++i)
    {
        const Elf64_Phdr* seg = &phdr[i];
        if (seg->p_type != PT_LOAD)
            continue;

        if (seg->p_memsz < seg->p_filesz ||
            seg->p_vaddr + seg->p_memsz < seg->p_vaddr ||
            !range_ok(seg->p_offset, seg->p_filesz, image_size))
            return -1;

        uintptr_t start = (uintptr_t)(seg->p_vaddr & ~(uint64_t)(PAGE_SIZE - 1));
        uint64_t end_value = seg->p_vaddr + seg->p_memsz;
        uintptr_t end = (uintptr_t)((end_value + PAGE_SIZE - 1) & ~(uint64_t)(PAGE_SIZE - 1));

        if (end <= start || start >= 0x0000800000000000ULL || end > 0x0000800000000000ULL)
            return -1;

        uint64_t flags = VMM_USER;
        if (seg->p_flags & PF_W)
            flags |= VMM_WRITABLE;
        if (!(seg->p_flags & PF_X))
            flags |= VMM_NX;

        for (uintptr_t va = start; va < end; va += PAGE_SIZE)
        {
            if (vmm_translate(space, va) != 0)
                continue;

            phys_addr_t phys = pmm_alloc_page();
            if (!phys)
                return -1;

            uint8_t* page = phys_to_virt(phys);
            memset(page, 0, PAGE_SIZE);

            if (!vmm_map_page(space, va, phys, flags))
            {
                pmm_free_page(phys);
                return -1;
            }
        }

        /* Copy the file-backed bytes into the newly mapped pages.  The
           caller is responsible for ensuring the image buffer is resident;
           this loader deliberately has no dependency on a filesystem. */
        const uint8_t* source = (const uint8_t*)image + seg->p_offset;
        uint64_t remaining = seg->p_filesz;
        uint64_t source_offset = seg->p_vaddr & (PAGE_SIZE - 1);
        uintptr_t va = start;

        while (remaining)
        {
            uint64_t chunk = PAGE_SIZE - source_offset;
            if (chunk > remaining)
                chunk = remaining;

            phys_addr_t phys = vmm_translate(space, va) & ~(phys_addr_t)(PAGE_SIZE - 1);
            uint8_t* destination = phys_to_virt(phys);
            memcpy(destination + source_offset, source, (size_t)chunk);

            source += chunk;
            remaining -= chunk;
            va += PAGE_SIZE;
            source_offset = 0;
        }
    }

    *entry_out = loaded_entry;
    return 0;
}
