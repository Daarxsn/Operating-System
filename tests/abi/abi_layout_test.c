#include <assert.h>
#include <stddef.h>
#include <xyris/abi/version.h>
#include <xyris/abi/types.h>
#include <xyris/abi/syscalls.h>

_Static_assert(sizeof(xyris_u8) == 1, "u8 width");
_Static_assert(sizeof(xyris_u16) == 2, "u16 width");
_Static_assert(sizeof(xyris_u32) == 4, "u32 width");
_Static_assert(sizeof(xyris_u64) == 8, "u64 width");
_Static_assert(sizeof(xyris_i64) == 8, "i64 width");
_Static_assert(sizeof(xyris_pid_t) == 4, "pid width");
_Static_assert(sizeof(xyris_tid_t) == 4, "tid width");
_Static_assert(sizeof(xyris_handle_t) == 8, "handle width");
_Static_assert(sizeof(xyris_capability_t) == 8, "capability width");
_Static_assert(sizeof(xyris_abi_header_t) == 8, "ABI header size");
_Static_assert(offsetof(xyris_abi_header_t, size) == 0, "ABI header size offset");
_Static_assert(offsetof(xyris_abi_header_t, version) == 4, "ABI header version offset");
_Static_assert(offsetof(xyris_abi_header_t, flags) == 6, "ABI header flags offset");
_Static_assert(XYRIS_ABI_MAJOR_COMPATIBLE(XYRIS_ABI_VERSION, XYRIS_ABI_VERSION), "major compatibility");
_Static_assert(XYRIS_ABI_MINOR_SATISFIES(XYRIS_ABI_VERSION, XYRIS_ABI_VERSION), "minor compatibility");

int main(void) {
    assert(XYRIS_ABI_VERSION_MAJOR(XYRIS_ABI_VERSION) == XYRIS_ABI_MAJOR);
    assert(XYRIS_ABI_VERSION_MINOR(XYRIS_ABI_VERSION) == XYRIS_ABI_MINOR);
    assert(XYRIS_ABI_MAJOR_COMPATIBLE(0x00000001u, 0x00000001u));
    assert(!XYRIS_ABI_MAJOR_COMPATIBLE(0x00010000u, 0x00000001u));
    assert(XYRIS_ABI_MINOR_SATISFIES(0x00000002u, 0x00000001u));
    assert(!XYRIS_ABI_MINOR_SATISFIES(0x00000001u, 0x00000002u));
    return 0;
}
