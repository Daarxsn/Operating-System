# Xyris SDK Timers v0.1 — 7.3.8

## Purpose

The Timers module is the official SDK surface for timer-related ABI types
and handles that are currently public.

## ABI-backed surface

ABI v0.1 defines:

- `xyris_time_ns_t`
- `xyris_duration_ns_t`
- `xyris_handle_t`
- `XYRIS_INVALID_HANDLE`

The SDK exposes:

- `xyris_timer_handle_valid()`

The helper performs local sentinel validation only.

## Deliberate scope boundary

The public syscall table currently assigns READ, WRITE, OPEN, CLOSE, and
EXIT. It does not assign timer creation, arm, cancel, sleep, wait, or query
syscalls, and ABI v0.1 does not define a public timer object structure.

Accordingly, this module does not invent timer syscall numbers, timer
structures, clock semantics, flags, or private kernel object layouts.
Kernel timer operations remain deferred until an explicit public ABI
contract is assigned.

## Dependencies

The Timers SDK depends only on SDK Core and the public Xyris ABI.

## Validation

C and C++ public-header tests validate the invalid-handle sentinel and
representative valid handle values without requiring kernel execution.

## Definition of Done

A third-party application can include `<xyris/timers.h>` and consume the
currently public timer-related ABI contract without depending on private
kernel internals.
