#include "thermal.h"

#include "debug/print.h"

static bool g_thermal_initialized = false;
static bool g_thermal_available = false;

bool xk_thermal_init(void)
{
    g_thermal_initialized = true;
    g_thermal_available = false;

    debug_print_line("[THERMAL] Thermal monitoring initialized");
    debug_print_line("[THERMAL] No thermal sensor backend detected");

    return true;
}

bool xk_thermal_available(void)
{
    return g_thermal_initialized && g_thermal_available;
}

bool xk_thermal_temperature_celsius(int32_t *temperature)
{
    if (temperature == 0 || !xk_thermal_available())
        return false;

    *temperature = 0;
    return true;
}
