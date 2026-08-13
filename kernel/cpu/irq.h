#ifndef XYRIS_IRQ_H
#define XYRIS_IRQ_H

#include <stdint.h>
#include <stdbool.h>

#include "isr.h"

/* ------------------------------------------------------------
 * IRQ Handler Type
 * ------------------------------------------------------------ */

typedef void (*irq_handler_t)(registers_t *regs);

/* ------------------------------------------------------------
 * IRQ Framework API
 * ------------------------------------------------------------ */

void irq_dispatch(registers_t *regs);

bool xk_irq_register(
    uint8_t irq,
    irq_handler_t handler);

void xk_irq_unregister(
    uint8_t irq);

#endif