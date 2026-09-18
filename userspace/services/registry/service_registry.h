#ifndef XYRIS_USERSPACE_SERVICE_REGISTRY_H
#define XYRIS_USERSPACE_SERVICE_REGISTRY_H

#include <stddef.h>

#include <xyris/core.h>

#define XYRIS_SERVICE_REGISTRY_NAME_MAX 32u
#define XYRIS_SERVICE_REGISTRY_MAX 16u

/* An endpoint is optional until a service is hosted by an independent process. */
typedef struct
{
    char name[XYRIS_SERVICE_REGISTRY_NAME_MAX];
    xyris_u32 version;
    xyris_handle_t endpoint;
    xyris_capability_t capability;
} xyris_service_descriptor_t;

void xyris_service_registry_initialize(void);

int xyris_service_registry_publish(
    const char *name,
    xyris_u32 version,
    xyris_handle_t endpoint,
    xyris_capability_t capability
);

int xyris_service_registry_unpublish(const char *name);

int xyris_service_registry_lookup(
    const char *name,
    xyris_service_descriptor_t *descriptor
);

size_t xyris_service_registry_count(void);

int xyris_service_registry_get(
    size_t index,
    xyris_service_descriptor_t *descriptor
);

#endif /* XYRIS_USERSPACE_SERVICE_REGISTRY_H */
