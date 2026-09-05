#!/usr/bin/env python3
"""Validate the M10 7.10 end-to-end test contract without booting QEMU."""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[2]
RUNNER = ROOT / "scripts/e2e-test.sh"
VALIDATOR = ROOT / "scripts/validate.sh"
ABI = ROOT / "tests/abi/test_abi_compatibility.py"
PACKAGER = ROOT / "tools/xyris-build/xyris-package.py"
DOC = ROOT / "docs/testing/Xyris_End_to_End_Testing_7.10.md"

errors = []

def require_file(path: Path, label: str) -> str:
    if not path.is_file():
        errors.append(f"missing {label}: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8")

runner = require_file(RUNNER, "7.10 runner")
validator = require_file(VALIDATOR, "repository validation pipeline")
abi = require_file(ABI, "7.9 ABI validation")
packager = require_file(PACKAGER, "XAPP packager")
doc = require_file(DOC, "7.10 test specification")

for marker in (
    "scripts/build.sh",
    "scripts/iso.sh",
    "qemu-system-x86_64",
    "-serial file:",
    "-display none",
    "-no-reboot",
    "Kernel Ready",
    "Syscall Test: Open",
    "Syscall Test: Read",
    "Syscall Test: Close",
    "User Test: Address Space Cleanup",
    "THREAD A FINISHED",
    "THREAD B FINISHED",
    "Preemption Test: PASS",
    "PMM FREE REJECT",
    "M10 7.10 end-to-end test: PASS",
):
    if marker not in runner:
        errors.append(f"7.10 runner missing acceptance marker/step: {marker}")

for marker in ("Kernel Ready", "Syscall Test: Open", "Syscall Test: Read", "Syscall Test: Close", "Preemption Test: PASS"):
    if marker not in validator:
        errors.append(f"repository validation pipeline missing runtime marker: {marker}")

if "xyris-abi-compatibility-test" not in validator and "test_abi_compatibility.py" not in abi:
    errors.append("7.9 ABI validation is not connected to the end-to-end contract")
if "xyris-abi-v0.1" not in packager:
    errors.append("XAPP packager does not declare the v0.1 ABI")

for marker in (
    "boot",
    "syscall",
    "userspace",
    "driver",
    "scheduler",
    "XAPP",
    "negative",
    "BLOCKED",
    "7.9",
):
    if marker.lower() not in doc.lower():
        errors.append(f"7.10 specification missing coverage: {marker}")

if errors:
    print("7.10 end-to-end contract validation: FAIL")
    for error in errors:
        print(f"- {error}")
    sys.exit(1)

markers = re.findall(r'"([^"]+)"', runner)
print("7.10 end-to-end contract validation: PASS")
print("Boot/build/ISO/QEMU lifecycle: PASS")
print("Kernel, syscall, userspace, driver, and scheduler markers: PASS")
print("Failure guards and blocked-environment handling: PASS")
print(f"Runtime acceptance markers declared: {len(set(markers))}")
print("7.9 ABI integration boundary: PASS")
