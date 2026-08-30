# Xyris SDK Process v0.1 — 7.3.2

## Purpose

The Process module is the public SDK surface for process functionality
already available through the Xyris System ABI.

## Implemented operation

xyris_process_exit terminates the calling process with an application exit
code. It delegates to xyris_exit in SDK Core and therefore uses the existing
XYRIS_SYS_EXIT contract.

## Scope boundary

The public Xyris ABI v0.1 currently assigns only read, write, open, close,
and exit syscalls. Therefore this module does not invent process-create,
process-id, process-wait, process-signal, process-spawn, or process-status
syscall numbers or structures.

Those operations remain deferred until their public ABI contracts exist.

## Dependencies

The Process SDK depends only on Xyris SDK Core and the public System ABI.
It does not include private kernel headers.

## Validation

C and C++ public-header consumption tests are registered with CTest.

## Definition of Done

A third-party application can include <xyris/process.h> and link the Process
SDK without depending on private kernel internals, while unsupported process
operations remain explicitly deferred.
