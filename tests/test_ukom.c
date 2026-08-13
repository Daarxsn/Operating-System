#include <assert.h>
#include <stdio.h>

#include "foundation/ukom.h"

static void test_initialization(void)
{
    xkobject_init();

    assert(!xkobject_exists(1));

    printf("[PASS] UKOM initialization\n");
}

static void test_object_creation(void)
{
    XKObject *object;

    xkobject_init();

    object = xkobject_create(XK_OBJECT_PROCESS);

    assert(object != 0);
    assert(object->id == 1);
    assert(object->type == XK_OBJECT_PROCESS);
    assert(object->state == XK_STATE_CREATED);
    assert(object->ref_count == 1);
    assert(xkobject_exists(object->id));

    printf("[PASS] UKOM object creation\n");
}

static void test_object_lookup(void)
{
    XKObject *created;
    XKObject *found;

    xkobject_init();

    created = xkobject_create(XK_OBJECT_THREAD);

    assert(created != 0);

    found = xkobject_find(created->id);

    assert(found == created);
    assert(found->type == XK_OBJECT_THREAD);

    assert(xkobject_find(9999) == 0);
    assert(!xkobject_exists(9999));

    printf("[PASS] UKOM object lookup\n");
}

static void test_reference_counting(void)
{
    XKObject *object;
    uint64_t object_id;

    xkobject_init();

    object = xkobject_create(XK_OBJECT_RESOURCE);

    assert(object != 0);
    assert(object->ref_count == 1);

    object_id = object->id;

    xkobject_retain(object);

    assert(object->ref_count == 2);

    xkobject_release(object);

    assert(object->ref_count == 1);
    assert(xkobject_exists(object_id));

    xkobject_release(object);

    assert(!xkobject_exists(object_id));

    printf("[PASS] UKOM reference counting\n");
}

static void test_object_destruction(void)
{
    XKObject *object;
    uint64_t object_id;

    xkobject_init();

    object = xkobject_create(XK_OBJECT_DEVICE);

    assert(object != 0);

    object_id = object->id;

    assert(xkobject_exists(object_id));

    xkobject_destroy(object_id);

    assert(!xkobject_exists(object_id));
    assert(xkobject_find(object_id) == 0);

    printf("[PASS] UKOM object destruction\n");
}

static void test_null_safety(void)
{
    xkobject_retain(0);
    xkobject_release(0);

    xkobject_destroy(9999);

    assert(xkobject_find(9999) == 0);
    assert(!xkobject_exists(9999));

    printf("[PASS] UKOM null and invalid input handling\n");
}

static void test_multiple_object_types(void)
{
    XKObject *process;
    XKObject *thread;
    XKObject *driver;
    XKObject *device;

    xkobject_init();

    process = xkobject_create(XK_OBJECT_PROCESS);
    thread = xkobject_create(XK_OBJECT_THREAD);
    driver = xkobject_create(XK_OBJECT_DRIVER);
    device = xkobject_create(XK_OBJECT_DEVICE);

    assert(process != 0);
    assert(thread != 0);
    assert(driver != 0);
    assert(device != 0);

    assert(process->type == XK_OBJECT_PROCESS);
    assert(thread->type == XK_OBJECT_THREAD);
    assert(driver->type == XK_OBJECT_DRIVER);
    assert(device->type == XK_OBJECT_DEVICE);

    assert(process->id != thread->id);
    assert(thread->id != driver->id);
    assert(driver->id != device->id);

    printf("[PASS] UKOM multiple object types\n");
}

static void test_object_limit(void)
{
    XKObject *object;
    XKObject *last_object;

    xkobject_init();

    for (int i = 0; i < XKOBJECT_MAX_OBJECTS; i++)
    {
        object = xkobject_create(XK_OBJECT_CUSTOM);

        assert(object != 0);
    }

    last_object = xkobject_create(XK_OBJECT_CUSTOM);

    assert(last_object == 0);

    printf("[PASS] UKOM object limit\n");
}

int main(void)
{
    test_initialization();
    test_object_creation();
    test_object_lookup();
    test_reference_counting();
    test_object_destruction();
    test_null_safety();
    test_multiple_object_types();
    test_object_limit();

    printf("\nAll UKOM tests passed successfully.\n");

    return 0;
}