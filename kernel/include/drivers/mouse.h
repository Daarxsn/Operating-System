#ifndef XK_MOUSE_DRIVER_H
#define XK_MOUSE_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

#include "drivers/driver.h"
#include "cpu/irq.h"

bool xk_mouse_initialize(void);

void xk_mouse_shutdown(void);

void xk_mouse_irq_handler(registers_t *regs);

extern XKDriver xk_mouse_driver;

#endif