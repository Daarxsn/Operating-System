#ifndef XYRIS_USER_H
#define XYRIS_USER_H

#include <stdint.h>
#include "../memory/vmm.h"

typedef struct
{
    uint64_t entry;
    uint64_t stack;
    uint64_t stack_base;
    uint64_t stack_size;
    address_space_t* address_space;
} user_process_t;

int user_prepare(user_process_t* process, uint64_t entry);
void user_destroy(user_process_t* process);
void user_enter(user_process_t* process);

#endif
