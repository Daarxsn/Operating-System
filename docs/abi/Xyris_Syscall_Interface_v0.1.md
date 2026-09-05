# Xyris Syscall Interface v0.1

**Phase:** 6 — Userspace & System Integration  
**Member:** M10 — Purvesh — Xyris Platform, ABI & SDK Architect  
**Status:** v0.1 implemented and kernel-integrated

## 1. Purpose

This specification defines the first stable syscall contract layered on the Xyris System ABI v0.1.

## 2. Syscall numbering

The following numbers are assigned and must not be reused for a different operation:

| Number | Name | Purpose |
|---:|---|---|
| 0 | `XYRIS_SYS_READ` | Read bytes from an open file descriptor |
| 1 | `XYRIS_SYS_WRITE` | Write bytes to an open file descriptor |
| 2 | `XYRIS_SYS_OPEN` | Open a file by path |
| 3 | `XYRIS_SYS_CLOSE` | Close an open file descriptor |
| 4 | `XYRIS_SYS_EXIT` | Terminate the current userspace process |
| 5–9 | Process/thread | Current process identity, thread identity/info, yield, sleep |
| 10–12 | Memory | Map, unmap, and protect user virtual memory |
| 13–16 | IPC | Create, send, receive, and close queued endpoints |
| 17–20 | Events | Create, signal, wait, and close event objects |
| 21–24 | Timers | Create, cancel, wait, and close timers |
| 25–26 | Devices | Enumerate and inspect kernel driver/PCI devices |
| 27–32 | Networking | Socket, bind, connect, send, receive, close loopback endpoints |
| 33–34 | Security | Query identity and check capability rights |

Values at or above `XYRIS_SYS_MAX` are currently unassigned and return `XYRIS_ENOSYS`.

Numbers are never renumbered to fill gaps. Future calls are appended or assigned from explicitly reserved ranges.

## 3. x86-64 calling convention

For the current x86-64 implementation:

| Register | Meaning |
|---|---|
| RAX | syscall number on entry; result on return |
| RDI | argument 1 |
| RSI | argument 2 |
| RDX | argument 3 |
| RCX | argument 4 |

The syscall dispatcher receives five 64-bit ABI values: one number and four arguments.

The register convention is an implementation-specific part of the v0.1 syscall ABI and must not be changed without an ABI compatibility decision.

## 4. Return convention

The syscall result is a signed 64-bit value.

- **0 or greater:** success
- **negative:** Xyris ABI error code
- Positive operation-specific results may represent a file descriptor or byte count.

Examples:

```text
open  -> fd >= 0
read  -> bytes >= 0
write -> bytes >= 0
close -> 0
exit  -> does not normally return
```

An invalid/unimplemented syscall returns `XYRIS_ENOSYS`.

## 5. Current syscall semantics

### READ

Arguments:

```text
RDI = file descriptor
RSI = userspace buffer
RDX = byte count
```

The kernel validates the entire userspace destination range before calling the filesystem read implementation.

### WRITE

Arguments:

```text
RDI = file descriptor
RSI = userspace buffer
RDX = byte count
```

The kernel validates the entire userspace source range before calling the filesystem write implementation.

### OPEN

Arguments:

```text
RDI = userspace NUL-terminated path
```

The kernel copies and validates the path into a bounded kernel buffer of 64 bytes.

### CLOSE

Arguments:

```text
RDI = file descriptor
```

Invalid descriptors return `XYRIS_EBADHANDLE`.

### EXIT

Arguments:

```text
RDI = signed process exit code represented in the 64-bit argument slot
```

Kernel processes are not permitted to terminate through this syscall. A userspace process records its exit code and leaves the current scheduler context.

### Process and thread services

`GETPID` returns the current process ID. `THREAD_SELF` returns the current thread ID. `THREAD_INFO` copies the public thread-info structure to userspace. `THREAD_YIELD` voluntarily schedules another runnable thread. `THREAD_SLEEP` blocks the current thread for the requested duration.

### Memory services

`MEMORY_MAP` allocates page-aligned user pages in the caller's address space. `MEMORY_UNMAP` releases the mapping and owned physical pages. `MEMORY_PROTECT` changes page permissions. Only user processes may request these mappings.

### IPC services

`IPC_CREATE` creates a process-owned bounded endpoint. `IPC_SEND` copies a versioned message descriptor and payload into the endpoint queue. `IPC_RECV` copies the next queued payload to the receiver. `IPC_CLOSE` releases the endpoint. Capabilities are checked by the kernel.

### Event services

Events are process-owned queued kernel objects. Applications can create an event, signal it with four 64-bit payload words, wait for the next event, and close it.

### Timer services

Timers support one-shot and periodic modes with nanosecond ABI values. The current kernel time source has millisecond resolution, so deadlines/intervals are rounded up to the next millisecond.

### Device services

The device API exposes a stable `xyris_device_info_t` view of registered kernel drivers and enumerated PCI functions. Private driver and PCI structures never cross the ABI boundary.

### Networking services

Networking v0.1 is a kernel-managed loopback transport. It supports socket creation, binding, connecting, sending, receiving, and closing. External NIC/IP networking is deliberately outside this revision because the repository does not yet contain a production network stack.

### Security services

`SECURITY_IDENTITY` exposes the caller's stable public identity structure. `SECURITY_CHECK` validates a capability/object pair against requested public security rights using the kernel capability manager.

## 6. User/kernel boundary

Userspace data is untrusted.

For userspace processes the syscall layer checks:

- non-zero user address
- upper user virtual-address limit
- integer overflow in address + size
- process address-space existence
- page presence
- userspace page permission
- writable permission when the kernel writes into the buffer

Zero-length buffers are permitted without requiring a mapped buffer.

Kernel callers remain trusted because kernel tests and internal callers can legitimately use kernel addresses.

## 7. Error translation

The syscall layer translates the current kernel file/process failures into stable Xyris ABI statuses.

This v0.1 mapping is intentionally conservative:

| Kernel-side failure | ABI status |
|---|---|
| Invalid user memory | `XYRIS_EFAULT` |
| Invalid file descriptor | `XYRIS_EBADHANDLE` |
| Failed file open lookup | `XYRIS_ENOTFOUND` |
| Invalid argument/range | `XYRIS_EINVAL` |
| Unsupported syscall | `XYRIS_ENOSYS` |
| Forbidden kernel-process exit | `XYRIS_EPERM` |

Future filesystem and process APIs may expose more precise statuses once their kernel semantics distinguish them.

## 8. Initialization and dispatch

The syscall table is explicitly initialized during kernel boot immediately after ISR initialization and before interrupts are enabled.

`syscall_init()` clears the table and registers the complete v0.1 service set. `syscall_dispatch()` rejects out-of-range numbers and NULL handler slots with `XYRIS_ENOSYS`.

The assigned service groups are: process/thread (5–9), memory (10–12), IPC (13–16), events (17–20), timers (21–24), devices (25–26), networking (27–32), and security (33–34).

This boot-time initialization is required for the IDT/syscall path to function correctly.

## 9. ABI compatibility

The syscall numbers, calling convention, result width and error namespace are part of the v0.1 contract.

Future additions must not change the meaning of existing syscall numbers.

A future incompatible calling convention requires a new ABI major version or an explicitly versioned syscall entry mechanism.

## 10. Scope boundary

v0.1 exposes the kernel-backed SDK service surface listed above. Process creation, application packaging, C/C++ toolchain integration, Rust support, developer commands, compatibility policy, and end-to-end application lifecycle testing remain later M10 workstreams (7.4–7.10). External networking remains outside the v0.1 networking scope; loopback networking is the supported transport.

## 11. Validation

The current implementation is validated at the repository level by:

1. Kernel compilation with the public ABI definitions and `-Wall -Wextra -Werror`.
2. Kernel filesystem tests that exercise the syscall dispatcher for `open`, `read`, and `close`.
3. XyrisSim build/test validation.
4. Full XyrisOS CI validation.

Dedicated standalone C/C++ public-header compilation tests and complete Ring 3 application execution remain appropriate follow-up validation for SDK/userspace integration.

The final M10 acceptance test remains:

`application → SDK → ABI → syscall → kernel → result`.
