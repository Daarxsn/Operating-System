#ifndef XYRIS_THERMAL_H
#define XYRIS_THERMAL_H

#include <stdbool.h>
#include <stdint.h>

bool xk_thermal_init(void);
bool xk_thermal_available(void);
bool xk_thermal_temperature_celsius(int32_t *temperature);

#endif
