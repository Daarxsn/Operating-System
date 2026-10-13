#include "accessibility.h"

static XyrisAccessibilitySettings settings = {
    .keyboard_navigation = true,
    .high_contrast = false,
    .reduced_motion = false,
    .text_scale_percent = 100
};

void accessibility_init(void)
{
    settings.keyboard_navigation = true;
    settings.high_contrast = false;
    settings.reduced_motion = false;
    settings.text_scale_percent = 100;
}

const XyrisAccessibilitySettings *accessibility_get_settings(void)
{
    return &settings;
}

void accessibility_set_keyboard_navigation(bool enabled)
{
    settings.keyboard_navigation = enabled;
}

void accessibility_set_high_contrast(bool enabled)
{
    settings.high_contrast = enabled;
}

void accessibility_set_reduced_motion(bool enabled)
{
    settings.reduced_motion = enabled;
}

bool accessibility_set_text_scale(uint16_t percent)
{
    if (percent < XYRIS_TEXT_SCALE_MIN ||
        percent > XYRIS_TEXT_SCALE_MAX)
        return false;

    settings.text_scale_percent = percent;
    return true;
}
