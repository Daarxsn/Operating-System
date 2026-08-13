#ifndef XYRIS_ELF_H
#define XYRIS_ELF_H

#include <stdint.h>
#include <stddef.h>
#include "../memory/vmm.h"

#define ELF_MAGIC 0x464C457F
#define ELFCLASS64 2
#define ELFDATA2LSB 1
#define EM_X86_64 0x3E
#define PT_LOAD 1
#define PF_X 0x1
#define PF_W 0x2
#define PF_R 0x4

typedef struct __attribute__((packed))
{
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct __attribute__((packed))
{
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf64_Phdr;

int elf_validate(const void* image);
uint64_t elf_get_entry(const void* image);
int elf_load(const void* image);
int elf_load_into_space(const void* image, size_t image_size,
                        address_space_t* space, uint64_t* entry_out);

#endif
