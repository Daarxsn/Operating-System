#ifndef XYRIS_PAGE_FAULT_H
#define XYRIS_PAGE_FAULT_H

#include "isr.h"

void page_fault_handler(registers_t *regs);

#endif