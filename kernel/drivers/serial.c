#include "drivers/serial.h"

#include "cpu/io.h"
#include <stddef.h>

static bool serial_initialized = false;

/* ------------------------------------------------------------
 * Initialize COM1
 * ------------------------------------------------------------ */

bool xk_serial_initialize(void)
{
    outb(XK_COM1 + 1, 0x00);
    outb(XK_COM1 + 3, 0x80);
    outb(XK_COM1 + 0, 0x03);
    outb(XK_COM1 + 1, 0x00);
    outb(XK_COM1 + 3, 0x03);
    outb(XK_COM1 + 2, 0xC7);
    outb(XK_COM1 + 4, 0x0B);

    serial_initialized = true;

    return true;
}

/* ------------------------------------------------------------
 * Shutdown
 * ------------------------------------------------------------ */

void xk_serial_shutdown(void)
{
    serial_initialized = false;
}

/* ------------------------------------------------------------
 * Ready?
 * ------------------------------------------------------------ */

static bool serial_ready(void)
{
    return inb(XK_COM1 + 5) & 0x20;
}

/* ------------------------------------------------------------
 * Write Character
 * ------------------------------------------------------------ */

void xk_serial_write(char c)
{
    if (!serial_initialized)
    {
        return;
    }

    while (!serial_ready());

    outb(XK_COM1, c);
}

/* ------------------------------------------------------------
 * Write String
 * ------------------------------------------------------------ */

void xk_serial_write_string(const char *str)
{
    if (!serial_initialized || str == NULL)
    {
        return;
    }

    while (*str)
    {
        xk_serial_write(*str++);
    }
}

/* ------------------------------------------------------------
 * Driver Object
 * ------------------------------------------------------------ */

XKDriver xk_serial_driver =
{
    .name = "COM1 Serial",
    .type = XK_DRIVER_SERIAL,
    .state = XK_DRIVER_UNINITIALIZED,
    .initialize = xk_serial_initialize,
    .shutdown = xk_serial_shutdown
};