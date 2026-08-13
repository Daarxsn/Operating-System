#include "irq.h"
#include <stddef.h>
#include "pic.h"
#include "pit.h"

#define XK_IRQ_COUNT 16

/* ------------------------------------------------------------
 * IRQ Handler Table
 * ------------------------------------------------------------ */

static irq_handler_t irq_handlers[XK_IRQ_COUNT];

/* ------------------------------------------------------------
 * Register IRQ Handler
 * ------------------------------------------------------------ */

bool xk_irq_register(
    uint8_t irq,
    irq_handler_t handler)
{
    if (irq >= XK_IRQ_COUNT)
    {
        return false;
    }

    irq_handlers[irq] = handler;

    return true;
}

/* ------------------------------------------------------------
 * Unregister IRQ Handler
 * ------------------------------------------------------------ */

void xk_irq_unregister(
    uint8_t irq)
{
    if (irq >= XK_IRQ_COUNT)
    {
        return;
    }

    irq_handlers[irq] = NULL;
}

/* ------------------------------------------------------------
 * IRQ Dispatcher
 * ------------------------------------------------------------ */

void irq_dispatch(registers_t *regs)
{
    uint8_t irq = (uint8_t)(regs->vector - 32);

    /* Built-in PIT handler */
    if (irq == 0)
    {
        pit_handler();
    }

    /* Driver IRQ handlers */
    if (irq < XK_IRQ_COUNT &&
        irq_handlers[irq] != NULL)
    {
        irq_handlers[irq](regs);
    }

    pic_send_eoi(irq);
}