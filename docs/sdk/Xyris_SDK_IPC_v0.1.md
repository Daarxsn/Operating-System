# Xyris SDK IPC v0.1 — 7.3.6

## Purpose

The IPC module is the official SDK surface for ABI-defined IPC-related
identifiers and local validation.

## ABI-backed surface

The public ABI v0.1 defines generic identifiers that can participate in
kernel object and capability interfaces:

- `xyris_handle_t`
- `xyris_capability_t`
- `xyris_object_id_t`

The SDK exposes:

- `xyris_ipc_handle_valid()`
- `xyris_ipc_capability_valid()`
- `xyris_ipc_object_valid()`

These helpers perform local sentinel validation only.

## Deliberate scope boundary

The public syscall table in ABI v0.1 currently assigns only READ, WRITE,
OPEN, CLOSE, and EXIT. It does not assign IPC send, receive, endpoint,
channel, port, shared-memory, event, or message syscalls, and it does not
define a public IPC message structure.

Accordingly, this module does not invent syscall numbers, message layouts,
kernel object layouts, or private-kernel interfaces. Kernel IPC operations
remain deferred until an explicit public ABI contract is assigned.

## Dependencies

The IPC SDK depends on SDK Core and the public Xyris ABI only.

## Validation

Both C and C++ public-header consumption tests are registered with CTest.
Tests cover valid and invalid ABI sentinel values without requiring a
running kernel.

## Definition of Done

A third-party application can include `<xyris/ipc.h>` and consume the
currently public IPC-relevant ABI identifiers without depending on private
kernel internals.
