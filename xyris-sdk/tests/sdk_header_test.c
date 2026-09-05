#include <xyris/sdk.h>
#include <stddef.h>

int main(void)
{
    if (XYRIS_SDK_API_VERSION_MAJOR != 0u || XYRIS_SDK_API_VERSION_MINOR != 1u) return 1;
    if (sizeof(xyris_u64) != 8u || sizeof(xyris_handle_t) != 8u || sizeof(xyris_fd_t) != 4u) return 1;
    if (sizeof(xyris_abi_header_t) != 8u) return 1;
    if (offsetof(xyris_ipc_message_t, data) != 32u) return 1;
    if (sizeof(xyris_event_t) != 64u) return 1;
    if (sizeof(xyris_timer_spec_t) != 48u) return 1;
    if (sizeof(xyris_device_info_t) != 72u) return 1;
    if (sizeof(xyris_net_endpoint_t) != 44u) return 1;
    if (sizeof(xyris_security_policy_t) != 40u) return 1;
    return 0;
}
