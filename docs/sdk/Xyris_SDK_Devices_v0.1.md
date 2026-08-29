# Xyris SDK Devices v0.1 — 7.3.9

## Purpose

The Devices module is the official SDK surface for device-related ABI
identifiers currently published by XyrisOS.

## ABI-backed surface

ABI v0.1 currently provides generic handles and object identifiers:
`xyris_handle_t`, `xyris_object_id_t`, `XYRIS_INVALID_HANDLE`, and
`XYRIS_INVALID_OBJECT`.

The SDK exposes local validation helpers for those identifiers.

## Deliberate scope boundary

The public syscall table does not currently assign device discovery,
open/close, I/O-control, interrupt, or device-management syscalls, and the
ABI does not publish a device object layout. This module therefore does not
invent device syscall numbers, register layouts, driver interfaces, or
private kernel structures.

Device operations remain deferred until a public ABI contract is assigned.

## Definition of Done

A third-party application can include `<xyris/devices.h>` and consume the
currently public device-related identifier contract without private kernel
dependencies.
