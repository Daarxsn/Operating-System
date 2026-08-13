#include <stdio.h>

#include "../simulator/include/simulator.h"
#include "../simulator/include/system.h"
#include "../simulator/include/cpu.h"
#include "../simulator/include/memory.h"
#include "../simulator/include/ukom_sim.h"

#define ASSERT(condition, message)        \
    do                                    \
    {                                     \
        if (!(condition))                 \
        {                                 \
            printf("[FAIL] %s\n", message); \
            return 1;                     \
        }                                 \
        printf("[PASS] %s\n", message);   \
    } while (0)

int test_simulator_commands(void)
{
    UKOMSimObject *object;

    printf("\nRunning Simulator Command Tests...\n");

    /*
     * Initialize the simulator.
     */
    simulator_init();

    /*
     * Test help command.
     */
    ASSERT(
        simulator_execute("help") == 0,
        "Help command should execute successfully"
    );

    /*
     * Test status command.
     */
    ASSERT(
        simulator_execute("status") == 0,
        "Status command should execute successfully"
    );

    /*
     * Test boot command.
     */
    system_state.kernel_loaded = 0;

    ASSERT(
        simulator_execute("boot") == 0,
        "Boot command should execute successfully"
    );

    ASSERT(
        system_state.kernel_loaded == 1,
        "Boot command should load the kernel"
    );

    /*
     * Test boot command when kernel is already loaded.
     */
    ASSERT(
        simulator_execute("boot") == 0,
        "Boot command should be safe when kernel is already loaded"
    );

    ASSERT(
        system_state.kernel_loaded == 1,
        "Kernel should remain loaded after repeated boot"
    );

    /*
     * Test UKOM object creation command.
     */
    simulator_execute("reset");

    ASSERT(
        simulator_execute("ukom create process") == 0,
        "UKOM create command should execute successfully"
    );

    object = ukom_sim_find(1);

    ASSERT(
        object != NULL,
        "UKOM create command should create an object"
    );

    ASSERT(
        object->type == UKOM_SIM_OBJECT_PROCESS,
        "UKOM create command should create the requested object type"
    );

    ASSERT(
        object->ref_count == 1,
        "UKOM object should start with one reference"
    );

    /*
     * Test UKOM find command.
     */
    ASSERT(
        simulator_execute("ukom find 1") == 0,
        "UKOM find command should execute successfully"
    );

    ASSERT(
        ukom_sim_exists(1),
        "UKOM find command should locate the created object"
    );

    /*
     * Test UKOM retain command.
     */
    ASSERT(
        simulator_execute("ukom retain 1") == 0,
        "UKOM retain command should execute successfully"
    );

    object = ukom_sim_find(1);

    ASSERT(
        object->ref_count == 2,
        "UKOM retain command should increase reference count"
    );

    /*
     * Test UKOM release command.
     */
    ASSERT(
        simulator_execute("ukom release 1") == 0,
        "UKOM release command should execute successfully"
    );

    object = ukom_sim_find(1);

    ASSERT(
        object->ref_count == 1,
        "UKOM release command should decrease reference count"
    );

    /*
     * Test UKOM destroy command.
     */
    ASSERT(
        simulator_execute("ukom destroy 1") == 0,
        "UKOM destroy command should execute successfully"
    );

    ASSERT(
        !ukom_sim_exists(1),
        "UKOM destroy command should remove the object"
    );

    /*
     * Test UKOM reset integration.
     */
    ASSERT(
        simulator_execute("ukom create process") == 0,
        "UKOM object should be created before reset"
    );

    ASSERT(
        ukom_sim_count() == 1,
        "UKOM registry should contain one object before reset"
    );

    /*
     * Test reset command.
     */
    ASSERT(
        simulator_execute("reset") == 0,
        "Reset command should execute successfully"
    );

    ASSERT(
        system_state.kernel_loaded == 0,
        "Reset should unload the kernel"
    );

    ASSERT(
        system_state.cpu_online == 1,
        "Reset should restore CPU state"
    );

    ASSERT(
        system_state.memory_online == 1,
        "Reset should restore memory state"
    );

    ASSERT(
        system_state.display_online == 1,
        "Reset should restore display state"
    );

    ASSERT(
        system_state.ram_mb == 64,
        "Reset should restore RAM configuration"
    );

    ASSERT(
        ukom_sim_count() == 0,
        "Reset should clear all UKOM objects"
    );

    ASSERT(
        !ukom_sim_exists(1),
        "Reset should remove UKOM objects from the registry"
    );

    /*
     * Test repeated reset.
     */
    ASSERT(
        simulator_execute("reset") == 0,
        "Repeated reset should execute safely"
    );

    ASSERT(
        system_state.kernel_loaded == 0,
        "Repeated reset should keep kernel unloaded"
    );

    ASSERT(
        ukom_sim_count() == 0,
        "Repeated reset should keep the UKOM registry empty"
    );

    /*
     * Test unknown command handling.
     */
    ASSERT(
        simulator_execute("invalid_command") == 0,
        "Unknown command should be handled safely"
    );

    /*
     * Test empty command handling.
     */
    ASSERT(
        simulator_execute("") == 0,
        "Empty command should be handled safely"
    );

    /*
     * Test NULL command handling.
     */
    ASSERT(
        simulator_execute(NULL) == 0,
        "NULL command should be handled safely"
    );

    /*
     * Test UKOM command without a subcommand.
     */
    ASSERT(
        simulator_execute("ukom") == 0,
        "UKOM command without a subcommand should be handled safely"
    );

    /*
     * Test UKOM create command without an object type.
     */
    ASSERT(
        simulator_execute("ukom create") == 0,
        "UKOM create without an object type should be handled safely"
    );

    /*
     * Test invalid UKOM object type.
     */
    ASSERT(
        simulator_execute("ukom create invalid_type") == 0,
        "Invalid UKOM object type should be handled safely"
    );

    /*
     * Test finding a nonexistent UKOM object.
     */
    ASSERT(
        simulator_execute("ukom find 999") == 0,
        "Finding a nonexistent UKOM object should be handled safely"
    );

    /*
     * Test retaining a nonexistent UKOM object.
     */
    ASSERT(
        simulator_execute("ukom retain 999") == 0,
        "Retaining a nonexistent UKOM object should be handled safely"
    );

    /*
     * Test releasing a nonexistent UKOM object.
     */
    ASSERT(
        simulator_execute("ukom release 999") == 0,
        "Releasing a nonexistent UKOM object should be handled safely"
    );

    /*
     * Test destroying a nonexistent UKOM object.
     */
    ASSERT(
        simulator_execute("ukom destroy 999") == 0,
        "Destroying a nonexistent UKOM object should be handled safely"
    );

    /*
     * Test creating multiple UKOM objects.
     */
    ASSERT(
        simulator_execute("ukom create thread") == 0,
        "UKOM should create a thread object"
    );

    ASSERT(
        simulator_execute("ukom create resource") == 0,
        "UKOM should create a resource object"
    );

    ASSERT(
        ukom_sim_count() == 2,
        "UKOM registry should track multiple objects"
    );

    /*
     * Clean up objects created by edge-case tests.
     */
    ASSERT(
        simulator_execute("reset") == 0,
        "Reset should clean up edge-case test objects"
    );

    ASSERT(
        ukom_sim_count() == 0,
        "UKOM reset should leave the registry empty"
    );

    /*
     * Test exit command.
     */
    ASSERT(
        simulator_execute("exit") == 1,
        "Exit command should return exit status"
    );

    printf("Simulator command tests passed!\n");

    return 0;
}