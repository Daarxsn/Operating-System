#include <xyris/security.h>

int main(void)
{
    if (xyris_capability_valid(XYRIS_INVALID_CAP) != XYRIS_FALSE)
        return 1;
    if (xyris_capability_valid((xyris_capability_t)1u) != XYRIS_TRUE)
        return 1;
    if (xyris_capability_valid((xyris_capability_t)UINT64_MAX) != XYRIS_TRUE)
        return 1;
    return 0;
}
