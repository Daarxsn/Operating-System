# XyrisOS M9.3 — System Service

## Purpose

The first M9 userspace service provides a small system-state snapshot for the
userspace service layer. It proves that a userspace service can consume the
existing public Xyris SDK without accessing private kernel structures.

## Current implementation

`system-service` is registered by userspace `init` and depends on
`userspace-init`. The service currently runs in the init process.

The service records:

- its process ID;
- its thread ID;
- the kernel-reported device count;
- its security identity;
- its security group;
- its security flags.

The service can be started, stopped, queried, and checked for a running state.

## Process boundary

M9.3 does not create a separate daemon process. The current public ABI does not
provide an M9-owned spawn/exec contract. Independent service processes and
service discovery remain a later M9/M10 integration point.

## Dependency direction

```text
userspace init
      ↓
system-service
      ↓
Xyris SDK
      ↓
public ABI
      ↓
kernel services
```

The system service does not include private kernel headers.
