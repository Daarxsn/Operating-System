# XyrisOS Userspace Service Contract v0.1

## Purpose

This document defines the initial Member 9 service-manager contract.
It describes userspace service lifecycle and dependency rules.
It does not add or change Xyris ABI syscall numbers.

## Service model

A system service is a long-running userspace component that provides an OS-level function to other userspace components.

The initial service manager is part of userspace `init`.
A service is represented by a name, lifecycle state, dependency list, and optional start/stop callbacks.

M9.2 provides the lifecycle framework. Independent service processes and service-name IPC discovery require a process-spawn and service-discovery interface agreed with Member 10.

## Lifecycle

```text
REGISTERED
    |
    v
 STARTING
    |
    v
 RUNNING
    |
    v
 STOPPING
    |
    v
 STOPPED
```

A failed start moves the service to `FAILED`.
Unresolved dependencies and dependency cycles are also reported as `FAILED` by `start_all`.

## Limits

- Maximum services: 16.
- Maximum service-name length including terminator: 32 bytes.
- Maximum dependencies per service: 4.

## Dependencies

A service can start only when all declared dependencies are `RUNNING`.
Dependencies must be registered before the dependent service starts.
The manager starts services in dependency order.
A running service cannot be stopped while another running service depends on it.
`stop_all` therefore stops services in dependency-safe order.

## Ownership boundary

M9 owns:

- userspace `init`;
- service registration;
- service lifecycle state;
- startup ordering;
- failure detection;
- service shutdown coordination.

M10 owns:

- public ABI definitions;
- syscall numbering and calling convention;
- SDK wrappers;
- new public interfaces needed for process spawning or service discovery.

M11 owns hardware drivers.
M12 owns the GUI and desktop applications.

## M9.2 limitation

The first manager implementation is intentionally in-process.
It does not claim that registered callbacks are independent daemon processes.
The next service-process increment can replace or extend callback startup after the required public process-launch interface is available.
