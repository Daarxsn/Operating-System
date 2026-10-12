#include "desktop_shell.h"

#include <stddef.h>

#include "renderer.h"
#include "compositor.h"
#include "font.h"

static bool shell_initialized = false;

static uint32_t shell_width = 0;
static uint32_t shell_height = 0;


/* -------------------------------------------------
   Desktop Shell Colors
------------------------------------------------- */

#define SHELL_BACKGROUND        0x00060B16
#define SHELL_TOPBAR            0x000A1220
#define SHELL_TOPBAR_EDGE       0x00142A40
#define SHELL_DOCK              0x000A1220
#define SHELL_DOCK_EDGE         0x00142A40

#define SHELL_ACCENT             0x0038BDF8
#define SHELL_ACCENT_BRIGHT      0x0067D9FF
#define SHELL_TEXT               0x00E6F4FF
#define SHELL_MUTED              0x007C9BB5

#define SHELL_PANEL              0x00101D2C
#define SHELL_PANEL_HOVER        0x00182B40


/* -------------------------------------------------
   Desktop Shell Dimensions
------------------------------------------------- */

#define SHELL_TOPBAR_HEIGHT      54
#define SHELL_DOCK_HEIGHT        72

#define SHELL_DOCK_MARGIN        18
#define SHELL_DOCK_RADIUS        14

#define SHELL_STATUS_SIZE        8
#define SHELL_ICON_SIZE          34


/* -------------------------------------------------
   Internal Drawing Helpers
------------------------------------------------- */

static void shell_draw_top_bar(void)
{
    if (shell_height < SHELL_TOPBAR_HEIGHT)
        return;

    renderer_fill_rect(
        0,
        0,
        (int)shell_width,
        SHELL_TOPBAR_HEIGHT,
        SHELL_TOPBAR
    );

    renderer_fill_rect(
        0,
        SHELL_TOPBAR_HEIGHT - 1,
        (int)shell_width,
        1,
        SHELL_TOPBAR_EDGE
    );


    /* Xyris launcher indicator */

    renderer_fill_circle(
        27,
        27,
        8,
        SHELL_ACCENT
    );

    renderer_fill_circle(
        27,
        27,
        4,
        SHELL_BACKGROUND
    );


    /* Search field */

    renderer_fill_rect(
        54,
        12,
        260,
        30,
        SHELL_PANEL
    );

    renderer_draw_rect(
        54,
        12,
        260,
        30,
        SHELL_TOPBAR_EDGE
    );


    /* Search icon */

    renderer_draw_circle(
        72,
        25,
        6,
        SHELL_MUTED
    );

    renderer_draw_line(
        76,
        30,
        81,
        35,
        SHELL_MUTED
    );


    /* Right-side network indicator */

    renderer_fill_circle(
        (int)shell_width - 105,
        27,
        SHELL_STATUS_SIZE,
        SHELL_ACCENT
    );

    renderer_fill_circle(
        (int)shell_width - 105,
        27,
        3,
        SHELL_TOPBAR
    );


    /* System status indicator */

    renderer_fill_circle(
        (int)shell_width - 76,
        27,
        SHELL_STATUS_SIZE,
        SHELL_ACCENT_BRIGHT
    );


    /* Power indicator */

    renderer_fill_circle(
        (int)shell_width - 42,
        27,
        SHELL_STATUS_SIZE,
        SHELL_TEXT
    );

    renderer_fill_rect(
        (int)shell_width - 44,
        16,
        4,
        9,
        SHELL_TOPBAR
    );
}


static void shell_draw_dock(void)
{
    if (shell_height <= SHELL_DOCK_HEIGHT + SHELL_DOCK_MARGIN)
        return;

    int dock_y =
        (int)shell_height -
        SHELL_DOCK_HEIGHT -
        SHELL_DOCK_MARGIN;

    int dock_x = SHELL_DOCK_MARGIN;

    int dock_width =
        (int)shell_width -
        (SHELL_DOCK_MARGIN * 2);

    renderer_fill_rect(
        dock_x,
        dock_y,
        dock_width,
        SHELL_DOCK_HEIGHT,
        SHELL_DOCK
    );

    renderer_draw_rect(
        dock_x,
        dock_y,
        dock_width,
        SHELL_DOCK_HEIGHT,
        SHELL_DOCK_EDGE
    );


    /* Dock accent line */

    renderer_fill_rect(
        dock_x + 22,
        dock_y + 2,
        dock_width - 44,
        2,
        SHELL_ACCENT
    );


    /*
       Application icons
    */

    int icon_y =
        dock_y +
        ((SHELL_DOCK_HEIGHT - SHELL_ICON_SIZE) / 2);

    int first_icon_x = dock_x + 34;
    int icon_gap = 54;


    /* Apps */

    renderer_fill_rect(
        first_icon_x,
        icon_y,
        SHELL_ICON_SIZE,
        SHELL_ICON_SIZE,
        SHELL_PANEL
    );

    renderer_draw_rect(
        first_icon_x,
        icon_y,
        SHELL_ICON_SIZE,
        SHELL_ICON_SIZE,
        SHELL_ACCENT
    );

    renderer_fill_circle(
        first_icon_x + 17,
        icon_y + 17,
        7,
        SHELL_ACCENT
    );


    /* Files */

    int files_x =
        first_icon_x + icon_gap;

    renderer_fill_rect(
        files_x,
        icon_y,
        SHELL_ICON_SIZE,
        SHELL_ICON_SIZE,
        SHELL_PANEL
    );

    renderer_draw_rect(
        files_x,
        icon_y,
        SHELL_ICON_SIZE,
        SHELL_ICON_SIZE,
        SHELL_TOPBAR_EDGE
    );

    renderer_fill_rect(
        files_x + 8,
        icon_y + 9,
        18,
        16,
        SHELL_ACCENT
    );


    /* Terminal */

    int terminal_x =
        first_icon_x + (icon_gap * 2);

    renderer_fill_rect(
        terminal_x,
        icon_y,
        SHELL_ICON_SIZE,
        SHELL_ICON_SIZE,
        SHELL_PANEL
    );

    renderer_draw_rect(
        terminal_x,
        icon_y,
        SHELL_ICON_SIZE,
        SHELL_ICON_SIZE,
        SHELL_TOPBAR_EDGE
    );

    renderer_draw_line(
        terminal_x + 9,
        icon_y + 11,
        terminal_x + 16,
        icon_y + 17,
        SHELL_ACCENT
    );

    renderer_draw_line(
        terminal_x + 16,
        icon_y + 17,
        terminal_x + 9,
        icon_y + 23,
        SHELL_ACCENT
    );

    renderer_fill_rect(
        terminal_x + 20,
        icon_y + 22,
        7,
        2,
        SHELL_MUTED
    );


    /* AI */

    int ai_x =
        first_icon_x + (icon_gap * 3);

    renderer_fill_rect(
        ai_x,
        icon_y,
        SHELL_ICON_SIZE,
        SHELL_ICON_SIZE,
        SHELL_PANEL
    );

    renderer_draw_rect(
        ai_x,
        icon_y,
        SHELL_ICON_SIZE,
        SHELL_ICON_SIZE,
        SHELL_TOPBAR_EDGE
    );

    renderer_fill_circle(
        ai_x + 17,
        icon_y + 17,
        9,
        SHELL_ACCENT
    );

    renderer_fill_circle(
        ai_x + 17,
        icon_y + 17,
        4,
        SHELL_PANEL
    );


    /* Control Center */

    int control_x =
        first_icon_x + (icon_gap * 4);

    renderer_fill_rect(
        control_x,
        icon_y,
        SHELL_ICON_SIZE,
        SHELL_ICON_SIZE,
        SHELL_PANEL
    );

    renderer_draw_rect(
        control_x,
        icon_y,
        SHELL_ICON_SIZE,
        SHELL_ICON_SIZE,
        SHELL_TOPBAR_EDGE
    );

    renderer_fill_rect(
        control_x + 8,
        icon_y + 10,
        18,
        3,
        SHELL_MUTED
    );

    renderer_fill_rect(
        control_x + 8,
        icon_y + 16,
        18,
        3,
        SHELL_ACCENT
    );

    renderer_fill_rect(
        control_x + 8,
        icon_y + 22,
        18,
        3,
        SHELL_MUTED
    );


    /*
       Right-side workspace indicator
    */

    int workspace_x =
        dock_x +
        dock_width -
        88;

    int workspace_y =
        dock_y + 20;

    renderer_fill_rect(
        workspace_x,
        workspace_y,
        12,
        12,
        SHELL_ACCENT
    );

    renderer_fill_rect(
        workspace_x + 18,
        workspace_y,
        12,
        12,
        SHELL_PANEL_HOVER
    );

    renderer_fill_rect(
        workspace_x + 36,
        workspace_y,
        12,
        12,
        SHELL_PANEL_HOVER
    );
}


static void shell_draw_workspace(void)
{
    if (shell_height <=
        SHELL_TOPBAR_HEIGHT + SHELL_DOCK_HEIGHT)
    {
        return;
    }

    int workspace_top =
        SHELL_TOPBAR_HEIGHT + 1;

    int workspace_bottom =
        (int)shell_height -
        SHELL_DOCK_HEIGHT -
        SHELL_DOCK_MARGIN;

    if (workspace_bottom <= workspace_top)
        return;

    int workspace_height =
        workspace_bottom - workspace_top;

    /*
     * Subtle horizontal grid.
     */
    const int grid_spacing = 72;

    for (int y = workspace_top + grid_spacing;
         y < workspace_bottom;
         y += grid_spacing)
    {
        renderer_draw_line(
            0,
            y,
            (int)shell_width,
            y,
            0x00091420
        );
    }

    /*
     * Subtle vertical grid.
     */
    for (int x = grid_spacing;
         x < (int)shell_width;
         x += grid_spacing)
    {
        renderer_draw_line(
            x,
            workspace_top,
            x,
            workspace_bottom,
            0x00091420
        );
    }

    /*
     * Central ambient workspace zone.
     */
    int center_x =
        (int)shell_width / 2;

    int center_y =
        workspace_top +
        (workspace_height / 2);

    renderer_fill_circle(
        center_x,
        center_y,
        86,
        0x0008131F
    );

    renderer_draw_circle(
        center_x,
        center_y,
        86,
        0x000C1E30
    );

    renderer_draw_circle(
        center_x,
        center_y,
        62,
        0x0010263A
    );

    renderer_draw_circle(
        center_x,
        center_y,
        40,
        0x00122C42
    );

    /*
     * Central activity point.
     */
    renderer_fill_circle(
        center_x,
        center_y,
        7,
        SHELL_ACCENT
    );

    renderer_fill_circle(
        center_x,
        center_y,
        3,
        SHELL_BACKGROUND
    );

    /*
     * Directional ambient accents.
     */
    renderer_fill_rect(
        center_x - 120,
        center_y,
        42,
        1,
        0x00122C42
    );

    renderer_fill_rect(
        center_x + 78,
        center_y,
        42,
        1,
        0x00122C42
    );

    renderer_fill_rect(
        center_x,
        center_y - 120,
        1,
        42,
        0x00122C42
    );

    renderer_fill_rect(
        center_x,
        center_y + 78,
        1,
        42,
        0x00122C42
    );

    /*
     * Workspace identity.
     *
     * FONT_WIDTH is 8 pixels per character.
     * "XYRIS DESKTOP" = 13 characters = 104 pixels.
     */
    const char *title = "XYRIS DESKTOP";
    int title_width = 13 * FONT_WIDTH;

    font_draw_string(
        center_x - (title_width / 2),
        center_y - 30,
        title,
        SHELL_TEXT
    );

    /*
     * Active workspace indicator.
     *
     * "WORKSPACE 1" = 11 characters = 88 pixels.
     */
    const char *workspace_label = "WORKSPACE 1";
    int workspace_label_width = 11 * FONT_WIDTH;

    font_draw_string(
        center_x - (workspace_label_width / 2),
        center_y - 14,
        workspace_label,
        SHELL_ACCENT
    );

    /*
     * Session status.
     *
     * "READY" = 5 characters = 40 pixels.
     */
    const char *status = "READY";
    int status_width = 5 * FONT_WIDTH;

    font_draw_string(
        center_x - (status_width / 2),
        center_y + 28,
        status,
        SHELL_MUTED
    );
}

/* -------------------------------------------------
   Desktop Shell API
------------------------------------------------- */

void desktop_shell_init(void)
{
    shell_width = compositor_width();
    shell_height = compositor_height();

    shell_initialized = true;
}


void desktop_shell_render(void)
{
    if (!shell_initialized)
        return;

    renderer_begin_frame();

    renderer_clear(
        SHELL_BACKGROUND
    );

    shell_draw_workspace();
    shell_draw_top_bar();
    shell_draw_dock();

    renderer_end_frame();
}


void desktop_shell_update(void)
{
    if (!shell_initialized)
        return;

    desktop_shell_render();
}


bool desktop_shell_initialized(void)
{
    return shell_initialized;
}


uint32_t desktop_shell_width(void)
{
    return shell_width;
}


uint32_t desktop_shell_height(void)
{
    return shell_height;
}