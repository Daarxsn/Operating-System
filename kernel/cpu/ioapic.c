#include "ioapic.h"

#include "../memory/hhdm.h"
#include "../memory/vmm.h"
#include <stdint.h>
#include <stdbool.h>

#define IOAPIC_MMIO_INDEX 0x00
#define IOAPIC_MMIO_DATA  0x10

/*
 * Redirection-table LOW register bits.
 */
#define IOAPIC_REDIR_VECTOR_MASK   0x000000FFU
#define IOAPIC_REDIR_DELIVERY_FIXED 0x00000000U
#define IOAPIC_REDIR_DEST_PHYSICAL  0x00000000U

#define IOAPIC_REDIR_POLARITY_HIGH  0x00000000U
#define IOAPIC_REDIR_TRIGGER_EDGE   0x00000000U

#define IOAPIC_REDIR_MASKED         (1U << 16)

/*
 * Redirection-table HIGH register.
 *
 * Destination APIC ID occupies bits 24..31.
 */
#define IOAPIC_REDIR_DEST_SHIFT 24

static volatile uint8_t *ioapic_base =
    (volatile uint8_t *)0;

static uintptr_t ioapic_phys =
    IOAPIC_DEFAULT_PHYS;

static uint8_t ioapic_max_redir =
    0;

static bool ioapic_initialized =
    false;

/* -------------------------------------------------
   Raw MMIO access
------------------------------------------------- */

static void ioapic_select(uint8_t reg)
{
    if (ioapic_base == (volatile uint8_t *)0)
        return;

    volatile uint32_t *index =
        (volatile uint32_t *)
            (ioapic_base + IOAPIC_MMIO_INDEX);

    *index = reg;

    __asm__ volatile (
        "mfence"
        :
        :
        : "memory"
    );
}

uint32_t ioapic_read(uint8_t reg)
{
    if (ioapic_base == (volatile uint8_t *)0)
        return 0;

    ioapic_select(reg);

    volatile uint32_t *data =
        (volatile uint32_t *)
            (ioapic_base + IOAPIC_MMIO_DATA);

    return *data;
}

void ioapic_write(uint8_t reg, uint32_t value)
{
    if (ioapic_base == (volatile uint8_t *)0)
        return;

    ioapic_select(reg);

    volatile uint32_t *data =
        (volatile uint32_t *)
            (ioapic_base + IOAPIC_MMIO_DATA);

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
    if (!hhdm_map_mmio(
        ioapic_phys,
        0x1000
    ))
{
    ioapic_initialized = false;
    return false;
}

ioapic_base =
    (volatile uint8_t *)phys_to_virt(
        ioapic_phys
    );

    /*
     * IOAPICVER:
     *
     * bits 23:16 = maximum redirection entry.
     */
    uint32_t version =
        ioapic_read(IOAPIC_REG_VERSION);

    uint8_t max_entry =
        (uint8_t)((version >> 16) & 0xFF);

    /*
     * QEMU reports 23, meaning IRQs 0..23.
     */
    ioapic_max_redir =
        max_entry;

    /*
     * Mask every redirection entry first.
     *
     * This prevents unexpected devices from generating
     * interrupts before their handlers/routing are ready.
     */
    for (uint8_t irq = 0;
         irq <= ioapic_max_redir;
         irq++)
    {
        ioapic_mask_irq(irq);
    }

    ioapic_initialized = true;

    return true;
}

/* -------------------------------------------------
   Configure IRQ
------------------------------------------------- */

void ioapic_set_irq(
    uint8_t irq,
    uint8_t vector,
    uint8_t destination
)
{
    if (!ioapic_initialized)
        return;

    if (irq > ioapic_max_redir)
        return;

    /*
     * Fixed delivery:
     *
     * delivery mode = 000
     * destination mode = physical
     * polarity = active high
     * trigger = edge
     *
     * Start masked.
     */
    uint32_t low =
        ((uint32_t)vector &
         IOAPIC_REDIR_VECTOR_MASK)
        | IOAPIC_REDIR_DELIVERY_FIXED
        | IOAPIC_REDIR_DEST_PHYSICAL
        | IOAPIC_REDIR_POLARITY_HIGH
        | IOAPIC_REDIR_TRIGGER_EDGE
        | IOAPIC_REDIR_MASKED;

    uint32_t high =
        ((uint32_t)destination <<
         IOAPIC_REDIR_DEST_SHIFT);

    uint8_t low_reg =
        (uint8_t)(
            IOAPIC_REG_REDIR_BASE +
            irq * 2
        );

    uint8_t high_reg =
        (uint8_t)(low_reg + 1);

    /*
     * Write high first.
     *
     * This prevents a partially programmed entry from
     * targeting an unintended processor.
     */
    ioapic_write(
        high_reg,
        high
    );

    ioapic_write(
        low_reg,
        low
    );
}

/* -------------------------------------------------
   Mask
------------------------------------------------- */

void ioapic_mask_irq(uint8_t irq)
{
    if (!ioapic_initialized && ioapic_base == 0)
        return;

    if (irq > ioapic_max_redir)
        return;

    uint8_t low_reg =
        (uint8_t)(
            IOAPIC_REG_REDIR_BASE +
            irq * 2
        );

    uint32_t low =
        ioapic_read(low_reg);

    low |= IOAPIC_REDIR_MASKED;

    ioapic_write(
        low_reg,
        low
    );
}

/* -------------------------------------------------
   Unmask
------------------------------------------------- */

void ioapic_unmask_irq(uint8_t irq)
{
    if (!ioapic_initialized)
        return;

    if (irq > ioapic_max_redir)
        return;

    uint8_t low_reg =
        (uint8_t)(
            IOAPIC_REG_REDIR_BASE +
            irq * 2
        );

    uint32_t low =
        ioapic_read(low_reg);

    low &= ~IOAPIC_REDIR_MASKED;

    ioapic_write(
        low_reg,
        low
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

/*
 * Legacy ISA -> IOAPIC GSI mapping fallback.
 *
 * The PIT is ISA IRQ0, but on the PC/Q35 legacy-replacement wiring
 * represented by QEMU it enters the IOAPIC on GSI 2. ACPI MADT
 * interrupt-source overrides should replace this fallback on real
 * hardware when platform discovery is added.
 */
uint8_t ioapic_isa_irq_to_gsi(uint8_t irq)
{
    return (irq == 0U) ? 2U : irq;
}
