#ifndef XK_XHCI_DRIVER_H
#define XK_XHCI_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#include "drivers/driver.h"

#define XK_XHCI_CLASS_CODE 0x0C
#define XK_XHCI_SUBCLASS   0x03
#define XK_XHCI_PROG_IF    0x30

typedef struct
{
    uint16_t vendor_id;
    uint16_t device_id;

    uint8_t bus;
    uint8_t device;
    uint8_t function;

    uintptr_t mmio_base;
    uintptr_t operational_base;
    uintptr_t runtime_base;
    uintptr_t doorbell_base;

    uint8_t revision;
    uint8_t cap_length;

    uint8_t max_slots;
    uint8_t max_ports;
    uint8_t context_size;

    uint32_t page_size;

    uint8_t max_interrupters;

    uint8_t initialized;
    uint8_t controller_ready;
    uint8_t runtime_ready;
    uint8_t controller_running;

} XKXHCIController;

bool xk_xhci_initialize(void);
void xk_xhci_shutdown(void);

bool xk_xhci_is_present(void);

XKXHCIController xk_xhci_controller_info(void);

extern XKDriver xk_xhci_driver;

#endif