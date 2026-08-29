# Xyris System ABI Specification v0.1

**Phase:** 6 — Userspace & System Integration  
**Member:** M10 — Purvesh — Xyris Platform, ABI & SDK Architect  
**Status:** Initial implementation baseline

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

The public ABI must not expose private kernel structure layouts.

## 2. Scope of v0.1

v0.1 establishes the **fundamental ABI type and object model**. It does not yet freeze syscall numbers, syscall register conventions, or service-specific operation structures. Those belong to Workstream 7.2 and later ABI sections.

The service domains reserved for the full Xyris ABI are:

- processes
- threads
- memory
- files
- directories
- IPC
- events
- timers
- devices
- networking
- security
- capabilities

A service domain is not considered implemented merely because its type namespace exists; the kernel implementation must be validated before an operation becomes stable API/ABI.

## 3. Design principles

1. **Public ABI is separate from private kernel APIs.** Applications must not include private kernel headers.
2. **SDK/API sits above the ABI.** Developer-facing convenience APIs may change independently when the ABI remains compatible.
3. **ABI values have explicit widths.** Binary layout must not depend on compiler-specific native integer sizes.
4. **Kernel objects are opaque.** Applications use handles and identifiers rather than kernel structure pointers.
5. **User/kernel memory is untrusted.** The kernel validates pointers, sizes, ranges and permissions before access.
6. **ABI evolution is explicit.** Stable interfaces are not silently changed.
7. **Capabilities and permissions are enforced by the kernel.** Identifiers are not authority tokens.
8. **The kernel interface is minimal.** Higher-level functionality should remain in userspace when kernel privilege is unnecessary.

## 4. Fundamental types

| Type | Width | Purpose |
|---|---:|---|
| `xyris_u8` ... `xyris_u64` | 8–64 bit | Fixed-width unsigned ABI values |
| `xyris_i8` ... `xyris_i64` | 8–64 bit | Fixed-width signed ABI values |
| `xyris_size_t` | 64 bit | ABI buffer/object sizes |
| `xyris_ssize_t` | 64 bit | Signed size/result values |
| `xyris_addr_t` | 64 bit | ABI-visible virtual addresses |
| `xyris_user_ptr_t` | 64 bit | User virtual address passed through the ABI |
| `xyris_offset_t` | 64 bit | File/object offsets |
| `xyris_pid_t` | 32 bit | Process identifier |
| `xyris_tid_t` | 32 bit | Thread identifier |
| `xyris_handle_t` | 64 bit | Opaque kernel-object handle |
| `xyris_capability_t` | 64 bit | Capability identifier |
| `xyris_object_id_t` | 64 bit | Generic object/resource identifier |
| `xyris_fd_t` | 32 bit | File descriptor |
| `xyris_time_ns_t` | 64 bit | Timestamp in nanoseconds |
| `xyris_duration_ns_t` | 64 bit | Duration in nanoseconds |
| `xyris_status_t` | 64 bit | Generic ABI status/result |
| `xyris_bool_t` | 8 bit | ABI-stable boolean |

The explicit widths are part of the v0.1 contract for the initial x86-64 implementation.

## 5. Handles and identifiers

`xyris_handle_t` is opaque. Applications must not interpret its bit layout or convert it into a kernel pointer.

`0` is reserved as the invalid handle value.

Process IDs and thread IDs identify objects but do not grant authority to operate on them. Authorization is determined by the applicable security/capability rules.

`xyris_fd_t` uses a signed 32-bit representation; `-1` is reserved as the invalid descriptor sentinel.

## 6. Extensible ABI structures

Structures whose specification permits future extension begin with:

```c
struct xyris_abi_header {
    uint32_t size;
    uint16_t version;
    uint16_t flags;
};
```

`size` is the number of bytes supplied by the caller. `version` identifies the structure revision. `flags` is reserved for structure-specific semantics.

The kernel must validate `size` before reading optional fields. New fields must not change the meaning or position of existing fields in a stable structure revision.

## 7. Error model

v0.1 defines a Xyris-owned signed status namespace. `0` means success and negative values mean failure.

The initial namespace includes:

- `XYRIS_EINVAL` — invalid argument
- `XYRIS_EBADHANDLE` — invalid or unusable handle
- `XYRIS_ENOTFOUND` — requested object not found
- `XYRIS_EPERM` — operation not permitted
- `XYRIS_EACCES` — access denied
- `XYRIS_ENOMEM` — insufficient memory
- `XYRIS_EBUSY` — resource busy
- `XYRIS_EEXIST` — object already exists
- `XYRIS_ENOTSUP` — operation not supported
- `XYRIS_EIO` — I/O failure
- `XYRIS_EINTR` — interrupted
- `XYRIS_EAGAIN` — operation would block / retry later
- `XYRIS_EFAULT` — invalid user memory
- `XYRIS_EBADARG` — ABI argument structure is invalid
- `XYRIS_EOVERFLOW` — arithmetic/size overflow
- `XYRIS_EBADSTATE` — invalid object state
- `XYRIS_ENOSPC` — insufficient storage/resource space
- `XYRIS_ETIMEDOUT` — operation timed out
- `XYRIS_ECONN` — connection failure
- `XYRIS_ECAP` — capability failure
- `XYRIS_ENOSYS` — operation is not implemented

The syscall specification will define how these values are encoded when an operation also returns a positive result such as a byte count or handle.

## 8. User-memory contract

A userspace pointer is an address value, not proof that the referenced memory is valid.

For every syscall that accepts a userspace pointer or buffer, the kernel must validate at least:

- address/range overflow
- userspace address limits
- mapping presence
- required read/write permissions
- operation-specific size constraints
- structure `size`/`version` where applicable

Kernel code must not dereference unvalidated userspace memory directly.

## 9. Compatibility and versioning

The following versions are distinct:

- **ABI version:** binary contract between applications/runtime and the OS
- **SDK/API version:** developer-facing package version
- **Application version:** version chosen by an application developer
- **Package version:** version of an individual `.xapp` package

A compatible SDK release may target an existing ABI without requiring a new ABI version.

Stable ABI interfaces are extended rather than silently redefined. Deprecated interfaces remain documented until the defined removal policy permits removal.

## 10. Current repository alignment

The current repository contains kernel-native fixed-width types, process/thread identifiers, file descriptors, capability/resource foundations, VFS, memory management, an ELF framework, and a syscall dispatcher.

The current syscall implementation is still an early interface with `read`, `write`, `open`, `close`, and `exit`. Therefore this document intentionally does **not** assign final syscall numbers or freeze the CPU register calling convention. Those decisions belong to Workstream 7.2.

Likewise, the existence of a public ABI type does not imply that the corresponding userspace service is already executable end-to-end. Ring 3 entry, complete ELF loading, and userspace runtime integration remain implementation dependencies.

## 11. Public/private boundary

The following are private kernel implementation details and must not become ABI structures:

- `process_t`
- `thread_t`
- scheduler queues and scheduler internals
- VFS node/file implementation layouts
- address-space/page-table implementation structures
- driver-private structures
- kernel allocator internals
- capability implementation tables

The SDK should expose stable opaque handles, IDs and documented structures instead.

## 12. v0.1 acceptance criteria

The v0.1 ABI foundation is accepted when:

1. Public headers compile as C without private kernel includes.
2. Public headers compile as C++ without changing the ABI definitions.
3. Fundamental types have the documented widths on the target x86-64 environment.
4. Sentinel values are explicitly defined.
5. Extensible structure headers have a fixed documented layout.
6. Error constants are Xyris-owned and stable within v0.1.
7. No public ABI header exposes a private kernel structure.

## 13. Next workstream

**7.2 — Syscall Interface** will define and implement:

- syscall numbering
- calling convention/register mapping
- argument and return-value rules
- error encoding
- user/kernel transition
- ABI argument structures
- syscall compatibility/versioning
- validation and testing

Only after that contract is defined should `libxyris` and the complete SDK be built on top of it.
