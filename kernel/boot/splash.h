#ifndef XYRIS_SPLASH_H
#define XYRIS_SPLASH_H

/*
 * Display the XyrisOS boot splash.
 *
 * Phase 1:
 *   Black screen for approximately 1 second.
 *
 * Phase 2:
 *   XyrisOS logo + "Booting" + loading indicator
 *   for approximately 6 seconds.
 *
 * The logo and text remain completely static.
 * Only the loading indicator is animated.
 */
void boot_splash_show(void);

#endif