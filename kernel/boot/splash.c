#include "splash.h"

#include <stdint.h>

#include "graphics/framebuffer.h"
#include "graphics/font.h"
#include "graphics/font8x8_basic.h"
#include "image/image.h"
#include "image/logo.h"
#include "foundation/time.h"

/*
 * -------------------------------------------------------
 * Splash Screen Configuration
 * -------------------------------------------------------
 *
 * Phase 1:
 *   Completely black framebuffer.
 *
 * Phase 2:
 *   Static XyrisOS logo + larger "Booting"
 *   with a small animated loading indicator.
 */

#define SPLASH_BLACK_TIME_MS    1000
#define SPLASH_DISPLAY_TIME_MS  10000

#define SPLASH_LOGO_WIDTH   512
#define SPLASH_LOGO_HEIGHT  512

#define SPLASH_TEXT_COLOR 0x00FFFFFF

/*
 * -------------------------------------------------------
 * Loading Indicator
 * -------------------------------------------------------
 *
 * Four small squares are used as the loading indicator.
 *
 * The logo and "Booting" text never move.
 * Only the small indicator is animated.
 */

#define LOADER_SIZE   6
#define LOADER_GAP    6
#define LOADER_COUNT  4

static void splash_draw_loader(
    int center_x,
    int y,
    uint32_t active
)
{
    const int total_width =
        (LOADER_COUNT * LOADER_SIZE) +
        ((LOADER_COUNT - 1) * LOADER_GAP);

    const int start_x =
        center_x - (total_width / 2);

    for (uint32_t i = 0; i < LOADER_COUNT; i++)
    {
        const uint32_t color =
            (i == active)
                ? 0x00FFFFFF
                : 0x00555555;

        framebuffer_fill_rect(
            start_x +
                (int)i * (LOADER_SIZE + LOADER_GAP),
            y,
            LOADER_SIZE,
            LOADER_SIZE,
            color
        );
    }
}

/*
 * -------------------------------------------------------
 * Draw "Booting" at 2x font size
 * -------------------------------------------------------
 *
 * The normal system font remains unchanged.
 * Only the splash-screen "Booting" text is enlarged.
 */

static void splash_draw_booting(
    int x,
    int y,
    uint32_t color
)
{
    const char *text = "Booting";

    while (*text)
    {
        const unsigned char *glyph =
            (const unsigned char *)font8x8_basic[
                (unsigned char)*text
            ];

        for (int row = 0; row < 8; row++)
        {
            uint8_t bits = glyph[row];

            for (int col = 0; col < 8; col++)
            {
                if (bits & (1 << col))
                {
                    framebuffer_fill_rect(
                        x + (col * 2),
                        y + (row * 2),
                        2,
                        2,
                        color
                    );
                }
            }
        }

        /*
         * Each character is now 16 pixels wide
         * instead of the normal 8 pixels.
         */
        x += 16;

        text++;
    }
}

/*
 * -------------------------------------------------------
 * Boot Splash
 * -------------------------------------------------------
 */

void boot_splash_show(void)
{
    /*
     * Make sure a valid framebuffer exists.
     */
    if (framebuffer_width() == 0 ||
        framebuffer_height() == 0)
    {
        return;
    }

    /*
     * ---------------------------------------------------
     * Phase 1 — Black Screen
     * ---------------------------------------------------
     */

    framebuffer_clear(0x00000000);

    xk_sleep(SPLASH_BLACK_TIME_MS);

    /*
     * ---------------------------------------------------
     * Phase 2 — XyrisOS Splash
     * ---------------------------------------------------
     */

    framebuffer_clear(0x00000000);

    /*
     * Center the 512x512 XyrisOS logo.
     *
     * The vertical offset leaves comfortable space
     * for the larger "Booting" text and loader.
     */
    const int logo_x =
        ((int)framebuffer_width() -
         SPLASH_LOGO_WIDTH) / 2;

    const int logo_y =
        ((int)framebuffer_height() -
         SPLASH_LOGO_HEIGHT) / 2 - 35;

    draw_image_scaled(
        logo_x,
        logo_y,
        &xyris_logo,
        SPLASH_LOGO_WIDTH,
        SPLASH_LOGO_HEIGHT
    );

    /*
     * ---------------------------------------------------
     * "Booting"
     * ---------------------------------------------------
     *
     * 7 characters × 16 pixels = 112 pixels.
     *
     * The text is centered independently from the logo.
     */

    const int booting_width = 7 * 16;

    const int text_x =
        ((int)framebuffer_width() -
         booting_width) / 2;

    const int text_y =
        logo_y +
        SPLASH_LOGO_HEIGHT +
        20;

    splash_draw_booting(
        text_x,
        text_y,
        SPLASH_TEXT_COLOR
    );

    /*
     * ---------------------------------------------------
     * Loading Indicator
     * ---------------------------------------------------
     */

    const int loader_y =
        text_y +
        (FONT_HEIGHT * 2) +
        14;

    /*
     * Update the loader every 250 ms.
     *
     * The logo and "Booting" text remain completely
     * untouched.
     */
    const uint32_t frame_time = 250;

    uint32_t elapsed = 0;
    uint32_t frame = 0;

    while (elapsed < SPLASH_DISPLAY_TIME_MS)
    {
        /*
         * Clear only the small loader area.
         *
         * This prevents any movement or redraw of the
         * logo and "Booting" text.
         */
        framebuffer_fill_rect(
            ((int)framebuffer_width() / 2) - 40,
            loader_y - 2,
            80,
            LOADER_SIZE + 4,
            0x00000000
        );

        splash_draw_loader(
            (int)framebuffer_width() / 2,
            loader_y,
            frame % LOADER_COUNT
        );

        xk_sleep(frame_time);

        elapsed += frame_time;
        frame++;
    }

    /*
     * Leave a completely clean framebuffer for the
     * static boot-status page that follows.
     */
    framebuffer_clear(0x00000000);
}