#include "exceptions.h"
#include "page_fault.h"

#include "../debug/panic.h"

static const char *exception_names[32] =
{
    "Divide Error",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "BOUND Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating Point",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating Point",
    "Virtualization",
    "Control Protection",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection",
    "VMM Communication",
    "Security Exception",
    "Reserved"
};

void exception_dispatch(registers_t *regs)
{
    const char *name = "Unknown Exception";

    if (regs->vector < 32)
    {
        name = exception_names[regs->vector];
    }

    /* Dedicated Page Fault Handler */
    if (regs->vector == 14)
    {
        page_fault_handler(regs);
        return;
    }

    kernel_panic(regs, name);
}