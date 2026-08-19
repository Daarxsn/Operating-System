#ifndef XYRIS_IOAPIC_H
#define XYRIS_IOAPIC_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Standard IOAPIC physical address used by the QEMU Q35 machine.
 *
 * This is a fallback until XyrisOS grows ACPI MADT discovery.
 */
#define IOAPIC_DEFAULT_PHYS 0xFEC00000ULL

/* IOAPIC register numbers. */
#define IOAPIC_REG_ID       0x00
#define IOAPIC_REG_VERSION  0x01
#define IOAPIC_REG_ARB      0x02

/*
 * Redirection table starts at register 0x10.
 * Each input consumes two registers:
 *
 *   0x10 + input * 2       LOW
 *   0x10 + input * 2 + 1   HIGH
 */
#define IOAPIC_REG_REDIR_BASE 0x10

/* Redirection-table LOW register bits. */
#define IOAPIC_REDIR_MASKED (1U << 16)

bool ioapic_initialize(void);

uint32_t ioapic_read(uint8_t reg);
void ioapic_write(uint8_t reg, uint32_t value);

void ioapic_set_irq(
    uint8_t input,
    uint8_t vector,
    uint8_t destination
);

void ioapic_mask_irq(uint8_t input);
void ioapic_unmask_irq(uint8_t input);

uint8_t ioapic_max_irq(void);
bool ioapic_is_initialized(void);

/*
 * Read a redirection-table entry. These helpers are used by the
 * boot diagnostic to verify exactly which IOAPIC input was programmed.
 */
uint32_t ioapic_read_redir_low(uint8_t input);
uint32_t ioapic_read_redir_high(uint8_t input);

/*
 * Legacy ISA IRQ -> IOAPIC input mapping used by the current QEMU Q35
 * platform. In the Q35 ACPI topology, ISA IRQ0 (the PIT) is presented
 * through the interrupt-source override as GSI/input 2.
 *
 * This remains a fallback until ACPI MADT parsing is implemented.
 */
uint8_t ioapic_isa_irq_to_gsi(uint8_t irq);

#endif