# Xyris SDK v0.1

The Xyris SDK is the public application-facing layer for XyrisOS. All modules
use the public System ABI and do not include private kernel headers.

## 7.3 service coverage

The v0.1 SDK exposes kernel-backed operations for:

- Core: syscall entry, status/error helpers, file primitives.
- Process: current process identity and process exit.
- Thread: current thread identity, information, yield, and sleep.
- Memory: user virtual mapping, unmapping, and protection changes.
- Filesystem: open, close, read, and write against the VFS/RAMFS boundary.
- IPC: owned endpoints, bounded queued messages, capabilities, send/receive, close.
- Events: owned event objects, signal, queued wait, close.
- Timers: one-shot/periodic timers, monotonic/realtime contract, wait/cancel/close.
- Devices: driver and PCI enumeration through a stable device-info structure.
- Networking: deterministic kernel loopback sockets with socket/bind/connect/send/receive/close.
- Security: process identity and capability-right checks.

Networking in v0.1 is intentionally loopback-only because the current kernel has
no production NIC/IP stack. The SDK does not claim external network access.

## Ownership and safety

Kernel-created handles and sockets are owned by the creating process. User
buffers are validated against the current process address space before reads or
writes. Process destruction reclaims SDK-owned mappings, endpoints, events,
timers, and sockets.

## Acceptance criteria for 7.3

1. Every listed SDK module has a public header.
2. Every service module with kernel support has a syscall-backed operation.
3. ABI structures remain fixed-width and versioned.
4. Invalid user pointers and invalid ownership are rejected by the kernel.
5. C and C++ headers compile through the aggregate `xyris/sdk.h` interface.
6. The SDK remains independent of private kernel implementation headers.

Later Member 10 sections (7.4–7.10) add the developer toolchain, Rust,
packaging, documentation depth, compatibility policy, and end-to-end application
lifecycle tests.
