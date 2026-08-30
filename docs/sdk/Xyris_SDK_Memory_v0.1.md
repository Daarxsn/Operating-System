# Xyris SDK Memory v0.1 — 7.3.4

## Purpose

The Memory module is the public SDK surface for ABI-defined memory address
and size quantities and for validation that can be performed without a
kernel call.

## ABI-backed surface

The public System ABI v0.1 defines:

- `xyris_addr_t` as a 64-bit address quantity.
- `xyris_user_ptr_t` as a 64-bit user pointer quantity.
- `xyris_size_t` as a 64-bit size quantity.
- `xyris_offset_t` as a 64-bit offset quantity.

The SDK exposes:

- `xyris_memory_address_valid()`
- `xyris_memory_range_valid()`

These helpers perform local validation only and do not claim ownership,
mapping, allocation, or protection changes in the kernel.

## Deliberate scope boundary

The public syscall table currently assigns only read, write, open, close,
and exit. It does not assign memory allocation, virtual-memory mapping,
unmapping, protection, shared-memory, or memory-query syscalls.

Therefore this module does not invent memory syscall numbers, structures, or
private-kernel interfaces. Operations requiring kernel participation remain
deferred until a corresponding public ABI contract is assigned.

## Dependencies

The Memory SDK depends on the public Xyris ABI and SDK Core only. No private
kernel headers are required.

## Validation

Both C and C++ public-header consumption tests are registered with CTest.
Tests cover zero-size ranges, valid ranges, valid addresses, and integer
range overflow.

## Definition of Done

A third-party application can include `<xyris/memory.h>` and use the
ABI-backed memory validation helpers without depending on private kernel
internals, while unsupported memory-management operations remain explicitly
deferred.
