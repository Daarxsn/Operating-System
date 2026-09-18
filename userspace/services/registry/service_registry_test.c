#include "service_registry.h"

int main(void)
{
    xyris_service_descriptor_t descriptor;

    xyris_service_registry_initialize();

    if (xyris_service_registry_count() != 0u)
        return 1;

    if (xyris_service_registry_publish(
            "system-service", 1u,
            (xyris_handle_t)10u, (xyris_capability_t)10u) != 0)
        return 2;

    if (xyris_service_registry_lookup("system-service", &descriptor) != 0)
        return 3;

    if (descriptor.version != 1u ||
        descriptor.endpoint != (xyris_handle_t)10u ||
        descriptor.capability != (xyris_capability_t)10u)
        return 4;

    if (xyris_service_registry_lookup("missing", &descriptor) == 0)
        return 5;

    if (xyris_service_registry_publish(
            "duplicate", 1u,
            XYRIS_INVALID_HANDLE, (xyris_capability_t)20u) == 0)
        return 6;

    if (xyris_service_registry_publish(
            "system-service", 1u,
            XYRIS_INVALID_HANDLE, XYRIS_INVALID_CAP) == 0)
        return 7;

    if (xyris_service_registry_unpublish("system-service") != 0)
        return 8;

    if (xyris_service_registry_count() != 0u)
        return 9;

    if (xyris_service_registry_unpublish("system-service") == 0)
        return 10;

    return 0;
}
