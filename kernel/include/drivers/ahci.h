#ifndef XK_AHCI_DRIVER_H
#define XK_AHCI_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#include "drivers/block.h"

#define XK_AHCI_CLASS_CODE 0x01
#define XK_AHCI_SUBCLASS   0x06
#define XK_AHCI_PROG_IF    0x01

#define XK_AHCI_MAX_PORTS 32

typedef struct
{
    uint8_t port_number;
    uint8_t implemented;
    uint8_t device_present;
    uint8_t device_type;
} XKAHCIPort;

typedef struct
{
    uint16_t vendor_id;
    uint16_t device_id;

    uint8_t bus;
    uint8_t device;
    uint8_t function;

    uintptr_t abar;

    uint32_t port_count;
    uint32_t implemented_ports;
} XKAHCIController;

bool xk_ahci_initialize(void);
void xk_ahci_shutdown(void);

bool xk_ahci_is_present(void);
XKAHCIController xk_ahci_controller_info(void);

uint32_t xk_ahci_port_count(void);
int xk_ahci_port_get(
    uint32_t index,
    XKAHCIPort *port
);

#endif