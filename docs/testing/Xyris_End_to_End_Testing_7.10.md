# XyrisOS End-to-End Testing — M10 7.10

**Status:** Implemented
**Scope:** complete boot-to-application lifecycle validation

## 1. Purpose

M10 7.10 verifies that the XyrisOS developer platform works as one integrated system rather than only as isolated unit or contract tests.

The acceptance path is:

```text
source tree
   -> clean kernel build
   -> ISO generation
   -> QEMU boot
   -> kernel initialization
   -> syscall execution
   -> userspace lifecycle
   -> device/input paths
   -> thread scheduling/preemption
   -> clean runtime acceptance
```

The test complements the 7.9 ABI contract tests. 7.9 proves compatibility invariants; 7.10 proves that the assembled system reaches the expected runtime checkpoints.

## 2. Canonical runner

Run from the repository root:

```bash
./scripts/e2e-test.sh
```

The runner performs a clean build, creates the bootable ISO, launches `qemu-system-x86_64` headlessly, captures the serial console, and waits for the complete acceptance marker set.

The timeout is 300 seconds by default and can be overridden with `XYRIS_E2E_TIMEOUT`. The serial log can be redirected with `XYRIS_E2E_LOG`.

## 3. Acceptance checkpoints

The runtime must report all of these checkpoints:

- `Kernel Ready`
- syscall open/read/close checks
- `User Test: Address Space Cleanup`
- `Foundation Test: Millisecond Accounting`
- keyboard modifier and pause-sequence checks
- mouse packet event check
- `THREAD A FINISHED`
- `THREAD B FINISHED`
- `Preemption Test: PASS`

The runner also fails on `[FAIL]`, physical-memory `PMM FREE REJECT`, empty serial output, or a missing required marker.

## 4. Test layers

### 4.1 Build layer

A clean CMake/Ninja kernel build is required before runtime testing. This prevents stale objects from masking integration failures.

### 4.2 Image layer

`./scripts/iso.sh` creates the bootable image from the freshly built kernel and the repository's Limine boot assets.

### 4.3 Boot layer

QEMU boots the generated ISO using the same x86-64 machine profile used by the repository validation pipeline. Serial output is captured to a file so acceptance is deterministic and suitable for CI.

### 4.4 Kernel/userspace layer

The marker sequence exercises kernel initialization, syscall dispatch, userspace execution, and address-space cleanup. Syscall checks cover the v0.1 open/read/close boundary.

### 4.5 Driver layer

Keyboard modifier handling, keyboard pause-sequence handling, and mouse packet events must complete successfully before the runtime is accepted.

### 4.6 Scheduler layer

Both test threads must finish and the preemption test must report `PASS`. This establishes that the runtime reached the scheduler completion boundary rather than merely booting.

## 5. XAPP boundary

`.xapp` packaging and manifest integrity remain covered by the 7.6 packaging tests and the 7.9 ABI declaration checks. 7.10 does not invent a second package format. The end-to-end milestone verifies the assembled OS runtime and preserves the existing v1 `xyris-abi-v0.1` package contract.

## 6. Negative and environment-dependent results

A missing `xorriso` or `qemu-system-x86_64` dependency is reported as **BLOCKED** with exit code 2. It is not reported as a passing runtime test.

Once the required host tools exist, a boot that fails to emit the acceptance markers is a **FAIL**. Runtime kernel failure markers also force a failure.

## 7. Existing validation integration

The repository's broader `scripts/validate.sh` pipeline already performs clean build, unresolved-symbol, simulator/CTest, ISO, and QEMU checks. The dedicated 7.10 runner provides a focused, repeatable end-to-end acceptance entry point while retaining the same runtime markers.

The 7.9 ABI compatibility suite remains part of the SDK CTest registration and is a prerequisite contract layer for the application-facing platform.

## 8. Completion criteria

M10 7.10 is complete when:

1. the clean kernel build succeeds;
2. the bootable ISO is generated;
3. QEMU starts the ISO successfully;
4. all required kernel, syscall, userspace, driver, and scheduler markers are observed;
5. no runtime failure or PMM rejection marker is present; and
6. the result is recorded as `M10 7.10 end-to-end test: PASS`.

A source-only environment that cannot run QEMU is **not** equivalent to a runtime pass.
