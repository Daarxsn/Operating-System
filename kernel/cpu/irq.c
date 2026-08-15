#include "irq.h"
#include <stddef.h>
#include "pic.h"
#include "pit.h"
#include "../process/scheduler.h"

#define XK_IRQ_COUNT 16

static irq_handler_t irq_handlers[XK_IRQ_COUNT];

static volatile uint64_t irq0_debug_count = 0;

bool xk_irq_register(uint8_t irq, irq_handler_t handler)
{
    if (irq >= XK_IRQ_COUNT) return false;
    irq_handlers[irq] = handler;
    return true;
}

void xk_irq_unregister(uint8_t irq)
{
    if (irq >= XK_IRQ_COUNT) return;
    irq_handlers[irq] = NULL;
}

void irq_dispatch(registers_t *regs)
{
    if (regs == NULL || regs->vector < 32 || regs->vector >= 48)
        return;

    uint8_t irq = (uint8_t)(regs->vector - 32);

    if (irq == 0)
    {
        irq0_debug_count++;
        pit_handler();
    }

    if (irq < XK_IRQ_COUNT && irq_handlers[irq] != NULL)
        irq_handlers[irq](regs);

    /* The PIC must receive EOI before a timer-triggered context switch.
     * Otherwise the old IRQ remains in service and subsequent PIT ticks
     * can be blocked while the new thread is running. */
    pic_send_eoi(irq);

    if (irq == 0 && scheduler_preemption_pending())
    {
        scheduler_clear_preemption();
        scheduler_schedule();
    }
}

uint64_t irq0_debug_get_count(void)
{
    return irq0_debug_count;
}