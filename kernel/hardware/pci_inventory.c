#include "pci_inventory.h"

#include <stddef.h>

#include "../include/drivers/pci.h"
#include "../debug/print.h"
#include "../debug/hex.h"

static pci_inventory_t g_pci_inventory;


/*
 * Classify a PCI device using the standard
 * PCI class code and subclass values.
 */
pci_hardware_class_t pci_hardware_classify(
    uint8_t class_code,
    uint8_t subclass)
{
    switch (class_code)
    {
        /* Mass Storage Controller */
        case 0x01:
            return PCI_HARDWARE_STORAGE;

        /* Network Controller */
        case 0x02:
            return PCI_HARDWARE_NETWORK;

        /* Display Controller */
        case 0x03:
            return PCI_HARDWARE_DISPLAY;

        /* Multimedia Controller */
        case 0x04:
            return PCI_HARDWARE_MULTIMEDIA;

        /* Bridge Device */
        case 0x06:
            return PCI_HARDWARE_BRIDGE;

        /* Input Device */
        case 0x09:
            return PCI_HARDWARE_INPUT;

        /* Serial Bus Controller */
        case 0x0C:
            /*
             * USB controllers use subclass 0x03.
             */
            if (subclass == 0x03)
                return PCI_HARDWARE_USB;

            return PCI_HARDWARE_OTHER;

        default:
            return PCI_HARDWARE_OTHER;
    }
}


/*
 * Initialize PCI hardware inventory.
 */
void pci_inventory_init(void)
{
    g_pci_inventory = (pci_inventory_t){0};

    uint32_t count =
        xk_pci_device_count();

    g_pci_inventory.total_devices =
        count;

    for (uint32_t i = 0; i < count; i++)
    {
        const XKPCIDevice *device =
            xk_pci_device_get(i);

        if (device == NULL)
            continue;

        pci_hardware_class_t type =
            pci_hardware_classify(
                device->class_code,
                device->subclass
            );

        switch (type)
        {
            case PCI_HARDWARE_STORAGE:
                g_pci_inventory.storage_devices++;
                break;

            case PCI_HARDWARE_NETWORK:
                g_pci_inventory.network_devices++;
                break;

            case PCI_HARDWARE_DISPLAY:
                g_pci_inventory.display_devices++;
                break;

            case PCI_HARDWARE_MULTIMEDIA:
                g_pci_inventory.multimedia_devices++;
                break;

            case PCI_HARDWARE_USB:
                g_pci_inventory.usb_devices++;
                break;

            case PCI_HARDWARE_BRIDGE:
                g_pci_inventory.bridge_devices++;
                break;

            case PCI_HARDWARE_INPUT:
                g_pci_inventory.input_devices++;
                break;

            case PCI_HARDWARE_OTHER:
            default:
                g_pci_inventory.other_devices++;
                break;
        }
    }
}


/*
 * Return information about one discovered PCI device.
 */
int pci_inventory_device_get(
    uint32_t index,
    pci_inventory_device_t *info)
{
    if (info == NULL)
        return 0;

    if (index >= xk_pci_device_count())
        return 0;

    const XKPCIDevice *device =
        xk_pci_device_get(index);

    if (device == NULL)
        return 0;

    info->bus =
        device->bus;

    info->device =
        device->device;

    info->function =
        device->function;

    info->vendor_id =
        device->vendor_id;

    info->device_id =
        device->device_id;

    info->class_code =
        device->class_code;

    info->subclass =
        device->subclass;

    info->prog_if =
        device->prog_if;

    info->hardware_class =
        pci_hardware_classify(
            device->class_code,
            device->subclass
        );

    return 1;
}


/*
 * Return PCI hardware inventory.
 */
pci_inventory_t pci_inventory_info(void)
{
    return g_pci_inventory;
}


/*
 * Print PCI hardware inventory diagnostics.
 */
void pci_inventory_dump(void)
{
    debug_print_line(
        "========== PCI HARDWARE INVENTORY =========="
    );

    debug_print("Total PCI Devices: ");
    debug_print_hex64(
        (uint64_t)g_pci_inventory.total_devices
    );
    debug_print_line("");

    debug_print("Storage Controllers: ");
    debug_print_hex64(
        (uint64_t)g_pci_inventory.storage_devices
    );
    debug_print_line("");

    debug_print("Network Controllers: ");
    debug_print_hex64(
        (uint64_t)g_pci_inventory.network_devices
    );
    debug_print_line("");

    debug_print("Display Controllers: ");
    debug_print_hex64(
        (uint64_t)g_pci_inventory.display_devices
    );
    debug_print_line("");

    debug_print("Multimedia Controllers: ");
    debug_print_hex64(
        (uint64_t)g_pci_inventory.multimedia_devices
    );
    debug_print_line("");

    debug_print("USB Controllers: ");
    debug_print_hex64(
        (uint64_t)g_pci_inventory.usb_devices
    );
    debug_print_line("");

    debug_print("Bridge Devices: ");
    debug_print_hex64(
        (uint64_t)g_pci_inventory.bridge_devices
    );
    debug_print_line("");

    debug_print("Input Devices: ");
    debug_print_hex64(
        (uint64_t)g_pci_inventory.input_devices
    );
    debug_print_line("");

    debug_print("Other Devices: ");
    debug_print_hex64(
        (uint64_t)g_pci_inventory.other_devices
    );
    debug_print_line("");

    /*
     * Print details for every discovered PCI device.
     */
    debug_print_line(
        "---------- PCI DEVICE DETAILS ----------"
    );

    for (uint32_t i = 0;
         i < g_pci_inventory.total_devices;
         i++)
    {
        pci_inventory_device_t device;

        if (!pci_inventory_device_get(i, &device))
            continue;

        debug_print("PCI Device ");
        debug_print_hex64((uint64_t)i);
        debug_print_line(":");

        debug_print("  Bus: ");
        debug_print_hex64((uint64_t)device.bus);
        debug_print_line("");

        debug_print("  Device: ");
        debug_print_hex64((uint64_t)device.device);
        debug_print_line("");

        debug_print("  Function: ");
        debug_print_hex64((uint64_t)device.function);
        debug_print_line("");

        debug_print("  Vendor ID: ");
        debug_print_hex64((uint64_t)device.vendor_id);
        debug_print_line("");

        debug_print("  Device ID: ");
        debug_print_hex64((uint64_t)device.device_id);
        debug_print_line("");

        debug_print("  Class: ");
        debug_print_hex64((uint64_t)device.class_code);
        debug_print_line("");

        debug_print("  Subclass: ");
        debug_print_hex64((uint64_t)device.subclass);
        debug_print_line("");

        debug_print("  Prog IF: ");
        debug_print_hex64((uint64_t)device.prog_if);
        debug_print_line("");

                const XKPCIDevice *raw_device =
            xk_pci_device_get(i);

        if (raw_device != NULL)
        {
            for (uint8_t bar = 0;
                 bar < raw_device->bar_count;
                 bar++)
            {
                debug_print("  BAR");
                debug_print_hex64((uint64_t)bar);
                debug_print(": ");
                debug_print_hex64(
                    (uint64_t)raw_device->bars[bar]
                );
                debug_print_line("");
            }
        }
    }

    debug_print_line(
        "============================================"
    );
}