#include "gdt.h"

static gdt_entry_t gdt[7];
static tss_t tss;
static gdt_ptr_t gdt_ptr;

static uint8_t tss_stack[16 * 1024] __attribute__((aligned(16)));

static void gdt_set_entry(int index, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t granularity)
{
    gdt[index].limit_low = limit & 0xFFFF;
    gdt[index].base_low = base & 0xFFFF;
    gdt[index].base_mid = (base >> 16) & 0xFF;
    gdt[index].base_high = (base >> 24) & 0xFF;
    gdt[index].access = access;
    gdt[index].granularity = ((limit >> 16) & 0x0F) | (granularity & 0xF0);
}

static void gdt_set_tss(uint64_t base, uint32_t limit)
{
    uint64_t low = 0;
    uint64_t high = 0;

    low |= (limit & 0xFFFFULL);
    low |= (base & 0xFFFFFFULL) << 16;
    low |= (uint64_t)0x89 << 40;
    low |= ((limit >> 16) & 0xFULL) << 48;
    low |= ((base >> 24) & 0xFFULL) << 56;
    high = base >> 32;

    *((uint64_t*)&gdt[3]) = low;
    *((uint64_t*)&gdt[4]) = high;
}

static inline void gdt_load(gdt_ptr_t* ptr)
{
    __asm__ volatile(
        "lgdt (%0)\n\t"
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"
        "pushq $0x08\n\t"
        "leaq 1f(%%rip), %%rax\n\t"
        "pushq %%rax\n\t"
        "lretq\n\t"
        "1:\n\t"
        : : "r"(ptr) : "rax", "memory"
    );

    __asm__ volatile(
        "mov $0x18, %%ax\n\t"
        "ltr %%ax\n\t"
        : : : "ax", "memory"
    );
}

void gdt_init(void)
{
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint64_t)&gdt;

    gdt_set_entry(0, 0, 0, 0, 0);
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xA0); /* 0x08 kernel code */
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xC0); /* 0x10 kernel data */

    tss.rsp0 = (uint64_t)(tss_stack + sizeof(tss_stack));
    tss.io_map_base = sizeof(tss);
    gdt_set_tss((uint64_t)&tss, sizeof(tss) - 1); /* 0x18 */

    gdt_set_entry(5, 0, 0xFFFFF, 0xFA, 0xA0); /* 0x28 user code */
    gdt_set_entry(6, 0, 0xFFFFF, 0xF2, 0xC0); /* 0x30 user data */

    gdt_load(&gdt_ptr);
}
