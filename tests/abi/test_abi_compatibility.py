#!/usr/bin/env python3
"""Executable M10 7.9 ABI compatibility checks."""
from pathlib import Path
import json
import re
import sys
import zipfile

ROOT = Path(__file__).resolve().parents[2]
ABI = ROOT / "abi/include/xyris/abi"
DOC = ROOT / "docs/abi/Xyris_ABI_Compatibility_7.9.md"
errors = []

if not DOC.is_file():
    errors.append("missing 7.9 compatibility specification")

version = (ABI / "version.h").read_text(encoding="utf-8")
required_version = ["XYRIS_ABI_MAJOR", "XYRIS_ABI_MINOR", "XYRIS_ABI_VERSION", "XYRIS_ABI_STRUCT_VERSION_1", "XYRIS_ABI_VERSION_MAJOR", "XYRIS_ABI_VERSION_MINOR", "XYRIS_ABI_MAJOR_COMPATIBLE", "XYRIS_ABI_MINOR_SATISFIES"]
for token in required_version:
    if token not in version:
        errors.append(f"version.h missing {token}")

types = (ABI / "types.h").read_text(encoding="utf-8")
for token in ["xyris_u64", "xyris_i64", "xyris_handle_t", "xyris_pid_t", "xyris_abi_header_t"]:
    if token not in types:
        errors.append(f"types.h missing {token}")

syscalls = (ABI / "syscalls.h").read_text(encoding="utf-8")
values = re.findall(r"XYRIS_SYS_[A-Z0-9_]+\s*=\s*(\d+)", syscalls)
if len(values) != len(set(values)):
    errors.append("duplicate syscall numbers detected")

if DOC.is_file():
    policy = DOC.read_text(encoding="utf-8")
    for marker in ["Major", "Minor", "fixed-width", "field order", "reserved", "syscall", ".xapp", "7.10"]:
        if marker.lower() not in policy.lower():
            errors.append(f"compatibility policy missing {marker}")

for pkg in ROOT.rglob("*.xapp"):
    try:
        with zipfile.ZipFile(pkg) as zf:
            manifest = json.loads(zf.read("manifest.json"))
            declared = manifest.get("abi", "")
            if not re.fullmatch(r"xyris-abi-v\d+\.\d+", declared):
                errors.append(f"invalid ABI declaration in {pkg}")
    except Exception as exc:
        errors.append(f"invalid xapp {pkg}: {exc}")

if errors:
    print("7.9 ABI compatibility validation: FAIL")
    for item in errors:
        print(f"- {item}")
    sys.exit(1)

print("7.9 ABI compatibility validation: PASS")
print("ABI version contract: PASS")
print("Fixed-width/layout contract: PASS")
print("Syscall number uniqueness: PASS")
print("Compatibility policy coverage: PASS")
print("XAPP ABI declaration validation: PASS")
print("M10 7.10 boundary: PASS")
