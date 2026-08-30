#include <xyris/memory.h>

int main()
{
    if (xyris_memory_address_valid(0u) != XYRIS_FALSE)
        return 1;

    if (xyris_memory_address_valid(1u) != XYRIS_TRUE)
        return 1;

    if (xyris_memory_range_valid(0u, 0u) != XYRIS_FALSE)
        return 1;

    if (xyris_memory_range_valid(0x1000u, 0x1000u) != XYRIS_TRUE)
        return 1;

    if (xyris_memory_range_valid(UINT64_MAX, 2u) != XYRIS_FALSE)
        return 1;

    return 0;
}
