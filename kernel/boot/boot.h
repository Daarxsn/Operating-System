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

#endif