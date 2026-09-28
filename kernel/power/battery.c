#include "battery.h"

#include "debug/print.h"

static bool g_battery_initialized = false;
static bool g_battery_available = false;
static bool g_battery_charging = false;
static uint8_t g_battery_percentage = 0;

bool xk_battery_init(void)
{
    g_battery_initialized = true;
    g_battery_available = false;
    g_battery_charging = false;
    g_battery_percentage = 0;

    debug_print_line("[BATTERY] Battery monitoring initialized");
    debug_print_line("[BATTERY] No battery backend detected");

    return true;
}

bool xk_battery_available(void)
{
    return g_battery_initialized && g_battery_available;
}

bool xk_battery_is_charging(void)
{
    return xk_battery_available() && g_battery_charging;
}

bool xk_battery_percentage(uint8_t *percentage)
{
    if (percentage == 0 || !xk_battery_available())
        return false;

    *percentage = g_battery_percentage;
    return true;
}
