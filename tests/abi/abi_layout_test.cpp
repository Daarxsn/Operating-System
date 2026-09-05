#include <cassert>
#include <cstddef>
#include <xyris/abi/version.h>
#include <xyris/abi/types.h>
#include <xyris/abi/syscalls.h>

static_assert(sizeof(xyris_u64) == 8);
static_assert(sizeof(xyris_i64) == 8);
static_assert(sizeof(xyris_pid_t) == 4);
static_assert(sizeof(xyris_handle_t) == 8);
static_assert(sizeof(xyris_abi_header_t) == 8);
static_assert(offsetof(xyris_abi_header_t, size) == 0);
static_assert(offsetof(xyris_abi_header_t, version) == 4);
static_assert(offsetof(xyris_abi_header_t, flags) == 6);
static_assert(XYRIS_ABI_MAJOR_COMPATIBLE(XYRIS_ABI_VERSION, XYRIS_ABI_VERSION));
static_assert(XYRIS_ABI_MINOR_SATISFIES(XYRIS_ABI_VERSION, XYRIS_ABI_VERSION));

int main() {
    assert(XYRIS_ABI_VERSION_MAJOR(XYRIS_ABI_VERSION) == XYRIS_ABI_MAJOR);
    assert(XYRIS_ABI_VERSION_MINOR(XYRIS_ABI_VERSION) == XYRIS_ABI_MINOR);
    assert(!XYRIS_ABI_MAJOR_COMPATIBLE(0x00010000u, 0x00000001u));
    assert(!XYRIS_ABI_MINOR_SATISFIES(0x00000001u, 0x00000002u));
    return 0;
}
