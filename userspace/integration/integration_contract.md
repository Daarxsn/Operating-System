# XyrisOS Userspace Integration Contract

## Scope

This module validates the composition of the Member 9 userspace components.
It does not redefine the System ABI and it does not access hardware or private
kernel structures.

## Composition

```text
Service Registry ─┐
Service Manager ──┤
System Service ───┤
Utilities ────────┤
Shell ────────────┤
Terminal ─────────┘
        |
        v
  Userspace init
```

## Boundary rules

- Member 9 owns the userspace modules and their composition.
- Member 10 owns the public ABI and SDK contracts used by those modules.
- Member 11 supplies hardware and driver adapters through terminal callbacks.
- Member 12 consumes the userspace services and applications for GUI integration.
- The integration test uses injected I/O and does not require keyboard hardware,
  framebuffer access, QEMU, or a new syscall.

## Validation goals

The integration test covers normal command execution, service lookup, line
editing, overlong-input recovery, input errors, and output errors.
