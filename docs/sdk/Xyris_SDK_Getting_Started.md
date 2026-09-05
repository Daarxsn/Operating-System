# Xyris SDK — Getting Started

This guide takes a developer from a clean XyrisOS source tree to a validated application package.

## 1. Prerequisites

The source tree uses CMake/Ninja for native builds and Python-based developer helpers. C/C++ application builds require a suitable compiler. Rust application builds additionally require Rust/Cargo and the supported `x86_64-unknown-none` target.

Check the environment first:

```bash
xyris-dev doctor
```

For machine-readable diagnostics:

```bash
xyris-dev doctor --json
```

## 2. Build XyrisOS

From the repository root:

```bash
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

The repository validation helper can also be used:

```bash
./scripts/validate.sh
```

Runtime ISO/QEMU checks depend on the corresponding host tools being installed.

## 3. Inspect the SDK

```bash
xyris-dev info
```

The command exposes the developer-tool version and the public ABI/package contract versions.

## 4. Create a C application

```bash
xyris-dev init hello-xyris --language c --name hello-xyris
cd hello-xyris
```

Build it from the repository root or through the generated project workflow:

```bash
xyris-dev build hello-xyris
```

Create a package in one step:

```bash
xyris-dev build hello-xyris --package
```

## 5. Create a C++ application

```bash
xyris-dev init hello-cpp --language cpp --name hello-cpp
xyris-dev build hello-cpp --package
```

The C++ application uses the same public Xyris ABI and SDK boundary as the C application.

## 6. Create a Rust application

```bash
xyris-dev init hello-rust --language rust --name hello-rust
```

Rust applications use `no_std` and the stable built-in target:

```bash
cargo build --target x86_64-unknown-none --release
```

The Rust SDK contract and entry-point requirements are documented in `Xyris_SDK_7.5_Rust.md`.

## 7. Validate a package

```bash
xyris-dev validate hello-xyris/hello-xyris.xapp
```

A successful validation means the `.xapp` container, manifest, payload size, and SHA-256 integrity checks satisfy the 7.6 package specification.

## 8. Clean generated artifacts

```bash
xyris-dev clean hello-xyris
```

This removes generated build/package artifacts without changing source files.

## 9. Next references

- ABI: `../abi/Xyris_System_ABI_v0.1.md`
- Syscalls: `../abi/Xyris_Syscall_Interface_v0.1.md`
- C/C++: `Xyris_SDK_7.4_Toolchain.md`
- Rust: `Xyris_SDK_7.5_Rust.md`
- Packaging: `Xyris_SDK_7.6_XAPP.md`
- Developer CLI: `Xyris_SDK_7.7_Developer_Tools.md`
- API overview: `Xyris_SDK_API_Reference.md`
- Contributor workflow: `../development/Xyris_Developer_Guide.md`
