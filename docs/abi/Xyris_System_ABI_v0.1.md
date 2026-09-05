# Xyris System ABI Specification v0.1

**Phase:** 6 — Userspace & System Integration  
**Member:** M10 — Purvesh — Xyris Platform, ABI & SDK Architect  
**Status:** Implemented — public service-domain ABI contract

## 1. Purpose

The Xyris System ABI is the stable application-facing contract between XyrisOS userspace and operating-system services.

```text
Application
    ↓
Xyris SDK / API
    ↓
Xyris System ABI
    ↓
Syscall Interface
    ↓
Kernel
```

The public ABI does not expose private kernel structure layouts.

## 2. Scope of v0.1

The v0.1 public ABI provides stable contracts for:

- processes
- threads
- memory
- files and directories
- IPC
- events
- timers
- devices
- networking
- security
- capabilities

Each domain has a public header under `abi/include/xyris/abi/` and uses the common ABI type, error, identifier, and versioning rules.

## 3. Design principles

1. Public ABI is separate from private kernel APIs.
2. SDK/API sits above the ABI.
3. ABI values have explicit widths.
4. Kernel objects are opaque and represented by handles/identifiers.
5. User/kernel memory is untrusted and must be validated.
6. ABI evolution is explicit and compatible.
7. Capabilities and permissions are enforced by the kernel.
8. The kernel interface remains minimal.

## 4. Fundamental types

The common `types.h` contract defines fixed-width integers plus ABI quantities including `xyris_size_t`, `xyris_ssize_t`, `xyris_addr_t`, `xyris_user_ptr_t`, `xyris_offset_t`, process/thread IDs, opaque handles, capabilities, object IDs, file descriptors, time/duration values, status values, and an ABI-stable boolean.

The target x86-64 ABI fixes their documented widths; native private kernel types are not substituted for these public types.

## 5. Handles, identifiers and capabilities

`xyris_handle_t` is opaque. Applications must not interpret its bit layout or convert it into a kernel pointer. Process and thread identifiers identify objects but do not themselves grant authority. Capability identifiers are accompanied by explicit rights and security policy structures.

## 6. Extensible ABI structures

Extensible public structures begin with:

```c
struct xyris_abi_header {
    uint32_t size;
    uint16_t version;
    uint16_t flags;
};
```

`size` is the supplied structure size, `version` identifies the structure revision, and `flags` carries structure-specific semantics. Existing field positions and meanings are preserved when structures are extended.

## 7. Service-domain contracts

The public headers define stable structures and enumerations for process state, thread state, memory protection/region information, file and directory entries, IPC messages/endpoints, events/subscriptions, timers, devices, network endpoints/sockets, security identities/policies, and capability information.

These definitions form the ABI contract. In v0.1 the corresponding SDK service layer is kernel-backed for process/thread control, virtual memory mapping, filesystem primitives, queued IPC, events, timers, device enumeration, loopback networking, and security identity/capability checks. Hardware-facing networking beyond loopback remains outside v0.1.

## 8. Error model

`xyris_status_t` uses `0` for success and negative Xyris-owned error values for failure. The initial namespace includes invalid arguments/handles, not-found, permission/access failures, memory/I/O failures, interruption/retry, invalid user memory, overflow, bad state, resource exhaustion, timeout, connection, capability, and unimplemented-operation errors.

## 9. User-memory contract

Userspace pointers are untrusted address values. Syscalls accepting pointers or buffers must validate address/range overflow, userspace limits, mapping presence, required permissions, operation-specific sizes, and ABI structure size/version before access.

## 10. Compatibility and versioning

ABI version, SDK/API version, application version, and package version are separate concepts. Stable ABI structures are extended without silently changing existing meanings. Deprecated interfaces remain documented until their defined removal policy permits removal.

## 11. Public/private boundary

Private structures such as `process_t`, `thread_t`, scheduler queues, VFS implementation layouts, address-space/page-table internals, driver-private structures, allocator internals, and capability tables are not ABI structures. Userspace receives documented handles, IDs, and public structures instead.

## 12. Acceptance criteria

The ABI workstream is accepted when:

1. All public service-domain headers compile without private kernel includes.
2. The public ABI compiles as both C and C++.
3. Fundamental widths and extensible-header layout are compile-time tested.
4. Sentinel values and stable constants are explicitly defined.
5. The aggregate `xyris_abi.h` exposes the complete v0.1 public ABI surface.
6. No public ABI header exposes a private kernel structure.

## 13. Assigned syscall surface

The v0.1 syscall namespace is append-only. The original filesystem/process calls remain stable at 0–4; SDK service calls occupy 5–34:

- 5–9: process/thread
- 10–12: memory
- 13–16: IPC
- 17–20: events
- 21–24: timers
- 25–26: devices
- 27–32: networking
- 33–34: security

Each service validates user pointers and ownership before touching userspace data.

## 14. Relationship to syscall ABI

Workstream **7.2 — Syscall Interface** defines the user/kernel calling convention, syscall numbering, argument structures, result/error semantics, transition vector, and compatibility rules used to transport the public ABI across the kernel boundary.
