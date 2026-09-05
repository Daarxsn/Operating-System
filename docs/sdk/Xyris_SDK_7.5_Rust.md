# Xyris SDK 7.5 — Rust Support

## Goal

7.5 extends the XyrisOS v0.1 developer SDK to Rust while preserving the same
System ABI and syscall contract used by C/C++ applications.

## Delivered

- `no_std` Rust SDK crate.
- Typed syscall/error wrappers over the Xyris `int 0x80` ABI.
- `xyris_entry!` and `xyris_panic_handler!` macros.
- `x86_64-unknown-none` application target configuration.
- Reference Rust application under `tests/rust-app`.
- Cargo integration through the developer build tooling.

## Scope boundary

7.5 provides the Rust SDK/toolchain integration. Full package installation,
application lifecycle management, compatibility policy and broader developer
shell tooling remain later M10 workstreams.
