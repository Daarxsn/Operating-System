#include <xyris/networking.h>

int main(void)
{
    if (xyris_network_fd_valid(XYRIS_INVALID_FD) != XYRIS_FALSE)
        return 1;
    if (xyris_network_fd_valid((xyris_fd_t)0) != XYRIS_TRUE)
        return 1;
    if (xyris_network_fd_valid((xyris_fd_t)42) != XYRIS_TRUE)
        return 1;
    return 0;
}
