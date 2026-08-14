#include "foundation_tests.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "foundation/config.h"
#include "foundation/event.h"
#include "foundation/resource.h"
#include "foundation/capability.h"
#include "drivers/driver.h"

#include "../boot/boot.h"

/*
 * ============================================================
 * XyrisOS Foundation and Driver Tests
 * ============================================================
 */

static void test_ok(const char *name, bool condition)
{
    if (condition)
        boot_step_ok(name);
    else
        boot_step_fail(name);
}


/*
 * ============================================================
 * Configuration Manager
 * ============================================================
 */

static void test_configuration_manager(void)
{
    xk_config_init();

    uint64_t value = 0;

    test_ok(
        "Foundation Test: Config Initially Missing",
        !xk_config_get("test.value", &value)
    );

    test_ok(
        "Foundation Test: Config Set",
        xk_config_set("test.value", 1234)
    );

    test_ok(
        "Foundation Test: Config Get",
        xk_config_get("test.value", &value) &&
        value == 1234
    );

    test_ok(
        "Foundation Test: Config Update",
        xk_config_set("test.value", 5678) &&
        xk_config_get("test.value", &value) &&
        value == 5678
    );

    test_ok(
        "Foundation Test: Config Missing Key",
        !xk_config_get("missing.value", &value)
    );

    test_ok(
        "Foundation Test: Config Null Key Rejected",
        !xk_config_set(NULL, 1)
    );
}


/*
 * ============================================================
 * Event Manager
 * ============================================================
 */

static uint32_t event_test_count;
static uint64_t event_test_data;

static void foundation_event_handler(const XKEvent *event)
{
    if (event == NULL)
        return;

    event_test_count++;
    event_test_data = event->data;
}

static void test_event_manager(void)
{
    xk_event_init();

    event_test_count = 0;
    event_test_data = 0;

    test_ok(
        "Foundation Test: Event Subscribe",
        xk_event_subscribe(
            XK_EVENT_CUSTOM,
            foundation_event_handler
        )
    );

    XKEvent event = {
        .type = XK_EVENT_CUSTOM,
        .source = 42,
        .data = 12345
    };

    test_ok(
        "Foundation Test: Event Publish",
        xk_event_publish(&event)
    );

    test_ok(
        "Foundation Test: Event Handler Called",
        event_test_count == 1 &&
        event_test_data == 12345
    );

    test_ok(
        "Foundation Test: Null Event Rejected",
        !xk_event_publish(NULL)
    );

    test_ok(
        "Foundation Test: Null Handler Rejected",
        !xk_event_subscribe(
            XK_EVENT_CUSTOM,
            NULL
        )
    );
}


/*
 * ============================================================
 * Resource Manager
 * ============================================================
 */

static void test_resource_manager(void)
{
    xk_resource_init();

    uint64_t object_value = 0x12345678;

    uint64_t resource_id =
        xk_resource_register(
            XK_RESOURCE_MEMORY,
            &object_value,
            100
        );

    test_ok(
        "Foundation Test: Resource Register",
        resource_id != 0
    );

    XKResource *resource =
        xk_resource_get(resource_id);

    test_ok(
        "Foundation Test: Resource Lookup",
        resource != NULL
    );

    if (resource != NULL)
    {
        test_ok(
            "Foundation Test: Resource Type",
            resource->type == XK_RESOURCE_MEMORY
        );

        test_ok(
            "Foundation Test: Resource Owner",
            resource->owner == 100
        );

        test_ok(
            "Foundation Test: Resource Object",
            resource->object == &object_value
        );

        test_ok(
            "Foundation Test: Resource State",
            resource->state == XK_RESOURCE_CREATED
        );

        test_ok(
            "Foundation Test: Resource References",
            resource->references == 1
        );
    }

    test_ok(
        "Foundation Test: Resource State Update",
        xk_resource_set_state(
            resource_id,
            XK_RESOURCE_READY
        )
    );

    resource = xk_resource_get(resource_id);

    test_ok(
        "Foundation Test: Resource State Verified",
        resource != NULL &&
        resource->state == XK_RESOURCE_READY
    );

    test_ok(
        "Foundation Test: Resource Unregister",
        xk_resource_unregister(resource_id)
    );

    test_ok(
        "Foundation Test: Resource Removed",
        xk_resource_get(resource_id) == NULL
    );

    test_ok(
        "Foundation Test: Invalid Resource Rejected",
        !xk_resource_set_state(
            resource_id,
            XK_RESOURCE_RUNNING
        )
    );
}


/*
 * ============================================================
 * Capability Manager
 * ============================================================
 */

static void test_capability_manager(void)
{
    xk_capability_init();

    const uint64_t object_id = 1001;

    test_ok(
        "Foundation Test: Capability Initially Absent",
        !xk_capability_check(
            object_id,
            XK_CAP_READ
        )
    );

    test_ok(
        "Foundation Test: Capability Grant",
        xk_capability_grant(
            object_id,
            XK_CAP_READ
        )
    );

    test_ok(
        "Foundation Test: Capability Check",
        xk_capability_check(
            object_id,
            XK_CAP_READ
        )
    );

    test_ok(
        "Foundation Test: Multiple Capability Grant",
        xk_capability_grant(
            object_id,
            XK_CAP_WRITE | XK_CAP_MODIFY
        )
    );

    test_ok(
        "Foundation Test: Multiple Capability Check",
        xk_capability_check(
            object_id,
            XK_CAP_READ |
            XK_CAP_WRITE |
            XK_CAP_MODIFY
        )
    );

    test_ok(
        "Foundation Test: Capability Revoke",
        xk_capability_revoke(
            object_id,
            XK_CAP_WRITE
        )
    );

    test_ok(
        "Foundation Test: Capability Revoke Verified",
        !xk_capability_check(
            object_id,
            XK_CAP_WRITE
        )
    );

    test_ok(
        "Foundation Test: Remaining Capability Preserved",
        xk_capability_check(
            object_id,
            XK_CAP_READ
        )
    );

    xk_capability_clear(object_id);

    test_ok(
        "Foundation Test: Capability Clear",
        !xk_capability_check(
            object_id,
            XK_CAP_READ
        )
    );
}


/*
 * ============================================================
 * Driver Manager
 * ============================================================
 */

static bool test_driver_initialize(void)
{
    return true;
}

static void test_driver_shutdown(void)
{
}

static void test_driver_manager(void)
{
    xk_driver_manager_init();

    test_ok(
        "Driver Test: Manager Initialization",
        xk_driver_count() == 0
    );

    XKDriver driver = {
        .name = "foundation-test-driver",
        .type = XK_DRIVER_CUSTOM,
        .state = XK_DRIVER_UNINITIALIZED,
        .initialize = test_driver_initialize,
        .shutdown = test_driver_shutdown
    };

    test_ok(
        "Driver Test: Driver Registration",
        xk_driver_register(&driver)
    );

    test_ok(
        "Driver Test: Driver Count",
        xk_driver_count() == 1
    );

    XKDriver *found =
        xk_driver_find("foundation-test-driver");

    test_ok(
        "Driver Test: Driver Lookup",
        found == &driver
    );

    test_ok(
        "Driver Test: Driver Registered State",
        driver.state == XK_DRIVER_REGISTERED
    );

    xk_driver_initialize_all();

    test_ok(
        "Driver Test: Driver Initialization",
        driver.state == XK_DRIVER_INITIALIZED
    );

    test_ok(
        "Driver Test: Invalid Driver Lookup",
        xk_driver_find("does-not-exist") == NULL
    );

    test_ok(
        "Driver Test: Driver Unregister",
        xk_driver_unregister(
            "foundation-test-driver"
        )
    );

    test_ok(
        "Driver Test: Driver Count After Unregister",
        xk_driver_count() == 0
    );

    test_ok(
        "Driver Test: Driver Removed",
        xk_driver_find(
            "foundation-test-driver"
        ) == NULL
    );

    test_ok(
        "Driver Test: Invalid Unregister",
        !xk_driver_unregister(
            "does-not-exist"
        )
    );
}


/*
 * ============================================================
 * Foundation Test Entry Point
 * ============================================================
 */

void run_foundation_tests(void)
{
    test_configuration_manager();
    test_event_manager();
    test_resource_manager();
    test_capability_manager();
    test_driver_manager();
}