#include "ui.h"
#include "../graphics/framebuffer.h"

void ui_init(void)
{
    /* Reserved for future initialization */
}

int screen_width(void)
{
    return framebuffer_width();
}

int screen_height(void)
{
    return framebuffer_height();
}

int center_x(int width)
{
    return (screen_width() - width) / 2;
}

int center_y(int height)
{
    return (screen_height() - height) / 2;
}