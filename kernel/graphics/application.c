#include "application.h"

#include <stddef.h>
#include "foundation/capability.h"

#define APP_CAPABILITY_NAMESPACE 0x4150500000000000ULL

static XyrisApplication applications[XYRIS_APPLICATION_MAX];
static uint32_t next_application_id = 1;

static bool app_is_registered(const XyrisApplication *app)
{
    if (app == NULL)
        return false;

    for (uint32_t i = 0; i < XYRIS_APPLICATION_MAX; ++i) {
        if (&applications[i] == app &&
            applications[i].state != XYRIS_APP_EMPTY)
            return true;
    }
    return false;
}

static uint64_t app_capability_id(const XyrisApplication *app)
{
    return APP_CAPABILITY_NAMESPACE | (uint64_t)app->id;
}

void application_manager_init(void)
{
    for (uint32_t i = 0; i < XYRIS_APPLICATION_MAX; ++i) {
        if (applications[i].state != XYRIS_APP_EMPTY) {
            xk_capability_clear(app_capability_id(&applications[i]));
            if (applications[i].window != NULL)
                window_destroy(applications[i].window);
        }

        applications[i].id = 0;
        applications[i].name[0] = '\0';
        applications[i].state = XYRIS_APP_EMPTY;
        applications[i].window = NULL;
    }
    next_application_id = 1;
}

XyrisApplication *application_create(
    const char *name, uint32_t width, uint32_t height,
    int32_t x, int32_t y)
{
    if (name == NULL || name[0] == '\0' ||
        width == 0 || height == 0)
        return NULL;

    uint32_t slot = XYRIS_APPLICATION_MAX;
    for (uint32_t i = 0; i < XYRIS_APPLICATION_MAX; ++i) {
        if (applications[i].state == XYRIS_APP_EMPTY) {
            slot = i;
            break;
        }
    }
    if (slot == XYRIS_APPLICATION_MAX)
        return NULL;

    XyrisWindow *window = window_create(width, height, x, y);
    if (window == NULL)
        return NULL;

    XyrisApplication *app = &applications[slot];
    uint32_t n = 0;
    while (n < XYRIS_APPLICATION_NAME_MAX - 1 && name[n] != '\0') {
        app->name[n] = name[n];
        ++n;
    }
    app->name[n] = '\0';

    app->id = next_application_id++;
    if (next_application_id == 0)
        next_application_id = 1;

    app->window = window;
    app->state = XYRIS_APP_CREATED;
    window_hide(window);
    return app;
}

bool application_launch(XyrisApplication *app)
{
    if (!app_is_registered(app) || app->window == NULL)
        return false;

    window_show(app->window);
    window_focus(app->window);
    app->state = XYRIS_APP_RUNNING;
    return true;
}

bool application_focus(XyrisApplication *app)
{
    if (!app_is_registered(app) || app->window == NULL ||
        app->state != XYRIS_APP_RUNNING)
        return false;

    window_focus(app->window);
    return true;
}

bool application_close(XyrisApplication *app)
{
    if (!app_is_registered(app))
        return false;

    xk_capability_clear(app_capability_id(app));
    if (app->window != NULL)
        window_destroy(app->window);

    app->window = NULL;
    app->name[0] = '\0';
    app->id = 0;
    app->state = XYRIS_APP_EMPTY;
    return true;
}

XyrisApplication *application_find(uint32_t id)
{
    if (id == 0)
        return NULL;

    for (uint32_t i = 0; i < XYRIS_APPLICATION_MAX; ++i) {
        if (applications[i].state != XYRIS_APP_EMPTY &&
            applications[i].id == id)
            return &applications[i];
    }
    return NULL;
}

XyrisWindow *application_window(const XyrisApplication *app)
{
    return app_is_registered(app) ? app->window : NULL;
}

const char *application_name(const XyrisApplication *app)
{
    return app_is_registered(app) ? app->name : NULL;
}

XyrisApplicationState application_state(const XyrisApplication *app)
{
    return app_is_registered(app) ? app->state : XYRIS_APP_EMPTY;
}

bool application_grant_capability(
    XyrisApplication *app, XKCapability capability)
{
    return app_is_registered(app) &&
        xk_capability_grant(app_capability_id(app), capability);
}

bool application_revoke_capability(
    XyrisApplication *app, XKCapability capability)
{
    return app_is_registered(app) &&
        xk_capability_revoke(app_capability_id(app), capability);
}

bool application_check_capability(
    const XyrisApplication *app, XKCapability capability)
{
    return app_is_registered(app) &&
        xk_capability_check(app_capability_id(app), capability);
}

bool application_focus_next(void)
{
    int32_t current_slot = -1;
    XyrisWindow *focused = window_focused();

    if (focused != NULL) {
        for (uint32_t i = 0; i < XYRIS_APPLICATION_MAX; ++i) {
            if (applications[i].state == XYRIS_APP_RUNNING &&
                applications[i].window == focused) {
                current_slot = (int32_t)i;
                break;
            }
        }
    }

    /*
     * Start after the focused application. If no application currently
     * owns focus, start at slot zero. Wrap once through the registry.
     */
    for (uint32_t step = 1; step <= XYRIS_APPLICATION_MAX; ++step) {
        uint32_t slot = current_slot < 0
            ? step - 1U
            : ((uint32_t)current_slot + step) % XYRIS_APPLICATION_MAX;

        if (applications[slot].state == XYRIS_APP_RUNNING &&
            applications[slot].window != NULL)
            return application_focus(&applications[slot]);
    }

    return false;
}
