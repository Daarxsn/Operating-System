#ifndef SYSTEM_H
#define SYSTEM_H

typedef struct
{
    int cpu_online;
    int memory_online;
    int display_online;
    int kernel_loaded;
    int ram_mb;
} SystemState;

extern SystemState system_state;

void system_init(void);

#endif