#include "pic.h"
#include "io.h"
#include "../debug/print.h"

/* PIC I/O Ports */
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21

#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

/* PIC Commands */
#define PIC_EOI      0x20

/* Initialization Command Words */
#define ICW1_ICW4    0x01
#define ICW1_INIT    0x10
#define ICW4_8086    0x01

void pic_initialize(void)
{
    uint8_t master_mask = inb(PIC1_DATA);
    io_wait();

    uint8_t slave_mask = inb(PIC2_DATA);
    io_wait();

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA, 0x20);
    io_wait();

    outb(PIC2_DATA, 0x28);
    io_wait();

    outb(PIC1_DATA, 4);
    io_wait();

    outb(PIC2_DATA, 2);
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();

    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, master_mask);
    io_wait();

    outb(PIC2_DATA, slave_mask);
    io_wait();
}

void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8)
    {
        outb(PIC2_COMMAND, PIC_EOI);
    }

    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_mask_irq(uint8_t irq)
{
    uint16_t port;
    uint8_t value;

    if (irq < 8)
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb(port);
    value |= (1 << irq);
    outb(port, value);
}

void pic_unmask_irq(uint8_t irq)
{
    uint16_t port;
    uint8_t value;

    if (irq < 8)
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb(port);
    value &= ~(1 << irq);
    outb(port, value);
}

uint8_t pic_debug_get_master_mask(void)
{
    return inb(PIC1_DATA);
}

uint8_t pic_debug_get_master_irr(void)
{
    outb(PIC1_COMMAND, 0x0A);
    return inb(PIC1_COMMAND);
}