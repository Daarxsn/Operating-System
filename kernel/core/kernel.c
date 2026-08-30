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

extern volatile uint64_t context_switch_if0_count;
extern const unsigned char _binary_test_elf_start[];
extern const unsigned char _binary_test_elf_end[];
extern const unsigned char _binary_test_elf_size[];

/* -------------------------------------------------
   Test Thread Declarations
------------------------------------------------- */

static void thread_a(void);
static void thread_b(void);
static void preemption_test_a(void);
static void preemption_test_b(void);

static void context_test_a(void);
static void context_test_b(void);
static void context_test_coordinator(void);
static volatile uint64_t context_test_initial_if0_count = 0;

/* Shared state for the timer-preemption runtime test.  The test threads
 * deliberately never call scheduler_yield().  Thread A waits for B to run;
 * B can only run before A finishes if timer-driven preemption is working. */
static volatile bool preemption_test_b_started = false;
static volatile bool preemption_test_a_passed = false;

/* -------------------------------------------------
   Context-Switch Runtime Test State
------------------------------------------------- */

#define CONTEXT_STRESS_ITERATIONS 10000ULL

static volatile uint64_t context_test_a_runs = 0;
static volatile uint64_t context_test_b_runs = 0;
static volatile uint64_t context_test_failures = 0;

static volatile bool context_test_a_done = false;
static volatile bool context_test_b_done = false;
static volatile bool context_test_complete = false;

static const uint64_t context_test_a_seed =
    0x1111111111111111ULL;

static const uint64_t context_test_b_seed =
    0xAAAAAAAAAAAAAAAAULL;

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
     * Initialize the legacy PIC so the existing x86 interrupt
     * infrastructure is left in a known state. It will be completely
     * masked before CPU interrupts are enabled.
     */
    pic_initialize();

    boot_step_ok(
        "Programmable Interrupt Controller Initialized"
    );

    /* -------------------------------------------------
       Local APIC
    ------------------------------------------------- */

    if (!lapic_initialize())
    {
        boot_step_fail(
            "Local APIC Initialization Failed"
        );
        return;
    }

    boot_step_ok(
        "Local APIC Initialized"
    );

    debug_print("LAPIC physical base = ");
    debug_print_hex64(
        (uint64_t)lapic_physical_base()
    );
    debug_print_line("");

    const uint8_t destination_lapic =
        (uint8_t)lapic_id();

    debug_print("LAPIC ID = ");
    debug_print_hex64(
        (uint64_t)destination_lapic
    );
    debug_print_line("");

    /* -------------------------------------------------
       IOAPIC
    ------------------------------------------------- */

    if (!ioapic_initialize())
    {
        boot_step_fail(
            "IOAPIC Initialization Failed"
        );
        return;
    }

    boot_step_ok(
        "IOAPIC Initialized"
    );

    debug_print("IOAPIC Max IRQ = ");
    debug_print_hex64(
        (uint64_t)ioapic_max_irq()
    );
    debug_print_line("");

    /* -------------------------------------------------
       PIT
    ------------------------------------------------- */

    pit_initialize(100);

    boot_step_ok(
        "Programmable Interval Timer Initialized"
    );

    /* -------------------------------------------------
       IOAPIC routing
    ------------------------------------------------- */

    /*
     * The current QEMU Q35 platform uses the ACPI MADT-style
     * interrupt-source override:
     *
     *     ISA IRQ0 (PIT) -> GSI 2 -> IOAPIC input 2
     *
     * We do not yet parse MADT, so this mapping is supplied by the
     * small platform fallback in ioapic_isa_irq_to_gsi().
     */
    const uint8_t pit_gsi =
        ioapic_isa_irq_to_gsi(0);

    const uint8_t keyboard_gsi =
        ioapic_isa_irq_to_gsi(1);

    const uint8_t mouse_gsi =
        ioapic_isa_irq_to_gsi(12);

    ioapic_set_irq(
        pit_gsi,
        32,
        destination_lapic
    );

    ioapic_set_irq(
        keyboard_gsi,
        33,
        destination_lapic
    );

    ioapic_set_irq(
        mouse_gsi,
        44,
        destination_lapic
    );

    /* -------------------------------------------------
       APIC routing diagnostic
    ------------------------------------------------- */

    debug_print_line(
        "========== IOAPIC ROUTING DIAGNOSTIC =========="
    );

    debug_print("PIT ISA IRQ = ");
    debug_print_hex64(0);
    debug_print(" -> GSI = ");
    debug_print_hex64((uint64_t)pit_gsi);
    debug_print_line("");

    debug_print("IOAPIC GSI 0 LOW  = ");
    debug_print_hex64(
        (uint64_t)ioapic_read_redir_low(0)
    );
    debug_print_line("");

    debug_print("IOAPIC GSI 0 HIGH = ");
    debug_print_hex64(
        (uint64_t)ioapic_read_redir_high(0)
    );
    debug_print_line("");

    debug_print("IOAPIC PIT LOW  = ");
    debug_print_hex64(
        (uint64_t)ioapic_read_redir_low(pit_gsi)
    );
    debug_print_line("");

    debug_print("IOAPIC PIT HIGH = ");
    debug_print_hex64(
        (uint64_t)ioapic_read_redir_high(pit_gsi)
    );
    debug_print_line("");

    debug_print("IOAPIC GSI 1 LOW  = ");
    debug_print_hex64(
        (uint64_t)ioapic_read_redir_low(keyboard_gsi)
    );
    debug_print_line("");

    debug_print("IOAPIC GSI 12 LOW = ");
    debug_print_hex64(
        (uint64_t)ioapic_read_redir_low(mouse_gsi)
    );
    debug_print_line("");

    debug_print_line(
        "==============================================="
    );

    /*
     * Unmask only the APIC inputs we have explicitly programmed.
     * They remain masked while the redirection entries are written.
     */
    ioapic_unmask_irq(pit_gsi);
    ioapic_unmask_irq(keyboard_gsi);
    ioapic_unmask_irq(mouse_gsi);

    /*
     * Disable legacy PIC delivery completely. This is important:
     * hardware IRQs are now acknowledged with LAPIC EOI, not PIC EOI.
     */
    for (uint8_t irq = 0; irq < 16U; ++irq)
        pic_mask_irq(irq);

    boot_step_ok(
        "IOAPIC PIT IRQ0 Routed To LAPIC Vector 32"
    );

    /*
     * Enable CPU interrupt recognition only after the complete APIC
     * route has been programmed and the legacy PIC has been masked.
     */
    __asm__ volatile ("sti");

    boot_step_ok(
        "CPU Interrupts Enabled"
    );

    /* -------------------------------------------------
       Hardware IRQ0 diagnostic
    ------------------------------------------------- */

    debug_print_line(
        "========== IRQ0 DIAGNOSTIC =========="
    );

    uint64_t irq0_before =
        irq0_debug_get_count();

    /* Wait for one real hardware IRQ0, with a bounded timeout. */
    uint64_t timeout = 50000000ULL;

    while (irq0_debug_get_count() == irq0_before && timeout--)
    {
        __asm__ volatile ("pause");
    }

    debug_print("IRQ0 count = ");
    debug_print_hex64(
        irq0_debug_get_count()
    );
    debug_print_line("");

    debug_print("PIT handler count = ");
    debug_print_hex64(
        pit_debug_get_handler_count()
    );
    debug_print_line("");

    debug_print("PIT ticks = ");
    debug_print_hex64(
        pit_get_ticks()
    );
    debug_print_line("");

    debug_print("Scheduler ticks = ");
    debug_print_hex64(
        scheduler_debug_get_tick_count()
    );
    debug_print_line("");

    debug_print("Final PIT LOW = ");
    debug_print_hex64(
        (uint64_t)ioapic_read_redir_low(pit_gsi)
    );
    debug_print_line("");

    debug_print_line(
        "====================================="
    );
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
    uint64_t start_if0_context_switches =
    context_switch_if0_count;
    uint64_t start_rflags = 0;

    __asm__ volatile("pushfq; popq %0" : "=r"(start_rflags));

    debug_print("Preemption Test: RFLAGS = ");
    debug_print_hex64(start_rflags);
    debug_print(" IF = ");
    debug_print((start_rflags & (1ULL << 9)) ? "1\n" : "0\n");

    uint8_t pic_mask = pic_debug_get_master_mask();
    uint8_t pic_irr = pic_debug_get_master_irr();

    debug_print("Preemption Test: PIC Master Mask = ");
    debug_print_hex64((uint64_t)pic_mask);
    debug_print(" IRQ0 Masked = ");
    debug_print((pic_mask & 0x01) ? "1\n" : "0\n");

    debug_print("Preemption Test: PIC Master IRR = ");
    debug_print_hex64((uint64_t)pic_irr);
    debug_print(" IRQ0 Pending = ");
    debug_print((pic_irr & 0x01) ? "1\n" : "0\n");

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
        uint64_t if0_context_switch_delta =
        context_switch_if0_count -
        start_if0_context_switches;

    uint8_t end_pic_irr = pic_debug_get_master_irr();
    debug_print("Preemption Test: Final PIC Master IRR = ");
    debug_print_hex64((uint64_t)end_pic_irr);
    debug_print(" IRQ0 Pending = ");
    debug_print((end_pic_irr & 0x01) ? "1\n" : "0\n");

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
    else if (if0_context_switch_delta == 0)
    {
        boot_step_fail("Preemption Test: IRQ Context Did Not Save IF=0");
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
   Context-Switch Runtime Test
------------------------------------------------- */

static void context_test_a(void)
{
    volatile uint64_t stack_sentinel =
        0x13579BDF2468ACE0ULL;

    for (uint64_t i = 0;
         i < CONTEXT_STRESS_ITERATIONS;
         ++i)
    {
        const uint64_t expected =
            context_test_a_seed ^ i;

        uint64_t rbx_value;
        uint64_t r12_value;
        uint64_t r13_value;
        uint64_t r14_value;
        uint64_t r15_value;

        __asm__ volatile(
            "mov %[value], %%rbx\n"
            "mov %[value], %%r12\n"
            "mov %[value], %%r13\n"
            "mov %[value], %%r14\n"
            "mov %[value], %%r15\n"
            :
            : [value] "r"(expected)
            : "rbx", "r12", "r13", "r14", "r15"
        );

        stack_sentinel =
            (stack_sentinel << 7) ^
            (stack_sentinel >> 3) ^
            expected;

        scheduler_yield();

        __asm__ volatile(
            "mov %%rbx, %[rbx]\n"
            "mov %%r12, %[r12]\n"
            "mov %%r13, %[r13]\n"
            "mov %%r14, %[r14]\n"
            "mov %%r15, %[r15]\n"
            : [rbx] "=r"(rbx_value),
              [r12] "=r"(r12_value),
              [r13] "=r"(r13_value),
              [r14] "=r"(r14_value),
              [r15] "=r"(r15_value)
        );

        if (rbx_value != expected ||
            r12_value != expected ||
            r13_value != expected ||
            r14_value != expected ||
            r15_value != expected)
        {
            context_test_failures++;
            context_test_a_done = true;

            debug_print(
                "CONTEXT TEST A: REGISTER PRESERVATION FAIL\n"
            );

            return;
        }

        if (stack_sentinel == 0)
        {
            context_test_failures++;
            context_test_a_done = true;

            debug_print(
                "CONTEXT TEST A: STACK STATE FAIL\n"
            );

            return;
        }

        context_test_a_runs++;
    }

    context_test_a_done = true;

    debug_print(
        "CONTEXT TEST A COMPLETE\n"
    );

    while (!context_test_complete)
        scheduler_yield();
}


static void context_test_b(void)
{
    volatile uint64_t stack_sentinel =
        0x0FEDCBA987654321ULL;

    for (uint64_t i = 0;
         i < CONTEXT_STRESS_ITERATIONS;
         ++i)
    {
        const uint64_t expected =
            context_test_b_seed ^ i;

        uint64_t rbx_value;
        uint64_t r12_value;
        uint64_t r13_value;
        uint64_t r14_value;
        uint64_t r15_value;

        __asm__ volatile(
            "mov %[value], %%rbx\n"
            "mov %[value], %%r12\n"
            "mov %[value], %%r13\n"
            "mov %[value], %%r14\n"
            "mov %[value], %%r15\n"
            :
            : [value] "r"(expected)
            : "rbx", "r12", "r13", "r14", "r15"
        );

        stack_sentinel =
            (stack_sentinel << 5) ^
            (stack_sentinel >> 2) ^
            expected;

        scheduler_yield();

        __asm__ volatile(
            "mov %%rbx, %[rbx]\n"
            "mov %%r12, %[r12]\n"
            "mov %%r13, %[r13]\n"
            "mov %%r14, %[r14]\n"
            "mov %%r15, %[r15]\n"
            : [rbx] "=r"(rbx_value),
              [r12] "=r"(r12_value),
              [r13] "=r"(r13_value),
              [r14] "=r"(r14_value),
              [r15] "=r"(r15_value)
        );

        if (rbx_value != expected ||
            r12_value != expected ||
            r13_value != expected ||
            r14_value != expected ||
            r15_value != expected)
        {
            context_test_failures++;
            context_test_b_done = true;

            debug_print(
                "CONTEXT TEST B: REGISTER PRESERVATION FAIL\n"
            );

            return;
        }

        if (stack_sentinel == 0)
        {
            context_test_failures++;
            context_test_b_done = true;

            debug_print(
                "CONTEXT TEST B: STACK STATE FAIL\n"
            );

            return;
        }

        context_test_b_runs++;
    }

    context_test_b_done = true;

    debug_print(
        "CONTEXT TEST B COMPLETE\n"
    );

    while (!context_test_complete)
        scheduler_yield();
}


static void context_test_coordinator(void)
{
    context_test_initial_if0_count =
    context_switch_if0_count;

    while (!context_test_a_done ||
           !context_test_b_done)
    {   
        if (context_test_failures != 0)
        break;

        scheduler_yield();
    }

    if (context_test_failures != 0)
{
    boot_step_fail(
        "Context Test: Register/Stack Preservation"
    );
}
else if (context_test_a_runs !=
             CONTEXT_STRESS_ITERATIONS ||
         context_test_b_runs !=
             CONTEXT_STRESS_ITERATIONS)
{
    boot_step_fail(
        "Context Test: Iteration Count"
    );
}
else
{
    boot_step_ok(
        "Context Test: Register Preservation"
    );

    boot_step_ok(
        "Context Test: Stack Preservation"
    );

    boot_step_ok(
        "Context Test: Context-Switch Stress"
    );

    boot_step_ok(
        "Context Test: Cooperative IF Preservation"
    );
}

    context_test_complete = true;
}

static process_t *user_test_process = NULL;

static void launch_user_test_process(void)
{
    const unsigned char *elf_start =
    _binary_test_elf_start;

    size_t elf_size =
        (size_t)(_binary_test_elf_end -
                _binary_test_elf_start);

    if (elf_start == NULL || elf_size == 0)
    {
        boot_step_fail(
            "Ring3 Test: Embedded ELF Missing"
        );
        return;
    }

    user_test_process =
        process_create_user(
            "ring3-test",
            elf_start,
            elf_size
        );

    if (user_test_process != NULL)
    {
        debug_print(
            "RING3 TEST: process_create_user SUCCESS\n"
        );
    }
    else
    {
        debug_print(
            "RING3 TEST: process_create_user FAILED\n"
        );
    }

    if (user_test_process == NULL)
    {
        boot_step_fail(
            "Ring3 Test: process_create_user Failed"
        );
        return;
    }

    if (user_test_process->main_thread == NULL)
    {
        boot_step_fail(
            "Ring3 Test: Main User Thread Missing"
        );
        return;
    }

    boot_step_ok(
        "Ring3 Test: User Process Created"
    );

    boot_step_ok(
        "Ring3 Test: User Thread Scheduled"
    );
}

static void user_test_waiter(void)
{
    if (user_test_process == NULL)
    {
        boot_step_fail(
            "Ring3 Test: Waiter Has No User Process"
        );
        return;
    }

    int32_t exit_code = -1;

    int result =
        process_wait(
            user_test_process->pid,
            &exit_code
        );

    if (result != 0)
    {
        boot_step_fail(
            "Ring3 Test: Parent Wait Failed"
        );
        return;
    }

    if (exit_code != 42)
    {
        boot_step_fail(
            "Ring3 Test: Wrong Exit Status"
        );
        return;
    }

    boot_step_ok(
        "Ring3 Test: Parent Wait"
    );

    boot_step_ok(
        "Ring3 Test: Parent Reap"
    );

    user_test_process = NULL;

    scheduler_exit_current();
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

thread_t *context_test_a_handle =
    thread_create(
        kernel_process,
        context_test_a,
        THREAD_PRIORITY_NORMAL
    );

if (context_test_a_handle == NULL)
{
    boot_step_fail(
        "Failed To Create Context Test Thread A"
    );
}
else
{
    scheduler_add_thread(
        context_test_a_handle
    );

    boot_step_ok(
        "Context Test Thread A Created"
    );
}


thread_t *context_test_b_handle =
    thread_create(
        kernel_process,
        context_test_b,
        THREAD_PRIORITY_NORMAL
    );

if (context_test_b_handle == NULL)
{
    boot_step_fail(
        "Failed To Create Context Test Thread B"
    );
}
else
{
    scheduler_add_thread(
        context_test_b_handle
    );

    boot_step_ok(
        "Context Test Thread B Created"
    );
}


thread_t *context_test_coordinator_handle =
    thread_create(
        kernel_process,
        context_test_coordinator,
        THREAD_PRIORITY_NORMAL
    );

if (context_test_coordinator_handle == NULL)
{
    boot_step_fail(
        "Failed To Create Context Test Coordinator"
    );
}
else
{
    scheduler_add_thread(
        context_test_coordinator_handle
    );

    boot_step_ok(
        "Context Test Coordinator Created"
    );
}

debug_print("RING3 TEST: ABOUT TO LAUNCH\n");
launch_user_test_process();
thread_t *user_test_waiter_handle =
    thread_create(
        kernel_process,
        user_test_waiter,
        THREAD_PRIORITY_NORMAL
    );

if (user_test_waiter_handle == NULL)
{
    boot_step_fail(
        "Failed To Create Ring3 Test Waiter"
    );
}
else
{
    scheduler_add_thread(
        user_test_waiter_handle
    );

    boot_step_ok(
        "Ring3 Test Waiter Created"
    );
}
debug_print("RING3 TEST: LAUNCH CALL RETURNED\n");

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