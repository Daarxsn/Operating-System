#include "power.h"

#include <stdbool.h>

#include "compiler.h"
#include "drivers/driver.h"
#include "debug/print.h"
#include "thermal.h"
#include "battery.h"

static XKPowerState g_power_state = XK_POWER_STATE_UNKNOWN;

bool xk_power_init(void)
{
    g_power_state = XK_POWER_STATE_RUNNING;

    xk_battery_init();
    xk_thermal_init();

    debug_print_line("[POWER] Power management initialized");
    debug_print_line("[POWER] Shutdown/reboot framework available");
    debug_print_line("[POWER] Battery, ACPI and suspend backends are not enabled");

    return true;
}

XKPowerState xk_power_state(void)
{
    return g_power_state;
}

void xk_power_shutdown(void)
{
    if (g_power_state == XK_POWER_STATE_SHUTTING_DOWN ||
        g_power_state == XK_POWER_STATE_HALTED)
    {
        return;
    }

    g_power_state = XK_POWER_STATE_SHUTTING_DOWN;

    debug_print_line("[POWER] Shutting down drivers...");

    /*
     * Give registered drivers an opportunity to release
     * hardware resources before the CPU is halted.
     */
    xk_driver_shutdown_all();

    debug_print_line("[POWER] Drivers shut down");
    debug_print_line("[POWER] Halting CPU");

    xk_power_halt();
}

void xk_power_reboot(void)
{
    /*
     * A real reboot backend is platform-specific.
     *
     * Do not write blindly to ACPI, keyboard-controller,
     * or chipset ports because that could be unsafe on
     * real hardware.
     *
     * For now, perform controlled driver cleanup and
     * halt until a validated reboot backend is implemented.
     */
    if (g_power_state == XK_POWER_STATE_REBOOTING ||
        g_power_state == XK_POWER_STATE_HALTED)
    {
        return;
    }

    g_power_state = XK_POWER_STATE_REBOOTING;

    debug_print_line("[POWER] Reboot requested");
    debug_print_line("[POWER] Shutting down drivers...");

    xk_driver_shutdown_all();

    debug_print_line("[POWER] Reboot backend not enabled");
    debug_print_line("[POWER] Halting CPU safely");

    xk_power_halt();
}

void xk_power_halt(void)
{
    g_power_state = XK_POWER_STATE_HALTED;

    disable_interrupts();

    for (;;)
    {
        halt_cpu();
    }
}