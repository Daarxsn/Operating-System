#ifndef XYRIS_IO_H
#define XYRIS_IO_H

#include <stdint.h>

/* Port I/O */

uint8_t  inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);

void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
void outl(uint16_t port, uint32_t value);

/* Small delay for legacy devices (PIC, etc.) */
void io_wait(void);

#endif