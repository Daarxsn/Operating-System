#include "drivers/mouse.h"
#include <stddef.h>
#include "cpu/io.h"

#define XK_PS2_DATA_PORT 0x60
#define XK_PS2_STATUS_PORT 0x64
#define XK_PS2_COMMAND_PORT 0x64

static bool mouse_initialized = false;
static uint8_t packet[4];
static uint8_t packet_index = 0;
static bool has_wheel = false;
static int16_t mouse_x_position = 0;
static int16_t mouse_y_position = 0;
static uint8_t mouse_buttons_state = 0;
static XKMouseEvent event_queue[XK_MOUSE_BUFFER_SIZE];
static volatile uint32_t queue_head = 0;
static volatile uint32_t queue_tail = 0;

static bool wait_input_clear(void)
{
    for (uint32_t i = 0; i < 100000U; i++)
    {
        if ((inb(XK_PS2_STATUS_PORT) & 0x02U) == 0)
            return true;
        __asm__ volatile ("pause");
    }
    return false;
}

static bool wait_output_full(void)
{
    for (uint32_t i = 0; i < 100000U; i++)
    {
        if ((inb(XK_PS2_STATUS_PORT) & 0x01U) != 0)
            return true;
        __asm__ volatile ("pause");
    }
    return false;
}

static bool controller_write(uint8_t command)
{
    if (!wait_input_clear()) return false;
    outb(XK_PS2_COMMAND_PORT, command);
    return true;
}

static bool mouse_write(uint8_t value)
{
    if (!controller_write(0xD4)) return false;
    if (!wait_input_clear()) return false;
    outb(XK_PS2_DATA_PORT, value);
    return true;
}

static bool mouse_read_ack(void)
{
    if (!wait_output_full()) return false;
    return inb(XK_PS2_DATA_PORT) == 0xFA;
}

static bool queue_push(const XKMouseEvent *event)
{
    uint32_t next = (queue_head + 1U) % XK_MOUSE_BUFFER_SIZE;
    if (next == queue_tail) return false;
    event_queue[queue_head] = *event;
    queue_head = next;
    return true;
}

bool xk_mouse_event_available(void)
{
    return queue_head != queue_tail;
}

bool xk_mouse_read_event(XKMouseEvent *event)
{
    if (event == NULL || queue_head == queue_tail) return false;
    *event = event_queue[queue_tail];
    queue_tail = (queue_tail + 1U) % XK_MOUSE_BUFFER_SIZE;
    return true;
}

int16_t xk_mouse_x(void) { return mouse_x_position; }
int16_t xk_mouse_y(void) { return mouse_y_position; }
uint8_t xk_mouse_buttons(void) { return mouse_buttons_state; }

bool xk_mouse_initialize(void)
{
    /* Enable auxiliary device. */
    if (!controller_write(0xA8)) return false;

    /* Read controller configuration byte. */
    if (!controller_write(0x20) || !wait_output_full()) return false;
    uint8_t config = inb(XK_PS2_DATA_PORT);
    config |= 0x02U;  /* IRQ12 enable */
    config &= (uint8_t)~0x20U; /* enable mouse clock */

    if (!controller_write(0x60) || !wait_input_clear()) return false;
    outb(XK_PS2_DATA_PORT, config);

    /* Set defaults, then enable streaming. */
    if (!mouse_write(0xF6U) || !mouse_read_ack()) return false;
    if (!mouse_write(0xF4U) || !mouse_read_ack()) return false;

    mouse_initialized = true;
    packet_index = 0;
    queue_head = 0;
    queue_tail = 0;
    mouse_x_position = 0;
    mouse_y_position = 0;
    mouse_buttons_state = 0;

    if (!xk_irq_register(12, xk_mouse_irq_handler))
    {
        mouse_initialized = false;
        return false;
    }

    return true;
}

void xk_mouse_shutdown(void)
{
    xk_irq_unregister(12);
    mouse_initialized = false;
    packet_index = 0;
    queue_head = 0;
    queue_tail = 0;
}

bool xk_mouse_process_byte(uint8_t byte)
{
    /* First byte must have bit 3 set for packet synchronization. */
    if (packet_index == 0 && (byte & 0x08U) == 0)
        return false;

    packet[packet_index++] = byte;
    uint8_t packet_size = has_wheel ? 4U : 3U;
    if (packet_index < packet_size)
        return true;

    XKMouseEvent event = {0};
    event.x = (int16_t)(int8_t)packet[1];
    event.y = (int16_t)(-(int8_t)packet[2]);
    event.buttons = packet[0] & 0x07U;
    event.overflow_x = (packet[0] & 0x40U) != 0;
    event.overflow_y = (packet[0] & 0x80U) != 0;
    event.wheel = has_wheel ? (int8_t)packet[3] : 0;

    if (!event.overflow_x) mouse_x_position += event.x;
    if (!event.overflow_y) mouse_y_position += event.y;
    mouse_buttons_state = event.buttons;
    packet_index = 0;

    return queue_push(&event);
}

void xk_mouse_irq_handler(registers_t *regs)
{
    (void)regs;
    if (!mouse_initialized)
        return;

    while ((inb(XK_PS2_STATUS_PORT) & 0x01U) != 0 &&
           (inb(XK_PS2_STATUS_PORT) & 0x20U) != 0)
    {
        (void)xk_mouse_process_byte(inb(XK_PS2_DATA_PORT));
    }
}

XKDriver xk_mouse_driver =
{
    .name = "PS/2 Mouse",
    .type = XK_DRIVER_MOUSE,
    .state = XK_DRIVER_UNINITIALIZED,
    .initialize = xk_mouse_initialize,
    .shutdown = xk_mouse_shutdown
};
