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

`syscall_init()` clears the table and registers the five supported handlers. `syscall_dispatch()` rejects out-of-range numbers and NULL handler slots with `XYRIS_ENOSYS`.

This boot-time initialization is required for the IDT/syscall path to function correctly.

## 9. ABI compatibility

The syscall numbers, calling convention, result width and error namespace are part of the v0.1 contract.

Future additions must not change the meaning of existing syscall numbers.

A future incompatible calling convention requires a new ABI major version or an explicitly versioned syscall entry mechanism.

## 10. Scope boundary

v0.1 intentionally does not expose process creation, threads, memory mapping, IPC, events, timers, devices or networking because the current syscall layer does not yet provide stable implementations for those services.

Those domains remain reserved for subsequent M10/kernel integration work.

## 11. Validation

The syscall implementation must be validated at three levels:

1. ABI header compile tests.
2. Kernel build with the public ABI definitions.
3. End-to-end userspace execution once Ring 3 entry and complete ELF loading are available.

The final M10 acceptance test remains:

`application → SDK → ABI → syscall → kernel → result`.
