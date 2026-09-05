#include "sdk_services_tests.h"

#include "../syscall/sdk_services.h"
#include "../syscall/syscall.h"
#include "../memory/vmm.h"
#include "../process/process.h"
#include "../process/thread.h"
#include "../include/drivers/pci.h"

#include "boot/boot.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>

static int service_test_memory(void)
{
    process_t *kernel = process_current();
    if (kernel == NULL || !kernel->kernel_process)
        return 0;

    process_t *test_process = process_create("sdk-memory-test", false);
    if (test_process == NULL)
        return 0;

    test_process->address_space = vmm_create_space();
    if (test_process->address_space == NULL)
    {
        process_destroy(test_process);
        return 0;
    }

    process_set_current(test_process);

    xyris_addr_t base = (xyris_addr_t)xyris_sdk_service_memory_map(
        8192,
        XYRIS_MEMORY_PROT_READ | XYRIS_MEMORY_PROT_WRITE,
        XYRIS_MEMORY_FLAG_PRIVATE
    );

    int ok = base != 0 &&
        base != 0 &&
        xyris_sdk_service_memory_protect(base, 8192, XYRIS_MEMORY_PROT_READ) == XYRIS_OK &&
        xyris_sdk_service_memory_unmap(base, 8192) == XYRIS_OK;

    process_set_current(kernel);
    process_destroy(test_process);
    return ok;
}

void run_sdk_services_tests(void)
{
    boot_step_ok("SDK Service Tests Started");

    if (xyris_sdk_service_getpid() > 0)
        boot_step_ok("SDK Process Service Passed");
    else
        boot_step_fail("SDK Process Service Failed");

    xyris_handle_t ipc = (xyris_handle_t)xyris_sdk_service_ipc_create(0);
    char message[] = "xyris-ipc";
    xyris_ipc_message_t descriptor;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.header.size = sizeof(descriptor);
    descriptor.header.version = 1;
    descriptor.endpoint = ipc;
    descriptor.capability = ipc;
    descriptor.data = (xyris_user_ptr_t)(uintptr_t)message;
    descriptor.length = sizeof(message);
    descriptor.tag = 0x1234;
    char received[32] = {0};
    xyris_u64 tag = 0;
    if (ipc != XYRIS_INVALID_HANDLE &&
        xyris_sdk_service_ipc_send(ipc, ipc, (xyris_user_ptr_t)(uintptr_t)&descriptor) == (xyris_syscall_result_t)sizeof(message) &&
        xyris_sdk_service_ipc_recv(ipc, (xyris_user_ptr_t)(uintptr_t)received, sizeof(received), (xyris_user_ptr_t)(uintptr_t)&tag) == (xyris_syscall_result_t)sizeof(message) &&
        strcmp(message, received) == 0 && tag == 0x1234)
        boot_step_ok("SDK IPC Service Passed");
    else
        boot_step_fail("SDK IPC Service Failed");
    if (ipc != XYRIS_INVALID_HANDLE)
        (void)xyris_sdk_service_ipc_close(ipc);

    xyris_handle_t event = (xyris_handle_t)xyris_sdk_service_event_create(XYRIS_EVENT_SIGNAL, 0);
    xyris_u64 payload[XYRIS_EVENT_PAYLOAD_WORDS] = {1, 2, 3, 4};
    xyris_event_t event_out;
    if (event != XYRIS_INVALID_HANDLE &&
        xyris_sdk_service_event_signal(event, (xyris_user_ptr_t)(uintptr_t)payload) == XYRIS_OK &&
        xyris_sdk_service_event_wait(event, (xyris_user_ptr_t)(uintptr_t)&event_out) == XYRIS_OK &&
        event_out.payload[0] == 1 && event_out.payload[3] == 4)
        boot_step_ok("SDK Event Service Passed");
    else
        boot_step_fail("SDK Event Service Failed");
    if (event != XYRIS_INVALID_HANDLE)
        (void)xyris_sdk_service_event_close(event);

    xyris_timer_spec_t timer_spec;
    memset(&timer_spec, 0, sizeof(timer_spec));
    timer_spec.header.size = sizeof(timer_spec);
    timer_spec.header.version = 1;
    timer_spec.mode = XYRIS_TIMER_ONESHOT;
    timer_spec.clock = XYRIS_TIMER_CLOCK_MONOTONIC;
    xyris_handle_t timer = (xyris_handle_t)xyris_sdk_service_timer_create((xyris_user_ptr_t)(uintptr_t)&timer_spec);
    if (timer != XYRIS_INVALID_HANDLE && xyris_sdk_service_timer_wait(timer) == XYRIS_OK)
        boot_step_ok("SDK Timer Service Passed");
    else
        boot_step_fail("SDK Timer Service Failed");
    if (timer != XYRIS_INVALID_HANDLE)
        (void)xyris_sdk_service_timer_close(timer);

    xyris_fd_t server = (xyris_fd_t)xyris_sdk_service_net_socket(XYRIS_NET_FAMILY_IPV4, XYRIS_NET_PROTOCOL_UDP);
    xyris_fd_t client = (xyris_fd_t)xyris_sdk_service_net_socket(XYRIS_NET_FAMILY_IPV4, XYRIS_NET_PROTOCOL_UDP);
    xyris_net_endpoint_t endpoint;
    memset(&endpoint, 0, sizeof(endpoint));
    endpoint.header.size = sizeof(endpoint);
    endpoint.header.version = 1;
    endpoint.family = XYRIS_NET_FAMILY_IPV4;
    endpoint.protocol = XYRIS_NET_PROTOCOL_UDP;
    endpoint.port = 41001;
    endpoint.address_length = 4;
    endpoint.address[0] = 127;
    endpoint.address[3] = 1;
    char net_message[] = "xyris-net";
    char net_received[32] = {0};
    if (server >= 0 && client >= 0 &&
        xyris_sdk_service_net_bind(server, (xyris_user_ptr_t)(uintptr_t)&endpoint) > 0 &&
        xyris_sdk_service_net_connect(client, (xyris_user_ptr_t)(uintptr_t)&endpoint) == XYRIS_OK &&
        xyris_sdk_service_net_send(client, (xyris_user_ptr_t)(uintptr_t)net_message, sizeof(net_message)) == (xyris_syscall_result_t)sizeof(net_message) &&
        xyris_sdk_service_net_recv(server, (xyris_user_ptr_t)(uintptr_t)net_received, sizeof(net_received)) == (xyris_syscall_result_t)sizeof(net_message) &&
        strcmp(net_message, net_received) == 0)
        boot_step_ok("SDK Networking Service Passed");
    else
        boot_step_fail("SDK Networking Service Failed");
    if (server >= 0) (void)xyris_sdk_service_net_close(server);
    if (client >= 0) (void)xyris_sdk_service_net_close(client);

    xyris_security_identity_t identity;
    if (xyris_sdk_service_security_identity((xyris_user_ptr_t)(uintptr_t)&identity) == XYRIS_OK && identity.identity != 0)
        boot_step_ok("SDK Security Identity Passed");
    else
        boot_step_fail("SDK Security Identity Failed");

    xyris_u32 device_count = (xyris_u32)xyris_sdk_service_device_count();
    xyris_device_info_t device_info;
    if (device_count == 0 || xyris_sdk_service_device_info(0, (xyris_user_ptr_t)(uintptr_t)&device_info) == XYRIS_OK)
        boot_step_ok("SDK Device Service Passed");
    else
        boot_step_fail("SDK Device Service Failed");

    if (service_test_memory())
        boot_step_ok("SDK Memory Service Passed");
    else
        boot_step_fail("SDK Memory Service Failed");

    boot_step_ok("SDK Service Tests Completed");
}
