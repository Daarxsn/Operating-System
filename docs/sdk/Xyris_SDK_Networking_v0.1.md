# Xyris SDK Networking v0.1 — 7.3.10

## Purpose

The Networking module is the official SDK surface for networking-related
ABI identifiers currently usable by applications.

## ABI-backed surface

ABI v0.1 provides `xyris_fd_t` and `XYRIS_INVALID_FD`. The SDK exposes
`xyris_network_fd_valid()` for local descriptor validation.

## Deliberate scope boundary

The public syscall table does not currently assign socket, bind, connect,
listen, accept, send, receive, or network-interface syscalls, and the ABI
does not publish socket/address structures.

This module does not invent networking syscall numbers, wire structures, or
private kernel networking interfaces. Full networking operations remain
deferred until the public ABI defines them.

## Definition of Done

A third-party application can include `<xyris/networking.h>` and consume
the currently public networking-related descriptor contract without private
kernel dependencies.
