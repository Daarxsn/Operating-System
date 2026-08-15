#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "foundation/resource.h"
#include "boot/limine.h"

#include "foundation/ukom.h"
#include "foundation/capability.h"
#include "foundation/event.h"
#include "foundation/time.h"
#include "foundation/config.h"

#include "graphics/framebuffer.h"
#include "ui/ui.h"

#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/isr.h"
#include "cpu/pic.h"
#include "cpu/pit.h"
#include "cpu/irq.h"
#include "cpu/lapic.h"
#include "cpu/ioapic.h"

#include "memory/memory_map.h"
#include "memory/hhdm.h"
#include "memory/pmm.h"
#include "memory/heap.h"
#include "memory/vmm.h"

#include "process/process.h"
#include "process/thread.h"
#include "process/context.h"
#include "process/scheduler.h"
#include "process/idle.h"


#include "drivers/driver.h"
#include "drivers/keyboard.h"
#include "drivers/mouse.h"
#include "drivers/serial.h"
#include "drivers/pci.h"

#include "fs/vfs.h"
#include "fs/ramfs.h"
#include "fs/file.h"
#include "syscall/syscall.h"

#include "boot/boot.h"

#include "image/image.h"
#include "image/logo.h"

#include "../tests/tests.h"
#include "../tests/fs_tests.h"

#include "debug/print.h"
#include "debug/hex.h"
#include "logger/logger.h"
#include "string.h"


/* -------------------------------------------------
   Test Thread Declarations
------------------------------------------------- */

static void thread_a(void);
static void thread_b(void);
static void preemption_test_a(void);
static void preemption_test_b(void);

/* Shared state for the timer-preemption runtime test.  The test threads
 * deliberately never call scheduler_yield().  Thread A waits for B to run;
 * B can only run before A finishes if timer-driven preemption is working. */
static volatile bool preemption_test_b_started = false;
static volatile bool preemption_test_a_passed = false;

/* -------------------------------------------------
   Limine Requests
------------------------------------------------- */

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] =
    LIMINE_BASE_REVISION(6);


__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request =
{
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_mp_request mp_request =
{
    .id = LIMINE_MP_REQUEST_ID,
    .revision = 0,
    .flags = 0
};


__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] =
    LIMINE_REQUESTS_START_MARKER;


__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] =
    LIMINE_REQUESTS_END_MARKER;


/* -------------------------------------------------
   Idle Loop
------------------------------------------------- */

static void kernel_idle(void)
{
    while (1)
    {
        __asm__ volatile ("hlt");
    }
}


/* -------------------------------------------------
   Boot Verification
------------------------------------------------- */

static void kernel_diag_mp(void)
{
    debug_print_line("");
    debug_print_line("========== MP / LAPIC DIAGNOSTIC ==========");

    if (mp_request.response == NULL)
    {
        debug_print_line("MP Response: NULL");
        debug_print_line("Limine did not provide an MP response.");
        debug_print_line("===========================================");
        return;
    }

    struct limine_mp_response *mp = mp_request.response;

    debug_print_line("MP Response: OK");

    debug_print("CPU Count: ");
    debug_print_hex64(mp->cpu_count);
    debug_print_line("");

    debug_print("BSP LAPIC ID: ");
    debug_print_hex64(mp->bsp_lapic_id);
    debug_print_line("");

    debug_print("MP Flags: ");
    debug_print_hex64(mp->flags);
    debug_print_line("");

    debug_print("X2APIC: ");
    if (mp->flags & LIMINE_MP_RESPONSE_X86_64_X2APIC)
        debug_print_line("YES");
    else
        debug_print_line("NO");

    for (uint64_t i = 0; i < mp->cpu_count; i++)
    {
        struct limine_mp_info *cpu = mp->cpus[i];

        debug_print("CPU ");
        debug_print_hex64(i);

        debug_print(" processor_id=");
        debug_print_hex64(cpu->processor_id);

        debug_print(" lapic_id=");
        debug_print_hex64(cpu->lapic_id);

        debug_print_line("");
    }

    debug_print_line("===========================================");
}

static struct limine_framebuffer *kernel_verify_bootloader(void)
{
    if (!LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision))
        kernel_idle();

    if (framebuffer_request.response == NULL)
        kernel_idle();

    if (framebuffer_request.response->framebuffer_count == 0)
        kernel_idle();

    return framebuffer_request.response->framebuffers[0];
}

static void kernel_dump_mp_info(void)
{
    if (mp_request.response == NULL)
    {
        debug_print_line("MP: No response from Limine");
        return;
    }

    struct limine_mp_response *mp = mp_request.response;

    debug_print("MP: CPU count = ");
    debug_print_hex64(mp->cpu_count);
    debug_print_line("");

    debug_print("MP: BSP LAPIC ID = ");
    debug_print_hex64(mp->bsp_lapic_id);
    debug_print_line("");

    debug_print("MP: Flags = ");
    debug_print_hex64(mp->flags);
    debug_print_line("");

    for (uint64_t i = 0; i < mp->cpu_count; i++)
    {
        struct limine_mp_info *cpu = mp->cpus[i];

        debug_print("MP: CPU ");
        debug_print_hex64(i);

        debug_print(" processor=");
        debug_print_hex64(cpu->processor_id);

        debug_print(" LAPIC=");
        debug_print_hex64(cpu->lapic_id);

        debug_print_line("");
    }
}


/* -------------------------------------------------
   Graphics Initialization
------------------------------------------------- */

static void kernel_initialize_graphics(
    struct limine_framebuffer *fb
)
{
    framebuffer_init(fb);

    framebuffer_clear(0x1E1E2E);

    ui_init();

    boot_init();
    boot_header();

    kernel_diag_mp();
    kernel_dump_mp_info();

    boot_step_ok("Framebuffer Initialized");
    boot_step_ok("Graphics Engine Initialized");
}


/* -------------------------------------------------
   CPU Initialization
------------------------------------------------- */

static void kernel_initialize_cpu(void)
{
    gdt_init();

    boot_step_ok(
        "Global Descriptor Table Loaded"
    );

    idt_init();

    boot_step_ok(
        "Interrupt Descriptor Table Loaded"
    );

    isr_init();

    boot_step_ok(
        "Interrupt Service Routines Loaded"
    );
}


/* -------------------------------------------------
   Interrupt Initialization
------------------------------------------------- */

static void kernel_initialize_interrupts(void)
{
    
    /*
     * Keep the legacy PIC initialized for compatibility,
     * but hardware interrupt delivery will use APIC.
     */
    pic_initialize();

    boot_step_ok(
        "Programmable Interrupt Controller Initialized"
    );

    /*
     * Initialize the Local APIC first.
     */
    if (lapic_initialize())
    {
        boot_step_ok(
            "Local APIC Initialized"
        );
    }
    else
    {
        boot_step_fail(
            "Local APIC Initialization Failed"
        );

        return;
    }

    debug_print("LAPIC physical base = ");
    debug_print_hex64(
        (uint64_t)lapic_physical_base()
    );
    debug_print_line("");

    debug_print("LAPIC ID = ");
    debug_print_hex64(
        (uint64_t)lapic_id()
    );
    debug_print_line("");

    /*
     * Initialize IOAPIC.
     */
    if (ioapic_initialize())
    {
        boot_step_ok(
            "IOAPIC Initialized"
        );
    }
    else
    {
        boot_step_fail(
            "IOAPIC Initialization Failed"
        );

        return;
    }

    debug_print("IOAPIC Max IRQ = ");
    debug_print_hex64(
        (uint64_t)ioapic_max_irq()
    );
    debug_print_line("");

    /*
     * PIT still generates IRQ0.
     */
    pit_initialize(100);

    boot_step_ok(
        "Programmable Interval Timer Initialized"
    );

    /*
     * Route PIT IRQ0 to vector 32 on the BSP LAPIC.
     *
     * Start masked while programming the entry.
     */
    ioapic_set_irq(
        0,
        32,
        (uint8_t)lapic_id()
    );

    /*
     * Unmask IOAPIC IRQ0.
     */
    ioapic_unmask_irq(0);

    /*
     * IMPORTANT:
     *
     * We are now using IOAPIC/LAPIC for hardware IRQ delivery.
     * Mask the legacy PIC so it cannot compete with APIC routing.
     */
    pic_mask_irq(0);
    pic_mask_irq(1);
    pic_mask_irq(2);
    pic_mask_irq(12);

    boot_step_ok(
        "IOAPIC IRQ0 Routed To LAPIC Vector 32"
    );

    /*
     * Enable CPU interrupts only after APIC routing is ready.
     */
    __asm__ volatile ("sti");

    boot_step_ok(
        "CPU Interrupts Enabled"
    );

    boot_step_ok(
        "Programmable Interrupt Controller Initialized"
    );

    pit_initialize(100);

    boot_step_ok(
        "Programmable Interval Timer Initialized"
    );

    pic_unmask_irq(0);
    pic_unmask_irq(1);

    /*
     * IRQ12 is delivered through the slave PIC, so the master
     * cascade line (IRQ2) must also be unmasked.
     */
    pic_unmask_irq(2);
    pic_unmask_irq(12);

    boot_step_ok(
        "Timer / Keyboard / Mouse IRQs Enabled"
    );

    __asm__ volatile ("sti");

    boot_step_ok(
        "CPU Interrupts Enabled"
    );

    debug_print_line("========== IRQ0 DIAGNOSTIC ==========");

    uint64_t irq0_before = irq0_debug_get_count();

    /* Wait until at least one hardware IRQ0 arrives, or until timeout. */
    uint64_t timeout = 50000000ULL;

    while (irq0_debug_get_count() == irq0_before && timeout--)
    {
        __asm__ volatile ("pause");
    }

    debug_print("IRQ0 count = ");
    debug_print_hex64(irq0_debug_get_count());
    debug_print_line("");

    debug_print("PIT handler count = ");
    debug_print_hex64(pit_debug_get_handler_count());
    debug_print_line("");

    debug_print("PIT ticks = ");
    debug_print_hex64(pit_get_ticks());
    debug_print_line("");

    debug_print("Scheduler ticks = ");
    debug_print_hex64(scheduler_debug_get_tick_count());
    debug_print_line("");

    debug_print_line("======================================");
}


/* -------------------------------------------------
   Memory Initialization
------------------------------------------------- */

static void kernel_initialize_memory(void)
{
    memory_map_init();

    boot_step_ok(
        "Memory Map Initialized"
    );

    hhdm_init();

    boot_step_ok(
        "HHDM Initialized"
    );

    pmm_init();

    pmm_stats_t stats = pmm_get_stats();

    if (stats.free_pages > 0)
    {
        boot_step_ok(
            "PMM Has Free Pages"
        );
    }
    else
    {
        boot_step_fail(
            "PMM Has NO Free Pages"
        );
    }

    boot_step_ok(
        "Physical Memory Manager Initialized"
    );


    /* -------------------------------------------------
       Memory Statistics
    ------------------------------------------------- */

    debug_print_line("");
    debug_print_line(
        "========== MEMORY STATISTICS =========="
    );

    uint64_t total_mb =
        stats.total_memory / (1024 * 1024);

    uint64_t usable_mb =
        stats.usable_memory / (1024 * 1024);

    uint64_t reserved_mb =
        stats.reserved_memory / (1024 * 1024);

    char buffer[32];

    debug_print("Total Memory    : ");
    itoa(total_mb, buffer, 10);
    debug_print(buffer);
    debug_print_line(" MB");

    debug_print("Usable Memory   : ");
    itoa(usable_mb, buffer, 10);
    debug_print(buffer);
    debug_print_line(" MB");

    debug_print("Reserved Memory : ");
    itoa(reserved_mb, buffer, 10);
    debug_print(buffer);
    debug_print_line(" MB");

    debug_print("Total Pages     : ");
    itoa(stats.total_pages, buffer, 10);
    debug_print_line(buffer);

    debug_print("Free Pages      : ");
    itoa(stats.free_pages, buffer, 10);
    debug_print_line(buffer);

    debug_print("Used Pages      : ");
    itoa(stats.used_pages, buffer, 10);
    debug_print_line(buffer);

    debug_print("Reserved Pages  : ");
    itoa(stats.reserved_pages, buffer, 10);
    debug_print_line(buffer);

    debug_print_line(
        "======================================="
    );

    debug_print_line("");


    /* -------------------------------------------------
       Heap
    ------------------------------------------------- */

    heap_init();

    void *heap_debug = kmalloc(16);

    if (heap_debug)
    {
        boot_step_ok(
            "Heap Allocation Working"
        );

        /*
         * This allocation is only a boot-time smoke test.
         * Release it immediately so later subsystem tests start
         * from a stable heap state.
         */
        kfree(heap_debug);
    }
    else
    {
        boot_step_fail(
            "Heap Allocation Broken"
        );
    }

    boot_step_ok(
        "Kernel Heap Initialized"
    );


    /* -------------------------------------------------
       Virtual Memory
    ------------------------------------------------- */

    vmm_init();

    boot_step_ok(
        "Virtual Memory Manager Initialized"
    );
}


/* -------------------------------------------------
   Execution Initialization
------------------------------------------------- */


static void kernel_initialize_execution(void)
{
    process_initialize();

    boot_step_ok(
        "Process Manager Initialized"
    );

    thread_initialize();

    boot_step_ok(
        "Thread Manager Initialized"
    );

    context_initialize();

    boot_step_ok(
        "Context Manager Initialized"
    );

    scheduler_initialize();

    boot_step_ok(
        "Scheduler Initialized"
    );

    idle_initialize();

    if (idle_get_thread() != NULL)
    {
        boot_step_ok(
            "Idle Process Initialized"
        );
    }
    else
    {
        boot_step_fail(
            "Idle Process Initialization Failed"
        );
    }
}


/* -------------------------------------------------
   Test Threads
------------------------------------------------- */

/* Cooperative scheduler test thread A. */
static void thread_a(void)
{
    for (int i = 0; i < 10; i++)
    {
        debug_print("THREAD A\n");
        scheduler_yield();
    }

    debug_print("THREAD A FINISHED\n");
}

/* Cooperative scheduler test thread B. */
static void thread_b(void)
{
    for (int i = 0; i < 10; i++)
    {
        debug_print("THREAD B\n");
        scheduler_yield();
    }

    debug_print("THREAD B FINISHED\n");
}

/*
 * Timer-preemption test thread A.
 *
 * No scheduler_yield() is used here.  A deliberately waits for B to set
 * preemption_test_b_started.  If timer-driven preemption works, A will be
 * interrupted after its time slice and B will run.  If preemption does not
 * work, A eventually times out and reports failure instead of hanging the
 * kernel forever.
 */
static void preemption_test_a(void)
{
    const uint64_t max_wait_ticks = 30;
    const uint64_t cpu_guard = 100000000ULL;

    uint64_t start_pit_ticks = pit_get_ticks();
    uint64_t start_irq0 = irq0_debug_get_count();
    uint64_t start_pit_handlers = pit_debug_get_handler_count();
    uint64_t start_scheduler_ticks = scheduler_debug_get_tick_count();
    uint64_t start_rflags = 0;

    __asm__ volatile("pushfq; popq %0" : "=r"(start_rflags));

    debug_print("Preemption Test: RFLAGS = ");
    debug_print_hex64(start_rflags);
    debug_print(" IF = ");
    debug_print((start_rflags & (1ULL << 9)) ? "1\\n" : "0\\n");

    uint8_t pic_mask = pic_debug_get_master_mask();
    uint8_t pic_irr = pic_debug_get_master_irr();

    debug_print("Preemption Test: PIC Master Mask = ");
    debug_print_hex64((uint64_t)pic_mask);
    debug_print(" IRQ0 Masked = ");
    debug_print((pic_mask & 0x01) ? "1\\n" : "0\\n");

    debug_print("Preemption Test: PIC Master IRR = ");
    debug_print_hex64((uint64_t)pic_irr);
    debug_print(" IRQ0 Pending = ");
    debug_print((pic_irr & 0x01) ? "1\\n" : "0\\n");

    debug_print("PREEMPTION TEST A START\\n");

    /* Diagnostic: invoke the IRQ0 vector as a software interrupt. */
    uint64_t software_irq0_before = irq0_debug_get_count();
    __asm__ volatile("int $32" : : : "memory");
    uint64_t software_irq0_after = irq0_debug_get_count();

    debug_print("Preemption Test: Software INT32 Delta = ");
    debug_print_hex64(software_irq0_after - software_irq0_before);
    debug_print("\\n");

    if ((software_irq0_after - software_irq0_before) == 0)
    {
        boot_step_fail("Preemption Test: Software INT32 Did Not Reach irq_dispatch");
        return;
    }

    for (uint64_t i = 0; i < cpu_guard; ++i)
    {
        if (preemption_test_b_started)
        {
            preemption_test_a_passed = true;
            debug_print("Preemption Test: PASS\\n");
            return;
        }

        if ((pit_get_ticks() - start_pit_ticks) >= max_wait_ticks)
            break;

        __asm__ volatile("pause");
    }

    uint64_t irq0_delta =
        irq0_debug_get_count() - start_irq0;
    uint64_t pit_handler_delta =
        pit_debug_get_handler_count() - start_pit_handlers;
    uint64_t scheduler_tick_delta =
        scheduler_debug_get_tick_count() - start_scheduler_ticks;

    uint8_t end_pic_irr = pic_debug_get_master_irr();
    debug_print("Preemption Test: Final PIC Master IRR = ");
    debug_print_hex64((uint64_t)end_pic_irr);
    debug_print(" IRQ0 Pending = ");
    debug_print((end_pic_irr & 0x01) ? "1\\n" : "0\\n");

    if ((start_rflags & (1ULL << 9)) == 0)
    {
        boot_step_fail("Preemption Test: Interrupt Flag IF Is Disabled");
    }
    else if (irq0_delta == 0)
    {
        boot_step_fail("Preemption Test: IRQ0 Did Not Enter irq_dispatch");
    }
    else if (pit_handler_delta == 0)
    {
        boot_step_fail("Preemption Test: IRQ0 Entered, But PIT Handler Did Not Run");
    }
    else if (scheduler_tick_delta == 0)
    {
        boot_step_fail("Preemption Test: PIT Handler Ran, But scheduler_tick Did Not Run");
    }
    else
    {
        boot_step_fail("Preemption Test: IRQ0/PIT/Scheduler Ran, But Thread B Did Not Run");
    }
}

/*
 * Timer-preemption test thread B.  It does not yield; reaching this function
 * before A finishes proves that the timer forced a context switch.
 */
static void preemption_test_b(void)
{
    debug_print("PREEMPTION TEST B START\\n");
    preemption_test_b_started = true;

    for (volatile uint64_t i = 0; i < 1000000ULL; ++i)
        __asm__ volatile("pause");

    if (preemption_test_a_passed)
        debug_print("Preemption Test: A Observed B\\n");
}

/* -------------------------------------------------
   Kernel Services
------------------------------------------------- */

static void kernel_initialize_kernel(void)
{
    logger_init();

    /* -------------------------------------------------
       Universal Kernel Object Manager
    ------------------------------------------------- */

    xkobject_init();

    boot_step_ok(
        "UKOM Initialized"
    );


    /* -------------------------------------------------
       Capability Manager
    ------------------------------------------------- */

    xk_capability_init();

    boot_step_ok(
        "Capability Manager Initialized"
    );


    /* -------------------------------------------------
       Resource Manager
    ------------------------------------------------- */

    xk_resource_init();

    boot_step_ok(
        "Resource Manager Initialized"
    );


    /* -------------------------------------------------
       Event Manager
    ------------------------------------------------- */

    xk_event_init();

    boot_step_ok(
        "Event Manager Initialized"
    );


    /* -------------------------------------------------
       Time Manager
    ------------------------------------------------- */

    xk_time_init();

    boot_step_ok(
        "Time Manager Initialized"
    );


    /* -------------------------------------------------
       Configuration Manager
    ------------------------------------------------- */

    xk_config_init();

    boot_step_ok(
        "Configuration Manager Initialized"
    );


    /* =====================================================
       Driver Framework
       ===================================================== */

    xk_driver_manager_init();

    boot_step_ok(
        "Driver Manager Initialized"
    );


    xk_driver_register(
        &xk_keyboard_driver
    );

    boot_step_ok(
        "Keyboard Driver Registered"
    );


    xk_driver_register(
        &xk_mouse_driver
    );

    boot_step_ok(
        "Mouse Driver Registered"
    );


    xk_driver_register(
        &xk_serial_driver
    );

    boot_step_ok(
        "Serial Driver Registered"
    );


    xk_driver_register(
        &xk_pci_driver
    );

    boot_step_ok(
        "PCI Driver Registered"
    );


    /*
     * Initialize all registered drivers only after the complete
     * driver set has been registered.
     */
    xk_driver_initialize_all();

    /*
     * Kernel file-system and system-call foundations.
     */
    vfs_init();
    boot_step_ok(
        "VFS Initialized"
    );

    file_init();
    boot_step_ok(
        "File Table Initialized"
    );

    if (vfs_register_filesystem(&ramfs_filesystem) == 0 &&
        vfs_mount("/", &ramfs_filesystem) == 0)
    {
        boot_step_ok(
            "RAMFS Mounted"
        );
    }
    else
    {
        boot_step_fail(
            "RAMFS Mount Failed"
        );
    }

    syscall_init();
    boot_step_ok(
        "System Call Table Initialized"
    );


    /* =====================================================
         Process / Thread Framework Test
   ===================================================== */

process_t *kernel_process =
    process_create(
        "kernel",
        true
    );

if (kernel_process == NULL)
{
    boot_step_fail(
        "Failed To Create Kernel Process"
    );
}
else
{
    kernel_process->state =
        PROCESS_RUNNING;

    process_set_current(
        kernel_process
    );

    boot_step_ok(
        "Kernel Process Created"
    );
}


thread_t *thread_a_handle =
    thread_create(
        kernel_process,
        thread_a,
        THREAD_PRIORITY_NORMAL
    );

if (thread_a_handle == NULL)
{
    boot_step_fail(
        "Failed To Create Scheduler Test Thread A"
    );
}
else
{
    scheduler_add_thread(
        thread_a_handle
    );

    boot_step_ok(
        "Scheduler Test Thread A Created"
    );
}


thread_t *thread_b_handle =
    thread_create(
        kernel_process,
        thread_b,
        THREAD_PRIORITY_NORMAL
    );

if (thread_b_handle == NULL)
{
    boot_step_fail(
        "Failed To Create Scheduler Test Thread B"
    );
}
else
{
    scheduler_add_thread(
        thread_b_handle
    );

    boot_step_ok(
        "Scheduler Test Thread B Created"
    );
}


thread_t *preemption_test_a_handle =
    thread_create(
        kernel_process,
        preemption_test_a,
        THREAD_PRIORITY_NORMAL
    );

if (preemption_test_a_handle == NULL)
{
    boot_step_fail(
        "Failed To Create Preemption Test Thread A"
    );
}
else
{
    scheduler_add_thread(
        preemption_test_a_handle
    );

    boot_step_ok(
        "Preemption Test Thread A Created"
    );
}

thread_t *preemption_test_b_handle =
    thread_create(
        kernel_process,
        preemption_test_b,
        THREAD_PRIORITY_NORMAL
    );

if (preemption_test_b_handle == NULL)
{
    boot_step_fail(
        "Failed To Create Preemption Test Thread B"
    );
}
else
{
    scheduler_add_thread(
        preemption_test_b_handle
    );

    boot_step_ok(
        "Preemption Test Thread B Created"
    );
}

    /* -------------------------------------------------
       Known Platform Limitations
    ------------------------------------------------- */

    boot_step_warn(
        "ACPI Not Found"
    );

    if (xk_pci_device_count() > 0)
    {
        boot_step_ok(
            "PCI Enumeration Completed"
        );
    }
    else
    {
        boot_step_warn(
            "No PCI Devices Found"
        );
    }


    /* -------------------------------------------------
       Heap Test
    ------------------------------------------------- */

    void *heap_test1 = kmalloc(64);
    void *heap_test2 = kmalloc(128);

    if (heap_test1 != NULL &&
        heap_test2 != NULL)
    {
        boot_step_ok(
            "Kernel Heap Test Passed"
        );
    }
    else
    {
        boot_step_fail(
            "Kernel Heap Test Failed"
        );
    }

    kfree(heap_test1);
    kfree(heap_test2);


    /* -------------------------------------------------
       VMM Test
    ------------------------------------------------- */

    phys_addr_t page =
        pmm_alloc_page();

    if (page != 0)
    {
        const uintptr_t test_virtual =
            0xFFFF900000000000ULL;

        if (vmm_map_page(
                vmm_kernel_space(),
                test_virtual,
                page,
                VMM_WRITABLE
            ))
        {
            boot_step_ok(
                "Virtual Memory Manager Test Passed"
            );

            /*
             * This is a smoke-test mapping, not a permanent
             * kernel mapping. Remove it and release the frame.
             */
            if (!vmm_unmap_page(
                    vmm_kernel_space(),
                    test_virtual))
            {
                boot_step_fail(
                    "Virtual Memory Manager Test Cleanup Failed"
                );
            }

            pmm_free_page(page);
        }
        else
        {
            boot_step_fail(
                "Virtual Memory Manager Test Failed"
            );

            pmm_free_page(page);
        }
    }
    else
    {
        boot_step_fail(
            "PMM Allocation Failed"
        );
    }


    /* -------------------------------------------------
       Kernel Tests
    ------------------------------------------------- */

    run_kernel_tests();


    boot_success(
        "Kernel Ready"
    );
}


/* -------------------------------------------------
   Kernel Entry
------------------------------------------------- */

void kernel_main(void)
{
    struct limine_framebuffer *framebuffer =
        kernel_verify_bootloader();


    /* -------------------------------------------------
       Graphics
    ------------------------------------------------- */

    kernel_initialize_graphics(
        framebuffer
    );


    /* -------------------------------------------------
       CPU
    ------------------------------------------------- */

    kernel_initialize_cpu();


    /* -------------------------------------------------
       Memory
    ------------------------------------------------- */

    kernel_initialize_memory();


    /* -------------------------------------------------
       Execution Manager
    ------------------------------------------------- */

    kernel_initialize_execution();


    /* -------------------------------------------------
       Kernel Services / Drivers / Threads
    ------------------------------------------------- */

    kernel_initialize_kernel();


    /* -------------------------------------------------
       Interrupts
    ------------------------------------------------- */

    kernel_initialize_interrupts();


    /*
     * Start the cooperative scheduler.
     *
     * At this point all kernel initialization has completed
     * and the test threads are present in the ready queue.
     *
     * scheduler_start() performs the initial transition from
     * the kernel bootstrap context into the first thread.
     *
     * The scheduler normally does not return to kernel_main()
     * after this point.
     */
scheduler_start();


    /*
     * This point should not be reached during normal
     * cooperative scheduling.
     *
     * Keep an idle fallback in case the scheduler eventually
     * returns because of a future scheduler implementation.
     */

    kernel_idle();
}