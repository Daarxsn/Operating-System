#include <xyris/thread.h>

int main(void)
{
    if (xyris_thread_id_valid(XYRIS_INVALID_TID) != XYRIS_FALSE)
        return 1;

    return xyris_thread_id_valid((xyris_tid_t)1u) == XYRIS_TRUE ? 0 : 1;
}
