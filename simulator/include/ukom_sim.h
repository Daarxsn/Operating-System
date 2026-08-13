#ifndef UKOM_SIM_H
#define UKOM_SIM_H

#define UKOM_SIM_MAX_OBJECTS 256

typedef enum
{
    UKOM_SIM_OBJECT_NONE = 0,
    UKOM_SIM_OBJECT_PROCESS,
    UKOM_SIM_OBJECT_THREAD,
    UKOM_SIM_OBJECT_DRIVER,
    UKOM_SIM_OBJECT_DEVICE,
    UKOM_SIM_OBJECT_TIMER,
    UKOM_SIM_OBJECT_EVENT,
    UKOM_SIM_OBJECT_RESOURCE,
    UKOM_SIM_OBJECT_CUSTOM

} UKOMSimObjectType;

typedef enum
{
    UKOM_SIM_STATE_CREATED = 0,
    UKOM_SIM_STATE_ACTIVE,
    UKOM_SIM_STATE_SUSPENDED,
    UKOM_SIM_STATE_DESTROYED

} UKOMSimObjectState;

typedef struct
{
    unsigned long long id;
    UKOMSimObjectType type;
    UKOMSimObjectState state;
    unsigned int ref_count;

} UKOMSimObject;

void ukom_sim_init(void);

UKOMSimObject *ukom_sim_create(UKOMSimObjectType type);

UKOMSimObject *ukom_sim_find(unsigned long long id);

int ukom_sim_exists(unsigned long long id);

unsigned int ukom_sim_count(void);

void ukom_sim_retain(UKOMSimObject *object);

void ukom_sim_release(UKOMSimObject *object);

void ukom_sim_destroy(unsigned long long id);

int ukom_sim_activate(unsigned long long id);

int ukom_sim_suspend(unsigned long long id);

const char *ukom_sim_type_name(UKOMSimObjectType type);

const char *ukom_sim_state_name(UKOMSimObjectState state);

void ukom_sim_print_objects(void);

#endif