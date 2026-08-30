# Xyris SDK Filesystem v0.1 — 7.3.5

## Purpose

The Filesystem module provides the official SDK surface for the filesystem
syscalls currently assigned by the public Xyris System ABI v0.1.

## ABI-backed operations

The public syscall table assigns:

- `XYRIS_SYS_OPEN` — open a path and return a file descriptor.
- `XYRIS_SYS_CLOSE` — close a file descriptor.
- `XYRIS_SYS_READ` — read from a file descriptor.
- `XYRIS_SYS_WRITE` — write to a file descriptor.

The SDK exposes these operations as:

- `xyris_file_open()`
- `xyris_file_close()`
- `xyris_file_read()`
- `xyris_file_write()`

It also provides `xyris_fd_valid()` for local descriptor validation.

The wrappers delegate to SDK Core, which is the only layer responsible for
the actual ABI syscall invocation.

## ABI and compatibility boundary

The module uses only public ABI types and syscall entry points. It does not
include private kernel headers, kernel filesystem structures, or hard-coded
implementation details beyond the public ABI contract.

No directory, rename, remove, stat, permissions, mount, or filesystem-query
API is exposed because those operations do not have assigned public syscall
numbers in ABI v0.1.

## Dependencies

The Filesystem SDK depends on SDK Core and the public Xyris ABI.

## Validation

Both C and C++ public-header consumption tests are registered with CTest.
The tests validate the public file-descriptor contract without performing
kernel I/O during host-side SDK builds.

## Definition of Done

A third-party application can include `<xyris/filesystem.h>` and access all
filesystem operations currently assigned by the public ABI without depending
on private kernel internals.
