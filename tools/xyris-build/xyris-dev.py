#!/usr/bin/env python3
"""XyrisOS 7.7 developer CLI."""
from __future__ import annotations

import argparse
import json
import os
import pathlib
import shutil
import subprocess
import sys

VERSION = "0.1.0"


def root_from_cli(sdk: str | None) -> pathlib.Path:
    if sdk:
        return pathlib.Path(sdk).expanduser().resolve()
    if os.environ.get("XYRIS_SDK_ROOT"):
        return pathlib.Path(os.environ["XYRIS_SDK_ROOT"]).expanduser().resolve()
    return pathlib.Path(__file__).resolve().parents[2]


def command_exists(name: str) -> str | None:
    return shutil.which(name)


def doctor(sdk: pathlib.Path) -> dict[str, object]:
    cmake = command_exists("cmake")
    ninja = command_exists("ninja")
    cargo = command_exists("cargo")
    rustc = command_exists("rustc")
    gcc = command_exists("x86_64-elf-gcc") or command_exists("gcc")
    gxx = command_exists("x86_64-elf-g++") or command_exists("g++")
    checks = {
        "sdk": sdk.exists(),
        "sdk_cmake_package": (sdk / "XyrisSDKConfig.cmake").exists()
        or (sdk / "lib/cmake/XyrisSDK/XyrisSDKConfig.cmake").exists(),
        "userspace_toolchain": (sdk / "toolchain/xyris-user.cmake").exists()
        or (sdk / "share/xyris/cmake/xyris-user.cmake").exists(),
        "rust_target": cargo is not None and rustc is not None,
        "cmake": cmake is not None,
        "ninja": ninja is not None,
        "c_compiler": gcc is not None,
        "cxx_compiler": gxx is not None,
        "xyris_build": (sdk / "tools/xyris-build/xyris-build.py").exists()
        or (sdk / "bin/xyris-build.py").exists(),
        "xyris_package": (sdk / "tools/xyris-build/xyris-package.py").exists()
        or (sdk / "bin/xyris-package.py").exists(),
    }
    return {"version": VERSION, "sdk": str(sdk), "checks": checks,
            "ready": all(checks.values())}


def write(path: pathlib.Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def init_project(path: pathlib.Path, language: str, name: str) -> None:
    path.mkdir(parents=True, exist_ok=True)
    if not name:
        name = path.name
    if language == "c":
        write(path / "CMakeLists.txt", f'''cmake_minimum_required(VERSION 3.20)\nproject({name} C)\nset(CMAKE_C_STANDARD 23)\nset(CMAKE_C_STANDARD_REQUIRED ON)\ninclude("${{CMAKE_CURRENT_LIST_DIR}}/../../cmake/xyris/XyrisApplication.cmake")\nxyris_add_application({name} SOURCES main.c LINKER_SCRIPT "${{CMAKE_CURRENT_LIST_DIR}}/user.ld")\n''')
        write(path / "main.c", '#include <xyris/sdk.h>\n\nvoid _start(void) {\n    static const char message[] = "Hello from XyrisOS\\n";\n    (void)xyris_write(1, message, sizeof(message) - 1);\n    (void)xyris_exit(0);\n    for (;;) { __asm__ volatile ("hlt"); }\n}\n')
        write(path / "user.ld", 'ENTRY(_start)\nSECTIONS {\n  . = 0x400000;\n  .text : ALIGN(0x1000) { *(.text*) }\n  .rodata : ALIGN(0x1000) { *(.rodata*) }\n  .data : ALIGN(0x1000) { *(.data*) }\n  .bss : ALIGN(0x1000) { *(COMMON) *(.bss*) }\n  /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }\n}\n')
    elif language == "cpp":
        write(path / "CMakeLists.txt", f'''cmake_minimum_required(VERSION 3.20)\nproject({name} CXX)\nset(CMAKE_CXX_STANDARD 20)\nset(CMAKE_CXX_STANDARD_REQUIRED ON)\ninclude("${{CMAKE_CURRENT_LIST_DIR}}/../../cmake/xyris/XyrisApplication.cmake")\nxyris_add_application({name} SOURCES main.cpp LINKER_SCRIPT "${{CMAKE_CURRENT_LIST_DIR}}/user.ld")\n''')
        write(path / "main.cpp", '#include <xyris/sdk.h>\n\nextern "C" void _start() {\n    static const char message[] = "Hello from XyrisOS C++\\n";\n    (void)xyris_write(1, message, sizeof(message) - 1);\n    (void)xyris_exit(0);\n    for (;;) { __asm__ volatile ("hlt"); }\n}\n')
        write(path / "user.ld", 'ENTRY(_start)\nSECTIONS {\n  . = 0x400000;\n  .text : ALIGN(0x1000) { *(.text*) }\n  .rodata : ALIGN(0x1000) { *(.rodata*) }\n  .data : ALIGN(0x1000) { *(.data*) }\n  .bss : ALIGN(0x1000) { *(COMMON) *(.bss*) }\n  /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }\n}\n')
    elif language == "rust":
        write(path / "Cargo.toml", f'''[package]\nname = "{name}"\nversion = "0.1.0"\nedition = "2021"\n\n[dependencies]\nxyris-sdk = {{ path = "../../xyris-sdk/rust" }}\n''')
        write(path / "src/main.rs", '#![no_std]\n#![no_main]\n\nuse xyris_sdk::write;\n\nfn main() {\n    let message = b"Hello from XyrisOS Rust\\n";\n    let _ = write(1, message);\n}\n\nxyris_sdk::xyris_entry!(main);\nxyris_sdk::xyris_panic_handler!();\n')
    else:
        raise ValueError("language must be c, cpp, or rust")
    write(path / ".xyris-project", json.dumps({"format": "xyris-project", "version": 1, "name": name, "language": language}, indent=2) + "\n")


def run_script(sdk: pathlib.Path, script: str, args: list[str]) -> int:
    candidates = [sdk / "tools/xyris-build" / script, sdk / "bin" / script]
    target = next((p for p in candidates if p.exists()), None)
    if target is None:
        raise SystemExit(f"xyris-dev: {script} not found under {sdk}")
    return subprocess.call([sys.executable, str(target), *args])


def main() -> int:
    parser = argparse.ArgumentParser(prog="xyris-dev", description="XyrisOS 7.7 developer tools")
    parser.add_argument("--sdk", help="SDK source tree or installation prefix")
    parser.add_argument("--version", action="version", version=f"xyris-dev {VERSION}")
    sub = parser.add_subparsers(dest="command", required=True)

    d = sub.add_parser("doctor", help="check developer environment")
    d.add_argument("--json", action="store_true")
    i = sub.add_parser("init", help="create an Xyris application skeleton")
    i.add_argument("path")
    i.add_argument("--language", choices=("c", "cpp", "rust"), default="c")
    i.add_argument("--name")
    info = sub.add_parser("info", help="show SDK/toolchain information")
    info.add_argument("--json", action="store_true")
    clean = sub.add_parser("clean", help="remove application build/package artifacts")
    clean.add_argument("project")
    b = sub.add_parser("build", help="build an application")
    b.add_argument("project")
    b.add_argument("--package", action="store_true")
    b.add_argument("--clean", action="store_true")
    p = sub.add_parser("package", help="build a .xapp package")
    p.add_argument("project")
    p.add_argument("build_dir")
    p.add_argument("--version", default="0.1.0")
    v = sub.add_parser("validate", help="validate a .xapp package")
    v.add_argument("package")

    args = parser.parse_args()
    sdk = root_from_cli(args.sdk)
    if args.command == "doctor":
        result = doctor(sdk)
        print(json.dumps(result, indent=2, sort_keys=True) if args.json else "\n".join(
            [f"Xyris SDK: {result['sdk']}"] + [f"{'PASS' if value else 'FAIL'}  {key}" for key, value in result["checks"].items()] +
            [f"\n{'READY' if result['ready'] else 'NOT READY'}"]
        ))
        return 0 if result["ready"] else 1
    if args.command == "info":
        result = {"tool": "xyris-dev", "version": VERSION, "sdk": str(sdk),
                  "abi": "xyris-abi-v0.1", "xapp_format": "xyris-xapp-v1",
                  "languages": ["c", "cpp", "rust"]}
        print(json.dumps(result, indent=2, sort_keys=True) if args.json else "\n".join(f"{k}: {v}" for k, v in result.items()))
        return 0
    if args.command == "init":
        init_project(pathlib.Path(args.path).resolve(), args.language, args.name or pathlib.Path(args.path).name)
        print(f"created XyrisOS {args.language} project at {pathlib.Path(args.path).resolve()}")
        return 0
    if args.command == "clean":
        project = pathlib.Path(args.project).resolve()
        for rel in ("build", "target"):
            target = project / rel
            if target.exists(): shutil.rmtree(target)
        for package in project.glob("*.xapp"):
            package.unlink()
        print(f"cleaned {project}")
        return 0
    if args.command == "build":
        extra = [str(pathlib.Path(args.project).resolve())]
        if args.clean: extra.append("--clean")
        if args.package: extra.append("--package")
        return run_script(sdk, "xyris-build.py", extra)
    if args.command == "package":
        return run_script(sdk, "xyris-package.py", ["build", str(pathlib.Path(args.project).resolve()), str(pathlib.Path(args.build_dir).resolve()), "--version", args.version])
    if args.command == "validate":
        return run_script(sdk, "xyris-package.py", ["validate", str(pathlib.Path(args.package).resolve())])
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
