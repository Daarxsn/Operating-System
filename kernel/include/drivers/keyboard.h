#ifndef XK_KEYBOARD_DRIVER_H
#define XK_KEYBOARD_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "cpu/irq.h"
#include "drivers/driver.h"

#define XK_PS2_DATA_PORT     0x60
#define XK_PS2_STATUS_PORT   0x64
#define XK_PS2_COMMAND_PORT  0x64
#define XK_KEYBOARD_BUFFER_SIZE 128

#define XK_KEY_MOD_SHIFT 0x01
#define XK_KEY_MOD_CTRL  0x02
#define XK_KEY_MOD_ALT   0x04
#define XK_KEY_MOD_CAPS  0x08

#define XK_KEY_EXTENDED  0x80
#define XK_KEY_UP        0x100
#define XK_KEY_DOWN      0x000

typedef struct
{
    uint16_t keycode;
    char ascii;
    uint8_t modifiers;
    bool pressed;
} XKKeyboardEvent;

bool xk_keyboard_initialize(void);
void xk_keyboard_shutdown(void);
uint8_t xk_keyboard_read_scancode(void);
bool xk_keyboard_has_data(void);
void xk_keyboard_irq_handler(registers_t *regs);

/* Hardware-independent input processing used by the IRQ path and tests. */
bool xk_keyboard_process_scancode(uint8_t scancode);
bool xk_keyboard_read_event(XKKeyboardEvent *event);
bool xk_keyboard_event_available(void);
uint8_t xk_keyboard_modifiers(void);

extern XKDriver xk_keyboard_driver;

#endif
