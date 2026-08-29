# Xyris SDK Events v0.1 — 7.3.7

## Purpose

The Events module is the official SDK surface for event-related ABI
identifiers that can be represented safely using the current public ABI.

## ABI-backed surface

ABI v0.1 currently provides the generic `xyris_handle_t` type and
`XYRIS_INVALID_HANDLE` sentinel. The SDK exposes:

- `xyris_event_handle_valid()`

This performs local validation only.

## Deliberate scope boundary

The public syscall table currently assigns READ, WRITE, OPEN, CLOSE, and
EXIT. It does not assign an event wait, signal, poll, subscribe, or event
creation syscall, and ABI v0.1 does not define a public event structure.

Accordingly, this module does not invent event syscall numbers, event flags,
message layouts, or private kernel object layouts. Kernel event operations
remain deferred until an explicit public ABI contract is assigned.

## Dependencies

The Events SDK depends only on SDK Core and the public Xyris ABI.

## Validation

C and C++ public-header tests validate the invalid-handle sentinel and
representative valid handle values without requiring kernel execution.

## Definition of Done

A third-party application can include `<xyris/events.h>` and consume the
currently public event-relevant ABI handle contract without depending on
private kernel internals.
