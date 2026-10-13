#ifndef XYRIS_GUI_INPUT_H
#define XYRIS_GUI_INPUT_H

#include <stdbool.h>
#include <stdint.h>
#include "drivers/keyboard.h"

/* Process one queued keyboard event. Returns true when handled. */
bool gui_input_process_keyboard_event(const XKKeyboardEvent *event);

/* Drain queued PS/2 keyboard events and process GUI navigation keys. */
uint32_t gui_input_poll(void);

#endif
