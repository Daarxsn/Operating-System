#include "foundation/event.h"

#include <stddef.h>

/* ============================================================
 * XyrisOS Kernel Event Manager
 *
 * Responsible for event subscription and dispatch between
 * kernel subsystems.
 * ============================================================
 */

typedef struct
{
    XKEventType type;
    XKEventHandler handler;
    bool active;

} XKEventSubscription;

/* ------------------------------------------------------------
 * Static Subscription Table
 * ------------------------------------------------------------ */

static XKEventSubscription subscriptions[XK_EVENT_MAX_SUBSCRIBERS];

/* ------------------------------------------------------------
 * Initialize Event Manager
 * ------------------------------------------------------------ */

void xk_event_init(void)
{
    for (uint32_t i = 0; i < XK_EVENT_MAX_SUBSCRIBERS; i++)
    {
        subscriptions[i].active = false;
        subscriptions[i].type = XK_EVENT_NONE;
        subscriptions[i].handler = NULL;
    }
}

/* ------------------------------------------------------------
 * Subscribe to an Event
 * ------------------------------------------------------------ */

bool xk_event_subscribe(
    XKEventType type,
    XKEventHandler handler)
{
    if (handler == NULL)
    {
        return false;
    }

    for (uint32_t i = 0; i < XK_EVENT_MAX_SUBSCRIBERS; i++)
    {
        if (!subscriptions[i].active)
        {
            subscriptions[i].active = true;
            subscriptions[i].type = type;
            subscriptions[i].handler = handler;

            return true;
        }
    }

    return false;
}

/* ------------------------------------------------------------
 * Publish an Event
 * ------------------------------------------------------------ */

bool xk_event_publish(
    const XKEvent *event)
{
    if (event == NULL)
    {
        return false;
    }

    for (uint32_t i = 0; i < XK_EVENT_MAX_SUBSCRIBERS; i++)
    {
        if (subscriptions[i].active &&
            subscriptions[i].type == event->type)
        {
            subscriptions[i].handler(event);
        }
    }

    return true;
}