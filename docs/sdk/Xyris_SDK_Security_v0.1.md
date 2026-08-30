# Xyris SDK Security v0.1 — 7.3.11

## Purpose

The Security module is the official SDK surface for capability identifiers
currently published by XyrisOS.

## ABI-backed surface

ABI v0.1 provides `xyris_capability_t` and `XYRIS_INVALID_CAP`. The SDK
exposes `xyris_capability_valid()` for local validation.

## Deliberate scope boundary

The public syscall table does not currently assign capability grant,
revoke, query, or security-policy syscalls, and the ABI does not publish a
permission or policy structure.

This module therefore does not invent capability semantics, policy layouts,
security syscalls, or private kernel security interfaces. Full security
management remains deferred until the public ABI defines those contracts.

## Definition of Done

A third-party application can include `<xyris/security.h>` and consume the
currently public capability identifier contract without private kernel
dependencies.
