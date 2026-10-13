#ifndef XYRIS_APPLICATION_H
#define XYRIS_APPLICATION_H

#include <stdbool.h>
#include <stdint.h>
#include "window_manager.h"
#include "foundation/capability.h"

#define XYRIS_APPLICATION_MAX 32
#define XYRIS_APPLICATION_NAME_MAX 32

typedef enum {
    XYRIS_APP_EMPTY = 0,
    XYRIS_APP_CREATED,
    XYRIS_APP_RUNNING
} XyrisApplicationState;

typedef struct {
    uint32_t id;
    char name[XYRIS_APPLICATION_NAME_MAX];
    XyrisApplicationState state;
    XyrisWindow *window;
} XyrisApplication;

/* Call after window_manager_init() and capability initialization. */
void application_manager_init(void);

XyrisApplication *application_create(
    const char *name,
    uint32_t width,
    uint32_t height,
    int32_t x,
    int32_t y);

bool application_launch(XyrisApplication *app);
bool application_focus(XyrisApplication *app);
bool application_close(XyrisApplication *app);

XyrisApplication *application_find(uint32_t id);
XyrisWindow *application_window(const XyrisApplication *app);
const char *application_name(const XyrisApplication *app);
XyrisApplicationState application_state(const XyrisApplication *app);

bool application_grant_capability(
    XyrisApplication *app, XKCapability capability);
bool application_revoke_capability(
    XyrisApplication *app, XKCapability capability);
bool application_check_capability(
    const XyrisApplication *app, XKCapability capability);

/* Focus the next running application, wrapping around the registry. */
bool application_focus_next(void);

#endif
