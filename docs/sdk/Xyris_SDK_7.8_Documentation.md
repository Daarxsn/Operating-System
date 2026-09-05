# Xyris SDK 7.8 — Documentation

**Status:** Complete
**Scope:** M10 / 7.8 Documentation

## 1. Purpose

Milestone 7.8 turns the SDK, ABI, syscall, packaging, and developer-tool work from 7.1–7.7 into a coherent developer documentation set.

The documentation has three audiences:

- **Application developers** — build a C, C++, or Rust userspace application.
- **SDK/OS developers** — understand the public ABI and service boundaries.
- **Project contributors** — reproduce builds, run validation, and understand the repository layout.

7.8 is documentation-only. It does not introduce a new kernel ABI, syscall number, package format, or runtime behavior.

## 2. Documentation hierarchy

```text
XyrisOS
├── Getting Started
│   ├── prerequisites
│   ├── build the tree
│   └── create a first application
├── Developer Guide
│   ├── repository layout
│   ├── xyris-dev workflow
│   ├── testing and validation
│   └── troubleshooting
├── SDK Reference
│   ├── C/C++ toolchain
│   ├── Rust SDK
│   ├── System ABI
│   ├── syscall interface
│   └── .xapp packaging
└── Architecture / ABI
    ├── public/private boundary
    ├── versioning
    └── compatibility
```

## 3. Source-of-truth documents

| Area | Document |
|---|---|
| System ABI | `docs/abi/Xyris_System_ABI_v0.1.md` |
| Syscalls | `docs/abi/Xyris_Syscall_Interface_v0.1.md` |
| SDK contract | `docs/abi/Xyris_SDK_v0.1.md` |
| C/C++ toolchain | `docs/sdk/Xyris_SDK_7.4_Toolchain.md` |
| Rust SDK | `docs/sdk/Xyris_SDK_7.5_Rust.md` |
| `.xapp` format | `docs/sdk/Xyris_SDK_7.6_XAPP.md` |
| Developer CLI | `docs/sdk/Xyris_SDK_7.7_Developer_Tools.md` |
| Getting started | `docs/sdk/Xyris_SDK_Getting_Started.md` |
| API reference | `docs/sdk/Xyris_SDK_API_Reference.md` |
| Contributor workflow | `docs/development/Xyris_Developer_Guide.md` |

The versioned ABI and package specifications remain authoritative for their respective contracts. The 7.8 documentation layer explains how those contracts fit together and how to use them.

## 4. Developer journey

### 4.1 Discover the environment

```bash
xyris-dev info --json
xyris-dev doctor --json
```

`info` reports the public tool and contract versions. `doctor` reports host prerequisites and returns failure when the environment is not ready.

### 4.2 Create an application

```bash
xyris-dev init hello-xyris --language c --name hello-xyris
```

The generated project contains the minimum metadata and source needed to enter the normal Xyris application workflow.

### 4.3 Build and package

```bash
xyris-dev build hello-xyris --package
```

The packaging path produces a `.xapp` artifact using the v1 package contract.

### 4.4 Validate the artifact

```bash
xyris-dev validate hello-xyris/hello-xyris.xapp
```

Validation checks package structure and manifest/payload integrity according to the 7.6 specification.

## 5. Contract map

```text
Application source
       │
       ▼
Xyris SDK headers / Rust SDK
       │
       ▼
Public System ABI v0.1
       │
       ▼
Syscall Interface v0.1
       │
       ▼
Kernel service implementation

Build output
       │
       ▼
app.elf
       │
       ▼
.xapp v1
       │
       ▼
package validation
```

The application-facing SDK and ABI are public contracts. Kernel-private structures remain outside the application interface.

## 6. Compatibility rules

- Do not reinterpret existing ABI fields or syscall numbers.
- Use the documented fixed-width ABI types instead of private kernel types.
- Treat handles, IDs, and capability values as opaque unless their public specification says otherwise.
- Keep extensible structures compatible by honoring their size/version header.
- Keep package metadata and ABI version fields explicit.
- Add new interfaces instead of silently changing an existing interface.

The detailed compatibility policy is intentionally reserved for M10 7.9.

## 7. Validation expectations

A documentation change is complete when:

1. Every command shown in the guide maps to an implemented tool or script.
2. Paths point to files present in the repository.
3. ABI/syscall/package statements agree with the versioned specifications.
4. Examples do not require private kernel headers.
5. The documentation clearly distinguishes implemented behavior from later roadmap work.

The repository documentation validator checks required documents, relative links, public SDK header coverage, developer CLI command coverage, and the 7.9/7.10 roadmap boundary.

## 8. Out of scope

7.8 does not claim completion of:

- ABI compatibility policy and compatibility testing — 7.9
- complete runtime end-to-end application lifecycle validation — 7.10
- package installation/update lifecycle management
- undocumented kernel internals
- hardware boot support unavailable in the host environment
