#ifndef XYRIS_BOOT_H
#define XYRIS_BOOT_H

void boot_init(void);

void boot_header(void);

void boot_step(const char *step);

void boot_success(const char *message);

void boot_error(const char *message);

void boot_step_ok(const char *text);

void boot_step_warn(const char *text);

void boot_step_fail(const char *text);

void boot_ui_ok(const char *text);

void boot_ui_warn(const char *text);

void boot_ui_fail(const char *text);
/*
 * Render the collected boot diagnostics as a
 * fixed-position graphical boot-status page.
 *
 * This renderer never scrolls the framebuffer.
 */
void boot_status_render(void);

#endif