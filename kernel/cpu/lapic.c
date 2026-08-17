#include "lapic.h"

#include "../memory/hhdm.h"
#include "../memory/vmm.h"
#include <stdint.h>
#include <stdbool.h>

#define LAPIC_SPURIOUS_VECTOR 0xFF

static volatile uint8_t *lapic_base =
    (volatile uint8_t *)0;

static uintptr_t lapic_phys =
    LAPIC_DEFAULT_PHYS;

static bool lapic_initialized =
    false;

/* -------------------------------------------------
   MSR helpers
------------------------------------------------- */

static uint64_t lapic_rdmsr(uint32_t msr)
{
    uint32_t low;
    uint32_t high;

    __asm__ volatile (
        "rdmsr"
        : "=a"(low), "=d"(high)
        : "c"(msr)
    );

    return ((uint64_t)high << 32) | low;
}

static void lapic_wrmsr(uint32_t msr, uint64_t value)
{
    uint32_t low =
        (uint32_t)(value & 0xFFFFFFFFULL);

    uint32_t high =
        (uint32_t)(value >> 32);

    __asm__ volatile (
        "wrmsr"
        :
        : "c"(msr),
          "a"(low),
          "d"(high)
    );
}

/* -------------------------------------------------
   MMIO
------------------------------------------------- */

uint32_t lapic_read(uint32_t offset)
{
    if (lapic_base == (volatile uint8_t *)0)
        return 0;

    volatile uint32_t *reg =
        (volatile uint32_t *)(lapic_base + offset);

    return *reg;
}

void lapic_write(uint32_t offset, uint32_t value)
{
    if (lapic_base == (volatile uint8_t *)0)
        return;

    volatile uint32_t *reg =
        (volatile uint32_t *)(lapic_base + offset);

    *reg = value;

    /*
     * Ensure the MMIO write has completed before returning.
     */
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

bool lapic_initialize(void)
{
    uint64_t apic_base =
        lapic_rdmsr(IA32_APIC_BASE_MSR);

    /*
     * If x2APIC is already enabled, this simple xAPIC
     * MMIO implementation cannot operate on the LAPIC.
     *
     * QEMU's normal x86-64 configuration for this project
     * uses xAPIC, so fail cleanly instead of touching the
     * wrong interface.
     */
    if (apic_base & LAPIC_BASE_X2APIC)
    {
        lapic_initialized = false;
        return false;
    }

    /*
     * Obtain the LAPIC physical address from IA32_APIC_BASE.
     *
     * Bits 12..35 contain the physical base.
     */
    lapic_phys =
        (uintptr_t)(apic_base & 0x0000000FFFFF000ULL);

    if (lapic_phys == 0)
        lapic_phys = LAPIC_DEFAULT_PHYS;

    /*
     * Enable the Local APIC through IA32_APIC_BASE.
     */
    apic_base |= LAPIC_BASE_ENABLE;

    lapic_wrmsr(
        IA32_APIC_BASE_MSR,
        apic_base
    );

    /*
     * Map the physical LAPIC through the HHDM.
     */
    
      if (!hhdm_map_mmio(
        lapic_phys,
        0x1000
    ))
{
    lapic_initialized = false;
    return false;
}

lapic_base =
    (volatile uint8_t *)phys_to_virt(
        lapic_phys
    );
    /*
     * Set the Task Priority Register to zero.
     *
     * This allows all normal-priority interrupts.
     */
    lapic_write(
        LAPIC_REG_TPR,
        0
    );

    /*
     * Enable the LAPIC and assign a spurious vector.
     *
     * Bit 8 = APIC software enable.
     * Vector 0xFF is reserved here for the spurious vector.
     */
    lapic_write(
        LAPIC_REG_SVR,
        LAPIC_SPURIOUS_VECTOR | (1U << 8)
    );

    lapic_initialized = true;

    return true;
}

/* -------------------------------------------------
   Enable
------------------------------------------------- */

void lapic_enable(void)
{
    if (!lapic_initialized)
        return;

    uint32_t svr =
        lapic_read(LAPIC_REG_SVR);

    svr |= (1U << 8);

    lapic_write(
        LAPIC_REG_SVR,
        svr
    );
}

/* -------------------------------------------------
   End Of Interrupt
------------------------------------------------- */

void lapic_eoi(void)
{
    if (!lapic_initialized)
        return;

    lapic_write(
        LAPIC_REG_EOI,
        0
    );
}

/* -------------------------------------------------
   Information
------------------------------------------------- */

uintptr_t lapic_physical_base(void)
{
    return lapic_phys;
}

uint32_t lapic_id(void)
{
    if (!lapic_initialized)
        return 0;

    /*
     * xAPIC ID occupies bits 24..31.
     */
    return lapic_read(LAPIC_REG_ID) >> 24;
}

bool lapic_is_enabled(void)
{
    return lapic_initialized;
}