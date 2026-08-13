#include <stdio.h>

#include "ukom_sim.h"

static UKOMSimObject object_registry[UKOM_SIM_MAX_OBJECTS];

static unsigned int object_count = 0;

static unsigned long long next_object_id = 1;

void ukom_sim_init(void)
{
    object_count = 0;
    next_object_id = 1;

    for (unsigned int i = 0; i < UKOM_SIM_MAX_OBJECTS; i++)
    {
        object_registry[i].id = 0;
        object_registry[i].type = UKOM_SIM_OBJECT_NONE;
        object_registry[i].state = UKOM_SIM_STATE_DESTROYED;
        object_registry[i].ref_count = 0;
    }
}

UKOMSimObject *ukom_sim_create(UKOMSimObjectType type)
{
    if (object_count >= UKOM_SIM_MAX_OBJECTS)
    {
        return NULL;
    }

    for (unsigned int i = 0; i < UKOM_SIM_MAX_OBJECTS; i++)
    {
        if (object_registry[i].id == 0)
        {
            object_registry[i].id = next_object_id++;
            object_registry[i].type = type;
            object_registry[i].state = UKOM_SIM_STATE_CREATED;
            object_registry[i].ref_count = 1;

            object_count++;

            return &object_registry[i];
        }
    }

    return NULL;
}

UKOMSimObject *ukom_sim_find(unsigned long long id)
{
    for (unsigned int i = 0; i < UKOM_SIM_MAX_OBJECTS; i++)
    {
        if (object_registry[i].id == id)
        {
            return &object_registry[i];
        }
    }

    return NULL;
}

int ukom_sim_exists(unsigned long long id)
{
    return ukom_sim_find(id) != NULL;
}

unsigned int ukom_sim_count(void)
{
    return object_count;
}

void ukom_sim_retain(UKOMSimObject *object)
{
    if (object == NULL)
    {
        return;
    }

    object->ref_count++;
}

void ukom_sim_release(UKOMSimObject *object)
{
    if (object == NULL)
    {
        return;
    }

    if (object->ref_count > 0)
    {
        object->ref_count--;

        if (object->ref_count == 0)
        {
            ukom_sim_destroy(object->id);
        }
    }
}

void ukom_sim_destroy(unsigned long long id)
{
    UKOMSimObject *object = ukom_sim_find(id);

    if (object == NULL)
    {
        return;
    }

    object->id = 0;
    object->type = UKOM_SIM_OBJECT_NONE;
    object->state = UKOM_SIM_STATE_DESTROYED;
    object->ref_count = 0;

    object_count--;
}

int ukom_sim_activate(unsigned long long id)
{
    UKOMSimObject *object = ukom_sim_find(id);

    if (object == NULL)
    {
        return 0;
    }

    if (object->state == UKOM_SIM_STATE_DESTROYED)
    {
        return 0;
    }

    object->state = UKOM_SIM_STATE_ACTIVE;

    return 1;
}

int ukom_sim_suspend(unsigned long long id)
{
    UKOMSimObject *object = ukom_sim_find(id);

    if (object == NULL)
    {
        return 0;
    }

    if (object->state == UKOM_SIM_STATE_DESTROYED)
    {
        return 0;
    }

    object->state = UKOM_SIM_STATE_SUSPENDED;

    return 1;
}

const char *ukom_sim_type_name(UKOMSimObjectType type)
{
    switch (type)
    {
        case UKOM_SIM_OBJECT_PROCESS:
            return "PROCESS";

        case UKOM_SIM_OBJECT_THREAD:
            return "THREAD";

        case UKOM_SIM_OBJECT_DRIVER:
            return "DRIVER";

        case UKOM_SIM_OBJECT_DEVICE:
            return "DEVICE";

        case UKOM_SIM_OBJECT_TIMER:
            return "TIMER";

        case UKOM_SIM_OBJECT_EVENT:
            return "EVENT";

        case UKOM_SIM_OBJECT_RESOURCE:
            return "RESOURCE";

        case UKOM_SIM_OBJECT_CUSTOM:
            return "CUSTOM";

        default:
            return "NONE";
    }
}

const char *ukom_sim_state_name(UKOMSimObjectState state)
{
    switch (state)
    {
        case UKOM_SIM_STATE_CREATED:
            return "CREATED";

        case UKOM_SIM_STATE_ACTIVE:
            return "ACTIVE";

        case UKOM_SIM_STATE_SUSPENDED:
            return "SUSPENDED";

        case UKOM_SIM_STATE_DESTROYED:
            return "DESTROYED";

        default:
            return "UNKNOWN";
    }
}

void ukom_sim_print_objects(void)
{
    printf("\nUKOM Objects\n");
    printf("------------------\n");

    if (object_count == 0)
    {
        printf("No active objects.\n");
        return;
    }

    for (unsigned int i = 0; i < UKOM_SIM_MAX_OBJECTS; i++)
    {
        if (object_registry[i].id != 0)
        {
            printf(
                "ID: %llu | Type: %s | State: %s | References: %u\n",
                object_registry[i].id,
                ukom_sim_type_name(object_registry[i].type),
                ukom_sim_state_name(object_registry[i].state),
                object_registry[i].ref_count
            );
        }
    }
}