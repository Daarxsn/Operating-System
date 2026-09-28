#include "ioapic.h"

#include "../memory/hhdm.h"
#include "../debug/print.h"

#include <stdint.h>
#include <stdbool.h>

#define IOAPIC_MMIO_INDEX 0x00
#define IOAPIC_MMIO_DATA  0x10

/* Redirection-table LOW register fields. */
#define IOAPIC_REDIR_VECTOR_MASK      0x000000FFU
#define IOAPIC_REDIR_DELIVERY_FIXED   0x00000000U
#define IOAPIC_REDIR_DEST_PHYSICAL    0x00000000U
#define IOAPIC_REDIR_POLARITY_HIGH    0x00000000U
#define IOAPIC_REDIR_TRIGGER_EDGE     0x00000000U
#define IOAPIC_REDIR_MASKED           (1U << 16)

/* Redirection-table HIGH register destination field. */
#define IOAPIC_REDIR_DEST_SHIFT 24

static volatile uint8_t *ioapic_base =
    (volatile uint8_t *)0;

static uintptr_t ioapic_phys =
    IOAPIC_DEFAULT_PHYS;

static uint8_t ioapic_max_redir = 0;
static bool ioapic_initialized = false;

/* -------------------------------------------------
   Raw MMIO access
------------------------------------------------- */

static void ioapic_select(uint8_t reg)
{
    if (ioapic_base == (volatile uint8_t *)0)
        return;

    debug_print_line("IOAPIC TRACE: before index write");

    volatile uint32_t *index =
        (volatile uint32_t *)(ioapic_base + IOAPIC_MMIO_INDEX);

    *index = (uint32_t)reg;

    debug_print_line("IOAPIC TRACE: after index write");

    __asm__ volatile (
        "mfence"
        :
        :
        : "memory"
    );

    debug_print_line("IOAPIC TRACE: after index mfence");
}

uint32_t ioapic_read(uint8_t reg)
{
    if (ioapic_base == (volatile uint8_t *)0)
        return 0;

    debug_print_line("IOAPIC TRACE: before ioapic_select");

    ioapic_select(reg);

    debug_print_line("IOAPIC TRACE: after ioapic_select");
    debug_print_line("IOAPIC TRACE: before data read");

    volatile uint32_t *data =
        (volatile uint32_t *)(ioapic_base + IOAPIC_MMIO_DATA);

    uint32_t value = *data;

    debug_print_line("IOAPIC TRACE: after data read");

    return value;
}

void ioapic_write(uint8_t reg, uint32_t value)
{
    if (ioapic_base == (volatile uint8_t *)0)
        return;

    ioapic_select(reg);

    volatile uint32_t *data =
        (volatile uint32_t *)(ioapic_base + IOAPIC_MMIO_DATA);

    *data = value;

    __asm__ volatile (
        "mfence"
        :
        :
        : "memory"
    );
}

/* -------------------------------------------------
   Initialization
------------------------------------------------- */

bool ioapic_initialize(void)
{
    debug_print_line("IOAPIC TRACE: entered initialize");
    debug_print_line("IOAPIC TRACE: before hhdm_map_mmio");

    if (!hhdm_map_mmio(ioapic_phys, 0x1000)) {
        ioapic_initialized = false;
        ioapic_base = (volatile uint8_t *)0;
        return false;
    }

    debug_print_line("IOAPIC TRACE: after hhdm_map_mmio");
    debug_print_line("IOAPIC TRACE: before phys_to_virt");

    ioapic_base = (volatile uint8_t *)phys_to_virt(ioapic_phys);

    debug_print_line("IOAPIC TRACE: after phys_to_virt");

    if (ioapic_base == (volatile uint8_t *)0) {
        ioapic_initialized = false;
        return false;
    }

    debug_print_line("IOAPIC TRACE: before version read");

    uint32_t version = ioapic_read(IOAPIC_REG_VERSION);

    debug_print_line("IOAPIC TRACE: after version read");

    uint8_t max_entry = (uint8_t)((version >> 16) & 0xFFU);

    if (max_entry == 0 || max_entry > 239U) {
        ioapic_initialized = false;
        return false;
    }

    ioapic_max_redir = max_entry;
    ioapic_initialized = true;

    for (uint16_t input = 0;
         input <= ioapic_max_redir;
         ++input)
    {
        ioapic_mask_irq((uint8_t)input);
    }

    return true;
}

/* -------------------------------------------------
   Configure redirection entry
------------------------------------------------- */

void ioapic_set_irq(
    uint8_t input,
    uint8_t vector,
    uint8_t destination
)
{
    if (!ioapic_initialized)
        return;

    if (input > ioapic_max_redir)
        return;

    /*
     * Fixed delivery, physical destination, active-high, edge-triggered.
     * Keep the entry masked until the caller explicitly unmasks it.
     */
    uint32_t low =
        ((uint32_t)vector & IOAPIC_REDIR_VECTOR_MASK)
        | IOAPIC_REDIR_DELIVERY_FIXED
        | IOAPIC_REDIR_DEST_PHYSICAL
        | IOAPIC_REDIR_POLARITY_HIGH
        | IOAPIC_REDIR_TRIGGER_EDGE
        | IOAPIC_REDIR_MASKED;

    uint32_t high =
        ((uint32_t)destination << IOAPIC_REDIR_DEST_SHIFT);

    uint8_t low_reg =
        (uint8_t)(IOAPIC_REG_REDIR_BASE + input * 2U);

    uint8_t high_reg =
        (uint8_t)(low_reg + 1U);

    /* Program destination before publishing the low entry. */
    ioapic_write(high_reg, high);
    ioapic_write(low_reg, low);
}

/* -------------------------------------------------
   Mask / unmask
------------------------------------------------- */

void ioapic_mask_irq(uint8_t input)
{
    if (!ioapic_initialized)
        return;

    if (input > ioapic_max_redir)
        return;

    uint8_t low_reg =
        (uint8_t)(IOAPIC_REG_REDIR_BASE + input * 2U);

    uint32_t low = ioapic_read(low_reg);
    low |= IOAPIC_REDIR_MASKED;
    ioapic_write(low_reg, low);
}

void ioapic_unmask_irq(uint8_t input)
{
    if (!ioapic_initialized)
        return;

    if (input > ioapic_max_redir)
        return;

    uint8_t low_reg =
        (uint8_t)(IOAPIC_REG_REDIR_BASE + input * 2U);

    uint32_t low = ioapic_read(low_reg);
    low &= ~IOAPIC_REDIR_MASKED;
    ioapic_write(low_reg, low);
}

/* -------------------------------------------------
   Redirection diagnostics
------------------------------------------------- */

uint32_t ioapic_read_redir_low(uint8_t input)
{
    if (!ioapic_initialized || input > ioapic_max_redir)
        return 0;

    return ioapic_read(
        (uint8_t)(IOAPIC_REG_REDIR_BASE + input * 2U)
    );
}

uint32_t ioapic_read_redir_high(uint8_t input)
{
    if (!ioapic_initialized || input > ioapic_max_redir)
        return 0;

    return ioapic_read(
        (uint8_t)(IOAPIC_REG_REDIR_BASE + input * 2U + 1U)
    );
}

/* -------------------------------------------------
   Information
------------------------------------------------- */

uint8_t ioapic_max_irq(void)
{
    return ioapic_max_redir;
}

bool ioapic_is_initialized(void)
{
    return ioapic_initialized;
}

/* -------------------------------------------------
   ISA -> GSI fallback
------------------------------------------------- */

uint8_t ioapic_isa_irq_to_gsi(uint8_t irq)
{
    /*
     * QEMU Q35 exposes the PIT's ISA IRQ0 through GSI 2 because the
     * ACPI interrupt-source override remaps ISA IRQ0 to IOAPIC input 2.
     * IRQ1 and IRQ12 remain their corresponding GSI numbers.
     */
    if (irq == 0U)
        return 2U;

    return irq;
}