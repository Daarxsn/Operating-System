#include "service_manager.h"

static xyris_service_t service_table[XYRIS_SERVICE_MAX];
static size_t service_count_value;
static int service_manager_initialized;

static int service_name_valid(const char *name)
{
    size_t length = 0;

    if (name == NULL || name[0] == '\0')
        return 0;

    while (name[length] != '\0')
    {
        if (length + 1u >= XYRIS_SERVICE_NAME_MAX)
            return 0;
        ++length;
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

static size_t service_name_copy(char *destination, const char *source)
{
    size_t index = 0;

    while (source[index] != '\0' && index + 1u < XYRIS_SERVICE_NAME_MAX)
    {
        destination[index] = source[index];
        ++index;
    }

    destination[index] = '\0';
    return index;
}

static int service_find_index(const char *name, size_t *index_out)
{
    size_t index;

    if (!service_name_valid(name))
        return -1;

    for (index = 0; index < service_count_value; ++index)
    {
        if (service_name_equal(service_table[index].name, name))
        {
            if (index_out != NULL)
                *index_out = index;
            return 0;
        }
    }

    return -1;
}

static int service_dependencies_running(const xyris_service_t *service)
{
    size_t index;

    for (index = 0; index < service->dependency_count; ++index)
    {
        size_t dependency_index;
        const char *dependency = service->dependencies[index];

        if (service_find_index(dependency, &dependency_index) != 0)
            return -1;

        if (service_table[dependency_index].state != XYRIS_SERVICE_RUNNING)
            return 0;
    }

    return 1;
}

void xyris_service_manager_initialize(void)
{
    size_t index;

    service_count_value = 0;
    service_manager_initialized = 1;

    for (index = 0; index < XYRIS_SERVICE_MAX; ++index)
    {
        service_table[index].name[0] = '\0';
        service_table[index].state = XYRIS_SERVICE_STOPPED;
        service_table[index].dependency_count = 0;
        service_table[index].start = NULL;
        service_table[index].stop = NULL;
        service_table[index].context = NULL;
    }
}

int xyris_service_register(
    const char *name,
    const char *const *dependencies,
    size_t dependency_count,
    xyris_service_start_fn start,
    xyris_service_stop_fn stop,
    void *context
)
{
    xyris_service_t *service;
    size_t index;

    if (!service_manager_initialized ||
        !service_name_valid(name) ||
        service_count_value >= XYRIS_SERVICE_MAX ||
        dependency_count > XYRIS_SERVICE_MAX_DEPENDENCIES)
        return -1;

    if (service_find_index(name, &index) == 0)
        return -1;

    if (dependency_count > 0 && dependencies == NULL)
        return -1;

    for (index = 0; index < dependency_count; ++index)
    {
        if (!service_name_valid(dependencies[index]))
            return -1;
    }

    service = &service_table[service_count_value];
    service_name_copy(service->name, name);
    service->state = XYRIS_SERVICE_REGISTERED;
    service->dependency_count = dependency_count;

    for (index = 0; index < dependency_count; ++index)
        service->dependencies[index] = dependencies[index];

    for (; index < XYRIS_SERVICE_MAX_DEPENDENCIES; ++index)
        service->dependencies[index] = NULL;

    service->start = start;
    service->stop = stop;
    service->context = context;
    ++service_count_value;

    return 0;
}

int xyris_service_start(const char *name)
{
    size_t index;
    int dependency_state;
    xyris_service_t *service;

    if (service_find_index(name, &index) != 0)
        return -1;

    service = &service_table[index];

    if (service->state == XYRIS_SERVICE_RUNNING)
        return 0;

    if (service->state != XYRIS_SERVICE_REGISTERED &&
        service->state != XYRIS_SERVICE_STOPPED)
        return -1;

    dependency_state = service_dependencies_running(service);
    if (dependency_state != 1)
        return -1;

    service->state = XYRIS_SERVICE_STARTING;

    if (service->start != NULL && service->start(service->context) != 0)
    {
        service->state = XYRIS_SERVICE_FAILED;
        return -1;
    }

    service->state = XYRIS_SERVICE_RUNNING;
    return 0;
}

int xyris_service_start_all(void)
{
    size_t pass;
    int progress;
    int remaining = 0;
    int failure = 0;

    for (pass = 0; pass < XYRIS_SERVICE_MAX && pass < service_count_value; ++pass)
    {
        size_t index;
        progress = 0;

        for (index = 0; index < service_count_value; ++index)
        {
            xyris_service_t *service = &service_table[index];

            if (service->state != XYRIS_SERVICE_REGISTERED)
                continue;

            if (xyris_service_start(service->name) == 0)
                progress = 1;
        }

        if (!progress)
            break;
    }

    {
        size_t index;
        for (index = 0; index < service_count_value; ++index)
        {
            if (service_table[index].state == XYRIS_SERVICE_REGISTERED)
            {
                service_table[index].state = XYRIS_SERVICE_FAILED;
                failure = 1;
            }
            else if (service_table[index].state == XYRIS_SERVICE_FAILED)
            {
                failure = 1;
            }
        }
    }

    {
        size_t index;
        for (index = 0; index < service_count_value; ++index)
        {
            if (service_table[index].state == XYRIS_SERVICE_RUNNING)
                ++remaining;
        }
    }

    return failure || remaining != (int)service_count_value ? -1 : 0;
}

int xyris_service_stop(const char *name)
{
    size_t index;
    size_t dependent_index;
    xyris_service_t *service;

    if (service_find_index(name, &index) != 0)
        return -1;

    service = &service_table[index];

    if (service->state == XYRIS_SERVICE_STOPPED ||
        service->state == XYRIS_SERVICE_REGISTERED)
        return 0;

    if (service->state != XYRIS_SERVICE_RUNNING)
        return -1;

    for (dependent_index = 0;
         dependent_index < service_count_value;
         ++dependent_index)
    {
        size_t dependency_index;
        size_t dependency_number;
        const xyris_service_t *dependent = &service_table[dependent_index];

        if (dependent->state != XYRIS_SERVICE_RUNNING)
            continue;

        for (dependency_number = 0;
             dependency_number < dependent->dependency_count;
             ++dependency_number)
        {
            if (service_find_index(
                    dependent->dependencies[dependency_number],
                    &dependency_index) == 0 &&
                dependency_index == index)
            {
                return -1;
            }
        }
    }

    service->state = XYRIS_SERVICE_STOPPING;

    if (service->stop != NULL && service->stop(service->context) != 0)
    {
        service->state = XYRIS_SERVICE_FAILED;
        return -1;
    }

    service->state = XYRIS_SERVICE_STOPPED;
    return 0;
}

int xyris_service_stop_all(void)
{
    size_t pass;
    int remaining;

    for (pass = 0; pass < XYRIS_SERVICE_MAX && pass < service_count_value; ++pass)
    {
        size_t index;
        int progress = 0;

        for (index = service_count_value; index > 0; --index)
        {
            if (service_table[index - 1u].state != XYRIS_SERVICE_RUNNING)
                continue;

            if (xyris_service_stop(service_table[index - 1u].name) == 0)
                progress = 1;
        }

        remaining = 0;
        for (index = 0; index < service_count_value; ++index)
        {
            if (service_table[index].state == XYRIS_SERVICE_RUNNING)
                ++remaining;
        }

        if (remaining == 0)
            return 0;
        if (!progress)
            break;
    }

    return -1;
}

const xyris_service_t *xyris_service_find(const char *name)
{
    size_t index;

    if (service_find_index(name, &index) != 0)
        return NULL;

    return &service_table[index];
}

size_t xyris_service_count(void)
{
    return service_count_value;
}
