# Xyris SDK Core v0.1 — 7.3.1

## Purpose

Xyris SDK Core is the first developer-facing layer above the locked Xyris System ABI v0.1 and Syscall Interface v0.1.

Application code should depend on the SDK rather than private kernel headers.

Application
    ↓
Xyris SDK
    ↓
Core
    ↓
Xyris System ABI
    ↓
Syscall Interface
    ↓
Kernel

## Scope

7.3.1 provides:

- raw syscall helpers for zero through four arguments
- one architecture-specific syscall entry implementation
- result and error helpers
- wrappers for the five v0.1 syscalls
- a CMake build target
- C and C++ public-header consumption tests
- install integration for the core static library and public headers

## Public API

The public header is xyris/core.h.

Raw helpers:

- xyris_syscall0
- xyris_syscall1
- xyris_syscall2
- xyris_syscall3
- xyris_syscall4

Convenience wrappers:

- xyris_read
- xyris_write
- xyris_open
- xyris_close
- xyris_exit

Result helpers:

- xyris_succeeded
- xyris_failed
- xyris_error

## ABI relationship

Core does not define a second syscall numbering scheme. It consumes the official XYRIS_SYS_* constants from the public ABI.

The x86-64 implementation maps:

- RAX → syscall number/result
- RDI → argument 1
- RSI → argument 2
- RDX → argument 3
- RCX → argument 4

The entry instruction is int 0x80, matching the locked v0.1 kernel syscall interface.

## Security boundary

Core does not bypass kernel validation.

Pointers are passed as userspace addresses to the kernel, where the syscall layer validates address ranges, mappings, permissions and bounded path access.

Core does not expose process, VFS, memory-manager, scheduler or driver-private structures.

## Build

From xyris-sdk:

    cmake -S . -B build
    cmake --build build

The core library target is xyris-sdk-core.

The public-header tests are:

- xyris-sdk-core-c-header-test
- xyris-sdk-core-cpp-header-test

## Compatibility

SDK versioning is separate from ABI versioning.

Core v0.1 targets Xyris System ABI v0.1. Higher-level SDK modules must preserve that boundary and must not duplicate kernel-private contracts.

## Definition of Done

7.3.1 is complete when a developer can include the public core header and link the core library without including private kernel headers, while the public interface remains aligned with the locked ABI and syscall contracts.

Higher-level modules are intentionally deferred to subsequent 7.3 workstreams.
