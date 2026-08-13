#ifndef XYRIS_UI_H
#define XYRIS_UI_H

#include <stdint.h>

void ui_init(void);

int screen_width(void);

int screen_height(void);

int center_x(int width);

int center_y(int height);

#endif