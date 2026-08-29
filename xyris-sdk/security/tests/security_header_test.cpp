#include <xyris/security.h>

int main()
{
    if (xyris_capability_valid(XYRIS_INVALID_CAP) != XYRIS_FALSE)
        return 1;
    if (xyris_capability_valid(static_cast<xyris_capability_t>(1u)) != XYRIS_TRUE)
        return 1;
    if (xyris_capability_valid(static_cast<xyris_capability_t>(~0ull)) != XYRIS_TRUE)
        return 1;
    return 0;
}
