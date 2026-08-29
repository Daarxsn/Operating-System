#include <xyris/timers.h>

int main(void)
{
    if (xyris_timer_handle_valid(XYRIS_INVALID_HANDLE) != XYRIS_FALSE)
        return 1;

    if (xyris_timer_handle_valid((xyris_handle_t)1u) != XYRIS_TRUE)
        return 1;

    if (xyris_timer_handle_valid((xyris_handle_t)UINT64_MAX) != XYRIS_TRUE)
        return 1;

    return 0;
}
