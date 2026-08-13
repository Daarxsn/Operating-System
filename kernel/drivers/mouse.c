#include "drivers/mouse.h"

#include "cpu/io.h"

static bool mouse_initialized = false;

/* ------------------------------------------------------------
 * Driver Initialization
 * ------------------------------------------------------------ */

bool xk_mouse_initialize(void)
{
    mouse_initialized = true;

    xk_irq_register(
        12,
        xk_mouse_irq_handler
    );

    return true;
}

/* ------------------------------------------------------------
 * Driver Shutdown
 * ------------------------------------------------------------ */

void xk_mouse_shutdown(void)
{
    xk_irq_unregister(12);

    mouse_initialized = false;
}

/* ------------------------------------------------------------
 * IRQ12 Handler
 * ------------------------------------------------------------ */

void xk_mouse_irq_handler(registers_t *regs)
{
    (void)regs;

    if (!mouse_initialized)
    {
        return;
    }

    /*
     * Phase 2:
     * Read PS/2 mouse packet.
     */
}

/* ------------------------------------------------------------
 * Driver Object
 * ------------------------------------------------------------ */

XKDriver xk_mouse_driver =
{
    .name = "PS/2 Mouse",
    .type = XK_DRIVER_MOUSE,
    .state = XK_DRIVER_UNINITIALIZED,
    .initialize = xk_mouse_initialize,
    .shutdown = xk_mouse_shutdown
};