#include <xyris/timers.h>

int main()
{
    if (xyris_timer_handle_valid(XYRIS_INVALID_HANDLE) != XYRIS_FALSE)
        return 1;

    if (xyris_timer_handle_valid(static_cast<xyris_handle_t>(1u)) != XYRIS_TRUE)
        return 1;

    if (xyris_timer_handle_valid(static_cast<xyris_handle_t>(~0ull)) != XYRIS_TRUE)
        return 1;

    return 0;
}
