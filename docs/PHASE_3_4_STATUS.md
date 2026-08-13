# XyrisOS Phase 3 / Phase 4 Status

This document is the hand-off point for the next team members.

## Phase 3 — File System, System Calls and User-Space Foundation

### Stabilized in this integration

- VFS filesystem registration and mount routing.
- RAMFS hierarchy creation and path lookup.
- File descriptor table and per-open file offsets.
- RAMFS read/write/close behavior.
- Kernel-side syscall dispatch.
- `int 0x80` IDT entry and ISR dispatch.
- Basic user-pointer validation for real user processes.
- Safe copying of syscall path strings into kernel memory.
- `SYS_EXIT` now terminates the current non-kernel process/thread instead of being a silent placeholder.
- ELF header and program-header validation framework.
- User address-space and user-stack allocation/cleanup foundation.

### Still intentionally pending

1. Complete ELF process loader ownership:
   - segment ownership tracking;
   - rollback on partial load;
   - freeing executable/data frames during process destruction.
2. Connect ELF loading to `user_process_t`.
3. Create a user thread whose initial context enters Ring 3.
4. Validate user-mode page permissions for code/data/stack.
5. Complete process-wide file descriptor tables instead of the current global file table.
6. Add filesystem object lifetime/refcounting before arbitrary unmount of filesystems with open files.
7. Implement `seek`, directory operations, and richer VFS operations.
8. Add end-to-end Ring-3 syscall tests.

Do not mark Phase 3 complete until these behaviors are runtime tested.

## Phase 4 — Process, Thread and Scheduler Foundation

### Stabilized in this integration

- Process table and PCB creation.
- Thread table and TCB creation.
- Kernel stack allocation.
- Initial x86-64 thread context.
- Bootstrap context.
- Assembly context switching.
- Ready/blocked/sleeping queues.
- Cooperative round-robin scheduling.
- Yield/block/unblock/sleep/wakeup transitions.
- Idle thread fallback.
- Thread-return termination path.
- Scheduler registration/removal accounting.
- Duplicate scheduler registration protection.
- Dedicated scheduler lifecycle tests.

### Current runtime model

Phase 4 is currently **cooperative**, not preemptive:

- PIT interrupts call `scheduler_tick()`.
- `scheduler_tick()` accounts time and wakes sleepers.
- Threads voluntarily switch through `scheduler_yield()`.
- Automatic time-slice context switching is intentionally not enabled yet.

The finite A/B smoke test in `kernel/core/kernel.c` exercises the actual context-switch path after kernel initialization.

### Still intentionally pending

1. Deferred thread reaping so terminated stacks/TCBs can be reclaimed safely.
2. Process/thread exit and wait semantics.
3. Full process/address-space attachment.
4. User-thread creation.
5. Preemptive time-slice switching from the timer interrupt.
6. Scheduler statistics API and stress tests.
7. SMP/per-CPU scheduler design.
8. Priority-aware scheduling if required by the final architecture.

## Authoritative execution path

```text
kernel/core/kernel.c
    -> kernel/process/process.c
    -> kernel/process/thread.c
    -> kernel/process/scheduler.c
    -> kernel/process/context.c
    -> kernel/process/switch.S
```

The legacy `kernel/execution/*` tree remains dormant. New production process/scheduler work must go into `kernel/process/*`.

## Validation performed for this integration

- Kernel CMake/Ninja build: PASS
- Kernel link: PASS
- `nm -u build/kernel.elf`: no unresolved symbols
- Simulator CMake/Ninja build: PASS
- CTest: 5/5 PASS
- ISO/QEMU runtime: not validated in this environment because the required host tools are unavailable.

## Final integration validation (2026-08-12)

The repository was rebuilt from a clean build directory with GCC 14.2 and Ninja.

- Kernel CMake/Ninja build: PASS
- Kernel build with `-Werror`: PASS
- `nm -u build/kernel.elf`: no unresolved symbols
- Simulator CMake/Ninja build: PASS
- CTest: 5/5 PASS
- Process ownership is synchronized with scheduler thread switches.
- `scripts/build.sh` now works without requiring a nonexistent toolchain file and will use a dedicated toolchain automatically if one is later added.
- Runtime QEMU validation remains environment-dependent; the repository's `run.sh` reports a clear dependency error when QEMU/xorriso are unavailable.
