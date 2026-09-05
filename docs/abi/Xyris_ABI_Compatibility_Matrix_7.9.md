# XyrisOS ABI Compatibility Matrix — 7.9

| Provider | Consumer | Compatibility | Notes |
|---|---|---|---|
| 0.1 | 0.1 | Yes | Baseline contract |
| 0.1 | 0.2+ | Conditional | Only if required interfaces remain compatible |
| 0.2+ | 0.1 | No automatic guarantee | Newer provider may require extensions |
| 1.x | 0.x | No | Major ABI mismatch |
| 0.x | 1.x | No | Major ABI mismatch |

## Required checks

1. Compare ABI major versions.
2. Reject unknown major versions.
3. Verify the required minor-level contract before using optional extensions.
4. For extensible structures, validate `size` and `version` before reading fields.
5. Preserve syscall numbers and existing semantics within a major ABI.
6. Reject `.xapp` packages whose declared ABI is unsupported.

## Layout invariants

- Fixed-width ABI scalar types remain fixed-width.
- Existing field order and alignment remain unchanged.
- Existing sentinel values retain their meaning.
- Reserved fields remain zero until explicitly assigned.
- Existing syscall IDs are never silently repurposed.
