#ifndef XYRIS_IOAPIC_H
#define XYRIS_IOAPIC_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Standard x86 IOAPIC physical address used by QEMU's
 * PC/Q35 platform.
 *
 * Later this can be replaced with ACPI MADT discovery.
 */
#define IOAPIC_DEFAULT_PHYS 0xFEC00000ULL

/*
 * IOAPIC registers.
 */
#define IOAPIC_REG_ID       0x00
#define IOAPIC_REG_VERSION  0x01
#define IOAPIC_REG_ARB      0x02

/*
 * Redirection table starts at register 0x10.
 *
 * Each IRQ consumes two registers:
 *
 *   0x10 + IRQ*2       LOW
 *   0x10 + IRQ*2 + 1   HIGH
 */
#define IOAPIC_REG_REDIR_BASE 0x10

bool ioapic_initialize(void);

uint32_t ioapic_read(uint8_t reg);

void ioapic_write(uint8_t reg, uint32_t value);

void ioapic_set_irq(
    uint8_t irq,
    uint8_t vector,
    uint8_t destination
);

void ioapic_mask_irq(uint8_t irq);

void ioapic_unmask_irq(uint8_t irq);

uint8_t ioapic_max_irq(void);

bool ioapic_is_initialized(void);

/*
 * Convert a legacy ISA IRQ number to the IOAPIC GSI used by the
 * platform's legacy interrupt wiring.  Without ACPI MADT overrides,
 * XyrisOS uses the standard/QEMU PC mapping where PIT IRQ0 is GSI 2.
 */
uint8_t ioapic_isa_irq_to_gsi(uint8_t irq);

#endif