#include "isr.h"
#include "exceptions.h"
#include "irq.h"

#include "../syscall/syscall_isr.h"

void isr_handler(registers_t *regs)
{
    if (regs->vector < 32)
    {
        exception_dispatch(regs);
        return;
    }

    if (regs->vector == 128)
    {
        syscall_interrupt_handler(regs);
        return;
    }

    irq_dispatch(regs);
}

void isr_init(void)
{
}