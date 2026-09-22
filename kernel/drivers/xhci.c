#include "drivers/xhci.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "drivers/pci.h"
#include "memory/hhdm.h"
#include "memory/pmm.h"
#include "include/compiler.h"
#include "debug/print.h"

/*
 * ================================================================
 * xHCI Capability Registers
 * ================================================================
 */

#define XHCI_CAPLENGTH                  0x00
#define XHCI_HCIVERSION                 0x02
#define XHCI_HCSPARAMS1                0x04
#define XHCI_HCSPARAMS2                0x08
#define XHCI_HCCPARAMS1                0x10
#define XHCI_DBOFF                     0x14
#define XHCI_RTSOFF                    0x18

/*
 * ================================================================
 * xHCI Operational Registers
 * ================================================================
 */

#define XHCI_USBCMD                    0x00
#define XHCI_USBSTS                    0x04
#define XHCI_PAGESIZE                  0x08
#define XHCI_CRCR                     0x18
#define XHCI_DCBAAP                   0x30
#define XHCI_CONFIG                   0x38

/*
 * ================================================================
 * xHCI Root Port Registers
 * ================================================================
 */

#define XHCI_PORT_REGISTER_BASE        0x400
#define XHCI_PORT_REGISTER_STRIDE      0x10
#define XHCI_PORTSC                    0x00

#define XHCI_PORTSC_CCS               (1U << 0)
#define XHCI_PORTSC_PED               (1U << 1)
#define XHCI_PORTSC_PR                (1U << 4)
#define XHCI_PORTSC_PP                (1U << 9)

#define XHCI_PORTSC_SPEED_SHIFT       10
#define XHCI_PORTSC_SPEED_MASK        0xFU

#define XHCI_PORTSC_CSC               (1U << 17)
#define XHCI_PORTSC_PRC               (1U << 21)

/*
 * ================================================================
 * xHCI USB Command / Status Bits
 * ================================================================
 */

#define XHCI_USBCMD_RUN_STOP           0x00000001U
#define XHCI_USBCMD_HC_RESET           0x00000002U

#define XHCI_USBSTS_HCHALTED           0x00000001U
#define XHCI_USBSTS_CNR                0x00000800U

#define XHCI_HCCPARAMS1_CSZ            (1U << 2)

/*
 * ================================================================
 * xHCI Runtime Interrupter
 * ================================================================
 */

#define XHCI_INTR0_BASE                0x20

#define XHCI_IMAN                      0x00
#define XHCI_IMOD                      0x04
#define XHCI_ERSTSZ                    0x08
#define XHCI_ERSTBA                    0x10
#define XHCI_ERDP                      0x18

#define XHCI_IMAN_IP                   (1U << 0)
#define XHCI_IMAN_IE                   (1U << 1)

/*
 * ================================================================
 * xHCI Ring / DMA Constants
 * ================================================================
 */

#define XHCI_TRB_SIZE                 16
#define XHCI_RING_TRBS                256

#define XHCI_COMMAND_RING_SIZE \
    (XHCI_TRB_SIZE * XHCI_RING_TRBS)

#define XHCI_EVENT_RING_SIZE \
    (XHCI_TRB_SIZE * XHCI_RING_TRBS)

#define XHCI_COMMAND_RING_ALIGNMENT   64
#define XHCI_EVENT_RING_ALIGNMENT     64
#define XHCI_DCBAA_ALIGNMENT          64
#define XHCI_ERST_ALIGNMENT           64
#define XHCI_EP0_RING_ALIGNMENT       64

#define XHCI_RESET_TIMEOUT             1000000U
#define XHCI_TRANSFER_TIMEOUT          1000000U

/*
 * ================================================================
 * xHCI TRB Definitions
 * ================================================================
 */

#define XHCI_TRB_TYPE_SHIFT            10
#define XHCI_TRB_TYPE_MASK             0x3FU
#define XHCI_TRB_CYCLE                 (1U << 0)

#define XHCI_TRB_TYPE_NORMAL           1U
#define XHCI_TRB_TYPE_SETUP_STAGE      2U
#define XHCI_TRB_TYPE_DATA_STAGE       3U
#define XHCI_TRB_TYPE_STATUS_STAGE     4U
#define XHCI_TRB_TYPE_LINK             6U

#define XHCI_TRB_TYPE_ENABLE_SLOT      9U
#define XHCI_TRB_TYPE_ADDRESS_DEVICE   11U

#define XHCI_TRB_TYPE_TRANSFER_EVENT   32U
#define XHCI_TRB_TYPE_COMMAND_COMPLETE 33U

#define XHCI_COMPLETION_SUCCESS        1U

#define XHCI_TRB_IOC                   (1U << 5)
#define XHCI_TRB_IDT                   (1U << 6)

#define XHCI_TRB_TRANSFER_DIR_IN       (1U << 16)

#define XHCI_SETUP_TRT_SHIFT           16
#define XHCI_SETUP_TRT_NONE            0U
#define XHCI_SETUP_TRT_OUT             2U
#define XHCI_SETUP_TRT_IN              3U

#define XHCI_STATUS_TRANSFER_LENGTH_MASK 0x0001FFFFU

#define XHCI_DOORBELL_COMMAND          0U

/*
 * ================================================================
 * xHCI Input / Output Context
 * ================================================================
 */

#define XHCI_INPUT_CONTROL_CONTEXT_OFFSET   0U
#define XHCI_SLOT_CONTEXT_OFFSET            1U
#define XHCI_EP0_CONTEXT_OFFSET             2U

#define XHCI_INPUT_CONTROL_ADD_SLOT         (1U << 0)
#define XHCI_INPUT_CONTROL_ADD_EP0          (1U << 1)

#define XHCI_SLOT_ROUTE_STRING_MASK         0x000FFFFFU
#define XHCI_SLOT_SPEED_SHIFT               20U
#define XHCI_SLOT_SPEED_MASK                0x0FU

#define XHCI_SLOT_CONTEXT_ENTRIES_SHIFT    27U
#define XHCI_SLOT_CONTEXT_ENTRIES_MASK     0x1FU

#define XHCI_SLOT_ROOT_HUB_PORT_SHIFT      16U
#define XHCI_SLOT_ROOT_HUB_PORT_MASK       0xFFU

#define XHCI_EP_TYPE_SHIFT                  3U
#define XHCI_EP_TYPE_CONTROL               4U

#define XHCI_EP_MAX_PACKET_SHIFT           16U

#define XHCI_EP_CERR_SHIFT                  1U
#define XHCI_EP_CERR_VALUE                 3U

#define XHCI_EP_DCS                        (1U << 0)

/*
 * ================================================================
 * USB Requests
 * ================================================================
 */

#define USB_REQ_GET_DESCRIPTOR              0x06U

#define USB_DESC_DEVICE                     0x01U
#define USB_DESC_CONFIGURATION              0x02U

#define USB_REQTYPE_STANDARD_IN             0x80U

#define USB_DEVICE_DESCRIPTOR_LENGTH        18U
#define USB_CONFIGURATION_DESCRIPTOR_LENGTH 9U

/*
 * ================================================================
 * xHCI Structures
 * ================================================================
 */

typedef struct
{
    uint32_t parameter_low;
    uint32_t parameter_high;
    uint32_t status;
    uint32_t control;
} XKXHCITRB;

typedef struct
{
    uint16_t enqueue_index;
    uint8_t cycle_state;
} XKXHCICommandRingState;

typedef struct
{
    uint16_t dequeue_index;
    uint8_t cycle_state;
} XKXHCIEventRingState;

typedef struct
{
    uint16_t enqueue_index;
    uint8_t cycle_state;
} XKXHCITransferRingState;

/*
 * ================================================================
 * xHCI DMA Resources
 * ================================================================
 */

typedef struct
{
    phys_addr_t dcbaa_physical;
    uint64_t *dcbaa_virtual;

    phys_addr_t command_ring_physical;
    uint8_t *command_ring_virtual;

    phys_addr_t event_ring_physical;
    uint8_t *event_ring_virtual;

    phys_addr_t erst_physical;
    uint8_t *erst_virtual;

    phys_addr_t ep0_ring_physical;
    uint8_t *ep0_ring_virtual;

    phys_addr_t device_context_physical;
    uint8_t *device_context_virtual;

    phys_addr_t input_context_physical;
    uint8_t *input_context_virtual;

    /*
     * Descriptor buffer used by EP0 GET_DESCRIPTOR.
     */
    phys_addr_t descriptor_physical;
    uint8_t *descriptor_virtual;

    uint8_t active_slot_id;
    uint8_t active_port;
    uint8_t active_speed;

    uint16_t ep0_max_packet;

    bool allocated;
} XKXHCIRuntimeResources;

/*
 * ================================================================
 * Global xHCI State
 * ================================================================
 */

static XKXHCIController g_xhci_controller;

static XKXHCIRuntimeResources
    g_xhci_runtime;

static XKXHCICommandRingState
    g_xhci_command_ring_state;

static XKXHCIEventRingState
    g_xhci_event_ring_state;

static XKXHCITransferRingState
    g_xhci_ep0_ring_state;

static bool g_xhci_present;

/*
 * ================================================================
 * MMIO Helpers
 * ================================================================
 */

static uint8_t xhci_read8(uintptr_t address)
{
    return *(volatile uint8_t *)address;
}

static uint16_t xhci_read16(uintptr_t address)
{
    return *(volatile uint16_t *)address;
}

static uint32_t xhci_read32(uintptr_t address)
{
    return *(volatile uint32_t *)address;
}

static void xhci_write32(
    uintptr_t address,
    uint32_t value
)
{
    *(volatile uint32_t *)address = value;
}

static void xhci_write64(
    uintptr_t address,
    uint64_t value
)
{
    xhci_write32(
        address,
        (uint32_t)(value & 0xFFFFFFFFULL)
    );

    xhci_write32(
        address + 4,
        (uint32_t)(value >> 32)
    );
}

/*
 * ================================================================
 * PCI BAR
 * ================================================================
 */

static uintptr_t xhci_bar_address(
    const XKPCIDevice *device
)
{
    if (device == NULL ||
        device->bar_count == 0)
    {
        return 0;
    }

    uint32_t bar = device->bars[0];

    if ((bar & 0x01U) != 0)
    {
        return 0;
    }

    return (uintptr_t)(bar & ~0x0FU);
}

/*
 * ================================================================
 * Controller Wait Helpers
 * ================================================================
 */

static bool xhci_wait_halted(
    uintptr_t operational_base
)
{
    for (uint32_t i = 0;
         i < XHCI_RESET_TIMEOUT;
         i++)
    {
        uint32_t status =
            xhci_read32(
                operational_base +
                XHCI_USBSTS
            );

        if ((status & XHCI_USBSTS_HCHALTED) != 0)
        {
            return true;
        }
    }

    return false;
}

static bool xhci_wait_reset_complete(
    uintptr_t operational_base
)
{
    for (uint32_t i = 0;
         i < XHCI_RESET_TIMEOUT;
         i++)
    {
        uint32_t command =
            xhci_read32(
                operational_base +
                XHCI_USBCMD
            );

        if ((command & XHCI_USBCMD_HC_RESET) == 0)
        {
            return true;
        }
    }

    return false;
}

static bool xhci_wait_controller_ready(
    uintptr_t operational_base
)
{
    for (uint32_t i = 0;
         i < XHCI_RESET_TIMEOUT;
         i++)
    {
        uint32_t status =
            xhci_read32(
                operational_base +
                XHCI_USBSTS
            );

        if ((status & XHCI_USBSTS_CNR) == 0)
        {
            return true;
        }
    }

    return false;
}

/*
 * ================================================================
 * Controller Reset
 * ================================================================
 */

static bool xhci_controller_reset(
    uintptr_t operational_base
)
{
    uint32_t command =
        xhci_read32(
            operational_base +
            XHCI_USBCMD
        );

    command &= ~XHCI_USBCMD_RUN_STOP;

    xhci_write32(
        operational_base +
        XHCI_USBCMD,
        command
    );

    if (!xhci_wait_halted(
            operational_base))
    {
        return false;
    }

    command =
        xhci_read32(
            operational_base +
            XHCI_USBCMD
        );

    command |= XHCI_USBCMD_HC_RESET;

    xhci_write32(
        operational_base +
        XHCI_USBCMD,
        command
    );

    if (!xhci_wait_reset_complete(
            operational_base))
    {
        return false;
    }

    if (!xhci_wait_controller_ready(
            operational_base))
    {
        return false;
    }

    return true;
}

/*
 * ================================================================
 * DMA Cleanup
 * ================================================================
 */

static void xhci_free_runtime_resources(void)
{
    if (g_xhci_runtime.dcbaa_physical != 0)
    {
        pmm_free_page(
            g_xhci_runtime.dcbaa_physical
        );
    }

    if (g_xhci_runtime.command_ring_physical != 0)
    {
        pmm_free_page(
            g_xhci_runtime.command_ring_physical
        );
    }

    if (g_xhci_runtime.event_ring_physical != 0)
    {
        pmm_free_page(
            g_xhci_runtime.event_ring_physical
        );
    }

    if (g_xhci_runtime.erst_physical != 0)
    {
        pmm_free_page(
            g_xhci_runtime.erst_physical
        );
    }

    if (g_xhci_runtime.ep0_ring_physical != 0)
    {
        pmm_free_page(
            g_xhci_runtime.ep0_ring_physical
        );
    }

    if (g_xhci_runtime.device_context_physical != 0)
    {
        pmm_free_page(
            g_xhci_runtime.device_context_physical
        );
    }

    if (g_xhci_runtime.input_context_physical != 0)
    {
        pmm_free_page(
            g_xhci_runtime.input_context_physical
        );
    }

    if (g_xhci_runtime.descriptor_physical != 0)
    {
        pmm_free_page(
            g_xhci_runtime.descriptor_physical
        );
    }

    g_xhci_runtime =
        (XKXHCIRuntimeResources){0};

    g_xhci_ep0_ring_state =
        (XKXHCITransferRingState){0};
}

/*
 * ================================================================
 * DMA Allocation
 * ================================================================
 */

static bool xhci_allocate_runtime_resources(void)
{
    g_xhci_runtime =
        (XKXHCIRuntimeResources){0};

    g_xhci_runtime.dcbaa_physical =
        pmm_alloc_page();

    if (g_xhci_runtime.dcbaa_physical == 0)
    {
        goto fail;
    }

    g_xhci_runtime.dcbaa_virtual =
        (uint64_t *)phys_to_virt(
            g_xhci_runtime.dcbaa_physical
        );

    g_xhci_runtime.command_ring_physical =
        pmm_alloc_page();

    if (g_xhci_runtime.command_ring_physical == 0)
    {
        goto fail;
    }

    g_xhci_runtime.command_ring_virtual =
        (uint8_t *)phys_to_virt(
            g_xhci_runtime.command_ring_physical
        );

    g_xhci_runtime.event_ring_physical =
        pmm_alloc_page();

    if (g_xhci_runtime.event_ring_physical == 0)
    {
        goto fail;
    }

    g_xhci_runtime.event_ring_virtual =
        (uint8_t *)phys_to_virt(
            g_xhci_runtime.event_ring_physical
        );

    g_xhci_runtime.erst_physical =
        pmm_alloc_page();

    if (g_xhci_runtime.erst_physical == 0)
    {
        goto fail;
    }

    g_xhci_runtime.erst_virtual =
        (uint8_t *)phys_to_virt(
            g_xhci_runtime.erst_physical
        );

    g_xhci_runtime.ep0_ring_physical =
        pmm_alloc_page();

    if (g_xhci_runtime.ep0_ring_physical == 0)
    {
        goto fail;
    }

    g_xhci_runtime.ep0_ring_virtual =
        (uint8_t *)phys_to_virt(
            g_xhci_runtime.ep0_ring_physical
        );

    g_xhci_runtime.device_context_physical =
        pmm_alloc_page();

    if (g_xhci_runtime.device_context_physical == 0)
    {
        goto fail;
    }

    g_xhci_runtime.device_context_virtual =
        (uint8_t *)phys_to_virt(
            g_xhci_runtime.device_context_physical
        );

    g_xhci_runtime.input_context_physical =
        pmm_alloc_page();

    if (g_xhci_runtime.input_context_physical == 0)
    {
        goto fail;
    }

    g_xhci_runtime.input_context_virtual =
        (uint8_t *)phys_to_virt(
            g_xhci_runtime.input_context_physical
        );

    g_xhci_runtime.descriptor_physical =
        pmm_alloc_page();

    if (g_xhci_runtime.descriptor_physical == 0)
    {
        goto fail;
    }

    g_xhci_runtime.descriptor_virtual =
        (uint8_t *)phys_to_virt(
            g_xhci_runtime.descriptor_physical
        );

    memset(
        g_xhci_runtime.dcbaa_virtual,
        0,
        PAGE_SIZE
    );

    memset(
        g_xhci_runtime.command_ring_virtual,
        0,
        PAGE_SIZE
    );

    memset(
        g_xhci_runtime.event_ring_virtual,
        0,
        PAGE_SIZE
    );

    memset(
        g_xhci_runtime.erst_virtual,
        0,
        PAGE_SIZE
    );

    memset(
        g_xhci_runtime.ep0_ring_virtual,
        0,
        PAGE_SIZE
    );

    memset(
        g_xhci_runtime.device_context_virtual,
        0,
        PAGE_SIZE
    );

    memset(
        g_xhci_runtime.input_context_virtual,
        0,
        PAGE_SIZE
    );

    memset(
        g_xhci_runtime.descriptor_virtual,
        0,
        PAGE_SIZE
    );

    g_xhci_ep0_ring_state.enqueue_index = 0;
    g_xhci_ep0_ring_state.cycle_state = 1;

    g_xhci_runtime.allocated = true;

    return true;

fail:

    xhci_free_runtime_resources();

    return false;
}

/*
 * ================================================================
 * Command Ring
 * ================================================================
 */

static bool xhci_configure_command_ring(void)
{
    uintptr_t operational_base =
        g_xhci_controller.operational_base;

    g_xhci_command_ring_state.enqueue_index = 0;
    g_xhci_command_ring_state.cycle_state = 1;

    uint64_t command_ring =
        (uint64_t)
        g_xhci_runtime.command_ring_physical;

    if ((command_ring &
         (XHCI_COMMAND_RING_ALIGNMENT - 1)) != 0)
    {
        return false;
    }

    command_ring |= 0x1ULL;

    xhci_write64(
        operational_base +
        XHCI_CRCR,
        command_ring
    );

    return true;
}

/*
 * ================================================================
 * DCBAA
 * ================================================================
 */

static bool xhci_configure_dcbaa(void)
{
    uintptr_t operational_base =
        g_xhci_controller.operational_base;

    uint64_t dcbaa =
        (uint64_t)
        g_xhci_runtime.dcbaa_physical;

    if ((dcbaa &
         (XHCI_DCBAA_ALIGNMENT - 1)) != 0)
    {
        return false;
    }

    xhci_write64(
        operational_base +
        XHCI_DCBAAP,
        dcbaa
    );

    return true;
}

/*
 * ================================================================
 * Event Ring
 * ================================================================
 */

static bool xhci_configure_event_ring(void)
{
    uintptr_t runtime_base =
        g_xhci_controller.runtime_base;

    uintptr_t interrupter =
        runtime_base +
        XHCI_INTR0_BASE;

    g_xhci_event_ring_state.dequeue_index = 0;
    g_xhci_event_ring_state.cycle_state = 1;

    uint64_t event_ring =
        (uint64_t)
        g_xhci_runtime.event_ring_physical;

    uint64_t erst =
        (uint64_t)
        g_xhci_runtime.erst_physical;

    if ((event_ring &
         (XHCI_EVENT_RING_ALIGNMENT - 1)) != 0)
    {
        return false;
    }

    if ((erst &
         (XHCI_ERST_ALIGNMENT - 1)) != 0)
    {
        return false;
    }

    xhci_write64(
        (uintptr_t)
        g_xhci_runtime.erst_virtual,
        event_ring
    );

    xhci_write32(
        (uintptr_t)
        g_xhci_runtime.erst_virtual + 8,
        XHCI_RING_TRBS
    );

    xhci_write32(
        (uintptr_t)
        g_xhci_runtime.erst_virtual + 12,
        0
    );

    xhci_write32(
        interrupter +
        XHCI_ERSTSZ,
        1
    );

    xhci_write64(
        interrupter +
        XHCI_ERSTBA,
        erst
    );

    xhci_write64(
        interrupter +
        XHCI_ERDP,
        event_ring
    );

    uint32_t iman =
        xhci_read32(
            interrupter +
            XHCI_IMAN
        );

    iman &= ~XHCI_IMAN_IE;

    xhci_write32(
        interrupter +
        XHCI_IMAN,
        iman
    );

    return true;
}

/*
 * ================================================================
 * TRB Helpers
 * ================================================================
 */

static uint32_t xhci_trb_type(
    uint32_t control
)
{
    return
        (control >> XHCI_TRB_TYPE_SHIFT) &
        XHCI_TRB_TYPE_MASK;
}

static XKXHCITRB *xhci_command_ring_trb(
    uint16_t index
)
{
    return
        (XKXHCITRB *)
        (
            g_xhci_runtime.command_ring_virtual +
            ((uintptr_t)index * XHCI_TRB_SIZE)
        );
}

static XKXHCITRB *xhci_event_ring_trb(
    uint16_t index
)
{
    return
        (XKXHCITRB *)
        (
            g_xhci_runtime.event_ring_virtual +
            ((uintptr_t)index * XHCI_TRB_SIZE)
        );
}

static XKXHCITRB *xhci_ep0_ring_trb(
    uint16_t index
)
{
    return
        (XKXHCITRB *)
        (
            g_xhci_runtime.ep0_ring_virtual +
            ((uintptr_t)index * XHCI_TRB_SIZE)
        );
}

/*
 * ================================================================
 * Command Submission
 * ================================================================
 */

static bool xhci_submit_command(
    const XKXHCITRB *command
)
{
    if (command == NULL ||
        !g_xhci_controller.controller_running)
    {
        return false;
    }

    if (g_xhci_command_ring_state.enqueue_index >=
        XHCI_RING_TRBS - 1U)
    {
        return false;
    }

    XKXHCITRB *ring_trb =
        xhci_command_ring_trb(
            g_xhci_command_ring_state.enqueue_index
        );

    memset(
        ring_trb,
        0,
        sizeof(XKXHCITRB)
    );

    ring_trb->parameter_low =
        command->parameter_low;

    ring_trb->parameter_high =
        command->parameter_high;

    ring_trb->status =
        command->status;

    ring_trb->control =
        command->control |
        (uint32_t)
        g_xhci_command_ring_state.cycle_state;

    __asm__ volatile(
        "mfence"
        :
        :
        : "memory"
    );

    g_xhci_command_ring_state.enqueue_index++;

    xhci_write32(
        g_xhci_controller.doorbell_base +
        XHCI_DOORBELL_COMMAND,
        0
    );

    return true;
}

/*
 * ================================================================
 * Event Ring
 * ================================================================
 */

static bool xhci_event_ring_has_event(void)
{
    XKXHCITRB *event =
        xhci_event_ring_trb(
            g_xhci_event_ring_state.dequeue_index
        );

    uint32_t control =
        event->control;

    uint8_t cycle =
        (uint8_t)
        (control & XHCI_TRB_CYCLE);

    return
        cycle ==
        g_xhci_event_ring_state.cycle_state;
}

static void xhci_event_ring_advance(void)
{
    g_xhci_event_ring_state.dequeue_index++;

    if (g_xhci_event_ring_state.dequeue_index >=
        XHCI_RING_TRBS)
    {
        g_xhci_event_ring_state.dequeue_index = 0;

        g_xhci_event_ring_state.cycle_state ^= 1U;
    }

    uint64_t dequeue =
        (uint64_t)
        g_xhci_runtime.event_ring_physical +
        (
            (uint64_t)
            g_xhci_event_ring_state.dequeue_index *
            XHCI_TRB_SIZE
        );

    uintptr_t interrupter =
        g_xhci_controller.runtime_base +
        XHCI_INTR0_BASE;

    dequeue |= (1ULL << 3);

    xhci_write64(
        interrupter +
        XHCI_ERDP,
        dequeue
    );
}

/*
 * ================================================================
 * Command Completion
 * ================================================================
 */

static bool xhci_wait_for_command_completion(
    uint8_t *slot_id
)
{
    if (slot_id != NULL)
    {
        *slot_id = 0;
    }

    for (uint32_t timeout = 0;
         timeout < XHCI_COMMAND_RING_SIZE * 64U;
         timeout++)
    {
        if (!xhci_event_ring_has_event())
        {
            continue;
        }

        XKXHCITRB *event =
            xhci_event_ring_trb(
                g_xhci_event_ring_state.dequeue_index
            );

        uint32_t type =
            xhci_trb_type(
                event->control
            );

        if (type != XHCI_TRB_TYPE_COMMAND_COMPLETE)
        {
            xhci_event_ring_advance();
            continue;
        }

        uint8_t completion_code =
            (uint8_t)
            ((event->status >> 24) & 0xFFU);

        uint8_t completed_slot =
            (uint8_t)
            ((event->control >> 24) & 0xFFU);

        xhci_event_ring_advance();

        if (completion_code !=
            XHCI_COMPLETION_SUCCESS)
        {
            debug_print(
                "[ ERROR ] xHCI command completion failed\n"
            );

            return false;
        }

        if (slot_id != NULL)
        {
            *slot_id =
                completed_slot;
        }

        return true;
    }

    return false;
}

/*
 * ================================================================
 * Enable Slot
 * ================================================================
 */

static bool xhci_enable_slot(
    uint8_t *slot_id
)
{
    XKXHCITRB command =
    {
        .parameter_low = 0,
        .parameter_high = 0,
        .status = 0,
        .control =
            (
                XHCI_TRB_TYPE_ENABLE_SLOT
                << XHCI_TRB_TYPE_SHIFT
            )
    };

    if (!xhci_submit_command(
            &command))
    {
        return false;
    }

    return
        xhci_wait_for_command_completion(
            slot_id
        );
}

/*
 * ================================================================
 * Root Port Detection
 * ================================================================
 */

static bool xhci_find_connected_port(
    uint8_t *port_number,
    uint32_t *port_status
)
{
    if (port_number != NULL)
    {
        *port_number = 0;
    }

    if (port_status != NULL)
    {
        *port_status = 0;
    }

    if (g_xhci_controller.operational_base == 0 ||
        g_xhci_controller.max_ports == 0)
    {
        return false;
    }

    for (uint8_t port = 1;
         port <= g_xhci_controller.max_ports;
         port++)
    {
        uintptr_t portsc =
            g_xhci_controller.operational_base +
            XHCI_PORT_REGISTER_BASE +
            (
                (uintptr_t)(port - 1U) *
                XHCI_PORT_REGISTER_STRIDE
            );

        uint32_t status =
            xhci_read32(
                portsc +
                XHCI_PORTSC
            );

        if ((status & XHCI_PORTSC_CCS) == 0)
        {
            continue;
        }

        if (port_number != NULL)
        {
            *port_number = port;
        }

        if (port_status != NULL)
        {
            *port_status = status;
        }

        return true;
    }

    return false;
}

/*
 * ================================================================
 * Root Port Reset
 * ================================================================
 */

static bool xhci_reset_port(
    uint8_t port_number
)
{
    if (g_xhci_controller.operational_base == 0 ||
        port_number == 0 ||
        port_number > g_xhci_controller.max_ports)
    {
        return false;
    }

    uintptr_t portsc =
        g_xhci_controller.operational_base +
        XHCI_PORT_REGISTER_BASE +
        (
            (uintptr_t)(port_number - 1U) *
            XHCI_PORT_REGISTER_STRIDE
        ) +
        XHCI_PORTSC;

    uint32_t status =
        xhci_read32(portsc);

    if ((status & XHCI_PORTSC_CCS) == 0)
    {
        return false;
    }

    /*
     * Clear only the RW1C status bits.
     */
    xhci_write32(
        portsc,
        XHCI_PORTSC_CSC |
        XHCI_PORTSC_PRC
    );

    status =
        xhci_read32(portsc);

    /*
     * Preserve PP and request reset.
     */
    uint32_t reset_value =
        status & XHCI_PORTSC_PP;

    reset_value |=
        XHCI_PORTSC_PR;

    xhci_write32(
        portsc,
        reset_value
    );

    for (uint32_t timeout = 0;
         timeout < XHCI_RESET_TIMEOUT;
         timeout++)
    {
        uint32_t current =
            xhci_read32(portsc);

        if ((current & XHCI_PORTSC_CCS) == 0)
        {
            return false;
        }

        if ((current & XHCI_PORTSC_PR) == 0)
        {
            if ((current & XHCI_PORTSC_PED) != 0)
            {
                return true;
            }

            /*
             * QEMU/device combinations can expose a
             * connected port before PED becomes visible.
             * Treat reset completion itself as useful
             * enough for enumeration.
             */
            return true;
        }
    }

    return false;
}

/*
 * ================================================================
 * Port Speed
 * ================================================================
 */

static uint8_t xhci_port_speed(
    uint8_t port_number
)
{
    uintptr_t portsc =
        g_xhci_controller.operational_base +
        XHCI_PORT_REGISTER_BASE +
        (
            (uintptr_t)(port_number - 1U) *
            XHCI_PORT_REGISTER_STRIDE
        ) +
        XHCI_PORTSC;

    uint32_t status =
        xhci_read32(portsc);

    return
        (uint8_t)
        (
            (status >>
             XHCI_PORTSC_SPEED_SHIFT) &
            XHCI_PORTSC_SPEED_MASK
        );
}

/*
 * ================================================================
 * EP0 Maximum Packet Size
 * ================================================================
 */

static uint16_t xhci_ep0_packet_size(
    uint8_t speed
)
{
    switch (speed)
    {
        case 1:
        case 2:
            return 8;

        case 3:
            return 64;

        case 4:
        case 5:
        case 6:
        case 7:
            return 512;

        default:
            return 8;
    }
}

/*
 * ================================================================
 * Context Helpers
 * ================================================================
 */

static uint8_t *xhci_input_context_slot(
    void
)
{
    return
        g_xhci_runtime.input_context_virtual +
        (
            (uintptr_t)
            XHCI_SLOT_CONTEXT_OFFSET *
            g_xhci_controller.context_size
        );
}

static uint8_t *xhci_input_context_ep0(
    void
)
{
    return
        g_xhci_runtime.input_context_virtual +
        (
            (uintptr_t)
            XHCI_EP0_CONTEXT_OFFSET *
            g_xhci_controller.context_size
        );
}

static uint8_t *xhci_device_context_slot(
    void
)
{
    return
        g_xhci_runtime.device_context_virtual;
}

static uint8_t *xhci_device_context_ep0(
    void
)
{
    return
        g_xhci_runtime.device_context_virtual +
        g_xhci_controller.context_size;
}

static void xhci_write_context_dword(
    uint8_t *context,
    uint32_t dword,
    uint32_t value
)
{
    uint32_t *words =
        (uint32_t *)context;

    words[dword] = value;
}

/*
 * ================================================================
 * Build Address Device Input Context
 * ================================================================
 */

static bool xhci_build_address_device_context(
    uint8_t slot_id,
    uint8_t port_number,
    uint8_t speed
)
{
    if (slot_id == 0U ||
        port_number == 0U ||
        speed == 0U)
    {
        return false;
    }

    /*
     * ------------------------------------------------------------
     * Clear Input and Output Device Contexts
     * ------------------------------------------------------------
     */

    memset(
        g_xhci_runtime.input_context_virtual,
        0,
        PAGE_SIZE
    );

    memset(
        g_xhci_runtime.device_context_virtual,
        0,
        PAGE_SIZE
    );

    /*
     * ------------------------------------------------------------
     * Input Control Context
     *
     * DW0 = Drop Context Flags
     * DW1 = Add Context Flags
     *
     * We are adding:
     *   Context 1 = Slot Context
     *   Context 2 = Endpoint 0 Context
     * ------------------------------------------------------------
     */

    uint8_t *input_control =
        g_xhci_runtime.input_context_virtual;

    uint8_t *slot_context =
        xhci_input_context_slot();

    uint8_t *ep0_context =
        xhci_input_context_ep0();

    xhci_write_context_dword(
        input_control,
        0,
        0U
    );

    xhci_write_context_dword(
        input_control,
        1,
        XHCI_INPUT_CONTROL_ADD_SLOT |
        XHCI_INPUT_CONTROL_ADD_EP0
    );

    /*
     * ------------------------------------------------------------
     * Slot Context
     *
     * DW0:
     *
     * Route String     = 0
     * Speed            = PORTSC negotiated speed
     * Context Entries  = 1
     *
     * EP0 is endpoint context 1, so one context entry is required.
     * ------------------------------------------------------------
     */

    uint32_t slot_dw0 =
        ((uint32_t)(speed & XHCI_SLOT_SPEED_MASK)
         << XHCI_SLOT_SPEED_SHIFT)
        |
        (1U << XHCI_SLOT_CONTEXT_ENTRIES_SHIFT);

    xhci_write_context_dword(
        slot_context,
        0,
        slot_dw0
    );

    /*
     * Slot Context DW1:
     *
     * Root Hub Port Number
     */

    uint32_t slot_dw1 =
        ((uint32_t)port_number
         << XHCI_SLOT_ROOT_HUB_PORT_SHIFT);

    xhci_write_context_dword(
        slot_context,
        1,
        slot_dw1
    );

    /*
     * ------------------------------------------------------------
     * Endpoint 0 Context
     * ------------------------------------------------------------
     *
     * IMPORTANT:
     *
     * CErr and Endpoint Type belong in DW1.
     * DCS belongs in the Transfer Ring Dequeue Pointer
     * in DW2 bit 0.
     *
     * The previous implementation placed CErr/DCS in DW0.
     * That produced an invalid EP0 context.
     */

    /*
     * EP0 Context DW0
     *
     * Leave the endpoint state and other fields at zero for
     * Address Device.
     */

    xhci_write_context_dword(
        ep0_context,
        0,
        0U
    );

    /*
     * EP0 Context DW1
     *
     * CErr          = 3
     * Endpoint Type = Control (4)
     * Max Burst     = 0
     * Max Packet    = speed-dependent
     *
     * CErr occupies bits 2:1.
     * Endpoint Type occupies bits 5:3.
     * Max Packet Size occupies bits 31:16.
     */

    uint32_t ep0_dw1 =
        (XHCI_EP_CERR_VALUE << XHCI_EP_CERR_SHIFT)
        |
        (XHCI_EP_TYPE_CONTROL << XHCI_EP_TYPE_SHIFT)
        |
        (
            (uint32_t)
            g_xhci_runtime.ep0_max_packet
            << XHCI_EP_MAX_PACKET_SHIFT
        );

    xhci_write_context_dword(
        ep0_context,
        1,
        ep0_dw1
    );

    /*
     * ------------------------------------------------------------
     * EP0 Transfer Ring Dequeue Pointer
     * ------------------------------------------------------------
     *
     * DW2 = low 32 bits
     * DW3 = high 32 bits
     *
     * DCS = bit 0 of the dequeue pointer.
     */

    uint64_t ep0_ring =
        (uint64_t)
        g_xhci_runtime.ep0_ring_physical;

    ep0_ring |=
        XHCI_EP_DCS;

    xhci_write_context_dword(
        ep0_context,
        2,
        (uint32_t)
        (ep0_ring & 0xFFFFFFFFULL)
    );

    xhci_write_context_dword(
        ep0_context,
        3,
        (uint32_t)
        (ep0_ring >> 32)
    );

    /*
     * ------------------------------------------------------------
     * EP0 Average TRB Length
     * ------------------------------------------------------------
     */

    xhci_write_context_dword(
        ep0_context,
        4,
        8U
    );

    /*
     * Ensure all Input Context writes reach memory before the
     * Address Device command is submitted.
     */

    __asm__ volatile(
        "mfence"
        :
        :
        : "memory"
    );

    /*
     * slot_id is used by the caller to identify the device.
     * The slot context itself does not contain the slot ID.
     */

    (void)slot_id;

    return true;
}

/*
 * ================================================================
 * Address Device
 * ================================================================
 */

static bool xhci_address_device(
    uint8_t slot_id,
    phys_addr_t input_context_physical
)
{
    XKXHCITRB command;
    uint64_t input_context_address;

    if (slot_id == 0U)
    {
        debug_print("[ERROR] xHCI Address Device: invalid slot ID\n");
        return false;
    }

    if (input_context_physical == 0U)
    {
        debug_print("[ERROR] xHCI Address Device: invalid Input Context\n");
        return false;
    }

    /*
     * Address Device Command TRB
     *
     * Parameter:
     *   bits 63:0 = Input Context physical address
     *
     * Control:
     *   bits 15:10 = TRB Type (11 = Address Device)
     *   bits 31:24 = Slot ID
     *   bit 0      = Cycle
     */

    input_context_address = (uint64_t)input_context_physical;

    command.parameter_low =
        (uint32_t)(input_context_address & 0xFFFFFFFFULL);

    command.parameter_high =
        (uint32_t)((input_context_address >> 32) & 0xFFFFFFFFULL);

    command.status = 0U;

    command.control =
        XHCI_TRB_CYCLE |
        (XHCI_TRB_TYPE_ADDRESS_DEVICE << XHCI_TRB_TYPE_SHIFT) |
        ((uint32_t)slot_id << 24);

    debug_print("[INFO] xHCI: submitting Address Device command\n");

    if (!xhci_submit_command(&command))
    {
        debug_print("[ERROR] xHCI: failed to submit Address Device\n");
        return false;
    }

    /*
     * Wait for the Command Completion Event.
     */
    uint8_t completion_slot = 0U;

if (!xhci_wait_for_command_completion(
        &completion_slot))
{
    debug_print(
        "[ERROR] xHCI: Address Device completion timeout\n"
    );

    return false;
}

if (completion_slot != slot_id)
{
    debug_print(
        "[ERROR] xHCI: Address Device returned wrong slot\n"
    );

    return false;
}

    debug_print("[ OK ] xHCI USB Device Addressed\n");

    /*
     * Use the Device Context helpers so they are also validated
     * immediately after Address Device. This also prevents them
     * from becoming unused-function errors under -Werror.
     */
    uint8_t *slot_context =
    xhci_device_context_slot();

    uint8_t *ep0_context =
    xhci_device_context_ep0();

    if (slot_context == NULL || ep0_context == NULL)
    {
        debug_print("[ERROR] xHCI: Device Context validation failed\n");
        return false;
    }

    /*
     * The controller should now have populated the Output Device
     * Context. Check that the Slot Context is no longer empty.
     */
    uint32_t *slot_dw =
        (uint32_t *)(void *)slot_context;

    uint32_t *ep0_dw =
        (uint32_t *)(void *)ep0_context;

    if (slot_dw[0] == 0U)
    {
        debug_print("[ERROR] xHCI: Output Slot Context is invalid\n");
        return false;
    }

    if (ep0_dw[1] == 0U)
    {
        debug_print("[ERROR] xHCI: Output EP0 Context is invalid\n");
        return false;
    }

    debug_print("[ OK ] xHCI Device Context Activated\n");

    return true;
}

/*
 * ================================================================
 * EP0 Transfer Ring
 * ================================================================
 */

static void xhci_reset_ep0_ring(void)
{
    memset(
        g_xhci_runtime.ep0_ring_virtual,
        0,
        PAGE_SIZE
    );

    g_xhci_ep0_ring_state.enqueue_index = 0;
    g_xhci_ep0_ring_state.cycle_state = 1;
}

static bool xhci_ep0_submit_trb(
    const XKXHCITRB *source
)
{
    if (source == NULL)
    {
        return false;
    }

    if (g_xhci_ep0_ring_state.enqueue_index >=
        XHCI_RING_TRBS - 1U)
    {
        return false;
    }

    XKXHCITRB *target =
        xhci_ep0_ring_trb(
            g_xhci_ep0_ring_state.enqueue_index
        );

    memset(
        target,
        0,
        sizeof(XKXHCITRB)
    );

    target->parameter_low =
        source->parameter_low;

    target->parameter_high =
        source->parameter_high;

    target->status =
        source->status;

    target->control =
        source->control |
        (uint32_t)
        g_xhci_ep0_ring_state.cycle_state;

    g_xhci_ep0_ring_state.enqueue_index++;

    return true;
}

/*
 * ================================================================
 * Transfer Event Waiting
 * ================================================================
 */

static bool xhci_wait_for_transfer_event(
    uint8_t slot_id
)
{
    for (uint32_t timeout = 0;
         timeout < XHCI_TRANSFER_TIMEOUT;
         timeout++)
    {
        if (!xhci_event_ring_has_event())
        {
            continue;
        }

        XKXHCITRB *event =
            xhci_event_ring_trb(
                g_xhci_event_ring_state.dequeue_index
            );

        uint32_t type =
            xhci_trb_type(
                event->control
            );

        if (type != XHCI_TRB_TYPE_TRANSFER_EVENT)
        {
            xhci_event_ring_advance();
            continue;
        }

        uint8_t completion_code =
            (uint8_t)
            ((event->status >> 24) & 0xFFU);

        uint8_t event_slot =
            (uint8_t)
            ((event->control >> 24) & 0xFFU);

        xhci_event_ring_advance();

        if (event_slot != slot_id)
        {
            continue;
        }

        if (completion_code !=
            XHCI_COMPLETION_SUCCESS)
        {
            debug_print(
                "[ ERROR ] xHCI transfer failed\n"
            );

            return false;
        }

        return true;
    }

    debug_print(
        "[ ERROR ] xHCI transfer event timeout\n"
    );

    return false;
}

/*
 * ================================================================
 * EP0 Doorbell
 * ================================================================
 */

static void xhci_ring_ep0_doorbell(
    uint8_t slot_id
)
{
    uintptr_t doorbell =
        g_xhci_controller.doorbell_base +
        ((uintptr_t)slot_id * 4U);

    /*
     * Doorbell target 1 selects Endpoint 0.
     */
    xhci_write32(
        doorbell,
        1
    );
}

/*
 * ================================================================
 * USB SETUP Packet Builder
 * ================================================================
 */

static uint64_t xhci_setup_packet(
    uint8_t request_type,
    uint8_t request,
    uint16_t value,
    uint16_t index,
    uint16_t length
)
{
    uint64_t setup = 0;

    setup |=
        (uint64_t)request_type;

    setup |=
        (uint64_t)request << 8;

    setup |=
        (uint64_t)value << 16;

    setup |=
        (uint64_t)index << 32;

    setup |=
        (uint64_t)length << 48;

    return setup;
}

/*
 * ================================================================
 * GET_DESCRIPTOR
 * ================================================================
 */

static bool xhci_get_device_descriptor(
    uint8_t slot_id
)
{
    xhci_reset_ep0_ring();

    if (slot_id == 0)
    {
        return false;
    }

    memset(
        g_xhci_runtime.descriptor_virtual,
        0,
        PAGE_SIZE
    );

    /*
     * ------------------------------------------------------------
     * SETUP STAGE
     * ------------------------------------------------------------
     */

    uint64_t setup =
        xhci_setup_packet(
            USB_REQTYPE_STANDARD_IN,
            USB_REQ_GET_DESCRIPTOR,
            (uint16_t)
            (USB_DESC_DEVICE << 8),
            0,
            USB_DEVICE_DESCRIPTOR_LENGTH
        );

    XKXHCITRB setup_trb =
    {
        .parameter_low =
            (uint32_t)
            (setup & 0xFFFFFFFFULL),

        .parameter_high =
            (uint32_t)
            (setup >> 32),

        .status = 8,

        .control =
            (
                XHCI_TRB_TYPE_SETUP_STAGE
                << XHCI_TRB_TYPE_SHIFT
            )
            |
            XHCI_TRB_IDT
            |
            (
                XHCI_SETUP_TRT_IN
                << XHCI_SETUP_TRT_SHIFT
            )
    };

    if (!xhci_ep0_submit_trb(
            &setup_trb))
    {
        return false;
    }

    /*
     * ------------------------------------------------------------
     * DATA STAGE
     * ------------------------------------------------------------
     */

    uint64_t descriptor =
        (uint64_t)
        g_xhci_runtime.descriptor_physical;

    XKXHCITRB data_trb =
    {
        .parameter_low =
            (uint32_t)
            (descriptor & 0xFFFFFFFFULL),

        .parameter_high =
            (uint32_t)
            (descriptor >> 32),

        .status =
            USB_DEVICE_DESCRIPTOR_LENGTH,

        .control =
            (
                XHCI_TRB_TYPE_DATA_STAGE
                << XHCI_TRB_TYPE_SHIFT
            )
            |
            XHCI_TRB_TRANSFER_DIR_IN
    };

    if (!xhci_ep0_submit_trb(
            &data_trb))
    {
        return false;
    }

    /*
     * ------------------------------------------------------------
     * STATUS STAGE
     * ------------------------------------------------------------
     *
     * Data direction was IN, therefore status is OUT.
     */

    XKXHCITRB status_trb =
    {
        .parameter_low = 0,
        .parameter_high = 0,
        .status = 0,

        .control =
            (
                XHCI_TRB_TYPE_STATUS_STAGE
                << XHCI_TRB_TYPE_SHIFT
            )
            |
            XHCI_TRB_IOC
    };

    if (!xhci_ep0_submit_trb(
            &status_trb))
    {
        return false;
    }

    __asm__ volatile(
        "mfence"
        :
        :
        : "memory"
    );

    xhci_ring_ep0_doorbell(
        slot_id
    );

    if (!xhci_wait_for_transfer_event(
            slot_id))
    {
        return false;
    }

    /*
     * Validate the descriptor.
     */
    uint8_t length =
        g_xhci_runtime.descriptor_virtual[0];

    uint8_t type =
        g_xhci_runtime.descriptor_virtual[1];

    if (length < USB_DEVICE_DESCRIPTOR_LENGTH ||
        type != USB_DESC_DEVICE)
    {
        debug_print(
            "[ ERROR ] Invalid USB device descriptor\n"
        );

        return false;
    }

    debug_print(
        "[ OK ] USB Device Descriptor Retrieved\n"
    );

    /*
     * Print the most useful identification fields.
     *
     * USB device descriptor offsets:
     *
     * 8  = idVendor
     * 10 = idProduct
     */
    uint16_t vendor_id =
        (uint16_t)
        g_xhci_runtime.descriptor_virtual[8]
        |
        (
            (uint16_t)
            g_xhci_runtime.descriptor_virtual[9]
            << 8
        );

    uint16_t product_id =
        (uint16_t)
        g_xhci_runtime.descriptor_virtual[10]
        |
        (
            (uint16_t)
            g_xhci_runtime.descriptor_virtual[11]
            << 8
        );

    debug_print(
        "[ OK ] USB VID/PID descriptor available\n"
    );

    (void)vendor_id;
    (void)product_id;

    return true;
}

    /*
 * ================================================================
 * GET_CONFIGURATION_DESCRIPTOR
 * ================================================================
 */

/*
 * ================================================================
 * GET_CONFIGURATION_DESCRIPTOR
 * ================================================================
 */

static bool xhci_get_configuration_descriptor(
    uint8_t slot_id
)
{
    if (slot_id == 0)
    {
        return false;
    }

    /*
     * ------------------------------------------------------------
     * First request:
     *
     * Retrieve only the 9-byte configuration descriptor header.
     * This tells us the total descriptor size.
     *
     * IMPORTANT:
     *
     * Do NOT reset the EP0 transfer ring here.
     * The previous GET_DESCRIPTOR(Device) transfer has already
     * advanced the hardware EP0 ring.
     * The configuration request must continue from that position.
     * ------------------------------------------------------------
     */

    memset(
        g_xhci_runtime.descriptor_virtual,
        0,
        PAGE_SIZE
    );

    uint64_t setup =
        xhci_setup_packet(
            USB_REQTYPE_STANDARD_IN,
            USB_REQ_GET_DESCRIPTOR,
            (uint16_t)
            (USB_DESC_CONFIGURATION << 8),
            0,
            USB_CONFIGURATION_DESCRIPTOR_LENGTH
        );

    XKXHCITRB setup_trb =
    {
        .parameter_low =
            (uint32_t)
            (setup & 0xFFFFFFFFULL),

        .parameter_high =
            (uint32_t)
            (setup >> 32),

        .status = 8,

        .control =
            (
                XHCI_TRB_TYPE_SETUP_STAGE
                << XHCI_TRB_TYPE_SHIFT
            )
            |
            XHCI_TRB_IDT
            |
            (
                XHCI_SETUP_TRT_IN
                << XHCI_SETUP_TRT_SHIFT
            )
    };

    if (!xhci_ep0_submit_trb(
            &setup_trb))
    {
        debug_print(
            "[ ERROR ] USB Config: Setup TRB failed\n"
        );

        return false;
    }

    uint64_t descriptor =
        (uint64_t)
        g_xhci_runtime.descriptor_physical;

    XKXHCITRB data_trb =
    {
        .parameter_low =
            (uint32_t)
            (descriptor & 0xFFFFFFFFULL),

        .parameter_high =
            (uint32_t)
            (descriptor >> 32),

        .status =
            USB_CONFIGURATION_DESCRIPTOR_LENGTH,

        .control =
            (
                XHCI_TRB_TYPE_DATA_STAGE
                << XHCI_TRB_TYPE_SHIFT
            )
            |
            XHCI_TRB_TRANSFER_DIR_IN
    };

    if (!xhci_ep0_submit_trb(
            &data_trb))
    {
        debug_print(
            "[ ERROR ] USB Config: Data TRB failed\n"
        );

        return false;
    }

    XKXHCITRB status_trb =
    {
        .parameter_low = 0,
        .parameter_high = 0,
        .status = 0,

        .control =
            (
                XHCI_TRB_TYPE_STATUS_STAGE
                << XHCI_TRB_TYPE_SHIFT
            )
            |
            XHCI_TRB_IOC
    };

    if (!xhci_ep0_submit_trb(
            &status_trb))
    {
        debug_print(
            "[ ERROR ] USB Config: Status TRB failed\n"
        );

        return false;
    }

    __asm__ volatile(
        "mfence"
        :
        :
        : "memory"
    );

    xhci_ring_ep0_doorbell(
        slot_id
    );

    if (!xhci_wait_for_transfer_event(
            slot_id))
    {
        return false;
    }

    /*
     * ------------------------------------------------------------
     * Validate the configuration descriptor header.
     * ------------------------------------------------------------
     */

    uint8_t length =
        g_xhci_runtime.descriptor_virtual[0];

    uint8_t type =
        g_xhci_runtime.descriptor_virtual[1];

    if (length < USB_CONFIGURATION_DESCRIPTOR_LENGTH ||
        type != USB_DESC_CONFIGURATION)
    {
        debug_print(
            "[ ERROR ] Invalid USB configuration descriptor\n"
        );

        return false;
    }

    uint16_t total_length =
        (uint16_t)
        g_xhci_runtime.descriptor_virtual[2]
        |
        (
            (uint16_t)
            g_xhci_runtime.descriptor_virtual[3]
            << 8
        );

    if (total_length < USB_CONFIGURATION_DESCRIPTOR_LENGTH ||
        total_length > PAGE_SIZE)
    {
        debug_print(
            "[ ERROR ] Invalid USB configuration descriptor length\n"
        );

        return false;
    }

    /*
     * ------------------------------------------------------------
     * Second request:
     *
     * Retrieve the complete configuration descriptor tree.
     *
     * IMPORTANT:
     *
     * Do NOT reset the EP0 transfer ring here.
     * The first configuration request has already advanced the
     * hardware and software ring state.
     * ------------------------------------------------------------
     */

    memset(
        g_xhci_runtime.descriptor_virtual,
        0,
        PAGE_SIZE
    );

    setup =
        xhci_setup_packet(
            USB_REQTYPE_STANDARD_IN,
            USB_REQ_GET_DESCRIPTOR,
            (uint16_t)
            (USB_DESC_CONFIGURATION << 8),
            0,
            total_length
        );

    setup_trb =
    (XKXHCITRB)
    {
        .parameter_low =
            (uint32_t)
            (setup & 0xFFFFFFFFULL),

        .parameter_high =
            (uint32_t)
            (setup >> 32),

        .status = 8,

        .control =
            (
                XHCI_TRB_TYPE_SETUP_STAGE
                << XHCI_TRB_TYPE_SHIFT
            )
            |
            XHCI_TRB_IDT
            |
            (
                XHCI_SETUP_TRT_IN
                << XHCI_SETUP_TRT_SHIFT
            )
    };

    if (!xhci_ep0_submit_trb(
            &setup_trb))
    {
        debug_print(
            "[ ERROR ] USB Config: Full Setup TRB failed\n"
        );

        return false;
    }

    descriptor =
        (uint64_t)
        g_xhci_runtime.descriptor_physical;

    data_trb =
    (XKXHCITRB)
    {
        .parameter_low =
            (uint32_t)
            (descriptor & 0xFFFFFFFFULL),

        .parameter_high =
            (uint32_t)
            (descriptor >> 32),

        .status = total_length,

        .control =
            (
                XHCI_TRB_TYPE_DATA_STAGE
                << XHCI_TRB_TYPE_SHIFT
            )
            |
            XHCI_TRB_TRANSFER_DIR_IN
    };

    if (!xhci_ep0_submit_trb(
            &data_trb))
    {
        debug_print(
            "[ ERROR ] USB Config: Full Data TRB failed\n"
        );

        return false;
    }

    status_trb =
    (XKXHCITRB)
    {
        .parameter_low = 0,
        .parameter_high = 0,
        .status = 0,

        .control =
            (
                XHCI_TRB_TYPE_STATUS_STAGE
                << XHCI_TRB_TYPE_SHIFT
            )
            |
            XHCI_TRB_IOC
    };

    if (!xhci_ep0_submit_trb(
            &status_trb))
    {
        debug_print(
            "[ ERROR ] USB Config: Full Status TRB failed\n"
        );

        return false;
    }

    __asm__ volatile(
        "mfence"
        :
        :
        : "memory"
    );

    xhci_ring_ep0_doorbell(
        slot_id
    );

    if (!xhci_wait_for_transfer_event(
            slot_id))
    {
        return false;
    }

    /*
     * ------------------------------------------------------------
     * Validate the complete descriptor tree.
     * ------------------------------------------------------------
     */

    if (g_xhci_runtime.descriptor_virtual[0] <
            USB_CONFIGURATION_DESCRIPTOR_LENGTH ||
        g_xhci_runtime.descriptor_virtual[1] !=
            USB_DESC_CONFIGURATION)
    {
        debug_print(
            "[ ERROR ] Invalid USB configuration tree\n"
        );

        return false;
    }

    debug_print(
        "[ OK ] USB Configuration Descriptor Retrieved\n"
    );

    return true;
}


/*
 * ================================================================
 * Controller Runtime Configuration
 * ================================================================
 */

static bool xhci_configure_controller(void)
{
    uintptr_t operational_base =
        g_xhci_controller.operational_base;

    if (!xhci_configure_dcbaa())
    {
        return false;
    }

    if (!xhci_configure_command_ring())
    {
        return false;
    }

    if (!xhci_configure_event_ring())
    {
        return false;
    }

    uint32_t config =
        xhci_read32(
            operational_base +
            XHCI_CONFIG
        );

    config &= ~0xFFU;

    config |=
        g_xhci_controller.max_slots;

    xhci_write32(
        operational_base +
        XHCI_CONFIG,
        config
    );

    g_xhci_controller.runtime_ready = 1;

    return true;
}

/*
 * ================================================================
 * Controller Start
 * ================================================================
 */

static bool xhci_start_controller(void)
{
    uintptr_t operational_base =
        g_xhci_controller.operational_base;

    if (!g_xhci_controller.runtime_ready)
    {
        return false;
    }

    uint32_t command =
        xhci_read32(
            operational_base +
            XHCI_USBCMD
        );

    command |=
        XHCI_USBCMD_RUN_STOP;

    xhci_write32(
        operational_base +
        XHCI_USBCMD,
        command
    );

    for (uint32_t i = 0;
         i < XHCI_RESET_TIMEOUT;
         i++)
    {
        uint32_t status =
            xhci_read32(
                operational_base +
                XHCI_USBSTS
            );

        if ((status &
             XHCI_USBSTS_HCHALTED) == 0)
        {
            g_xhci_controller.controller_running =
                1;

            return true;
        }
    }

    return false;
}

/*
 * ================================================================
 * xHCI Initialization
 * ================================================================
 */

bool xk_xhci_initialize(void)
{
    g_xhci_present = false;

    g_xhci_controller =
        (XKXHCIController){0};

    g_xhci_runtime =
        (XKXHCIRuntimeResources){0};

    g_xhci_command_ring_state =
        (XKXHCICommandRingState){0};

    g_xhci_event_ring_state =
        (XKXHCIEventRingState){0};

    g_xhci_ep0_ring_state =
        (XKXHCITransferRingState){0};

    uint32_t device_count =
        xk_pci_device_count();

    for (uint32_t i = 0;
         i < device_count;
         i++)
    {
        const XKPCIDevice *device =
            xk_pci_device_get(i);

        if (device == NULL)
        {
            continue;
        }

        if (device->class_code !=
                XK_XHCI_CLASS_CODE ||
            device->subclass !=
                XK_XHCI_SUBCLASS ||
            device->prog_if !=
                XK_XHCI_PROG_IF)
        {
            continue;
        }

        uintptr_t physical_bar =
            xhci_bar_address(device);

        if (physical_bar == 0)
        {
            continue;
        }

        xk_pci_enable_bus_master(
            device->bus,
            device->device,
            device->function
        );

        /*
         * Map the xHCI MMIO region.
         */
        if (!hhdm_map_mmio(
                physical_bar,
                0x10000))
        {
            return false;
        }

        uintptr_t mmio_base =
            hhdm_offset() +
            physical_bar;

        uint8_t cap_length =
            xhci_read8(
                mmio_base +
                XHCI_CAPLENGTH
            );

        uint16_t hc_version =
            xhci_read16(
                mmio_base +
                XHCI_HCIVERSION
            );

        uint32_t hcsp1 =
            xhci_read32(
                mmio_base +
                XHCI_HCSPARAMS1
            );

        uint32_t hccparams1 =
            xhci_read32(
                mmio_base +
                XHCI_HCCPARAMS1
            );

        uint32_t dboff =
            xhci_read32(
                mmio_base +
                XHCI_DBOFF
            );

        uint32_t rtsoff =
            xhci_read32(
                mmio_base +
                XHCI_RTSOFF
            );

        uint8_t max_slots =
            (uint8_t)
            (hcsp1 & 0xFFU);

        uint8_t max_ports =
            (uint8_t)
            ((hcsp1 >> 24) & 0xFFU);

        uint8_t context_size =
            (hccparams1 &
             XHCI_HCCPARAMS1_CSZ)
                ? 64
                : 32;

        uintptr_t operational_base =
            mmio_base +
            cap_length;

        uintptr_t runtime_base =
            mmio_base +
            (rtsoff & ~0x1FU);

        uintptr_t doorbell_base =
            mmio_base +
            (dboff & ~0x3U);

        if (!xhci_controller_reset(
                operational_base))
        {
            return false;
        }

        uint32_t page_size_register =
            xhci_read32(
                operational_base +
                XHCI_PAGESIZE
            );

        uint32_t page_size = 0;

        if ((page_size_register & 0x1U) != 0)
        {
            page_size = PAGE_SIZE;
        }
        else
        {
            return false;
        }

        g_xhci_controller.vendor_id =
            device->vendor_id;

        g_xhci_controller.device_id =
            device->device_id;

        g_xhci_controller.bus =
            device->bus;

        g_xhci_controller.device =
            device->device;

        g_xhci_controller.function =
            device->function;

        g_xhci_controller.mmio_base =
            mmio_base;

        g_xhci_controller.operational_base =
            operational_base;

        g_xhci_controller.runtime_base =
            runtime_base;

        g_xhci_controller.doorbell_base =
            doorbell_base;

        g_xhci_controller.revision =
            (uint8_t)
            (hc_version >> 8);

        g_xhci_controller.cap_length =
            cap_length;

        g_xhci_controller.max_slots =
            max_slots;

        g_xhci_controller.max_ports =
            max_ports;

        g_xhci_controller.context_size =
            context_size;

        g_xhci_controller.page_size =
            page_size;

        g_xhci_controller.max_interrupters =
            (uint8_t)
            ((hcsp1 >> 16) & 0x7FFU);

        if (max_slots == 0 ||
            max_ports == 0)
        {
            return false;
        }

        if (!xhci_allocate_runtime_resources())
        {
            return false;
        }

        if (!xhci_configure_controller())
        {
            xhci_free_runtime_resources();
            return false;
        }

        if (!xhci_start_controller())
        {
            xhci_free_runtime_resources();
            return false;
        }

        debug_print(
            "[ OK ] xHCI USB Controller Initialized\n"
        );

        /*
         * --------------------------------------------------------
         * Enable Slot
         * --------------------------------------------------------
         */

        uint8_t slot_id = 0;

        if (!xhci_enable_slot(
                &slot_id))
        {
            debug_print(
                "[ ERROR ] xHCI Enable Slot failed\n"
            );

            xhci_free_runtime_resources();
            return false;
        }

        if (slot_id == 0 ||
            slot_id > g_xhci_controller.max_slots)
        {
            debug_print(
                "[ ERROR ] xHCI returned invalid slot ID\n"
            );

            xhci_free_runtime_resources();
            return false;
        }

        g_xhci_runtime.active_slot_id =
            slot_id;

        /*
         * --------------------------------------------------------
         * Detect USB root port
         * --------------------------------------------------------
         */

        uint8_t usb_port = 0;
        uint32_t usb_port_status = 0;

        if (!xhci_find_connected_port(
                &usb_port,
                &usb_port_status))
        {
            debug_print(
                "[ INFO ] No USB Device Connected\n"
            );

            /*
             * Controller itself is still initialized.
             */
            g_xhci_controller.initialized = 1;
            g_xhci_controller.controller_ready = 1;
            g_xhci_present = true;

            return true;
        }

        debug_print(
            "[ OK ] USB Device Connected\n"
        );

        /*
         * --------------------------------------------------------
         * Root-port reset
         * --------------------------------------------------------
         */

        if (!xhci_reset_port(
                usb_port))
        {
            debug_print(
                "[ ERROR ] USB Root Port Reset Failed\n"
            );

            xhci_free_runtime_resources();
            return false;
        }

        debug_print(
            "[ OK ] USB Root Port Reset Complete\n"
        );

        /*
         * Read the negotiated device speed after reset.
         */
        uint8_t speed =
            xhci_port_speed(
                usb_port
            );

        if (speed == 0)
        {
            debug_print(
                "[ ERROR ] xHCI could not determine USB speed\n"
            );

            xhci_free_runtime_resources();
            return false;
        }

        g_xhci_runtime.active_port =
            usb_port;

        g_xhci_runtime.active_speed =
            speed;

        g_xhci_runtime.ep0_max_packet =
            xhci_ep0_packet_size(
                speed
            );

        /*
         * --------------------------------------------------------
         * DCBAA[slot] = Device Context
         * --------------------------------------------------------
         */

        g_xhci_runtime.dcbaa_virtual[slot_id] =
            (uint64_t)
            g_xhci_runtime.device_context_physical;

        __asm__ volatile(
            "mfence"
            :
            :
            : "memory"
        );

        debug_print(
            "[ OK ] xHCI Device Context Assigned\n"
        );

        /*
         * --------------------------------------------------------
         * Build Slot + EP0 Input Context
         * --------------------------------------------------------
         */

        if (!xhci_build_address_device_context(
                slot_id,
                usb_port,
                speed))
        {
            debug_print(
                "[ ERROR ] xHCI Context Build Failed\n"
            );

            xhci_free_runtime_resources();
            return false;
        }

        debug_print(
            "[ OK ] xHCI Slot/EP0 Context Built\n"
        );

        /*
         * --------------------------------------------------------
         * Address Device
         * --------------------------------------------------------
         */

        if (!xhci_address_device(
        slot_id,
        g_xhci_runtime.input_context_physical))
        {
            debug_print(
                "[ ERROR ] xHCI Address Device Failed\n"
            );

            xhci_free_runtime_resources();
            return false;
        }

        /*
         * --------------------------------------------------------
         * Device Descriptor
         * --------------------------------------------------------
         */

        if (!xhci_get_device_descriptor(
        slot_id))
{
    debug_print(
        "[ ERROR ] USB Descriptor Enumeration Failed\n"
    );

    xhci_free_runtime_resources();
    return false;
}

if (!xhci_get_configuration_descriptor(
        slot_id))
{
    debug_print(
        "[ ERROR ] USB Configuration Descriptor Enumeration Failed\n"
    );

    xhci_free_runtime_resources();
    return false;
}

debug_print(
    "[ OK ] USB Device Enumeration Complete\n"
);

        /*
         * Controller initialization is complete.
         */
        g_xhci_controller.initialized = 1;
        g_xhci_controller.controller_ready = 1;

        g_xhci_present = true;

        (void)usb_port_status;

        return true;
    }

    return false;
}

/*
 * ================================================================
 * Shutdown
 * ================================================================
 */

void xk_xhci_shutdown(void)
{
    if (g_xhci_controller.operational_base != 0)
    {
        uint32_t command =
            xhci_read32(
                g_xhci_controller.operational_base +
                XHCI_USBCMD
            );

        command &=
            ~XHCI_USBCMD_RUN_STOP;

        xhci_write32(
            g_xhci_controller.operational_base +
            XHCI_USBCMD,
            command
        );

        xhci_wait_halted(
            g_xhci_controller.operational_base
        );
    }

    xhci_free_runtime_resources();

    g_xhci_present = false;

    g_xhci_controller =
        (XKXHCIController){0};

    g_xhci_command_ring_state =
        (XKXHCICommandRingState){0};

    g_xhci_event_ring_state =
        (XKXHCIEventRingState){0};

    g_xhci_ep0_ring_state =
        (XKXHCITransferRingState){0};
}

/*
 * ================================================================
 * Public State
 * ================================================================
 */

bool xk_xhci_is_present(void)
{
    return g_xhci_present;
}

XKXHCIController xk_xhci_controller_info(void)
{
    return g_xhci_controller;
}

/*
 * ================================================================
 * Driver Registration
 * ================================================================
 */

XKDriver xk_xhci_driver =
{
    .name =
        "xHCI USB Controller",

    .type =
        XK_DRIVER_CUSTOM,

    .state =
        XK_DRIVER_UNINITIALIZED,

    .initialize =
        xk_xhci_initialize,

    .shutdown =
        xk_xhci_shutdown
};