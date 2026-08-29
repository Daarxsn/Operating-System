#include <xyris/devices.h>

int main()
{
    if (xyris_device_handle_valid(XYRIS_INVALID_HANDLE) != XYRIS_FALSE)
        return 1;
    if (xyris_device_handle_valid(static_cast<xyris_handle_t>(1u)) != XYRIS_TRUE)
        return 1;
    if (xyris_device_object_valid(XYRIS_INVALID_OBJECT) != XYRIS_FALSE)
        return 1;
    if (xyris_device_object_valid(static_cast<xyris_object_id_t>(1u)) != XYRIS_TRUE)
        return 1;
    return 0;
}
