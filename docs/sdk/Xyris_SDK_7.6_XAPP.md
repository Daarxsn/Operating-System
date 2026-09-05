# Xyris SDK 7.6 — `.xapp` Packaging

## Goal

7.6 defines and validates the XyrisOS v0.1 application package format. A built
userspace ELF can be converted into a deterministic `.xapp`, inspected, and
validated before it is handed to a later installation/runtime layer.

## Package format v1

A `.xapp` is a ZIP archive containing exactly:

- `manifest.json`
- `app.elf`

The manifest contains:

| Field | Meaning |
| --- | --- |
| `format` | `xyris-xapp` |
| `version` | Package format version (`1`) |
| `name` | Application identifier, 1–64 characters |
| `app_version` | Application release version supplied by the developer |
| `entry` | Package entry executable (`app.elf`) |
| `arch` | Target architecture (`x86_64`) |
| `abi` | Required System ABI (`xyris-abi-v0.1`) |
| `size` | Exact `app.elf` payload size in bytes |
| `sha256` | SHA-256 digest of `app.elf` |

The payload digest and size are checked during package validation, preventing a
package from being accepted after its executable has been altered.

## Developer commands

Create a package after building an application:

```bash
python3 tools/xyris-build/xyris-package.py build tests/sdk-app tests/sdk-app/build
```

Validate an existing package:

```bash
python3 tools/xyris-build/xyris-package.py validate tests/sdk-app/sdk-app.xapp
```

The `xyris-build --package` workflow uses the same packager automatically.

## Determinism and safety

Package member order, timestamps, and metadata are fixed so identical payloads
produce reproducible archives. Validation rejects corrupt ZIPs, unexpected or
missing members, invalid manifest fields, and payload hash/size mismatches.

## Scope boundary

7.6 establishes the `.xapp` artifact and its validation contract. Package
installation, registration, application launching, runtime lifecycle, and
signature/trust policy remain later M10 workstreams.
