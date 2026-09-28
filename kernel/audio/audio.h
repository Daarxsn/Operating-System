#ifndef XYRIS_AUDIO_H
#define XYRIS_AUDIO_H

#include <stdbool.h>
#include <stdint.h>

#include "drivers/driver.h"

#define XK_AUDIO_MAX_DEVICES 8u
#define XK_AUDIO_DEVICE_NAME_MAX 32u

typedef enum
{
    XK_AUDIO_DEVICE_OUTPUT = 1u << 0,
    XK_AUDIO_DEVICE_INPUT  = 1u << 1
} XKAudioDeviceCapability;

typedef enum
{
    XK_AUDIO_FORMAT_UNKNOWN = 0,
    XK_AUDIO_FORMAT_PCM_S16,
    XK_AUDIO_FORMAT_PCM_S24,
    XK_AUDIO_FORMAT_PCM_S32
} XKAudioFormat;

typedef enum
{
    XK_AUDIO_STATE_UNAVAILABLE = 0,
    XK_AUDIO_STATE_DETECTED,
    XK_AUDIO_STATE_READY,
    XK_AUDIO_STATE_ERROR
} XKAudioState;

typedef struct
{
    uint32_t sample_rate;
    uint8_t channels;
    XKAudioFormat format;
} XKAudioFormatInfo;

typedef struct
{
    uint32_t id;

    uint16_t vendor_id;
    uint16_t device_id;

    uint8_t bus;
    uint8_t device;
    uint8_t function;

    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;

    uint32_t capabilities;

    XKAudioState state;

    uint8_t volume;
    bool muted;

    XKAudioFormatInfo format;

    char name[XK_AUDIO_DEVICE_NAME_MAX];
} XKAudioDevice;

/*
 * Initialize and shut down the audio subsystem.
 */
bool xk_audio_init(void);
void xk_audio_shutdown(void);

/*
 * Audio availability and device enumeration.
 */
bool xk_audio_available(void);
uint32_t xk_audio_device_count(void);

const XKAudioDevice *xk_audio_device_get(
    uint32_t index
);

const XKAudioDevice *xk_audio_default_output(void);
const XKAudioDevice *xk_audio_default_input(void);

/*
 * Volume and mute controls.
 */
bool xk_audio_set_volume(
    uint32_t device_id,
    uint8_t volume
);

bool xk_audio_get_volume(
    uint32_t device_id,
    uint8_t *volume
);

bool xk_audio_set_mute(
    uint32_t device_id,
    bool muted
);

bool xk_audio_get_mute(
    uint32_t device_id,
    bool *muted
);

/*
 * PCM format configuration.
 */
bool xk_audio_set_format(
    uint32_t device_id,
    uint32_t sample_rate,
    uint8_t channels,
    XKAudioFormat format
);

bool xk_audio_get_format(
    uint32_t device_id,
    XKAudioFormatInfo *format
);

/*
 * Audio data path.
 *
 * These return false until a supported hardware backend
 * is attached to the detected controller.
 */
bool xk_audio_play(
    uint32_t device_id,
    const void *buffer,
    uint32_t size
);

bool xk_audio_capture(
    uint32_t device_id,
    void *buffer,
    uint32_t size
);

/*
 * Diagnostics.
 */
void xk_audio_dump(void);

/*
 * Driver-manager integration.
 */
extern XKDriver xk_audio_driver;

#endif