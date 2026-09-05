# Xyris SDK 7.4 — C/C++ Developer Toolchain

## Goal

7.4 makes the v0.1 Xyris SDK usable as a real application-development kit.
A developer can configure a freestanding x86-64 application against the SDK,
link it without private kernel headers, and package the resulting ELF as a
`.xapp` artifact.

## Delivered

- Dedicated `toolchain/xyris-user.cmake` for x86-64 XyrisOS applications.
- `cmake/xyris/XyrisApplication.cmake` with `xyris_add_application()`.
- Installable `XyrisSDK` CMake package with `Xyris::SDK` target.
- `tools/xyris-build/xyris-build.py` for configure/build automation.
- `tools/xyris-build/xyris-package.py` for deterministic `.xapp` packaging.
- A buildable SDK application smoke test under `tests/sdk-app`.
- Manifest metadata records application name, architecture, ABI and SHA-256.

## Application contract

An application supplies `_start` and links against the public SDK. The default
link is freestanding (`-nostdlib`) and targets x86-64 ELF. SDK modules remain
layered over the public System ABI; private kernel headers are never required.

## Package format v1

A `.xapp` is a ZIP container containing:

- `manifest.json`
- `app.elf`

The manifest declares `format=xyris-xapp`, format `version=1`, `arch=x86_64`,
and `abi=xyris-abi-v0.1`, plus a SHA-256 digest of `app.elf`.

## Scope boundary

7.4 is the C/C++ developer toolchain layer. Rust support, richer application
packaging/install lifecycle, developer shell commands, compatibility policy,
and full end-to-end application lifecycle validation remain later workstreams.
