#include "init.h"

/*
 * ============================================================
 * XyrisOS Kernel Initialization Manager
 * ============================================================
 */

/* CPU */

#include "../cpu/gdt.h"
#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../cpu/pic.h"
#include "../cpu/pit.h"

/* Process */

#include "../process/process.h"
#include "../process/thread.h"
#include "../process/scheduler.h"
#include "../process/idle.h"

/*
 * ------------------------------------------------------------
 * Private Function Prototypes
 * ------------------------------------------------------------
 */

static void initialize_cpu(void);
static void initialize_memory(void);
static void initialize_kernel_foundation(void);
static void initialize_process_manager(void);
static void initialize_drivers(void);

/*
 * ============================================================
 * CPU Initialization
 * ============================================================
 */

static void initialize_cpu(void)
{
    gdt_init();

    idt_init();

    isr_init();

    pic_initialize();

    pit_initialize(100);

    pic_unmask_irq(0);

    __asm__ volatile("sti");
}

/*
 * ============================================================
 * Memory Initialization
 * ============================================================
 */

static void initialize_memory(void)
{
    /*
     * Phase 6
     *
     * Physical Memory Manager
     * Virtual Memory Manager
     * Kernel Heap
     */
}

/*
 * ============================================================
 * Kernel Foundation Initialization
 * ============================================================
 */

static void initialize_kernel_foundation(void)
{
    /*
     * Phase 2
     *
     * UKOM
     * Capability Manager
     * Resource Manager
     * Event Manager
     * Time Manager
     */
}

/*
 * ============================================================
 * Process & Scheduler Initialization
 * ============================================================
 */

static void initialize_process_manager(void)
{
    process_initialize();

    thread_initialize();

    scheduler_initialize();

    idle_initialize();

    scheduler_start();
}

/*
 * ============================================================
 * Driver Initialization
 * ============================================================
 */

static void initialize_drivers(void)
{
    /*
     * Future
     *
     * PCI
     * PS/2
     * USB
     * AHCI
     * Keyboard
     * Mouse
     * Storage
     */
}

/*
 * ============================================================
 * Kernel Initialization
 * ============================================================
 */

void kernel_initialize(void)
{
    initialize_cpu();

    initialize_memory();

    initialize_kernel_foundation();

    initialize_process_manager();

    initialize_drivers();
}