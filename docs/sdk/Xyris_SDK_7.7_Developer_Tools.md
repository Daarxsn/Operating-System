# Xyris SDK 7.7 — Developer Tools

## Goal

7.7 provides a single developer-facing CLI for discovering the Xyris SDK,
creating application skeletons, building applications, packaging `.xapp`
artifacts, validating packages, and cleaning generated artifacts.

## `xyris-dev`

The command is available from the source tree and is installed with the SDK.

```text
xyris-dev doctor [--json]
xyris-dev info [--json]
xyris-dev init <directory> [--language c|cpp|rust]
xyris-dev build <project> [--clean] [--package]
xyris-dev package <project> <build-dir> [--version VERSION]
xyris-dev validate <package.xapp>
xyris-dev clean <project>
```

### Environment diagnostics

`doctor` checks the SDK, CMake/Ninja, C/C++ compiler availability, Rust/Cargo
availability, userspace toolchain, and packaging/build helpers. It returns a
non-zero status when the environment is not ready.

### Project scaffolding

`init` creates a minimal public-SDK application for C, C++, or Rust and writes a
`.xyris-project` metadata file. Generated applications use the same public ABI
and SDK contract as hand-written applications.

### Build/package workflow

`build` delegates to the existing C/C++/Rust build helper and can immediately
produce a `.xapp`. `package` and `validate` expose the 7.6 package operations
without requiring users to know the underlying Python scripts.

## Scope boundary

7.7 is the developer tooling layer. Package installation/lifecycle management,
ABI compatibility policy, and end-to-end runtime lifecycle validation remain
later M10 workstreams.
