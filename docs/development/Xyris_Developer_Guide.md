# XyrisOS Developer Guide

## 1. Repository workflow

Work from a clean source tree. Use CMake/Ninja for the OS and SDK build, CTest for automated validation, and the `xyris-dev` CLI for application workflows.

## 2. Repository layout

- `abi/` — public ABI headers
- `boot-foundation/` — boot/kernel foundation
- `xyris-sdk/` — C/C++ and Rust SDK implementation
- `tools/xyris-build/` — developer and packaging helpers
- `tests/` — automated validation
- `docs/` — project and developer documentation

## 3. Developer CLI

The supported commands are:

```text
xyris-dev doctor
xyris-dev info
xyris-dev init
xyris-dev clean
xyris-dev build
xyris-dev package
xyris-dev validate
```

Use `xyris-dev doctor --json` when integrating diagnostics into scripts.

## 4. Build and test

Configure and build with:

```bash
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

Run repository validation with:

```bash
./scripts/validate.sh
```

Keep generated build directories and package artifacts out of commits.

## 5. SDK changes

Changes to public SDK headers must be checked against the System ABI and syscall specification. Avoid exposing private kernel structures through application-facing headers.

When adding a public interface:

1. Define or update the public contract.
2. Implement the SDK wrapper or binding.
3. Add a focused test.
4. Update the API reference and relevant guide.
5. Run the complete CTest suite.

## 6. ABI and syscall discipline

Do not silently change syscall numbers, field meanings, fixed-width ABI types, or public error semantics. Add a new interface when compatibility requires a new contract. Detailed compatibility policy belongs to M10 7.9.

## 7. Packaging

Application packages use the `.xapp` v1 contract documented in `Xyris_SDK_7.6_XAPP.md`. Package validation must be performed before treating an artifact as distributable.

## 8. Documentation rules

Every public developer-facing feature should have:

- a source-of-truth document or specification;
- an executable example where practical;
- a test or validation rule where practical;
- links from the documentation index.

The 7.8 documentation validator checks required documents, relative links, public SDK header coverage, every supported xyris-dev command, and explicit 7.9/7.10 boundaries.

## 9. Troubleshooting

If `xyris-dev doctor` reports missing tools, install the reported host dependency before attempting the affected workflow. A missing Rust compiler affects Rust application builds but does not invalidate the C/C++ documentation workflow. Missing ISO/QEMU host tools should be reported as unavailable rather than represented as a successful runtime boot.

## 10. Milestone boundaries

- **7.8:** documentation and developer guidance.
- **7.9:** ABI compatibility policy and compatibility validation.
- **7.10:** end-to-end runtime application lifecycle validation.
