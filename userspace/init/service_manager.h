#ifndef XYRIS_USERSPACE_SERVICE_MANAGER_H
#define XYRIS_USERSPACE_SERVICE_MANAGER_H

#include <stddef.h>
#include <stdint.h>

#define XYRIS_SERVICE_NAME_MAX          32u
#define XYRIS_SERVICE_MAX               16u
#define XYRIS_SERVICE_MAX_DEPENDENCIES   4u

typedef enum
{
    XYRIS_SERVICE_REGISTERED = 0,
    XYRIS_SERVICE_STARTING,
    XYRIS_SERVICE_RUNNING,
    XYRIS_SERVICE_STOPPING,
    XYRIS_SERVICE_STOPPED,
    XYRIS_SERVICE_FAILED
} xyris_service_state_t;

typedef int (*xyris_service_start_fn)(void *context);
typedef int (*xyris_service_stop_fn)(void *context);

typedef struct
{
    char name[XYRIS_SERVICE_NAME_MAX];
    xyris_service_state_t state;
    const char *dependencies[XYRIS_SERVICE_MAX_DEPENDENCIES];
    size_t dependency_count;
    xyris_service_start_fn start;
    xyris_service_stop_fn stop;
    void *context;
} xyris_service_t;

void xyris_service_manager_initialize(void);

int xyris_service_register(
    const char *name,
    const char *const *dependencies,
    size_t dependency_count,
    xyris_service_start_fn start,
    xyris_service_stop_fn stop,
    void *context
);

int xyris_service_start(const char *name);
int xyris_service_start_all(void);
int xyris_service_stop(const char *name);
int xyris_service_stop_all(void);

const xyris_service_t *xyris_service_find(const char *name);
size_t xyris_service_count(void);

#endif /* XYRIS_USERSPACE_SERVICE_MANAGER_H */
