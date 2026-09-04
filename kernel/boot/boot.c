#include "boot.h"

#include <stdint.h>

#include "../debug/print.h"
#include "../graphics/framebuffer.h"
#include "../graphics/font.h"

/*
 * -------------------------------------------------------
 * Boot Status Log
 * -------------------------------------------------------
 *
 * Boot diagnostics are recorded in memory while the kernel
 * initializes.
 *
 * Serial diagnostics continue normally through debug_print().
 *
 * The graphical framebuffer is rendered only once later,
 * after the splash screen has completed.
 */

#define BOOT_STATUS_MAX_ENTRIES 64

typedef enum
{
    BOOT_STATUS_OK,
    BOOT_STATUS_WARN,
    BOOT_STATUS_FAIL
} boot_status_type_t;

typedef struct
{
    const char *text;
    boot_status_type_t type;
} boot_status_entry_t;

static boot_status_entry_t
    boot_status_entries[BOOT_STATUS_MAX_ENTRIES];

static uint32_t boot_status_count = 0;

#define BOOT_UI_MAX_ENTRIES 20

static boot_status_entry_t
    boot_ui_entries[BOOT_UI_MAX_ENTRIES];

static uint32_t boot_ui_count = 0;

static void boot_ui_record(
    const char *text,
    boot_status_type_t type
)
{
    if (boot_ui_count >= BOOT_UI_MAX_ENTRIES)
        return;

    boot_ui_entries[boot_ui_count].text = text;
    boot_ui_entries[boot_ui_count].type = type;

    boot_ui_count++;
}
/*
 * -------------------------------------------------------
 * Internal Status Recording
 * -------------------------------------------------------
 */

static void boot_status_record(
    const char *text,
    boot_status_type_t type
)
{
    if (boot_status_count >= BOOT_STATUS_MAX_ENTRIES)
        return;

    boot_status_entries[boot_status_count].text = text;
    boot_status_entries[boot_status_count].type = type;

    boot_status_count++;
}


/*
 * -------------------------------------------------------
 * Boot Initialization
 * -------------------------------------------------------
 */

void boot_init(void)
{
    debug_print_init();

    boot_status_count = 0;
    boot_ui_count = 0;
}


/*
 * -------------------------------------------------------
 * Serial Boot Header
 * -------------------------------------------------------
 */

void boot_header(void)
{
    debug_print_line(
        "=================================================="
    );

    debug_print_line(
        "                 XYRISOS v0.1.0-alpha"
    );

    debug_print_line(
        "          Next Generation Operating System"
    );

    debug_print_line(
        "=================================================="
    );

    debug_print_line("");

    debug_print_line(
        "Boot Sequence"
    );

    debug_print_line("");
}


/*
 * -------------------------------------------------------
 * Serial Boot Messages
 * -------------------------------------------------------
 */

void boot_step(const char *step)
{
    boot_status_record(
        step,
        BOOT_STATUS_OK
    );

    debug_print("[ OK ] ");
    debug_print_line(step);
}


void boot_success(const char *message)
{
    debug_print_line("");

    debug_print_line(
        "--------------------------------------------------"
    );

    debug_print_line(message);
}


void boot_error(const char *message)
{
    boot_status_record(
        message,
        BOOT_STATUS_FAIL
    );

    debug_print("[FAIL] ");
    debug_print_line(message);
}


/*
 * -------------------------------------------------------
 * Boot Status Helpers
 * -------------------------------------------------------
 */

void boot_step_ok(const char *text)
{
    boot_status_record(
        text,
        BOOT_STATUS_OK
    );

    debug_print("[ OK ] ");
    debug_print_line(text);
}


void boot_step_warn(const char *text)
{
    boot_status_record(
        text,
        BOOT_STATUS_WARN
    );

    debug_print("[WARN] ");
    debug_print_line(text);
}


void boot_step_fail(const char *text)
{
    boot_status_record(
        text,
        BOOT_STATUS_FAIL
    );

    debug_print("[FAIL] ");
    debug_print_line(text);
}
void boot_ui_ok(const char *text)
{
    boot_status_record(
        text,
        BOOT_STATUS_OK
    );

    boot_ui_record(
        text,
        BOOT_STATUS_OK
    );

    debug_print("[ OK ] ");
    debug_print_line(text);
}

void boot_ui_warn(const char *text)
{
    boot_status_record(
        text,
        BOOT_STATUS_WARN
    );

    boot_ui_record(
        text,
        BOOT_STATUS_WARN
    );

    debug_print("[WARN] ");
    debug_print_line(text);
}

void boot_ui_fail(const char *text)
{
    boot_status_record(
        text,
        BOOT_STATUS_FAIL
    );

    boot_ui_record(
        text,
        BOOT_STATUS_FAIL
    );

    debug_print("[FAIL] ");
    debug_print_line(text);
}

/*
 * -------------------------------------------------------
 * Graphical Boot Status Page
 * -------------------------------------------------------
 *
 * The entire page is drawn at fixed coordinates.
 *
 * There is deliberately NO cursor management,
 * debug_print(), or framebuffer scrolling here.
 */

void boot_status_render(void)
{
    if (framebuffer_width() == 0 ||
        framebuffer_height() == 0)
    {
        return;
    }

    const uint32_t width = framebuffer_width();
    const uint32_t height = framebuffer_height();

    /*
     * Clear the entire framebuffer first.
     */
    framebuffer_clear(0x00000000);

    /*
     * ---------------------------------------------------
     * Header
     * ---------------------------------------------------
     */

    const char *title =
        "XYRISOS v0.1.0-alpha";

    const char *subtitle =
        "System Initialization";

    const int title_width =
        (int)((sizeof("XYRISOS v0.1.0-alpha") - 1) *
              FONT_WIDTH);

    const int subtitle_width =
        (int)((sizeof("System Initialization") - 1) *
              FONT_WIDTH);

    const int title_x =
        ((int)width - title_width) / 2;

    const int subtitle_x =
        ((int)width - subtitle_width) / 2;

    font_draw_string(
        title_x,
        48,
        title,
        0x00FFFFFF
    );

  font_draw_string(
    subtitle_x,
    72,
    subtitle,
    0x00AAAAAA
);

const int divider_y = 105;

framebuffer_fill_rect(
    120,
    divider_y,
    width - 240,
    1,
    0x00555555
);

    /*
     * ---------------------------------------------------
     * Boot Status Entries
     * ---------------------------------------------------
     */

    const int left_margin = 120;
    const int first_y = 150;
    const int line_height = 24;

    const uint32_t ok_color =
        0x00FFFFFF;

    const uint32_t warn_color =
        0x00FFFF00;

    const uint32_t fail_color =
        0x00FF5555;

    uint32_t visible_entries = 0;

    for (uint32_t i = 0;
     i < boot_ui_count;
     i++)
    {
        /*
         * Keep the entire page fixed.
         *
         * Never scroll if the log is larger than the
         * available framebuffer area.
         */
        const int y =
            first_y +
            ((int)visible_entries * line_height);

        if ((uint32_t)(y + FONT_HEIGHT + 8) >= height)
            break;

const boot_status_entry_t *entry =
    &boot_ui_entries[i];

        const char *prefix;
        uint32_t color;

        switch (entry->type)
        {
            case BOOT_STATUS_WARN:
                prefix = "[WARN]";
                color = warn_color;
                break;

            case BOOT_STATUS_FAIL:
                prefix = "[FAIL]";
                color = fail_color;
                break;

            case BOOT_STATUS_OK:
            default:
                prefix = "[ OK ]";
                color = ok_color;
                break;
        }

        font_draw_string(
            left_margin,
            y,
            prefix,
            color
        );

        font_draw_string(
            left_margin + (7 * FONT_WIDTH),
            y,
            entry->text,
            0x00FFFFFF
        );

        visible_entries++;
    }

    /*
     * ---------------------------------------------------
     * Kernel Ready
     * ---------------------------------------------------
     */

    const char *ready =
        "Kernel Ready";

    const int ready_width =
        (int)((sizeof("Kernel Ready") - 1) *
              FONT_WIDTH);

    const int ready_x =
        ((int)width - ready_width) / 2;

    const int ready_y =
        (int)height - 48;

    font_draw_string(
        ready_x,
        ready_y,
        ready,
        0x00FFFFFF
    );
}