#ifndef XK_KEYBOARD_DRIVER_H
#define XK_KEYBOARD_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "cpu/irq.h"
#include "drivers/driver.h"

/* ------------------------------------------------------------
 * PS/2 Ports
 * ------------------------------------------------------------ */

#define XK_PS2_DATA_PORT     0x60
#define XK_PS2_STATUS_PORT   0x64
#define XK_PS2_COMMAND_PORT  0x64


/* ------------------------------------------------------------
 * IRQ Handler
 * ------------------------------------------------------------ */

/* ------------------------------------------------------------
 * Keyboard API
 * ------------------------------------------------------------ */

bool xk_keyboard_initialize(void);

void xk_keyboard_shutdown(void);

uint8_t xk_keyboard_read_scancode(void);

bool xk_keyboard_has_data(void);

void xk_keyboard_irq_handler(
    registers_t *regs);

extern XKDriver xk_keyboard_driver;

#endif