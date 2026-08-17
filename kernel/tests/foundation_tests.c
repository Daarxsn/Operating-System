#include "foundation_tests.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "foundation/config.h"
#include "foundation/ukom.h"
#include "foundation/time.h"
#include "foundation/event.h"
#include "foundation/resource.h"
#include "foundation/capability.h"
#include "drivers/driver.h"
#include "drivers/keyboard.h"
#include "drivers/mouse.h"
#include "drivers/pci.h"

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

    test_ok(
        "Foundation Test: Config Overlong Key Rejected",
        !xk_config_set("this.key.is.intentionally.longer.than.thirty.one", 1)
    );

    test_ok(
        "Foundation Test: Config Count",
        xk_config_count() == 1
    );

    test_ok(
        "Foundation Test: Config Remove",
        xk_config_remove("test.value") && xk_config_count() == 0
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
        "Foundation Test: Event Subscriber Count",
        xk_event_subscriber_count() == 1
    );

    test_ok(
        "Foundation Test: Duplicate Event Rejected",
        !xk_event_subscribe(XK_EVENT_CUSTOM, foundation_event_handler)
    );

    test_ok(
        "Foundation Test: Event Unsubscribe",
        xk_event_unsubscribe(XK_EVENT_CUSTOM, foundation_event_handler) &&
        xk_event_subscriber_count() == 0
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
        ) &&
        xk_resource_register(XK_RESOURCE_NONE, NULL, 0) == 0
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

    test_ok(
        "Foundation Test: Capability Invalid Rejected",
        !xk_capability_grant(0, XK_CAP_READ) &&
        !xk_capability_grant(object_id, XK_CAP_NONE) &&
        xk_capability_count() == 0
    );
}


/*
 * ============================================================
 * UKOM and Time Manager
 * ============================================================
 */

static uint32_t timer_test_count;

static void foundation_timer_handler(void *context)
{
    uint32_t *value = (uint32_t *)context;
    if (value != NULL)
        (*value)++;
    timer_test_count++;
}

static void test_ukom_and_time(void)
{
    xkobject_init();

    test_ok(
        "Foundation Test: UKOM Invalid Type Rejected",
        xkobject_create(XK_OBJECT_NONE) == NULL
    );

    XKObject *object = xkobject_create(XK_OBJECT_CUSTOM);
    test_ok("Foundation Test: UKOM Create", object != NULL);

    if (object != NULL)
    {
        uint64_t id = object->id;
        test_ok("Foundation Test: UKOM Lookup", xkobject_find(id) == object);
        test_ok("Foundation Test: UKOM Count", xkobject_count() == 1);
        test_ok("Foundation Test: UKOM Exists", xkobject_exists(id));

        xkobject_retain(object);
        test_ok("Foundation Test: UKOM Retain", object->ref_count == 2);
        xkobject_release(object);
        test_ok("Foundation Test: UKOM Release", object->ref_count == 1);
        xkobject_release(object);
        test_ok("Foundation Test: UKOM Auto Destroy", !xkobject_exists(id));
    }

    xk_time_init();
    test_ok("Foundation Test: Time Initial Frequency", xk_time_frequency() == 100);
    test_ok("Foundation Test: Time Frequency Update", xk_time_set_frequency(250));
    test_ok("Foundation Test: Time Frequency Verified", xk_time_frequency() == 250);

    uint32_t callback_value = 0;
    timer_test_count = 0;
    XKTimer *timer = xk_timer_create(3, foundation_timer_handler, &callback_value);
    test_ok("Foundation Test: Timer Create", timer != NULL);

    xk_time_tick();
    xk_time_tick();
    test_ok("Foundation Test: Timer Not Early", timer_test_count == 0);
    xk_time_tick();
    test_ok("Foundation Test: Timer Callback", timer_test_count == 1 && callback_value == 1);

    XKTimer *cancelled = xk_timer_create(10, foundation_timer_handler, &callback_value);
    test_ok("Foundation Test: Timer Create For Cancel", cancelled != NULL);
    test_ok("Foundation Test: Timer Cancel", xk_timer_cancel(cancelled));
    for (uint32_t i = 0; i < 10; i++)
        xk_time_tick();
    test_ok("Foundation Test: Cancelled Timer Silent", timer_test_count == 1);

    xk_time_set_frequency(100);
    uint64_t milliseconds_before = xk_time_milliseconds();

    for (uint32_t i = 0; i < 100; i++)
        xk_time_tick();

    test_ok(
        "Foundation Test: Millisecond Accounting",
        (xk_time_milliseconds() - milliseconds_before) == 1000
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

static uint32_t test_driver_shutdown_count;

static void test_driver_shutdown(void)
{
    test_driver_shutdown_count++;
}

static void test_driver_manager(void)
{
    xk_driver_manager_init();
    test_driver_shutdown_count = 0;

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
        driver.state == XK_DRIVER_RUNNING
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
        "Driver Test: Shutdown On Unregister",
        test_driver_shutdown_count == 1
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


/* Registration-only drivers must still reach a usable running state. */
static void test_registration_only_driver(void)
{
    XKDriver driver = {
        .name = "registration-only-driver",
        .type = XK_DRIVER_CUSTOM,
        .state = XK_DRIVER_UNINITIALIZED,
        .initialize = NULL,
        .shutdown = NULL
    };

    test_ok(
        "Driver Test: Registration-Only Driver",
        xk_driver_register(&driver)
    );

    xk_driver_initialize_all();

    test_ok(
        "Driver Test: Registration-Only Running",
        driver.state == XK_DRIVER_RUNNING
    );

    test_ok(
        "Driver Test: Registration-Only Unregister",
        xk_driver_unregister(driver.name) &&
        xk_driver_count() == 0
    );
}

/*
 * ============================================================
 * Driver Input / PCI Tests
 * ============================================================
 */

static void test_driver_input_and_pci(void)
{
    /* Keyboard parser can be tested without touching hardware. */
    xk_keyboard_shutdown();
    test_ok("Driver Test: Keyboard Parser Initialization", xk_keyboard_process_scancode(0x1E));

    XKKeyboardEvent key_event = {0};
    test_ok("Driver Test: Keyboard Event Available", xk_keyboard_event_available());
    test_ok("Driver Test: Keyboard ASCII Decode",
            xk_keyboard_read_event(&key_event) && key_event.ascii == 'a' && key_event.pressed);

    (void)xk_keyboard_process_scancode(0x2A); /* left shift */
    (void)xk_keyboard_process_scancode(0x1E); /* A */
    test_ok("Driver Test: Keyboard Modifier Decode",
            xk_keyboard_read_event(&key_event) && key_event.ascii == 'A' &&
            (key_event.modifiers & XK_KEY_MOD_SHIFT) != 0);
    (void)xk_keyboard_process_scancode(0xAA); /* shift release */

    (void)xk_keyboard_process_scancode(0xE0);
    (void)xk_keyboard_process_scancode(0x48); /* up arrow */
    test_ok("Driver Test: Keyboard Extended Scancode",
            xk_keyboard_read_event(&key_event) &&
            (key_event.keycode & XK_KEY_EXTENDED) != 0);

    /* Pause/Break is a six-byte E1 sequence and must not leak
     * intermediate bytes into the input queue. */
    const uint8_t pause_sequence[] = {0xE1U, 0x1DU, 0x45U, 0xE1U, 0x9DU, 0xC5U};
    for (uint32_t i = 0; i < sizeof(pause_sequence); i++)
        (void)xk_keyboard_process_scancode(pause_sequence[i]);

    test_ok("Driver Test: Keyboard Pause Sequence",
            xk_keyboard_read_event(&key_event) &&
            (key_event.keycode & XK_KEY_EXTENDED) != 0 &&
            key_event.keycode == (uint16_t)(0x45U | XK_KEY_EXTENDED) &&
            key_event.pressed);

    /* Mouse parser: one standard 3-byte packet. */
    xk_mouse_shutdown();
    (void)xk_mouse_process_byte(0x09); /* sync + left button */
    (void)xk_mouse_process_byte(5);
    (void)xk_mouse_process_byte(0xFE); /* -2 */

    XKMouseEvent mouse_event = {0};
    test_ok("Driver Test: Mouse Packet Event",
            xk_mouse_event_available() && xk_mouse_read_event(&mouse_event) &&
            mouse_event.x == 5 && mouse_event.y == 2 &&
            (mouse_event.buttons & XK_MOUSE_BUTTON_LEFT) != 0);

    test_ok("Driver Test: Mouse State Tracking",
            xk_mouse_x() == 5 && xk_mouse_y() == 2 &&
            (xk_mouse_buttons() & XK_MOUSE_BUTTON_LEFT) != 0);

    /* PCI API bounds and scan results are runtime-dependent. */
    test_ok("Driver Test: PCI Invalid Device Rejected",
            xk_pci_device_get(xk_pci_device_count()) == NULL);
    if (xk_pci_device_count() > 0)
    {
        const XKPCIDevice *device = xk_pci_device_get(0);
        test_ok("Driver Test: PCI Device Metadata", device != NULL && device->vendor_id != 0xFFFFU);
        test_ok("Driver Test: PCI BAR Metadata", device != NULL && device->bar_count <= XK_PCI_MAX_BARS);
        test_ok("Driver Test: PCI Capability Metadata", device != NULL &&
                device->capability_count <= XK_PCI_MAX_CAPABILITIES);
    }
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
    test_ukom_and_time();
    test_driver_manager();
    test_registration_only_driver();
    test_driver_input_and_pci();
}