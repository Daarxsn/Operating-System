#ifndef XYRIS_DESKTOP_SHELL_H
#define XYRIS_DESKTOP_SHELL_H

#include <stdint.h>
#include <stdbool.h>

void desktop_shell_init(void);
void desktop_shell_render(void);
void desktop_shell_update(void);

bool desktop_shell_initialized(void);

uint32_t desktop_shell_width(void);
uint32_t desktop_shell_height(void);

#endif