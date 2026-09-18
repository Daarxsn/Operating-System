#include "service_registry.h"

static xyris_service_descriptor_t registry[XYRIS_SERVICE_REGISTRY_MAX];
static size_t registry_count_value;
static int registry_initialized;

static int service_name_valid(const char *name)
{
    size_t index = 0;

    if (name == NULL || name[0] == '\0')
        return 0;

    while (name[index] != '\0')
    {
        if (index + 1u >= XYRIS_SERVICE_REGISTRY_NAME_MAX)
            return 0;
        ++index;
    }

    return 1;
}

static int service_name_equal(const char *left, const char *right)
{
    size_t index = 0;

    if (left == NULL || right == NULL)
        return 0;

    while (left[index] != '\0' && right[index] != '\0')
    {
        if (left[index] != right[index])
            return 0;
        ++index;
    }

    return left[index] == right[index];
}

static void service_name_copy(char *destination, const char *source)
{
    size_t index = 0;

    while (source[index] != '\0' && index + 1u < XYRIS_SERVICE_REGISTRY_NAME_MAX)
    {
        destination[index] = source[index];
        ++index;
    }

    destination[index] = '\0';
}

static int service_find_index(const char *name, size_t *index_out)
{
    size_t index;

    if (!service_name_valid(name))
        return -1;

    for (index = 0; index < registry_count_value; ++index)
    {
        if (service_name_equal(registry[index].name, name))
        {
            if (index_out != NULL)
                *index_out = index;
            return 0;
        }
    }

    return -1;
}

void xyris_service_registry_initialize(void)
{
    size_t index;

    registry_count_value = 0;
    registry_initialized = 1;

    for (index = 0; index < XYRIS_SERVICE_REGISTRY_MAX; ++index)
    {
        registry[index].name[0] = '\0';
        registry[index].version = 0u;
        registry[index].endpoint = XYRIS_INVALID_HANDLE;
        registry[index].capability = XYRIS_INVALID_CAP;
    }
}

int xyris_service_registry_publish(
    const char *name,
    xyris_u32 version,
    xyris_handle_t endpoint,
    xyris_capability_t capability
)
{
    xyris_service_descriptor_t *descriptor;
    size_t index;

    if (!registry_initialized ||
        !service_name_valid(name) ||
        version == 0u ||
        registry_count_value >= XYRIS_SERVICE_REGISTRY_MAX)
        return -1;

    if (service_find_index(name, &index) == 0)
        return -1;

    if ((endpoint == XYRIS_INVALID_HANDLE) !=
        (capability == XYRIS_INVALID_CAP))
        return -1;

    descriptor = &registry[registry_count_value];
    service_name_copy(descriptor->name, name);
    descriptor->version = version;
    descriptor->endpoint = endpoint;
    descriptor->capability = capability;
    ++registry_count_value;

    return 0;
}

int xyris_service_registry_unpublish(const char *name)
{
    size_t index;
    size_t next;

    if (!registry_initialized || service_find_index(name, &index) != 0)
        return -1;

    for (next = index + 1u; next < registry_count_value; ++next)
        registry[next - 1u] = registry[next];

    --registry_count_value;
    registry[registry_count_value].name[0] = '\0';
    registry[registry_count_value].version = 0u;
    registry[registry_count_value].endpoint = XYRIS_INVALID_HANDLE;
    registry[registry_count_value].capability = XYRIS_INVALID_CAP;

    return 0;
}

int xyris_service_registry_lookup(
    const char *name,
    xyris_service_descriptor_t *descriptor
)
{
    size_t index;

    if (!registry_initialized || descriptor == NULL ||
        service_find_index(name, &index) != 0)
        return -1;

    *descriptor = registry[index];
    return 0;
}

size_t xyris_service_registry_count(void)
{
    return registry_count_value;
}

int xyris_service_registry_get(
    size_t index,
    xyris_service_descriptor_t *descriptor
)
{
    if (!registry_initialized ||
        descriptor == NULL ||
        index >= registry_count_value)
        return -1;

    *descriptor = registry[index];
    return 0;
}
