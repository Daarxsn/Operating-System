#!/usr/bin/env python3
"""Validate the XyrisOS 7.8 documentation contract against the source tree."""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[2]
DOCS = ROOT / "docs"
required = [
    "README.md",
    "sdk/Xyris_SDK_Getting_Started.md",
    "sdk/Xyris_SDK_API_Reference.md",
    "sdk/Xyris_SDK_7.4_Toolchain.md",
    "sdk/Xyris_SDK_7.5_Rust.md",
    "sdk/Xyris_SDK_7.6_XAPP.md",
    "sdk/Xyris_SDK_7.7_Developer_Tools.md",
    "sdk/Xyris_SDK_7.8_Documentation.md",
    "development/Xyris_Developer_Guide.md",
    "abi/Xyris_System_ABI_v0.1.md",
    "abi/Xyris_Syscall_Interface_v0.1.md",
    "abi/Xyris_SDK_v0.1.md",
]
errors = []
for rel in required:
    if not (DOCS / rel).is_file():
        errors.append(f"missing required document: docs/{rel}")
for md in DOCS.rglob("*.md"):
    text = md.read_text(encoding="utf-8")
    for target in re.findall(r"\[[^\]]+\]\(([^)]+)\)", text):
        target = target.split("#", 1)[0].strip()
        if not target or target.startswith(("http://", "https://", "mailto:")):
            continue
        if not (md.parent / target).resolve().exists():
            errors.append(f"broken link: {md.relative_to(ROOT)} -> {target}")
api = (DOCS / "sdk/Xyris_SDK_API_Reference.md").read_text(encoding="utf-8")
headers = sorted((ROOT / "xyris-sdk").glob("*/include/xyris/*.h"))
for header in headers:
    if f"`<xyris/{header.name}>`" not in api:
        errors.append(f"API reference missing public header: <{header.name}>")
dev = (DOCS / "development/Xyris_Developer_Guide.md").read_text(encoding="utf-8")
for command in ("doctor", "info", "init", "clean", "build", "package", "validate"):
    if f"xyris-dev {command}" not in dev and f"`{command}`" not in dev:
        errors.append(f"developer guide missing xyris-dev command: {command}")
spec = (DOCS / "sdk/Xyris_SDK_7.8_Documentation.md").read_text(encoding="utf-8")
for marker in ("7.9", "7.10"):
    if marker not in spec:
        errors.append(f"7.8 specification missing roadmap boundary: {marker}")
if errors:
    print("7.8 documentation validation: FAIL")
    for error in errors:
        print(f"- {error}")
    sys.exit(1)
print(f"7.8 documentation validation: PASS ({len(list(DOCS.rglob('*.md')))} Markdown files checked)")
print(f"Public SDK headers covered: {len(headers)}")
print("Relative Markdown links: PASS")
print("Developer CLI command coverage: PASS")
print("M10 7.9/7.10 boundary coverage: PASS")
