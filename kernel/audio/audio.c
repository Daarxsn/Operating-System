#include "audio.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "debug/hex.h"
#include "debug/print.h"
#include "drivers/driver.h"
#include "hardware/pci_inventory.h"

static XKAudioDevice g_audio_devices[XK_AUDIO_MAX_DEVICES];

static uint32_t g_audio_device_count = 0;
static bool g_audio_initialized = false;


/*
 * --------------------------------------------------------------------------
 * Internal helpers
 * --------------------------------------------------------------------------
 */

static XKAudioDevice *xk_audio_find_device(
    uint32_t device_id
)
{
    for (uint32_t i = 0; i < g_audio_device_count; i++)
    {
        if (g_audio_devices[i].id == device_id)
        {
            return &g_audio_devices[i];
        }
    }

    return NULL;
}


static const XKAudioDevice *xk_audio_find_device_const(
    uint32_t device_id
)
{
    for (uint32_t i = 0; i < g_audio_device_count; i++)
    {
        if (g_audio_devices[i].id == device_id)
        {
            return &g_audio_devices[i];
        }
    }

    return NULL;
}


static bool xk_audio_format_valid(
    uint32_t sample_rate,
    uint8_t channels,
    XKAudioFormat format
)
{
    if (sample_rate == 0)
    {
        return false;
    }

    if (channels == 0)
    {
        return false;
    }

    switch (format)
    {
        case XK_AUDIO_FORMAT_PCM_S16:
        case XK_AUDIO_FORMAT_PCM_S24:
        case XK_AUDIO_FORMAT_PCM_S32:
            return true;

        default:
            return false;
    }
}


static void xk_audio_device_reset(
    XKAudioDevice *device
)
{
    if (device == NULL)
    {
        return;
    }

    *device = (XKAudioDevice){0};

    device->state = XK_AUDIO_STATE_UNAVAILABLE;

    device->volume = 100;
    device->muted = false;

    device->format.sample_rate = 48000;
    device->format.channels = 2;
    device->format.format = XK_AUDIO_FORMAT_PCM_S16;
}


static bool xk_audio_register_pci_device(
    const pci_inventory_device_t *pci_device
)
{
    if (pci_device == NULL)
    {
        return false;
    }

    if (g_audio_device_count >= XK_AUDIO_MAX_DEVICES)
    {
        debug_print_line(
            "[AUDIO] Maximum audio device count reached"
        );

        return false;
    }

    XKAudioDevice *device =
        &g_audio_devices[g_audio_device_count];

    xk_audio_device_reset(device);

    device->id = g_audio_device_count + 1u;

    device->vendor_id = pci_device->vendor_id;
    device->device_id = pci_device->device_id;

    device->bus = pci_device->bus;
    device->device = pci_device->device;
    device->function = pci_device->function;

    device->class_code = pci_device->class_code;
    device->subclass = pci_device->subclass;
    device->prog_if = pci_device->prog_if;

    /*
     * A PCI multimedia controller is the hardware discovery point.
     *
     * This does not claim that every class-04 controller is an audio
     * controller. A controller-specific backend is required before
     * actual playback/capture is enabled.
     */
    device->capabilities =
        XK_AUDIO_DEVICE_OUTPUT |
        XK_AUDIO_DEVICE_INPUT;

    device->state = XK_AUDIO_STATE_DETECTED;

    device->name[0] = 'A';
    device->name[1] = 'u';
    device->name[2] = 'd';
    device->name[3] = 'i';
    device->name[4] = 'o';
    device->name[5] = ' ';
    device->name[6] = 'D';
    device->name[7] = 'e';
    device->name[8] = 'v';
    device->name[9] = 'i';
    device->name[10] = 'c';
    device->name[11] = 'e';
    device->name[12] = '\0';

    g_audio_device_count++;

    return true;
}


/*
 * --------------------------------------------------------------------------
 * Initialization
 * --------------------------------------------------------------------------
 */

bool xk_audio_init(void)
{
    g_audio_device_count = 0;
    g_audio_initialized = false;

    for (uint32_t i = 0; i < XK_AUDIO_MAX_DEVICES; i++)
    {
        xk_audio_device_reset(
            &g_audio_devices[i]
        );
    }

    pci_inventory_t inventory =
        pci_inventory_info();

    debug_print_line(
        "[AUDIO] Audio subsystem initializing"
    );

    debug_print(
        "[AUDIO] Multimedia controllers: "
    );

    debug_print_hex64(
        (uint64_t)inventory.multimedia_devices
    );

    debug_print_line("");

    /*
     * Discover PCI multimedia controllers.
     */
    for (uint32_t i = 0;
         i < inventory.total_devices;
         i++)
    {
        pci_inventory_device_t pci_device;

        if (!pci_inventory_device_get(
                i,
                &pci_device))
        {
            continue;
        }

        if (pci_device.hardware_class !=
            PCI_HARDWARE_MULTIMEDIA)
        {
            continue;
        }

        xk_audio_register_pci_device(
            &pci_device
        );
    }

    g_audio_initialized = true;

    if (g_audio_device_count == 0)
    {
        debug_print_line(
            "[AUDIO] No multimedia/audio controller detected"
        );

        return true;
    }

    /*
     * Devices are currently detected but not marked READY.
     *
     * READY requires a controller-specific hardware backend.
     */
    for (uint32_t i = 0;
         i < g_audio_device_count;
         i++)
    {
        XKAudioDevice *device =
            &g_audio_devices[i];

        device->state =
            XK_AUDIO_STATE_DETECTED;
    }

    debug_print(
        "[AUDIO] Detected audio devices: "
    );

    debug_print_hex64(
        (uint64_t)g_audio_device_count
    );

    debug_print_line("");

    return true;
}


void xk_audio_shutdown(void)
{
    if (!g_audio_initialized)
    {
        return;
    }

    for (uint32_t i = 0;
         i < g_audio_device_count;
         i++)
    {
        g_audio_devices[i].state =
            XK_AUDIO_STATE_UNAVAILABLE;
    }

    g_audio_device_count = 0;
    g_audio_initialized = false;

    debug_print_line(
        "[AUDIO] Audio subsystem shut down"
    );
}


/*
 * --------------------------------------------------------------------------
 * Device enumeration
 * --------------------------------------------------------------------------
 */

bool xk_audio_available(void)
{
    return
        g_audio_initialized &&
        g_audio_device_count > 0;
}


uint32_t xk_audio_device_count(void)
{
    return g_audio_device_count;
}


const XKAudioDevice *xk_audio_device_get(
    uint32_t index
)
{
    if (index >= g_audio_device_count)
    {
        return NULL;
    }

    return &g_audio_devices[index];
}


const XKAudioDevice *xk_audio_default_output(void)
{
    if (!g_audio_initialized)
    {
        return NULL;
    }

    for (uint32_t i = 0;
         i < g_audio_device_count;
         i++)
    {
        const XKAudioDevice *device =
            &g_audio_devices[i];

        if ((device->capabilities &
             XK_AUDIO_DEVICE_OUTPUT) != 0)
        {
            return device;
        }
    }

    return NULL;
}


const XKAudioDevice *xk_audio_default_input(void)
{
    if (!g_audio_initialized)
    {
        return NULL;
    }

    for (uint32_t i = 0;
         i < g_audio_device_count;
         i++)
    {
        const XKAudioDevice *device =
            &g_audio_devices[i];

        if ((device->capabilities &
             XK_AUDIO_DEVICE_INPUT) != 0)
        {
            return device;
        }
    }

    return NULL;
}


/*
 * --------------------------------------------------------------------------
 * Volume
 * --------------------------------------------------------------------------
 */

bool xk_audio_set_volume(
    uint32_t device_id,
    uint8_t volume
)
{
    XKAudioDevice *device =
        xk_audio_find_device(device_id);

    if (device == NULL)
    {
        return false;
    }

    if (volume > 100)
    {
        return false;
    }

    device->volume = volume;

    return true;
}


bool xk_audio_get_volume(
    uint32_t device_id,
    uint8_t *volume
)
{
    const XKAudioDevice *device =
        xk_audio_find_device_const(device_id);

    if (device == NULL || volume == NULL)
    {
        return false;
    }

    *volume = device->volume;

    return true;
}


/*
 * --------------------------------------------------------------------------
 * Mute
 * --------------------------------------------------------------------------
 */

bool xk_audio_set_mute(
    uint32_t device_id,
    bool muted
)
{
    XKAudioDevice *device =
        xk_audio_find_device(device_id);

    if (device == NULL)
    {
        return false;
    }

    device->muted = muted;

    return true;
}


bool xk_audio_get_mute(
    uint32_t device_id,
    bool *muted
)
{
    const XKAudioDevice *device =
        xk_audio_find_device_const(device_id);

    if (device == NULL || muted == NULL)
    {
        return false;
    }

    *muted = device->muted;

    return true;
}


/*
 * --------------------------------------------------------------------------
 * Audio format
 * --------------------------------------------------------------------------
 */

bool xk_audio_set_format(
    uint32_t device_id,
    uint32_t sample_rate,
    uint8_t channels,
    XKAudioFormat format
)
{
    XKAudioDevice *device =
        xk_audio_find_device(device_id);

    if (device == NULL)
    {
        return false;
    }

    if (!xk_audio_format_valid(
            sample_rate,
            channels,
            format))
    {
        return false;
    }

    device->format.sample_rate =
        sample_rate;

    device->format.channels =
        channels;

    device->format.format =
        format;

    return true;
}


bool xk_audio_get_format(
    uint32_t device_id,
    XKAudioFormatInfo *format
)
{
    const XKAudioDevice *device =
        xk_audio_find_device_const(device_id);

    if (device == NULL || format == NULL)
    {
        return false;
    }

    *format = device->format;

    return true;
}


/*
 * --------------------------------------------------------------------------
 * Playback / capture
 * --------------------------------------------------------------------------
 */

bool xk_audio_play(
    uint32_t device_id,
    const void *buffer,
    uint32_t size
)
{
    const XKAudioDevice *device =
        xk_audio_find_device_const(device_id);

    if (device == NULL)
    {
        return false;
    }

    if (buffer == NULL || size == 0)
    {
        return false;
    }

    if ((device->capabilities &
         XK_AUDIO_DEVICE_OUTPUT) == 0)
    {
        return false;
    }

    /*
     * No hardware playback backend is connected yet.
     *
     * Returning false is intentional: XyrisOS must never report
     * successful playback when no controller-specific backend exists.
     */
    return false;
}


bool xk_audio_capture(
    uint32_t device_id,
    void *buffer,
    uint32_t size
)
{
    const XKAudioDevice *device =
        xk_audio_find_device_const(device_id);

    if (device == NULL)
    {
        return false;
    }

    if (buffer == NULL || size == 0)
    {
        return false;
    }

    if ((device->capabilities &
         XK_AUDIO_DEVICE_INPUT) == 0)
    {
        return false;
    }

    /*
     * No hardware capture backend is connected yet.
     */
    return false;
}


/*
 * --------------------------------------------------------------------------
 * Diagnostics
 * --------------------------------------------------------------------------
 */

void xk_audio_dump(void)
{
    debug_print_line(
        "========== AUDIO DEVICES =========="
    );

    debug_print(
        "Audio devices: "
    );

    debug_print_hex64(
        (uint64_t)g_audio_device_count
    );

    debug_print_line("");

    for (uint32_t i = 0;
         i < g_audio_device_count;
         i++)
    {
        XKAudioDevice *device =
            &g_audio_devices[i];

        debug_print(
            "Audio device: "
        );

        debug_print_line(
            device->name
        );

        debug_print(
            "  Vendor ID: "
        );

        debug_print_hex64(
            (uint64_t)device->vendor_id
        );

        debug_print_line("");

        debug_print(
            "  Device ID: "
        );

        debug_print_hex64(
            (uint64_t)device->device_id
        );

        debug_print_line("");

        debug_print(
            "  Bus: "
        );

        debug_print_hex64(
            (uint64_t)device->bus
        );

        debug_print_line("");

        debug_print(
            "  Device: "
        );

        debug_print_hex64(
            (uint64_t)device->device
        );

        debug_print_line("");

        debug_print(
            "  Function: "
        );

        debug_print_hex64(
            (uint64_t)device->function
        );

        debug_print_line("");

        debug_print(
            "  Class: "
        );

        debug_print_hex64(
            (uint64_t)device->class_code
        );

        debug_print_line("");

        debug_print(
            "  Subclass: "
        );

        debug_print_hex64(
            (uint64_t)device->subclass
        );

        debug_print_line("");

        debug_print(
            "  Prog IF: "
        );

        debug_print_hex64(
            (uint64_t)device->prog_if
        );

        debug_print_line("");

        debug_print(
            "  Capabilities: "
        );

        debug_print_hex64(
            (uint64_t)device->capabilities
        );

        debug_print_line("");

        debug_print(
            "  State: "
        );

        debug_print_hex64(
            (uint64_t)device->state
        );

        debug_print_line("");

        debug_print(
            "  Volume: "
        );

        debug_print_hex64(
            (uint64_t)device->volume
        );

        debug_print_line("");

        debug_print(
            "  Muted: "
        );

        debug_print_hex64(
            device->muted ? 1u : 0u
        );

        debug_print_line("");

        debug_print(
            "  Sample Rate: "
        );

        debug_print_hex64(
            (uint64_t)device->format.sample_rate
        );

        debug_print_line("");

        debug_print(
            "  Channels: "
        );

        debug_print_hex64(
            (uint64_t)device->format.channels
        );

        debug_print_line("");

        debug_print_line("");
    }

    debug_print_line(
        "==================================="
    );
}


/*
 * --------------------------------------------------------------------------
 * Driver manager integration
 * --------------------------------------------------------------------------
 */

static bool xk_audio_driver_initialize(void)
{
    return xk_audio_init();
}


static void xk_audio_driver_shutdown(void)
{
    xk_audio_shutdown();
}


XKDriver xk_audio_driver =
{
    .name = "audio",
    .type = XK_DRIVER_AUDIO,
    .state = XK_DRIVER_UNINITIALIZED,
    .initialize = xk_audio_driver_initialize,
    .shutdown = xk_audio_driver_shutdown
};