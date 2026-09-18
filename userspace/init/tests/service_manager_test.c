#include "../service_manager.h"

static int starts;
static int stops;
static int fail_start;

static int start_service(void *context)
{
    (void)context;
    ++starts;
    return fail_start ? -1 : 0;
}

static int stop_service(void *context)
{
    (void)context;
    ++stops;
    return 0;
}

static int expect_state(const char *name, xyris_service_state_t state)
{
    const xyris_service_t *service = xyris_service_find(name);
    return service != NULL && service->state == state ? 0 : 1;
}

int main(void)
{
    const char *dependent_dependencies[] = { "base" };
    const char *unknown_dependencies[] = { "missing" };
    const char *cycle_a_dependencies[] = { "cycle-b" };
    const char *cycle_b_dependencies[] = { "cycle-a" };

    xyris_service_manager_initialize();

    if (xyris_service_register("base", NULL, 0, start_service, stop_service, NULL) != 0)
        return 1;
    if (xyris_service_register("dependent", dependent_dependencies, 1, start_service, stop_service, NULL) != 0)
        return 2;
    if (xyris_service_register("dependent", NULL, 0, start_service, stop_service, &starts) == 0)
        return 3;
    if (xyris_service_start("dependent") == 0)
        return 4;
    if (xyris_service_start_all() != 0)
        return 5;
    if (expect_state("base", XYRIS_SERVICE_RUNNING) != 0 ||
        expect_state("dependent", XYRIS_SERVICE_RUNNING) != 0)
        return 6;
    if (starts != 2)
        return 7;

    if (xyris_service_stop_all() != 0 || stops != 2)
        return 8;

    xyris_service_manager_initialize();
    if (xyris_service_register("orphan", unknown_dependencies, 1, NULL, NULL, NULL) != 0)
        return 9;
    if (xyris_service_start_all() == 0 ||
        expect_state("orphan", XYRIS_SERVICE_FAILED) != 0)
        return 10;

    xyris_service_manager_initialize();
    if (xyris_service_register("cycle-a", cycle_a_dependencies, 1, NULL, NULL, NULL) != 0)
        return 11;
    if (xyris_service_register("cycle-b", cycle_b_dependencies, 1, NULL, NULL, NULL) != 0)
        return 12;
    if (xyris_service_start_all() == 0 ||
        expect_state("cycle-a", XYRIS_SERVICE_FAILED) != 0 ||
        expect_state("cycle-b", XYRIS_SERVICE_FAILED) != 0)
        return 13;

    xyris_service_manager_initialize();
    fail_start = 1;
    if (xyris_service_register("failing", NULL, 0, start_service, stop_service, NULL) != 0)
        return 14;
    if (xyris_service_start("failing") == 0 ||
        expect_state("failing", XYRIS_SERVICE_FAILED) != 0)
        return 15;

    return 0;
}
