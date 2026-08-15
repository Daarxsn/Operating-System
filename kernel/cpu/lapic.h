#ifndef XYRIS_LAPIC_H
#define XYRIS_LAPIC_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Local APIC physical address used by the x86/xAPIC interface.
 *
 * The APIC base is normally reported by IA32_APIC_BASE (MSR 0x1B).
 * LAPIC_DEFAULT_PHYS is used as the architectural fallback.
 */
#define LAPIC_DEFAULT_PHYS 0xFEE00000ULL

/*
 * LAPIC register offsets.
 */
#define LAPIC_REG_ID       0x020
#define LAPIC_REG_VERSION  0x030
#define LAPIC_REG_TPR      0x080
#define LAPIC_REG_EOI      0x0B0
#define LAPIC_REG_SVR      0x0F0
#define LAPIC_REG_ICR_LOW  0x300
#define LAPIC_REG_ICR_HIGH 0x310

/*
 * IA32_APIC_BASE MSR.
 */
#define IA32_APIC_BASE_MSR 0x1B

/*
 * IA32_APIC_BASE bits.
 */
#define LAPIC_BASE_ENABLE  (1ULL << 11)
#define LAPIC_BASE_X2APIC  (1ULL << 10)

bool lapic_initialize(void);

void lapic_enable(void);

void lapic_eoi(void);

uint32_t lapic_read(uint32_t offset);

void lapic_write(uint32_t offset, uint32_t value);

uintptr_t lapic_physical_base(void);

uint32_t lapic_id(void);

bool lapic_is_enabled(void);

#endif