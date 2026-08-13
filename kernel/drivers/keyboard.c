#include "drivers/keyboard.h"
#include "debug/print.h"
#include "cpu/io.h"
#include "cpu/irq.h"

static bool keyboard_initialized = false;

/* ------------------------------------------------------------
 * PS/2 Set 1 Scancode Table
 * ------------------------------------------------------------ */

static const char scancode_table[128] =
{
    0,
    27,
    '1','2','3','4','5','6','7','8','9','0',
    '-','=',
    '\b',
    '\t',

    'q','w','e','r','t','y','u','i','o','p',
    '[',']',
    '\n',

    0,

    'a','s','d','f','g','h','j','k','l',
    ';','\'','`',

    0,

    '\\',

    'z','x','c','v','b','n','m',
    ',', '.', '/',

    0,

    '*',

    0,

    ' ',

    0
};

/* ------------------------------------------------------------
 * Driver Initialization
 * ------------------------------------------------------------ */

bool xk_keyboard_initialize(void)
{
    keyboard_initialized = true;

    xk_irq_register(
        1,
        xk_keyboard_irq_handler
    );

    return true;
}

/* ------------------------------------------------------------
 * Driver Shutdown
 * ------------------------------------------------------------ */

void xk_keyboard_shutdown(void)
{
    xk_irq_unregister(1);

    keyboard_initialized = false;
}

/* ------------------------------------------------------------
 * Status Check
 * ------------------------------------------------------------ */

bool xk_keyboard_has_data(void)
{
    return (inb(XK_PS2_STATUS_PORT) & 1);
}

/* ------------------------------------------------------------
 * Read Scan Code
 * ------------------------------------------------------------ */

uint8_t xk_keyboard_read_scancode(void)
{
    return inb(XK_PS2_DATA_PORT);
}

/* ------------------------------------------------------------
 * IRQ1 Handler
 * ------------------------------------------------------------ */

void xk_keyboard_irq_handler(registers_t *regs)
{
    (void)regs;

    if (!keyboard_initialized)
    {
        return;
    }

    if (xk_keyboard_has_data())
    {
        uint8_t scancode = xk_keyboard_read_scancode();

        /* Ignore key release events */
        if (scancode & 0x80)
        {
            return;
        }

        /* Prevent out-of-bounds access */
        if (scancode >= 128)
        {
            return;
        }

        char key = scancode_table[scancode];

        if (key != 0)
        {
            char buffer[2];

            buffer[0] = key;
            buffer[1] = '\0';

            debug_print(buffer);
        }
    }
}

/* ------------------------------------------------------------
 * Driver Object
 * ------------------------------------------------------------ */

XKDriver xk_keyboard_driver =
{
    .name = "PS/2 Keyboard",
    .type = XK_DRIVER_KEYBOARD,
    .state = XK_DRIVER_UNINITIALIZED,
    .initialize = xk_keyboard_initialize,
    .shutdown = xk_keyboard_shutdown
};