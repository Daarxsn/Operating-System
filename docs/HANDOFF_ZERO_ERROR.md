# XyrisOS Repo 3 — Phase 1–5 Handoff Baseline

## Status

This archive is the corrected shared development baseline intended for the other XyrisOS members.

### Verified in the audit environment

- Kernel CMake/Ninja build: **PASS — 61/61 objects**
- Kernel link: **PASS**
- Unresolved kernel symbols: **0**
- Kernel compiler flags: `-Wall -Wextra -Werror`
- Simulator build: **PASS**
- CTest: **5/5 PASS**
- Runtime marker strings present in `kernel.elf`:
  - `Kernel Ready`
  - `THREAD A FINISHED`
  - `THREAD B FINISHED`
- Successful PMM allocation/free trace strings are not embedded in the normal kernel build.

## Runtime fixes in this baseline

1. **Syscall kernel-pointer validation**
   - Kernel-side syscall tests are trusted before the user virtual-address restriction is applied.
   - `SYS_OPEN`, `SYS_READ`, and `SYS_CLOSE` therefore operate correctly for kernel-side integration tests.

2. **VMM physical-address masking**
   - Physical page-table addresses use `0x000FFFFFFFFFF000ULL`.
   - The NX bit cannot leak into physical addresses returned by `vmm_translate()`.
   - User stack cleanup can therefore return the correct physical frames to PMM.

3. **PMM runtime tracing**
   - Successful allocation/free tracing is disabled by default.
   - Enable with `-DXYRIS_PMM_TRACE=1` only when diagnosing PMM behavior.
   - Rejected frees remain visible.

4. **Headless QEMU serial validation**
   - `debug_print()` mirrors its output to COM1 after serial initialization.
   - Serial string output safely no-ops before initialization and for null strings.
   - This makes `-serial stdio` a reliable runtime evidence channel.

5. **Strict runtime gate**
   - `scripts/validate.sh` fails on `[FAIL]` markers.
   - It fails on `PMM FREE REJECT` diagnostics.
   - It requires syscall, user-cleanup, kernel-ready, and scheduler-completion markers.
   - A QEMU timeout is accepted only after all required markers are observed because the kernel intentionally remains alive in its idle loop.

## Host-side final command

On the development machine, run:

```bash
./scripts/validate.sh
```

The audit container used for this archive did not have `qemu-system-x86_64`/`xorriso`, so the final ISO/QEMU stage must be executed on the development host.

## Handoff boundary

Do not commit generated `build/`, `simulator/build/`, `iso_root/`, or `XyrisOS.iso` artifacts from this archive. Each member should build from the source tree and use the validation script before pushing changes.
