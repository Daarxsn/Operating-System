#ifndef XK_FOUNDATION_EVENT_H
#define XK_FOUNDATION_EVENT_H

#include <stdint.h>
#include <stdbool.h>

#define XK_EVENT_MAX_EVENTS      128
#define XK_EVENT_MAX_SUBSCRIBERS 16

typedef enum
{
    XK_EVENT_NONE = 0,

    XK_EVENT_BOOT,

    XK_EVENT_INTERRUPT,

    XK_EVENT_PROCESS,

    XK_EVENT_THREAD,

    XK_EVENT_DRIVER,

    XK_EVENT_DEVICE,

    XK_EVENT_TIMER,

    XK_EVENT_MEMORY,

    XK_EVENT_IPC,

    XK_EVENT_CUSTOM

} XKEventType;

typedef struct
{
    XKEventType type;

    uint64_t source;

    uint64_t data;

} XKEvent;

typedef void (*XKEventHandler)(const XKEvent *);

void xk_event_init(void);

bool xk_event_subscribe(
    XKEventType type,
    XKEventHandler handler);

bool xk_event_publish(
    const XKEvent *event);

#endif
