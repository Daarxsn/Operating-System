#include "drivers/ahci.h"

#include <stddef.h>
#include <string.h>

#include "drivers/pci.h"
#include "drivers/block.h"
#include "memory/hhdm.h"
#include "memory/pmm.h"
#include "include/compiler.h"


/*
 * AHCI HBA registers.
 */
#define AHCI_CAP                    0x00
#define AHCI_GHC                    0x04
#define AHCI_PI                     0x0C


/*
 * AHCI port registers.
 */
#define AHCI_PORT_BASE              0x100
#define AHCI_PORT_SIZE              0x80

#define AHCI_PORT_CLB               0x00
#define AHCI_PORT_CLBU              0x04
#define AHCI_PORT_FB                0x08
#define AHCI_PORT_FBU               0x0C
#define AHCI_PORT_CMD               0x18
#define AHCI_PORT_TFD               0x20
#define AHCI_PORT_SIG               0x24
#define AHCI_PORT_SSTS              0x28
#define AHCI_PORT_SERR              0x30
#define AHCI_PORT_CI                0x38


/*
 * Port status/control bits.
 */
#define AHCI_PORT_DET_MASK          0x0F

#define AHCI_PORT_CMD_ST            (1U << 0)
#define AHCI_PORT_CMD_FRE           (1U << 4)
#define AHCI_PORT_CMD_FR            (1U << 14)
#define AHCI_PORT_CMD_CR            (1U << 15)

#define AHCI_TFD_BSY                (1U << 7)
#define AHCI_TFD_DRQ                (1U << 3)
#define AHCI_TFD_ERR                (1U << 0)


/*
 * Device signatures.
 */
#define AHCI_SIG_ATA                0x00000101U


/*
 * SATA / FIS commands.
 */
#define AHCI_FIS_TYPE_REG_H2D       0x27
#define AHCI_FIS_CMD_IDENTIFY       0xEC
#define AHCI_FIS_CMD_READ_DMA_EXT   0x25


/*
 * AHCI command structures.
 */
#define AHCI_COMMAND_FIS_LENGTH     64
#define AHCI_PRDT_ENTRY_COUNT       1
#define AHCI_COMMAND_LIST_SIZE      1024
#define AHCI_COMMAND_TABLE_SIZE     256
#define AHCI_IDENTIFY_SIZE          512


/*
 * Polling timeouts.
 *
 * These are software-loop limits because the early kernel
 * does not yet have a suitable millisecond timer abstraction.
 */
#define AHCI_COMMAND_TIMEOUT        10000000U
#define AHCI_ENGINE_TIMEOUT         10000000U


/*
 * IDENTIFY DEVICE words.
 */
#define ATA_IDENTIFY_WORD_48BIT_LBA     83

#define ATA_IDENTIFY_WORD_LBA28_LOW     60
#define ATA_IDENTIFY_WORD_LBA28_HIGH    61

#define ATA_IDENTIFY_WORD_LBA48_LOW     100
#define ATA_IDENTIFY_WORD_LBA48_1       101
#define ATA_IDENTIFY_WORD_LBA48_2       102
#define ATA_IDENTIFY_WORD_LBA48_HIGH    103


/*
 * Register Host-to-Device FIS.
 */
typedef struct PACKED
{
    uint8_t fis_type;
    uint8_t flags;
    uint8_t command;
    uint8_t feature_low;

    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;

    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t feature_high;

    uint8_t count_low;
    uint8_t count_high;

    uint8_t icc;
    uint8_t control;

    uint8_t reserved[4];

} AHCIRegisterFIS;


/*
 * Physical Region Descriptor Table entry.
 */
typedef struct PACKED
{
    uint32_t data_base;
    uint32_t data_base_upper;

    uint32_t reserved;

    uint32_t byte_count_and_flags;

} AHCIPrdtEntry;


/*
 * AHCI command header.
 */
typedef struct PACKED
{
    uint32_t command_flags;

    uint32_t prdt_length;

    uint32_t physical_region_descriptor_byte_count;

    uint32_t command_table_base;
    uint32_t command_table_base_upper;

    uint32_t reserved[4];

} AHCICommandHeader;


/*
 * AHCI command table.
 */
typedef struct PACKED
{
    uint8_t command_fis[AHCI_COMMAND_FIS_LENGTH];

    uint8_t atapi_command[16];

    uint8_t reserved[48];

    AHCIPrdtEntry prdt[AHCI_PRDT_ENTRY_COUNT];

} AHCICommandTable;


/*
 * DMA resources used by AHCI commands.
 */
typedef struct
{
    phys_addr_t command_list_physical;
    AHCICommandHeader *command_list_virtual;

    phys_addr_t command_table_physical;
    AHCICommandTable *command_table_virtual;

    phys_addr_t fis_receive_physical;
    void *fis_receive_virtual;

    phys_addr_t identify_physical;
    uint16_t *identify_virtual;

    uint8_t port_number;

} AHCICommandResources;


static XKAHCIController g_ahci_controller;

static XKAHCIPort g_ahci_ports[XK_AHCI_MAX_PORTS];

static AHCICommandResources g_ahci_command_resources;

static bool g_ahci_present;


/*
 * AHCI block device exposed through the block subsystem.
 */
static XKBlockDevice g_ahci_block_device;

static bool g_ahci_block_registered;


/*
 * Detected disk information.
 */
static uint64_t g_ahci_sector_count;
static uint32_t g_ahci_sector_size;


/*
 * Get the physical AHCI BAR5 address.
 */
static uintptr_t ahci_bar5_address(
    const XKPCIDevice *device)
{
    if (device == NULL ||
        device->bar_count < 6)
    {
        return 0;
    }

    uint32_t bar5 =
        device->bars[5];

    /*
     * AHCI BAR5 is a memory-mapped I/O BAR.
     */
    if ((bar5 & 0x01U) != 0)
        return 0;

    return (uintptr_t)(bar5 & ~0x0FU);
}


/*
 * Read a 32-bit AHCI MMIO register.
 */
static uint32_t ahci_mmio_read32(
    uintptr_t base,
    uint32_t offset)
{
    volatile uint32_t *address =
        (volatile uint32_t *)(base + offset);

    return *address;
}


/*
 * Write a 32-bit AHCI MMIO register.
 */
static void ahci_mmio_write32(
    uintptr_t base,
    uint32_t offset,
    uint32_t value)
{
    volatile uint32_t *address =
        (volatile uint32_t *)(base + offset);

    *address = value;
}


/*
 * Get the MMIO base address of an AHCI port.
 */
static uintptr_t ahci_port_base(
    uint8_t port_number)
{
    return
        g_ahci_controller.abar +
        AHCI_PORT_BASE +
        ((uintptr_t)port_number * AHCI_PORT_SIZE);
}


/*
 * Reset AHCI driver state.
 */
static void ahci_reset_state(void)
{
    g_ahci_controller =
        (XKAHCIController){0};

    for (uint32_t i = 0;
         i < XK_AHCI_MAX_PORTS;
         i++)
    {
        g_ahci_ports[i] =
            (XKAHCIPort){0};
    }

    g_ahci_command_resources =
        (AHCICommandResources){0};

    g_ahci_sector_count = 0;
    g_ahci_sector_size = 0;

    g_ahci_present = false;
    g_ahci_block_registered = false;
}


/*
 * Free any DMA pages owned by the AHCI command path.
 */
static void ahci_free_command_resources(void)
{
    AHCICommandResources *resources =
        &g_ahci_command_resources;

    if (resources->command_list_physical != 0)
    {
        pmm_free_page(
            resources->command_list_physical
        );
    }

    if (resources->command_table_physical != 0)
    {
        pmm_free_page(
            resources->command_table_physical
        );
    }

    if (resources->fis_receive_physical != 0)
    {
        pmm_free_page(
            resources->fis_receive_physical
        );
    }

    if (resources->identify_physical != 0)
    {
        pmm_free_page(
            resources->identify_physical
        );
    }

    *resources =
        (AHCICommandResources){0};
}


/*
 * Allocate all DMA memory needed by the AHCI command path.
 */
static bool ahci_allocate_command_resources(
    uint8_t port_number)
{
    AHCICommandResources *resources =
        &g_ahci_command_resources;

    /*
     * Command list.
     */
    resources->command_list_physical =
        pmm_alloc_page();

    if (resources->command_list_physical == 0)
        goto fail;

    resources->command_list_virtual =
        (AHCICommandHeader *)phys_to_virt(
            resources->command_list_physical
        );

    /*
     * Command table.
     */
    resources->command_table_physical =
        pmm_alloc_page();

    if (resources->command_table_physical == 0)
        goto fail;

    resources->command_table_virtual =
        (AHCICommandTable *)phys_to_virt(
            resources->command_table_physical
        );

    /*
     * FIS receive area.
     */
    resources->fis_receive_physical =
        pmm_alloc_page();

    if (resources->fis_receive_physical == 0)
        goto fail;

    resources->fis_receive_virtual =
        phys_to_virt(
            resources->fis_receive_physical
        );

    /*
     * IDENTIFY / sector DMA buffer.
     */
    resources->identify_physical =
        pmm_alloc_page();

    if (resources->identify_physical == 0)
        goto fail;

    resources->identify_virtual =
        (uint16_t *)phys_to_virt(
            resources->identify_physical
        );

    /*
     * Clear all DMA memory before giving it to AHCI.
     */
    memset(
        resources->command_list_virtual,
        0,
        AHCI_COMMAND_LIST_SIZE
    );

    memset(
        resources->command_table_virtual,
        0,
        AHCI_COMMAND_TABLE_SIZE
    );

    memset(
        resources->fis_receive_virtual,
        0,
        PAGE_SIZE
    );

    memset(
        resources->identify_virtual,
        0,
        AHCI_IDENTIFY_SIZE
    );

    resources->port_number =
        port_number;

    return true;

fail:

    ahci_free_command_resources();

    return false;
}


/*
 * Find the first connected ATA disk.
 */
static int ahci_find_ata_port(void)
{
    uintptr_t abar =
        g_ahci_controller.abar;

    for (uint32_t port = 0;
         port < XK_AHCI_MAX_PORTS;
         port++)
    {
        if (!g_ahci_ports[port].implemented)
            continue;

        if (!g_ahci_ports[port].device_present)
            continue;

        uintptr_t port_base =
            abar +
            AHCI_PORT_BASE +
            ((uintptr_t)port * AHCI_PORT_SIZE);

        uint32_t signature =
            ahci_mmio_read32(
                port_base,
                AHCI_PORT_SIG
            );

        if (signature == AHCI_SIG_ATA)
            return (int)port;
    }

    return -1;
}


/*
 * Stop the AHCI command engine on a port.
 */
static bool ahci_stop_engine(
    uintptr_t port_base)
{
    uint32_t command =
        ahci_mmio_read32(
            port_base,
            AHCI_PORT_CMD
        );

    /*
     * Stop command processing and FIS reception.
     */
    command &=
        ~(AHCI_PORT_CMD_ST |
          AHCI_PORT_CMD_FRE);

    ahci_mmio_write32(
        port_base,
        AHCI_PORT_CMD,
        command
    );

    /*
     * Wait until the controller confirms that
     * the command engine and FIS engine stopped.
     */
    for (uint32_t timeout = 0;
         timeout < AHCI_ENGINE_TIMEOUT;
         timeout++)
    {
        command =
            ahci_mmio_read32(
                port_base,
                AHCI_PORT_CMD
            );

        if ((command &
             (AHCI_PORT_CMD_CR |
              AHCI_PORT_CMD_FR)) == 0)
        {
            return true;
        }

        cpu_relax();
    }

    return false;
}


/*
 * Start the AHCI command engine on a port.
 */
static bool ahci_start_engine(
    uintptr_t port_base)
{
    uint32_t command =
        ahci_mmio_read32(
            port_base,
            AHCI_PORT_CMD
        );

    /*
     * Start FIS reception first.
     */
    command |=
        AHCI_PORT_CMD_FRE;

    ahci_mmio_write32(
        port_base,
        AHCI_PORT_CMD,
        command
    );

    /*
     * Start command processing.
     */
    command |=
        AHCI_PORT_CMD_ST;

    ahci_mmio_write32(
        port_base,
        AHCI_PORT_CMD,
        command
    );

    return true;
}


/*
 * Configure the command list and FIS receive buffer.
 */
static bool ahci_configure_port(void)
{
    AHCICommandResources *resources =
        &g_ahci_command_resources;

    uintptr_t port_base =
        ahci_port_base(
            resources->port_number
        );

    if (!ahci_stop_engine(port_base))
        return false;

    /*
     * Clear pending port errors.
     */
    ahci_mmio_write32(
        port_base,
        AHCI_PORT_SERR,
        0xFFFFFFFFU
    );

    /*
     * Command List Base Address.
     */
    ahci_mmio_write32(
        port_base,
        AHCI_PORT_CLB,
        (uint32_t)
            resources->command_list_physical
    );

    ahci_mmio_write32(
        port_base,
        AHCI_PORT_CLBU,
        (uint32_t)
            (resources->command_list_physical >> 32)
    );

    /*
     * FIS Receive Base Address.
     */
    ahci_mmio_write32(
        port_base,
        AHCI_PORT_FB,
        (uint32_t)
            resources->fis_receive_physical
    );

    ahci_mmio_write32(
        port_base,
        AHCI_PORT_FBU,
        (uint32_t)
            (resources->fis_receive_physical >> 32)
    );

    /*
     * Clear command structures.
     */
    memset(
        resources->command_list_virtual,
        0,
        AHCI_COMMAND_LIST_SIZE
    );

    memset(
        resources->command_table_virtual,
        0,
        AHCI_COMMAND_TABLE_SIZE
    );

    return ahci_start_engine(port_base);
}


/*
 * Prepare command slot 0.
 */
static AHCICommandHeader *ahci_prepare_command(
    bool write)
{
    AHCICommandResources *resources =
        &g_ahci_command_resources;

    AHCICommandHeader *header =
        &resources->command_list_virtual[0];

    memset(
        header,
        0,
        sizeof(AHCICommandHeader)
    );

    /*
     * CFL = 5 DWORDs = 20-byte Register FIS.
     *
     * PRDTL = 1.
     */
    header->command_flags =
        5U |
        (1U << 16);

    /*
     * W bit = 1 for host-to-device writes.
     */
    if (write)
    {
        header->command_flags |=
            (1U << 6);
    }

    header->command_table_base =
        (uint32_t)
            resources->command_table_physical;

    header->command_table_base_upper =
        (uint32_t)
            (resources->command_table_physical >> 32);

    return header;
}


/*
 * Configure the single PRDT entry for a 512-byte transfer.
 */
static void ahci_prepare_prdt(void)
{
    AHCICommandResources *resources =
        &g_ahci_command_resources;

    AHCIPrdtEntry *prdt =
        &resources->command_table_virtual->prdt[0];

    memset(
        prdt,
        0,
        sizeof(AHCIPrdtEntry)
    );

    prdt->data_base =
        (uint32_t)
            resources->identify_physical;

    prdt->data_base_upper =
        (uint32_t)
            (resources->identify_physical >> 32);

    /*
     * Byte count is encoded as byte_count - 1.
     *
     * 512 bytes -> 511.
     *
     * IOC is bit 31.
     */
    prdt->byte_count_and_flags =
        (AHCI_IDENTIFY_SIZE - 1U) |
        (1U << 31);
}


/*
 * Wait until the device is ready for a command.
 */
static bool ahci_wait_device_ready(
    uintptr_t port_base)
{
    for (uint32_t timeout = 0;
         timeout < AHCI_COMMAND_TIMEOUT;
         timeout++)
    {
        uint32_t tfd =
            ahci_mmio_read32(
                port_base,
                AHCI_PORT_TFD
            );

        if (tfd & AHCI_TFD_ERR)
            return false;

        if ((tfd &
             (AHCI_TFD_BSY |
              AHCI_TFD_DRQ)) == 0)
        {
            return true;
        }

        cpu_relax();
    }

    return false;
}


/*
 * Issue command slot 0 and wait for completion.
 */
static bool ahci_issue_command(void)
{
    AHCICommandResources *resources =
        &g_ahci_command_resources;

    uintptr_t port_base =
        ahci_port_base(
            resources->port_number
        );

    /*
     * Slot 0 must not already be active.
     */
    uint32_t ci =
        ahci_mmio_read32(
            port_base,
            AHCI_PORT_CI
        );

    if (ci & 1U)
        return false;

    /*
     * Issue slot 0.
     */
    ahci_mmio_write32(
        port_base,
        AHCI_PORT_CI,
        ci | 1U
    );

    /*
     * Poll for completion.
     */
    for (uint32_t timeout = 0;
         timeout < AHCI_COMMAND_TIMEOUT;
         timeout++)
    {
        uint32_t status =
            ahci_mmio_read32(
                port_base,
                AHCI_PORT_TFD
            );

        if (status & AHCI_TFD_ERR)
            return false;

        uint32_t current_ci =
            ahci_mmio_read32(
                port_base,
                AHCI_PORT_CI
            );

        if ((current_ci & 1U) == 0)
            return true;

        cpu_relax();
    }

    return false;
}


/*
 * Build and issue IDENTIFY DEVICE.
 */
static bool ahci_identify_device(void)
{
    AHCICommandResources *resources =
        &g_ahci_command_resources;

    uintptr_t port_base =
        ahci_port_base(
            resources->port_number
        );

    if (!ahci_wait_device_ready(port_base))
        return false;

    /*
     * Clear the IDENTIFY buffer.
     */
    memset(
        resources->identify_virtual,
        0,
        AHCI_IDENTIFY_SIZE
    );

    /*
     * Clear command table.
     */
    memset(
        resources->command_table_virtual,
        0,
        AHCI_COMMAND_TABLE_SIZE
    );

    ahci_prepare_command(false);

    AHCICommandTable *table =
        resources->command_table_virtual;

    AHCIRegisterFIS *fis =
        (AHCIRegisterFIS *)table->command_fis;

    memset(
        fis,
        0,
        sizeof(AHCIRegisterFIS)
    );

    fis->fis_type =
        AHCI_FIS_TYPE_REG_H2D;

    /*
     * C bit = 1.
     */
    fis->flags =
        (1U << 7);

    fis->command =
        AHCI_FIS_CMD_IDENTIFY;

    /*
     * Device/head register.
     *
     * 0x40 selects LBA mode.
     */
    fis->device =
        0x40;

    ahci_prepare_prdt();

    return ahci_issue_command();
}


/*
 * Build and issue one-sector READ DMA EXT command.
 */
static bool ahci_read_sector(
    uint64_t sector)
{
    AHCICommandResources *resources =
        &g_ahci_command_resources;

    uintptr_t port_base =
        ahci_port_base(
            resources->port_number
        );

    if (sector >= g_ahci_sector_count)
        return false;

    if (!ahci_wait_device_ready(port_base))
        return false;

    /*
     * Reuse the IDENTIFY DMA page as the
     * one-sector read buffer.
     */
    memset(
        resources->identify_virtual,
        0,
        AHCI_IDENTIFY_SIZE
    );

    memset(
        resources->command_table_virtual,
        0,
        AHCI_COMMAND_TABLE_SIZE
    );

    ahci_prepare_command(false);

    AHCICommandTable *table =
        resources->command_table_virtual;

    AHCIRegisterFIS *fis =
        (AHCIRegisterFIS *)table->command_fis;

    memset(
        fis,
        0,
        sizeof(AHCIRegisterFIS)
    );

    fis->fis_type =
        AHCI_FIS_TYPE_REG_H2D;

    fis->flags =
        (1U << 7);

    fis->command =
        AHCI_FIS_CMD_READ_DMA_EXT;

    /*
     * 48-bit LBA.
     */
    fis->lba0 =
        (uint8_t)(sector & 0xFF);

    fis->lba1 =
        (uint8_t)((sector >> 8) & 0xFF);

    fis->lba2 =
        (uint8_t)((sector >> 16) & 0xFF);

    fis->lba3 =
        (uint8_t)((sector >> 24) & 0xFF);

    fis->lba4 =
        (uint8_t)((sector >> 32) & 0xFF);

    fis->lba5 =
        (uint8_t)((sector >> 40) & 0xFF);

    /*
     * LBA mode.
     */
    fis->device =
        0x40;

    /*
     * One sector.
     */
    fis->count_low =
        1;

    fis->count_high =
        0;

    ahci_prepare_prdt();

    return ahci_issue_command();
}


/*
 * Read sectors through the XyrisOS block-device API.
 *
 * The current implementation performs one 512-byte ATA read
 * at a time using the DMA buffer, then copies the data into
 * the caller's buffer.
 */
static int ahci_block_read(
    XKBlockDevice *device,
    uint64_t sector,
    uint32_t count,
    void *buffer)
{
    if (device == NULL ||
        buffer == NULL ||
        count == 0)
    {
        return -1;
    }

    if (device != &g_ahci_block_device)
        return -1;

    if (g_ahci_sector_size != 512)
        return -1;

    if (sector >= g_ahci_sector_count)
        return -1;

    if ((uint64_t)count >
        g_ahci_sector_count - sector)
    {
        return -1;
    }

    uint8_t *destination =
        (uint8_t *)buffer;

    for (uint32_t i = 0;
         i < count;
         i++)
    {
        if (!ahci_read_sector(
                sector + i))
        {
            return -1;
        }

        memcpy(
            destination +
                ((size_t)i * g_ahci_sector_size),

            g_ahci_command_resources.identify_virtual,

            g_ahci_sector_size
        );
    }

    return 0;
}


/*
 * Identify the disk and determine its capacity.
 */
static bool ahci_read_capacity(void)
{
    uint16_t *identify =
        g_ahci_command_resources.identify_virtual;

    /*
     * Word 83 bit 10 indicates 48-bit LBA support.
     */
    bool supports_48bit =
        (identify[ATA_IDENTIFY_WORD_48BIT_LBA] &
         (1U << 10)) != 0;

    uint64_t sector_count = 0;

    if (supports_48bit)
    {
        sector_count =
            ((uint64_t)identify[
                ATA_IDENTIFY_WORD_LBA48_HIGH
            ] << 48) |

            ((uint64_t)identify[
                ATA_IDENTIFY_WORD_LBA48_2
            ] << 32) |

            ((uint64_t)identify[
                ATA_IDENTIFY_WORD_LBA48_1
            ] << 16) |

            ((uint64_t)identify[
                ATA_IDENTIFY_WORD_LBA48_LOW
            ]);
    }
    else
    {
        sector_count =
            ((uint64_t)identify[
                ATA_IDENTIFY_WORD_LBA28_HIGH
            ] << 16) |

            ((uint64_t)identify[
                ATA_IDENTIFY_WORD_LBA28_LOW
            ]);
    }

    if (sector_count == 0)
        return false;

    /*
     * XyrisOS currently exposes ATA logical sectors
     * as 512-byte block devices.
     */
    g_ahci_sector_size = 512;
    g_ahci_sector_count = sector_count;

    return true;
}


/*
 * Initialize the AHCI block-device object.
 */
static bool ahci_register_block_device(void)
{
    memset(
        &g_ahci_block_device,
        0,
        sizeof(g_ahci_block_device)
    );

    g_ahci_block_device.name[0] = 's';
    g_ahci_block_device.name[1] = 'd';
    g_ahci_block_device.name[2] = 'a';
    g_ahci_block_device.name[3] = '\0';

    g_ahci_block_device.sector_size =
        g_ahci_sector_size;

    g_ahci_block_device.sector_count =
        g_ahci_sector_count;

    /*
     * Reads are supported.
     *
     * Writes remain disabled until separately validated.
     */
    static const XKBlockOperations operations =
    {
        .initialize = NULL,
        .shutdown = NULL,
        .read = ahci_block_read,
        .write = NULL
    };

    g_ahci_block_device.operations =
        &operations;

    g_ahci_block_device.read_only =
        true;

    if (xk_block_register(
            &g_ahci_block_device) != 0)
    {
        return false;
    }

    g_ahci_block_registered =
        true;

    return true;
}


/*
 * Initialize the AHCI controller and enumerate ports.
 */
bool xk_ahci_initialize(void)
{
    /*
     * Do not leak an existing block device
     * or DMA resources if initialization runs again.
     */
    if (g_ahci_block_registered)
    {
        xk_block_unregister(
            &g_ahci_block_device
        );
    }

    ahci_free_command_resources();

    ahci_reset_state();

    uint32_t pci_count =
        xk_pci_device_count();

    for (uint32_t i = 0;
         i < pci_count;
         i++)
    {
        const XKPCIDevice *device =
            xk_pci_device_get(i);

        if (device == NULL)
            continue;

        if (device->class_code != XK_AHCI_CLASS_CODE ||
            device->subclass != XK_AHCI_SUBCLASS ||
            device->prog_if != XK_AHCI_PROG_IF)
        {
            continue;
        }

        uintptr_t abar =
            ahci_bar5_address(device);

        if (abar == 0)
            continue;

        /*
         * AHCI BAR5 is a memory-mapped I/O region.
         */
        if (!hhdm_map_mmio(
                abar,
                0x1100))
        {
            continue;
        }

        abar =
            hhdm_offset() + abar;

        g_ahci_controller.vendor_id =
            device->vendor_id;

        g_ahci_controller.device_id =
            device->device_id;

        g_ahci_controller.bus =
            device->bus;

        g_ahci_controller.device =
            device->device;

        g_ahci_controller.function =
            device->function;

        g_ahci_controller.abar =
            abar;

        uint32_t pi =
            ahci_mmio_read32(
                abar,
                AHCI_PI
            );

        uint32_t port_count = 0;
        uint32_t implemented_ports = 0;

        for (uint32_t port = 0;
             port < XK_AHCI_MAX_PORTS;
             port++)
        {
            if ((pi & (1U << port)) == 0)
                continue;

            implemented_ports++;

            XKAHCIPort *info =
                &g_ahci_ports[port];

            info->port_number =
                (uint8_t)port;

            info->implemented =
                1;

            uintptr_t port_base =
                abar +
                AHCI_PORT_BASE +
                ((uintptr_t)port * AHCI_PORT_SIZE);

            uint32_t ssts =
                ahci_mmio_read32(
                    port_base,
                    AHCI_PORT_SSTS
                );

            info->device_present =
                ((ssts & AHCI_PORT_DET_MASK) == 0x03U);

            info->device_type =
                (uint8_t)(
                    (ssts >> 4) & 0x0FU
                );

            port_count++;
        }

        g_ahci_controller.port_count =
            port_count;

        g_ahci_controller.implemented_ports =
            implemented_ports;

        g_ahci_present =
            true;

        /*
         * Find a real ATA device.
         */
        int ata_port =
            ahci_find_ata_port();

        if (ata_port < 0)
            return true;

        /*
         * Allocate command/FIS/IDENTIFY DMA memory.
         */
        if (!ahci_allocate_command_resources(
                (uint8_t)ata_port))
        {
            return true;
        }

        /*
         * Connect the DMA structures to the AHCI port.
         */
        if (!ahci_configure_port())
        {
            ahci_free_command_resources();
            return true;
        }

        /*
         * IDENTIFY DEVICE.
         */
        if (!ahci_identify_device())
        {
            ahci_stop_engine(
                ahci_port_base(
                    (uint8_t)ata_port
                )
            );

            ahci_free_command_resources();

            return true;
        }

        /*
         * Extract disk capacity.
         */
        if (!ahci_read_capacity())
        {
            ahci_stop_engine(
                ahci_port_base(
                    (uint8_t)ata_port
                )
            );

            ahci_free_command_resources();

            return true;
        }

        /*
         * Register the block storage device.
         */
        if (!ahci_register_block_device())
        {
            ahci_stop_engine(
                ahci_port_base(
                    (uint8_t)ata_port
                )
            );

            ahci_free_command_resources();

            return true;
        }

        return true;
    }

    return false;
}


/*
 * Shut down AHCI.
 */
void xk_ahci_shutdown(void)
{
    if (g_ahci_block_registered)
    {
        xk_block_unregister(
            &g_ahci_block_device
        );

        g_ahci_block_registered =
            false;
    }

    if (g_ahci_present &&
        g_ahci_command_resources.command_list_physical != 0)
    {
        uintptr_t port_base =
            ahci_port_base(
                g_ahci_command_resources.port_number
            );

        ahci_stop_engine(
            port_base
        );
    }

    ahci_free_command_resources();

    ahci_reset_state();
}


/*
 * Return whether an AHCI controller was detected.
 */
bool xk_ahci_is_present(void)
{
    return g_ahci_present;
}


/*
 * Return AHCI controller information.
 */
XKAHCIController xk_ahci_controller_info(void)
{
    return g_ahci_controller;
}


/*
 * Return the number of implemented AHCI ports.
 */
uint32_t xk_ahci_port_count(void)
{
    return g_ahci_controller.port_count;
}


/*
 * Return information about an implemented AHCI port.
 */
int xk_ahci_port_get(
    uint32_t index,
    XKAHCIPort *port)
{
    if (port == NULL)
        return -1;

    if (index >= XK_AHCI_MAX_PORTS)
        return -1;

    if (!g_ahci_ports[index].implemented)
        return -1;

    *port =
        g_ahci_ports[index];

    return 0;
}