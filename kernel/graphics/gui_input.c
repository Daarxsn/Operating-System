#include "gui_input.h"

#include <stddef.h>

#include "accessibility.h"
#include "application.h"

bool gui_input_process_keyboard_event(const XKKeyboardEvent *event)
{
    if (event == NULL || !event->pressed)
        return false;

    const XyrisAccessibilitySettings *settings =
        accessibility_get_settings();

    if (settings == NULL || !settings->keyboard_navigation)
        return false;

    /* Tab cycles focus through currently running applications. */
    if (event->ascii == '\t')
        return application_focus_next();

    return false;
}

uint32_t gui_input_poll(void)
{
    uint32_t processed = 0;
    XKKeyboardEvent event;

    while (xk_keyboard_read_event(&event)) {
        (void)gui_input_process_keyboard_event(&event);
        ++processed;
    }

    return processed;
}
