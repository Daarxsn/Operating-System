#ifndef SIMULATOR_H
#define SIMULATOR_H

void simulator_init(void);
void simulator_reset(void);
int simulator_execute(const char *command);

#endif