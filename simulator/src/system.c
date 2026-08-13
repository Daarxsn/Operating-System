#include "system.h"

SystemState system_state;

void system_init(void)
{
    system_state.cpu_online = 1;
    system_state.memory_online = 1;
    system_state.display_online = 1;
    system_state.kernel_loaded = 0;
    system_state.ram_mb = 64;
}