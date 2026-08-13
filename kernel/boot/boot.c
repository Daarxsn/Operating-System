#include "boot.h"

#include "../debug/print.h"

void boot_init(void)
{
    debug_print_init();
}

void boot_header(void)
{
    debug_print_line("==================================================");
    debug_print_line("                 XYRISOS v0.1.0-alpha");
    debug_print_line("          Next Generation Operating System");
    debug_print_line("==================================================");
    debug_print_line("");
    debug_print_line("Boot Sequence");
    debug_print_line("");
}

void boot_step(const char *step)
{
    debug_print("[ OK ] ");
    debug_print_line(step);
}

void boot_success(const char *message)
{
    debug_print_line("");
    debug_print_line("--------------------------------------------------");
    debug_print_line(message);
}

void boot_error(const char *message)
{
    debug_print("[FAIL] ");
    debug_print_line(message);
}
void boot_step_ok(const char *text)
{
    debug_print("[ OK ] ");
    debug_print_line(text);
}

void boot_step_warn(const char *text)
{
    debug_print("[WARN] ");
    debug_print_line(text);
}

void boot_step_fail(const char *text)
{
    debug_print("[FAIL] ");
    debug_print_line(text);
}