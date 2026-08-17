#include "drivers/keyboard.h"
#include <stddef.h>
#include "cpu/io.h"
#include "cpu/irq.h"

static bool keyboard_initialized = false;
static uint8_t keyboard_modifiers = 0;
static bool keyboard_extended = false;
static uint8_t keyboard_e1_index = 0;
static const uint8_t keyboard_e1_pause_sequence[6] = {
    0xE1U, 0x1DU, 0x45U, 0xE1U, 0x9DU, 0xC5U
};
static XKKeyboardEvent event_queue[XK_KEYBOARD_BUFFER_SIZE];
static volatile uint32_t queue_head = 0;
static volatile uint32_t queue_tail = 0;

static const char scancode_table[128] =
{
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b','\t',
    'q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
    'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\',
    'z','x','c','v','b','n','m',',','.','/',0,'*',0,' ','0',
};

static bool queue_push(const XKKeyboardEvent *event)
{
    uint32_t next = (queue_head + 1U) % XK_KEYBOARD_BUFFER_SIZE;
    if (next == queue_tail)
        return false;
    event_queue[queue_head] = *event;
    queue_head = next;
    return true;
}

bool xk_keyboard_event_available(void)
{
    return queue_head != queue_tail;
}

bool xk_keyboard_read_event(XKKeyboardEvent *event)
{
    if (event == NULL || queue_head == queue_tail)
        return false;
    *event = event_queue[queue_tail];
    queue_tail = (queue_tail + 1U) % XK_KEYBOARD_BUFFER_SIZE;
    return true;
}

uint8_t xk_keyboard_modifiers(void)
{
    return keyboard_modifiers;
}

bool xk_keyboard_initialize(void)
{
    keyboard_initialized = true;
    keyboard_modifiers = 0;
    keyboard_extended = false;
    keyboard_e1_index = 0;
    queue_head = 0;
    queue_tail = 0;

    return xk_irq_register(1, xk_keyboard_irq_handler);
}

void xk_keyboard_shutdown(void)
{
    xk_irq_unregister(1);
    keyboard_initialized = false;
    keyboard_modifiers = 0;
    keyboard_extended = false;
    queue_head = 0;
    queue_tail = 0;
}

bool xk_keyboard_has_data(void)
{
    return (inb(XK_PS2_STATUS_PORT) & 1U) != 0;
}

uint8_t xk_keyboard_read_scancode(void)
{
    return inb(XK_PS2_DATA_PORT);
}

bool xk_keyboard_process_scancode(uint8_t scancode)
{
    bool released = (scancode & 0x80U) != 0;
    uint8_t code = scancode & 0x7FU;

    /* Set-1 Pause/Break is a six-byte E1 sequence. Consume the whole
     * sequence so its intermediate bytes never become fake key events. */
    if (keyboard_e1_index != 0 || scancode == 0xE1U)
    {
        if (keyboard_e1_index == 0)
        {
            keyboard_e1_index = 1;
            return true;
        }

        if (scancode == keyboard_e1_pause_sequence[keyboard_e1_index])
        {
            keyboard_e1_index++;
            if (keyboard_e1_index == 6U)
            {
                XKKeyboardEvent pause_event = {0};
                pause_event.keycode = 0x45U | XK_KEY_EXTENDED;
                pause_event.ascii = 0;
                pause_event.modifiers = keyboard_modifiers;
                pause_event.pressed = true;
                keyboard_e1_index = 0;
                return queue_push(&pause_event);
            }
            return true;
        }

        /* Invalid E1 sequence: reset state and process this byte normally. */
        keyboard_e1_index = 0;
    }

    if (scancode == 0xE0U)
    {
        keyboard_extended = true;
        return true;
    }

    /*
     * Set-1 modifier keys update state but do not generate standalone
     * character events. The modifier state is attached to the next
     * actionable key event, keeping the input queue free of
     * zero-ASCII modifier-only records.
     */
    bool modifier_key = false;

    if (code == 0x2AU || code == 0x36U) /* Shift */
    {
        modifier_key = true;
        if (released)
            keyboard_modifiers &= (uint8_t)~XK_KEY_MOD_SHIFT;
        else
            keyboard_modifiers |= XK_KEY_MOD_SHIFT;
    }
    else if (code == 0x1DU) /* Ctrl */
    {
        modifier_key = true;
        if (released)
            keyboard_modifiers &= (uint8_t)~XK_KEY_MOD_CTRL;
        else
            keyboard_modifiers |= XK_KEY_MOD_CTRL;
    }
    else if (code == 0x38U) /* Alt */
    {
        modifier_key = true;
        if (released)
            keyboard_modifiers &= (uint8_t)~XK_KEY_MOD_ALT;
        else
            keyboard_modifiers |= XK_KEY_MOD_ALT;
    }
    else if (code == 0x3AU && !released) /* Caps Lock */
    {
        modifier_key = true;
        keyboard_modifiers ^= XK_KEY_MOD_CAPS;
    }

    if (modifier_key)
    {
        keyboard_extended = false;
        return true;
    }

    XKKeyboardEvent event = {0};
    event.keycode = (uint16_t)code | (keyboard_extended ? XK_KEY_EXTENDED : 0U);
    event.pressed = !released;
    event.modifiers = keyboard_modifiers;

    if (!keyboard_extended && code < 128U)
    {
        char ascii = scancode_table[code];
        if (ascii != 0 && !released)
        {
            bool upper = (keyboard_modifiers & XK_KEY_MOD_SHIFT) != 0;
            bool caps = (keyboard_modifiers & XK_KEY_MOD_CAPS) != 0;
            if (ascii >= 'a' && ascii <= 'z' && (upper ^ caps))
                ascii = (char)(ascii - 'a' + 'A');
            event.ascii = ascii;
        }
    }

    keyboard_extended = false;
    return queue_push(&event);
}

void xk_keyboard_irq_handler(registers_t *regs)
{
    (void)regs;
    if (!keyboard_initialized || !xk_keyboard_has_data())
        return;
    (void)xk_keyboard_process_scancode(xk_keyboard_read_scancode());
}

XKDriver xk_keyboard_driver =
{
    .name = "PS/2 Keyboard",
    .type = XK_DRIVER_KEYBOARD,
    .state = XK_DRIVER_UNINITIALIZED,
    .initialize = xk_keyboard_initialize,
    .shutdown = xk_keyboard_shutdown
};
