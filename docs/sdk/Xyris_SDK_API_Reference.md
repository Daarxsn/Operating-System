# Xyris SDK — API Reference

## 1. API layers

Xyris application interfaces are layered:

1. **SDK API** — language-facing helpers and headers.
2. **System ABI** — stable public data types and service contracts.
3. **Syscall interface** — kernel transition and syscall numbering.
4. **Kernel implementation** — private service and object internals.

Application code should depend on layers 1–3 only.

## 2. Public service domains

The v0.1 ABI documents these service domains:

| Domain | Purpose |
|---|---|
| Process | Process identity and lifecycle operations |
| Thread | Thread identity and execution operations |
| Memory | User virtual-memory operations |
| Filesystem | Files, directories, and file descriptors |
| IPC | Endpoint/message communication |
| Events | Event objects and subscriptions |
| Timers | Timer creation and timing operations |
| Devices | Device discovery and access contracts |
| Networking | Network endpoints and socket contracts |
| Security | Identity, permissions, and capability checks |

See `docs/abi/Xyris_System_ABI_v0.1.md` for the normative contract.

## 3. Public SDK headers

The C/C++ SDK exposes these public headers:

- `<xyris/sdk.h>` — SDK version contract
- `<xyris/core.h>` — syscall transport and core wrappers
- `<xyris/process.h>` — process identity and lifecycle
- `<xyris/thread.h>` — thread identity and execution
- `<xyris/memory.h>` — memory helpers
- `<xyris/filesystem.h>` — filesystem operations
- `<xyris/ipc.h>` — IPC endpoints and messages
- `<xyris/events.h>` — event interfaces
- `<xyris/timers.h>` — timer interfaces
- `<xyris/devices.h>` — device interfaces
- `<xyris/networking.h>` — networking interfaces
- `<xyris/security.h>` — security and capability interfaces

Include only the headers required by the application. Do not include private kernel headers.

## 4. ABI types and errors

Public interfaces use the fixed-width types and error definitions from `abi/include/xyris/abi`. Handles, object IDs, capability values, and reserved fields must be treated according to the ABI specification rather than inferred from kernel implementation details.

## 5. Syscalls

The syscall interface defines syscall numbers, register transport, argument conventions, return values, and error behavior. Applications should normally use SDK wrappers instead of issuing raw syscalls directly.

The normative source is `docs/abi/Xyris_Syscall_Interface_v0.1.md`.

## 6. Language support

### C/C++

Use the Xyris CMake toolchain and `Xyris::SDK` target documented in `Xyris_SDK_7.4_Toolchain.md`.

### Rust

Use the `no_std` Xyris Rust SDK and `x86_64-unknown-none` target documented in `Xyris_SDK_7.5_Rust.md`.

## 7. Packaging API

The developer packaging interface produces `.xapp` v1 packages containing the documented manifest and `app.elf` payload. The package contract, deterministic layout, validation rules, size, and SHA-256 fields are defined by `Xyris_SDK_7.6_XAPP.md`.

## 8. Developer tooling

The unified `xyris-dev` interface provides:

```text
xyris-dev doctor
xyris-dev info
xyris-dev init
xyris-dev clean
xyris-dev build
xyris-dev package
xyris-dev validate
```

See `Xyris_SDK_7.7_Developer_Tools.md` for command options and workflow examples.

## 9. Compatibility boundary

7.8 documents the current public contract. Detailed ABI compatibility policy, compatibility matrices, and compatibility testing are part of M10 7.9. Full runtime lifecycle validation is part of M10 7.10.
