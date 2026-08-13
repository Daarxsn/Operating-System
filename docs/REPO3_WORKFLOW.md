# Repo 3 Development Workflow

Repo 3 is the validated integration branch produced from Repo 1 and Repo 2.

## Source of truth

- Repo 1 preserves the original architecture and history.
- Repo 2 contains the refurbished implementation.
- Repo 3 contains only changes that were compared, validated, and accepted.

## Required validation order

```text
CMake configure
    ↓
Kernel build
    ↓
Unresolved-symbol check
    ↓
Simulator build
    ↓
CTest
    ↓
ISO generation
    ↓
QEMU boot
    ↓
Kernel subsystem tests
    ↓
Release
```

Do not treat a successful CMake link as proof that the OS boots. QEMU/ISO validation is a separate acceptance stage.

## Authoritative process/scheduler path

```text
kernel/core/kernel.c
        ↓
kernel/process/process.c
        ↓
kernel/process/thread.c
        ↓
kernel/process/scheduler.c
        ↓
kernel/process/context.c
        ↓
kernel/process/switch.S
```

`kernel/execution/*` is currently dormant legacy execution code and should not receive parallel production scheduler implementations.

## Team completion rule

A member is considered 100% complete only when:

1. the assigned implementation exists;
2. it is part of the active build where applicable;
3. it is integrated with dependent subsystems;
4. tests exist for the critical behavior;
5. the behavior has been runtime validated where hardware/runtime is required.

This prevents placeholder frameworks from being counted as complete merely because files exist.
