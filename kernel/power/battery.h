#ifndef XYRIS_BATTERY_H
#define XYRIS_BATTERY_H

#include <stdbool.h>
#include <stdint.h>

bool xk_battery_init(void);
bool xk_battery_available(void);
bool xk_battery_is_charging(void);
bool xk_battery_percentage(uint8_t *percentage);

#endif
