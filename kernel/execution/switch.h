#ifndef XYRIS_SWITCH_H
#define XYRIS_SWITCH_H

#include "context.h"

void switch_context(
    context_t *old,
    context_t *next
);

__attribute__((noreturn))
void switch_to_context(
    const context_t *next
);

#endif