#include "idt.h"
#include "isr.h"

static idt_entry_t idt[256];
static idt_ptr_t idt_ptr;

/* CPU Exception ISR Table (0-31) */
static void (*const isr_table[32])(void) =
{
    isr0,  isr1,  isr2,  isr3,
    isr4,  isr5,  isr6,  isr7,
    isr8,  isr9,  isr10, isr11,
    isr12, isr13, isr14, isr15,
    isr16, isr17, isr18, isr19,
    isr20, isr21, isr22, isr23,
    isr24, isr25, isr26, isr27,
    isr28, isr29, isr30, isr31
};

static void (*const irq_table[16])(void) =
{
    irq0, irq1, irq2, irq3,
    irq4, irq5, irq6, irq7,
    irq8, irq9, irq10, irq11,
    irq12, irq13, irq14, irq15
};

static void idt_set_gate(
    uint8_t vector,
    uint64_t handler,
    uint16_t selector,
    uint8_t type_attr,
    uint8_t ist)
{
    idt[vector].offset_low  = (uint16_t)(handler & 0xFFFF);
    idt[vector].selector    = selector;
    idt[vector].ist         = ist;
    idt[vector].type_attr   = type_attr;
    idt[vector].offset_mid  = (uint16_t)((handler >> 16) & 0xFFFF);
    idt[vector].offset_high = (uint32_t)((handler >> 32) & 0xFFFFFFFF);
    idt[vector].zero        = 0;
}

static inline void idt_load(idt_ptr_t *ptr)
{
    __asm__ volatile(
        "lidt (%0)"
        :
        : "r"(ptr)
        : "memory"
    );
}

void idt_init(void)
{
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (uint64_t)&idt;

    /* Install CPU Exception Handlers (Vectors 0-31) */
    for (int i = 0; i < 32; i++)
    {
        idt_set_gate(
            (uint8_t)i,
            (uint64_t)isr_table[i],
            0x08,      /* Kernel Code Segment */
            0x8E,      /* Present | Ring 0 | Interrupt Gate */
            0          /* IST */
        );
    }

    /* Install Hardware IRQs (Vectors 32–47) */
for (int i = 0; i < 16; i++)
{
    idt_set_gate(
        (uint8_t)(32 + i),
        (uint64_t)irq_table[i],
        0x08,
        0x8E,
        0
    );
}

/* System Call Vector (0x80 / 128) */

idt_set_gate(
    128,
    (uint64_t)isr128,
    0x08,
    0xEE,
    0
);

    idt_load(&idt_ptr);
}