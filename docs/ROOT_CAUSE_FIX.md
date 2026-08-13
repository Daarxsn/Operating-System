# XyrisOS Runtime Root-Cause Fix

## Verified runtime symptoms

The Phase 1-5 QEMU run reached `Kernel Ready` and both scheduler test threads completed:

- `THREAD A FINISHED`
- `THREAD B FINISHED`

Two failures remained in the kernel test suite before this fix:

- `Syscall Test: Open/Read/Close`
- `User Test: Address Space Cleanup`

## Root cause 1: syscall tests rejected trusted kernel pointers

`syscall_validate_user_range()` applied the low-half `USER_ADDRESS_LIMIT` check before determining whether the caller was the kernel process. Kernel-side tests pass higher-half kernel pointers, including string literals and stack buffers. Those addresses were rejected before the existing kernel-trust exception could run.

### Fix

Identify the current process first. If there is no process or the current process is marked `kernel_process`, accept the kernel pointer as trusted. Only real user processes proceed through the user address-range and page-permission checks.

## Root cause 2: NX bit leaked into physical addresses

The VMM used:

`0xFFFFFFFFFFFFF000ULL`

as `PAGE_MASK` while also using bit 63 as `VMM_NX`.

When a user stack mapping was created with `VMM_NX`, `vmm_translate()` masked the page-table entry with a mask that preserved bit 63. The resulting value looked like:

`0x80000000000xxxxx`

The PMM correctly rejected these values as outside its physical-memory bitmap. The physical frame itself was valid; the translation mask was wrong.

### Fix

Use a 52-bit x86-64 physical-address mask:

`0x000FFFFFFFFFF000ULL`

This clears the NX bit and retains the supported physical-address bits plus page alignment.

## Root cause 3: runtime diagnostics were not mirrored to COM1

The validation script runs QEMU headlessly with `-serial stdio`, but the kernel debug console originally rendered only to the framebuffer. The serial driver was registered, but `debug_print()` did not mirror its text stream to COM1. As a result, QEMU could boot while the validation log remained empty and the runtime gate timed out without markers.

### Fix

`debug_print()` now mirrors every debug string to COM1 after serial initialization. `xk_serial_write_string()` safely no-ops before initialization and for null input. The framebuffer remains the primary console.

## Root cause 4: normal PMM tracing flooded the runtime console

The PMM implementation unconditionally printed every successful allocation/free. The finite VMM address-space stress test therefore generated a large amount of framebuffer work and obscured the scheduler runtime.

### Fix

Successful PMM allocation/free tracing is now opt-in through `XYRIS_PMM_TRACE=1`. Rejected frees remain visible because they are actual accounting defects. Normal runtime therefore stays quiet while preserving a diagnostic mode.

## Validation

After the source fixes:

- Kernel build: PASS (61/61 objects)
- Unresolved kernel symbols: PASS
- Simulator CTest: PASS (5/5)
- Kernel ELF contains `Kernel Ready`, `THREAD A FINISHED`, and `THREAD B FINISHED` markers
- Normal kernel ELF does not contain the successful PMM allocation/free trace strings
- `validate.sh` now fails on `[FAIL]` or `PMM FREE REJECT` and requires kernel-test, user-cleanup, and scheduler completion markers
- QEMU runtime still needs to be executed on the developer host because this audit container does not provide QEMU/xorriso
