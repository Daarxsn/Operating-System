# Xyris SDK Thread v0.1 — 7.3.3

## Purpose

The Thread module is the public SDK surface for thread identity and future
thread operations.

## ABI-backed surface

The public System ABI v0.1 defines:

- `xyris_tid_t` as a 32-bit thread identifier.
- `XYRIS_INVALID_TID` as the reserved invalid thread identifier.
- The generic ABI error/status types used by SDK Core.

The SDK exposes `xyris_thread_id_valid()` as a header-only validation helper.

## Deliberate scope boundary

The current public syscall table assigns read, write, open, close, and exit.
It does not yet assign thread creation, termination, join, detach, sleep,
yield, priority, or thread-status syscalls.

Accordingly, this module does not invent syscall numbers, structures, or
kernel-private interfaces for those operations. They remain deferred until
the corresponding public ABI contracts are assigned.

## Dependencies

The Thread SDK depends on the public Xyris ABI and SDK Core only. No private
kernel headers are required.

## Validation

Both C and C++ public-header consumption tests are registered with CTest.

## Definition of Done

A third-party application can include `<xyris/thread.h>` and use the
ABI-backed thread identity helper without depending on private kernel
internals. Unsupported thread operations remain explicitly deferred.
