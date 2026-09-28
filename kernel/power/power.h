#ifndef XYRIS_POWER_H
#define XYRIS_POWER_H

#include <stdbool.h>

typedef enum
{
    XK_POWER_STATE_UNKNOWN = 0,
    XK_POWER_STATE_RUNNING,
    XK_POWER_STATE_SHUTTING_DOWN,
    XK_POWER_STATE_REBOOTING,
    XK_POWER_STATE_HALTED
} XKPowerState;

bool xk_power_init(void);

XKPowerState xk_power_state(void);

void xk_power_shutdown(void);
void xk_power_reboot(void);
void xk_power_halt(void);

#endif