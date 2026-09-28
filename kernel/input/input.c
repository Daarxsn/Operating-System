#include "input.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "debug/print.h"
#include "drivers/keyboard.h"
#include "drivers/xhci.h"
#include "drivers/mouse.h"

static XKInputDevice g_input_devices[XK_INPUT_MAX_DEVICES];
static uint32_t g_input_device_count = 0;

static XKInputEvent g_input_events[XK_INPUT_EVENT_BUFFER_SIZE];
static uint32_t g_input_event_head = 0;
static uint32_t g_input_event_tail = 0;

static bool g_input_initialized = false;


/*
 * --------------------------------------------------------------------------
 * Internal helpers
 * --------------------------------------------------------------------------
 */

static bool input_queue_push(
    const XKInputEvent *event
)
{
    if (event == NULL)
    {
        return false;
    }

    uint32_t next =
        (g_input_event_head + 1U) %
        XK_INPUT_EVENT_BUFFER_SIZE;

    if (next == g_input_event_tail)
    {
        return false;
    }

    g_input_events[g_input_event_head] = *event;
    g_input_event_head = next;

    return true;
}


static bool input_register_device(
    XKInputDeviceType type,
    const char *name
)
{
    if (g_input_device_count >= XK_INPUT_MAX_DEVICES ||
        name == NULL)
    {
        return false;
    }

    XKInputDevice *device =
        &g_input_devices[g_input_device_count];

    *device = (XKInputDevice){0};

    device->id =
        g_input_device_count + 1U;

    device->type = type;
    device->connected = true;

    /*
     * Keep this independent of libc string helpers because
     * the kernel uses a freestanding environment.
     */
    uint32_t i = 0;

    while (name[i] != '\0' &&
           i < sizeof(device->name) - 1U)
    {
        device->name[i] = name[i];
        i++;
    }

    device->name[i] = '\0';

    g_input_device_count++;

    return true;
}


static uint32_t input_keyboard_device_id(void)
{
    for (uint32_t i = 0;
         i < g_input_device_count;
         i++)
    {
        if (g_input_devices[i].type ==
            XK_INPUT_DEVICE_KEYBOARD)
        {
            return g_input_devices[i].id;
        }
    }

    return 0;
}


static uint32_t input_mouse_device_id(void)
{
    for (uint32_t i = 0;
         i < g_input_device_count;
         i++)
    {
        if (g_input_devices[i].type ==
            XK_INPUT_DEVICE_MOUSE)
        {
            return g_input_devices[i].id;
        }
    }

    return 0;
}


/*
 * --------------------------------------------------------------------------
 * Initialization
 * --------------------------------------------------------------------------
 */

bool xk_input_init(void)
{
    g_input_device_count = 0;

    g_input_event_head = 0;
    g_input_event_tail = 0;

    for (uint32_t i = 0;
         i < XK_INPUT_MAX_DEVICES;
         i++)
    {
        g_input_devices[i] =
            (XKInputDevice){0};
    }

    for (uint32_t i = 0;
         i < XK_INPUT_EVENT_BUFFER_SIZE;
         i++)
    {
        g_input_events[i] =
            (XKInputEvent){0};
    }

    g_input_initialized = true;

    /*
     * Register the existing hardware drivers with
     * the common input layer.
     *
     * The drivers themselves remain responsible for
     * hardware access and IRQ handling.
     */
    (void)input_register_device(
        XK_INPUT_DEVICE_KEYBOARD,
        "Keyboard"
    );

    (void)input_register_device(
        XK_INPUT_DEVICE_MOUSE,
        "Mouse"
    );

    debug_print_line(
        "[INPUT] Input subsystem initialized"
    );

    return true;
}


void xk_input_shutdown(void)
{
    if (!g_input_initialized)
    {
        return;
    }

    for (uint32_t i = 0;
         i < g_input_device_count;
         i++)
    {
        g_input_devices[i].connected = false;
    }

    g_input_device_count = 0;

    g_input_event_head = 0;
    g_input_event_tail = 0;

    g_input_initialized = false;

    debug_print_line(
        "[INPUT] Input subsystem shut down"
    );
}


/*
 * --------------------------------------------------------------------------
 * Device enumeration
 * --------------------------------------------------------------------------
 */

uint32_t xk_input_device_count(void)
{
    return g_input_device_count;
}


const XKInputDevice *xk_input_device_get(
    uint32_t index
)
{
    if (index >= g_input_device_count)
    {
        return NULL;
    }

    return &g_input_devices[index];
}


/*
 * --------------------------------------------------------------------------
 * Event queue
 * --------------------------------------------------------------------------
 */

bool xk_input_event_available(void)
{
    return
        g_input_event_head !=
        g_input_event_tail;
}


bool xk_input_read_event(
    XKInputEvent *event
)
{
    if (event == NULL)
    {
        return false;
    }

    if (!xk_input_event_available())
    {
        return false;
    }

    *event =
        g_input_events[g_input_event_tail];

    g_input_event_tail =
        (g_input_event_tail + 1U) %
        XK_INPUT_EVENT_BUFFER_SIZE;

    return true;
}


/*
 * --------------------------------------------------------------------------
 * Driver event translation
 * --------------------------------------------------------------------------
 */

uint32_t xk_input_poll(void)
{
    if (!g_input_initialized)
    {
        return 0;
    }

    uint32_t translated = 0;

    /*
     * Service USB HID input before translating the common
     * keyboard/mouse event queues.
     */
    translated += xk_xhci_poll();

    uint32_t keyboard_id =
        input_keyboard_device_id();

    while (keyboard_id != 0 &&
           xk_keyboard_event_available())
    {
        XKKeyboardEvent keyboard_event;

        if (!xk_keyboard_read_event(
                &keyboard_event))
        {
            break;
        }

        XKInputEvent event = {0};

        event.type =
            XK_INPUT_EVENT_KEY;

        event.device_id =
            keyboard_id;

        event.data.key.keycode =
            keyboard_event.keycode;

        event.data.key.ascii =
            keyboard_event.ascii;

        event.data.key.modifiers =
            keyboard_event.modifiers;

        event.data.key.pressed =
            keyboard_event.pressed;

        if (input_queue_push(&event))
        {
            translated++;
        }
    }

    uint32_t mouse_id =
        input_mouse_device_id();

    while (mouse_id != 0 &&
           xk_mouse_event_available())
    {
        XKMouseEvent mouse_event;

        if (!xk_mouse_read_event(
                &mouse_event))
        {
            break;
        }

        XKInputEvent move_event = {0};

        move_event.type =
            XK_INPUT_EVENT_MOUSE_MOVE;

        move_event.device_id =
            mouse_id;

        move_event.data.mouse_move.x =
            mouse_event.x;

        move_event.data.mouse_move.y =
            mouse_event.y;

        if (input_queue_push(&move_event))
        {
            translated++;
        }

        if (mouse_event.buttons != 0)
        {
            XKInputEvent button_event = {0};

            button_event.type =
                XK_INPUT_EVENT_MOUSE_BUTTON;

            button_event.device_id =
                mouse_id;

            button_event.data.mouse_button.buttons =
                mouse_event.buttons;

            if (input_queue_push(&button_event))
            {
                translated++;
            }
        }

        if (mouse_event.wheel != 0)
        {
            XKInputEvent wheel_event = {0};

            wheel_event.type =
                XK_INPUT_EVENT_MOUSE_WHEEL;

            wheel_event.device_id =
                mouse_id;

            wheel_event.data.mouse_wheel.delta =
                mouse_event.wheel;

            if (input_queue_push(&wheel_event))
            {
                translated++;
            }
        }
    }

    return translated;
}


/*
 * --------------------------------------------------------------------------
 * Diagnostics
 * --------------------------------------------------------------------------
 */

void xk_input_dump(void)
{
    debug_print_line(
        "========== INPUT DEVICES =========="
    );

    debug_print_line(
        "[INPUT] Registered input devices:"
    );

    for (uint32_t i = 0;
         i < g_input_device_count;
         i++)
    {
        debug_print_line(
            g_input_devices[i].name
        );
    }

    debug_print_line(
        "==================================="
    );
}