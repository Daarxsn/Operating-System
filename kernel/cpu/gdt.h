#ifndef XYRIS_GDT_H
#define XYRIS_GDT_H

#include <stdint.h>

typedef struct __attribute__((packed))
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} gdt_entry_t;

typedef struct __attribute__((packed))
{
    uint16_t limit;
    uint64_t base;
} gdt_ptr_t;

/* Task State Segment */

typedef struct __attribute__((packed))
{
    uint32_t reserved0;

    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;

    uint64_t reserved1;

    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;

    uint64_t reserved2;

    uint16_t reserved3;
    uint16_t io_map_base;

} tss_t;

void gdt_init(void);

#endif