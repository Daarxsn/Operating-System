#include <xyris/sdk.h>
#include <cstddef>
#include <cstdint>

static_assert(XYRIS_SDK_API_VERSION_MAJOR == 0u);
static_assert(XYRIS_SDK_API_VERSION_MINOR == 1u);
static_assert(sizeof(xyris_u64) == 8u);
static_assert(sizeof(xyris_handle_t) == 8u);
static_assert(sizeof(xyris_fd_t) == 4u);
static_assert(sizeof(xyris_abi_header_t) == 8u);
static_assert(offsetof(xyris_ipc_message_t, data) == 32u);
static_assert(sizeof(xyris_event_t) == 64u);
static_assert(sizeof(xyris_timer_spec_t) == 48u);
static_assert(sizeof(xyris_device_info_t) == 72u);
static_assert(sizeof(xyris_net_endpoint_t) == 44u);
static_assert(sizeof(xyris_security_policy_t) == 40u);

int main() { return 0; }
