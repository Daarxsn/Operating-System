#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "simulator.h"
#include "cpu.h"
#include "memory.h"
#include "display.h"
#include "system.h"
#include "ukom_sim.h"

static UKOMSimObjectType parse_ukom_type(const char *type)
{
    if (strcmp(type, "process") == 0)
        return UKOM_SIM_OBJECT_PROCESS;

    if (strcmp(type, "thread") == 0)
        return UKOM_SIM_OBJECT_THREAD;

    if (strcmp(type, "driver") == 0)
        return UKOM_SIM_OBJECT_DRIVER;

    if (strcmp(type, "device") == 0)
        return UKOM_SIM_OBJECT_DEVICE;

    if (strcmp(type, "timer") == 0)
        return UKOM_SIM_OBJECT_TIMER;

    if (strcmp(type, "event") == 0)
        return UKOM_SIM_OBJECT_EVENT;

    if (strcmp(type, "resource") == 0)
        return UKOM_SIM_OBJECT_RESOURCE;

    if (strcmp(type, "custom") == 0)
        return UKOM_SIM_OBJECT_CUSTOM;

    return UKOM_SIM_OBJECT_NONE;
}

static int parse_ukom_id(
    const char *value,
    unsigned long long *id
)
{
    char *end;

    if (value == NULL || id == NULL || *value == '\0')
    {
        return 0;
    }

    *id = strtoull(value, &end, 10);

    if (*end != '\0')
    {
        return 0;
    }

    return 1;
}

void simulator_init(void)
{
    printf("Initializing XyrisSim...\n");

    system_init();
    cpu_init();
    memory_init();
    display_init();
    ukom_sim_init();

    printf("\nSystem Ready!\n");
}

void simulator_reset(void)
{
    printf("\nResetting XyrisSim...\n");

    system_init();
    cpu_init();
    memory_init();
    display_init();
    ukom_sim_init();

    printf("\nSystem Reset Complete!\n");
}

int simulator_execute(const char *command)
{
    char command_copy[100];
    char *operation;
    char *argument;

    if (command == NULL)
    {
        printf("Invalid command.\n");
        return 0;
    }

    strncpy(
        command_copy,
        command,
        sizeof(command_copy) - 1
    );

    command_copy[sizeof(command_copy) - 1] = '\0';

    operation = strtok(command_copy, " ");
    argument = strtok(NULL, " ");

    if (operation == NULL)
    {
        printf("Invalid command.\n");
        return 0;
    }

    if (strcmp(operation, "help") == 0)
    {
        printf("\nAvailable Commands\n");
        printf("------------------\n");
        printf("help\n");
        printf("status\n");
        printf("boot\n");
        printf("reset\n");
        printf("clear\n");
        printf("ukom create <type>\n");
        printf("ukom list\n");
        printf("ukom find <id>\n");
        printf("ukom retain <id>\n");
        printf("ukom release <id>\n");
        printf("ukom destroy <id>\n");
        printf("exit\n");
    }

    else if (strcmp(operation, "status") == 0)
    {
        printf("\nSystem Status\n");
        printf("------------------\n");
        printf(
            "CPU: %s\n",
            system_state.cpu_online ? "ONLINE" : "OFFLINE"
        );
        printf(
            "Memory: %s\n",
            system_state.memory_online ? "ONLINE" : "OFFLINE"
        );
        printf(
            "Display: %s\n",
            system_state.display_online ? "ONLINE" : "OFFLINE"
        );
        printf(
            "RAM: %d MB\n",
            system_state.ram_mb
        );
        printf(
            "Kernel: %s\n",
            system_state.kernel_loaded ? "LOADED" : "NOT LOADED"
        );
    }

    else if (strcmp(operation, "boot") == 0)
    {
        printf("\nLoading Kernel...\n");
        printf("Initializing Scheduler...\n");
        printf("Initializing Memory Manager...\n");
        printf("Starting Kernel...\n");

        system_state.kernel_loaded = 1;

        printf("\nKernel Boot Successful!\n");
    }

    else if (strcmp(operation, "reset") == 0)
    {
        simulator_reset();
    }

    else if (strcmp(operation, "clear") == 0)
    {
        for (int i = 0; i < 40; i++)
        {
            printf("\n");
        }
    }

    else if (strcmp(operation, "ukom") == 0)
    {
        char *ukom_argument = argument;
        char *value = strtok(NULL, " ");

        if (ukom_argument == NULL)
        {
            printf(
                "Usage: ukom "
                "<create|list|find|retain|release|destroy> "
                "[value]\n"
            );
        }

        else if (strcmp(ukom_argument, "create") == 0)
        {
            UKOMSimObjectType type;
            UKOMSimObject *object;

            if (value == NULL)
            {
                printf("Usage: ukom create <type>\n");
            }
            else
            {
                type = parse_ukom_type(value);

                if (type == UKOM_SIM_OBJECT_NONE)
                {
                    printf("Unknown UKOM object type.\n");
                }
                else
                {
                    object = ukom_sim_create(type);

                    if (object == NULL)
                    {
                        printf(
                            "UKOM object creation failed.\n"
                        );
                    }
                    else
                    {
                        printf(
                            "UKOM object created: "
                            "ID=%llu Type=%s References=%u\n",
                            object->id,
                            ukom_sim_type_name(object->type),
                            object->ref_count
                        );
                    }
                }
            }
        }

        else if (strcmp(ukom_argument, "list") == 0)
        {
            ukom_sim_print_objects();
        }

        else if (strcmp(ukom_argument, "find") == 0)
        {
            unsigned long long id;
            UKOMSimObject *object;

            if (value == NULL)
            {
                printf("Usage: ukom find <id>\n");
            }
            else if (!parse_ukom_id(value, &id))
            {
                printf("Invalid UKOM object ID.\n");
            }
            else
            {
                object = ukom_sim_find(id);

                if (object == NULL)
                {
                    printf("UKOM object not found.\n");
                }
                else
                {
                    printf(
                        "UKOM object found: "
                        "ID=%llu Type=%s References=%u\n",
                        object->id,
                        ukom_sim_type_name(object->type),
                        object->ref_count
                    );
                }
            }
        }

        else if (strcmp(ukom_argument, "retain") == 0)
        {
            unsigned long long id;
            UKOMSimObject *object;

            if (value == NULL)
            {
                printf("Usage: ukom retain <id>\n");
            }
            else if (!parse_ukom_id(value, &id))
            {
                printf("Invalid UKOM object ID.\n");
            }
            else
            {
                object = ukom_sim_find(id);

                if (object == NULL)
                {
                    printf("UKOM object not found.\n");
                }
                else
                {
                    ukom_sim_retain(object);

                    printf(
                        "UKOM object retained: "
                        "ID=%llu References=%u\n",
                        object->id,
                        object->ref_count
                    );
                }
            }
        }

        else if (strcmp(ukom_argument, "release") == 0)
        {
            unsigned long long id;
            UKOMSimObject *object;

            if (value == NULL)
            {
                printf("Usage: ukom release <id>\n");
            }
            else if (!parse_ukom_id(value, &id))
            {
                printf("Invalid UKOM object ID.\n");
            }
            else
            {
                object = ukom_sim_find(id);

                if (object == NULL)
                {
                    printf("UKOM object not found.\n");
                }
                else
                {
                    ukom_sim_release(object);

                    if (ukom_sim_exists(id))
                    {
                        object = ukom_sim_find(id);

                        printf(
                            "UKOM object released: "
                            "ID=%llu References=%u\n",
                            id,
                            object->ref_count
                        );
                    }
                    else
                    {
                        printf(
                            "UKOM object released and destroyed: "
                            "ID=%llu\n",
                            id
                        );
                    }
                }
            }
        }

        else if (strcmp(ukom_argument, "destroy") == 0)
        {
            unsigned long long id;

            if (value == NULL)
            {
                printf("Usage: ukom destroy <id>\n");
            }
            else if (!parse_ukom_id(value, &id))
            {
                printf("Invalid UKOM object ID.\n");
            }
            else if (!ukom_sim_exists(id))
            {
                printf("UKOM object not found.\n");
            }
            else
            {
                ukom_sim_destroy(id);

                printf(
                    "UKOM object destroyed: ID=%llu\n",
                    id
                );
            }
        }

        else
        {
            printf("Unknown UKOM command.\n");
        }
    }

    else if (strcmp(operation, "exit") == 0)
    {
        printf("Exiting XyrisSim...\n");
        return 1;
    }

    else
    {
        printf("Unknown command. Type 'help'.\n");
    }

    return 0;
}