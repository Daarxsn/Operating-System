#ifndef XYRIS_ACCESSIBILITY_H
#define XYRIS_ACCESSIBILITY_H

#include <stdbool.h>
#include <stdint.h>

#define XYRIS_TEXT_SCALE_MIN 75
#define XYRIS_TEXT_SCALE_MAX 200

typedef struct {
    bool keyboard_navigation;
    bool high_contrast;
    bool reduced_motion;
    uint16_t text_scale_percent;
} XyrisAccessibilitySettings;

/* Preferences API; visual/input effects require separate integration. */
void accessibility_init(void);
const XyrisAccessibilitySettings *accessibility_get_settings(void);
void accessibility_set_keyboard_navigation(bool enabled);
void accessibility_set_high_contrast(bool enabled);
void accessibility_set_reduced_motion(bool enabled);
bool accessibility_set_text_scale(uint16_t percent);

#endif
