#include "panic.h"

#include "../graphics/framebuffer.h"
#include "../debug/print.h"
#include "../debug/hex.h"
#include "../cpu/cpu.h"

#define PANIC_BACKGROUND 0x2B0000

static void panic_header(void)
{
    debug_print_line("==================================================");
    debug_print_line("              XYRISOS KERNEL PANIC");
    debug_print_line("==================================================");
    debug_print_line("");
}

static void panic_reason(registers_t *regs, const char *reason)
{
    debug_print("Reason      : ");
    debug_print_line(reason);

    debug_print("Vector      : ");
    debug_print_hex64(regs->vector);

    debug_print("Error Code  : ");
    debug_print_hex64(regs->error_code);

    debug_print_line("");
}

static void panic_cpu_state(registers_t *regs)
{
    debug_print_line("CPU State");
    debug_print_line("");

    debug_print("RIP         : ");
    debug_print_hex64(regs->rip);

    debug_print("CS          : ");
    debug_print_hex64(regs->cs);

    debug_print("RFLAGS      : ");
    debug_print_hex64(regs->rflags);

    debug_print("RSP         : ");
    debug_print_hex64(regs->rsp);

    debug_print("SS          : ");
    debug_print_hex64(regs->ss);

    debug_print_line("");
}

static void panic_registers(registers_t *regs)
{
    debug_print_line("General Registers");
    debug_print_line("");

    debug_print("RAX         : ");
    debug_print_hex64(regs->rax);

    debug_print("RBX         : ");
    debug_print_hex64(regs->rbx);

    debug_print("RCX         : ");
    debug_print_hex64(regs->rcx);

    debug_print("RDX         : ");
    debug_print_hex64(regs->rdx);

    debug_print("RSI         : ");
    debug_print_hex64(regs->rsi);

    debug_print("RDI         : ");
    debug_print_hex64(regs->rdi);

    debug_print("RBP         : ");
    debug_print_hex64(regs->rbp);

    debug_print("R8          : ");
    debug_print_hex64(regs->r8);

    debug_print("R9          : ");
    debug_print_hex64(regs->r9);

    debug_print("R10         : ");
    debug_print_hex64(regs->r10);

    debug_print("R11         : ");
    debug_print_hex64(regs->r11);

    debug_print("R12         : ");
    debug_print_hex64(regs->r12);

    debug_print("R13         : ");
    debug_print_hex64(regs->r13);

    debug_print("R14         : ");
    debug_print_hex64(regs->r14);

    debug_print("R15         : ");
    debug_print_hex64(regs->r15);

    debug_print_line("");
}

static void panic_footer(void)
{
    debug_print_line("==================================================");
    debug_print_line("");
    debug_print_line("System Halted.");
    debug_print_line("Please reboot XyrisOS.");
}

void kernel_panic(registers_t *regs, const char *reason)
{
    framebuffer_clear(PANIC_BACKGROUND);

    debug_print_init();

    panic_header();
    panic_reason(regs, reason);
    panic_cpu_state(regs);
    panic_registers(regs);
    panic_footer();

    cpu_halt_forever();
}