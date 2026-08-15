#include "irq.h"
#include <stddef.h>
#include <stdint.h>
#include "lapic.h"

#include "pic.h"
#include "pit.h"
#include "../process/scheduler.h"

#define XK_IRQ_COUNT 16

static irq_handler_t irq_handlers[XK_IRQ_COUNT];

/*
 * IRQ0 diagnostic counter.
 *
 * kernel.c uses this to verify that:
 *
 *     PIT -> IRQ0 -> irq_dispatch()
 *
 * is actually occurring.
 */
static volatile uint64_t irq0_debug_count = 0;


bool xk_irq_register(uint8_t irq, irq_handler_t handler)
{
    if (irq >= XK_IRQ_COUNT)
        return false;

    irq_handlers[irq] = handler;
    return true;
}


void xk_irq_unregister(uint8_t irq)
{
    if (irq >= XK_IRQ_COUNT)
        return;

    irq_handlers[irq] = NULL;
}


/*
 * Diagnostic accessor used by kernel.c.
 */
uint64_t irq0_debug_get_count(void)
{
    return irq0_debug_count;
}


void irq_dispatch(registers_t *regs)
{
    if (regs == NULL ||
        regs->vector < 32 ||
        regs->vector >= 48)
    {
        return;
    }

    uint8_t irq =
        (uint8_t)(regs->vector - 32);


    /*
     * IRQ0 = PIT timer.
     */
    if (irq == 0)
    {
        irq0_debug_count++;

        pit_handler();
    }


    /*
     * Dispatch registered IRQ handler.
     */
    if (irq < XK_IRQ_COUNT &&
        irq_handlers[irq] != NULL)
    {
        irq_handlers[irq](regs);
    }


    /*
     * Acknowledge the interrupt before attempting
     * a scheduler transition.
     */
    lapic_eoi();


    /*
     * Timer-driven preemption.
     *
     * scheduler_tick() sets the pending flag once
     * the current thread's time slice expires.
     */
    if (irq == 0 &&
        scheduler_preemption_pending())
    {
        scheduler_clear_preemption();

        scheduler_schedule();
    }
}