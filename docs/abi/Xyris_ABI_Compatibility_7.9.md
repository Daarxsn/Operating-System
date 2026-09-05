# XyrisOS ABI Compatibility — M10 7.9

## Purpose

M10 7.9 makes the ABI compatibility contract executable. The baseline is the public Xyris System ABI v0.1 already used by the SDK and `.xapp` packaging layers.

## Version policy

The encoded ABI version is `MAJOR << 16 | MINOR`.

- Same major: compatibility is possible; existing meanings and layouts must remain stable.
- Higher minor: may add compatible interfaces/extensions; consumers must only use contracts they understand.
- Different major: incompatible unless an explicit adapter exists.

ABI version is independent from SDK and application versions.

## Layout policy

ABI boundaries use fixed-width scalar types. Existing structure field order, alignment, widths, and meanings are immutable within a major version. Extensible structures use `xyris_abi_header_t` with caller-supplied `size` and a structure `version`. Consumers must never read beyond the supplied size.

Reserved fields are required to remain zero until a future ABI revision assigns them a meaning.

## Syscall policy

Existing syscall numbers and their argument/return semantics cannot be silently repurposed within an ABI major version. New functionality receives new syscall identifiers or an explicitly versioned interface.

## Compatibility matrix

| Provider | Consumer | Result |
|---|---|---|
| 0.1 | 0.1 | Compatible |
| 0.2+ | 0.1 | Not automatically compatible |
| 0.1 | 0.2+ | Conditional on required contract |
| 1.x | 0.x | Incompatible |
| 0.x | 1.x | Incompatible |

## `.xapp` policy

`.xapp` v1 packages declare an ABI such as `xyris-abi-v0.1`. A loader/tool must reject an unsupported ABI rather than silently executing under a different contract.

## Developer checklist

1. Identify affected public ABI elements.
2. Determine whether the change is additive or breaking.
3. Preserve existing syscall numbers, field order, widths, meanings, and sentinel values for compatible changes.
4. Update the normative specification and compatibility tests.
5. Add migration guidance for breaking changes.
6. Revalidate SDK and `.xapp` tooling.

## Boundary

7.9 validates ABI compatibility rules and contract invariants. M10 7.10 remains responsible for complete boot-to-application end-to-end lifecycle testing.
