# XyrisOS Userspace Utilities — M9.6

The utility layer contains reusable user-level command implementations used by
shells and future applications.

## Included utilities

- `echo` — prints command arguments.
- `cat` — reads a file through the public filesystem SDK and emits its bytes.
- `pid` — reports the current process identifier.
- `services` — lists services known to the userspace service registry.
- `service <name>` — reports one registered service.
- `system` — reports the registered system service.

The utility layer does not define new syscalls and does not include private
kernel headers.

## Scope limits

`ls` is not implemented because ABI v0.1 has no directory-enumeration or
`stat` operation. `ps` is not implemented because ABI v0.1 has no process
enumeration operation. These should be added only after an agreed public ABI
exists.

Output uses an injected callback. This keeps utility logic independent of the
future terminal device/input service.
