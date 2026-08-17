#ifndef XK_MOUSE_DRIVER_H
#define XK_MOUSE_DRIVER_H

#include <stdbool.h>
#include <stdint.h>
#include "drivers/driver.h"
#include "cpu/irq.h"

#define XK_MOUSE_BUFFER_SIZE 64
#define XK_MOUSE_BUTTON_LEFT   0x01
#define XK_MOUSE_BUTTON_RIGHT  0x02
#define XK_MOUSE_BUTTON_MIDDLE 0x04

typedef struct
{
    int16_t x;
    int16_t y;
    int8_t wheel;
    uint8_t buttons;
    bool overflow_x;
    bool overflow_y;
} XKMouseEvent;

bool xk_mouse_initialize(void);
void xk_mouse_shutdown(void);
void xk_mouse_irq_handler(registers_t *regs);

/* Hardware-independent packet processing used by IRQ path and tests. */
bool xk_mouse_process_byte(uint8_t byte);
bool xk_mouse_read_event(XKMouseEvent *event);
bool xk_mouse_event_available(void);
int16_t xk_mouse_x(void);
int16_t xk_mouse_y(void);
uint8_t xk_mouse_buttons(void);

extern XKDriver xk_mouse_driver;

#endif
