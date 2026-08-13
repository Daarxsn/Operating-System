#ifndef XK_SERIAL_DRIVER_H
#define XK_SERIAL_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

#include "drivers/driver.h"

#define XK_COM1 0x3F8

bool xk_serial_initialize(void);

void xk_serial_shutdown(void);

void xk_serial_write(char c);

void xk_serial_write_string(const char *str);

extern XKDriver xk_serial_driver;

#endif