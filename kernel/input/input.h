#ifndef XYRIS_INPUT_H
#define XYRIS_INPUT_H

#include <stdbool.h>
#include <stdint.h>

#define XK_INPUT_MAX_DEVICES 16u
#define XK_INPUT_EVENT_BUFFER_SIZE 128u

typedef enum
{
    XK_INPUT_DEVICE_UNKNOWN = 0,
    XK_INPUT_DEVICE_KEYBOARD,
    XK_INPUT_DEVICE_MOUSE,
    XK_INPUT_DEVICE_TOUCHPAD,
    XK_INPUT_DEVICE_TOUCHSCREEN,
    XK_INPUT_DEVICE_USB_HID
} XKInputDeviceType;

typedef enum
{
    XK_INPUT_EVENT_NONE = 0,
    XK_INPUT_EVENT_KEY,
    XK_INPUT_EVENT_MOUSE_MOVE,
    XK_INPUT_EVENT_MOUSE_BUTTON,
    XK_INPUT_EVENT_MOUSE_WHEEL
} XKInputEventType;

typedef struct
{
    uint32_t id;
    XKInputDeviceType type;
    bool connected;
    char name[32];
} XKInputDevice;

typedef struct
{
    XKInputEventType type;
    uint32_t device_id;

    union
    {
        struct
        {
            uint16_t keycode;
            char ascii;
            uint8_t modifiers;
            bool pressed;
        } key;

        struct
        {
            int16_t x;
            int16_t y;
        } mouse_move;

        struct
        {
            uint8_t buttons;
        } mouse_button;

        struct
        {
            int8_t delta;
        } mouse_wheel;
    } data;
} XKInputEvent;

/*
 * Input subsystem lifecycle.
 */
bool xk_input_init(void);
void xk_input_shutdown(void);

/*
 * Input device enumeration.
 */
uint32_t xk_input_device_count(void);

const XKInputDevice *xk_input_device_get(
    uint32_t index
);

/*
 * Common input event interface.
 */
bool xk_input_event_available(void);

bool xk_input_read_event(
    XKInputEvent *event
);

/*
 * Poll existing keyboard and mouse drivers and
 * translate their events into the common input format.
 */
uint32_t xk_input_poll(void);

/*
 * Diagnostics.
 */
void xk_input_dump(void);

#endif