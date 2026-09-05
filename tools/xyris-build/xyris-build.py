#!/usr/bin/env python3
"""XyrisOS 7.4 native application build/package helper."""
from __future__ import annotations
import argparse, pathlib, shutil, subprocess, sys

def run(cmd, cwd=None):
    print("+", " ".join(map(str, cmd)))
    subprocess.run([str(x) for x in cmd], cwd=cwd, check=True)

def main():
    p = argparse.ArgumentParser(prog="xyris-build", description="Build a freestanding XyrisOS application")
    p.add_argument("project", help="application source directory")
    p.add_argument("--build-dir", default="build", help="build directory")
    p.add_argument("--generator", default="Ninja")
    p.add_argument("--sdk", default=None, help="installed SDK prefix or source tree")
    p.add_argument("--toolchain", default=None, help="Xyris userspace CMake toolchain file")
    p.add_argument("--clean", action="store_true")
    p.add_argument("--package", action="store_true", help="also emit a .xapp package")
    args = p.parse_args()

    project = pathlib.Path(args.project).resolve()
    build = (project / args.build_dir).resolve()
    if args.clean and build.exists(): shutil.rmtree(build)

    sdk = pathlib.Path(args.sdk).resolve() if args.sdk else None
    toolchain = pathlib.Path(args.toolchain).resolve() if args.toolchain else pathlib.Path(__file__).resolve().parents[2] / "toolchain" / "xyris-user.cmake"
    if not toolchain.exists() and sdk:
        candidate = sdk / "share/xyris/cmake/xyris-user.cmake"
        if candidate.exists(): toolchain = candidate
    cmake_args = ["cmake", "-S", project, "-B", build, "-G", args.generator,
                  f"-DCMAKE_TOOLCHAIN_FILE={toolchain}"]
    if sdk:
        cmake_args.append(f"-DXYRIS_SDK_ROOT={sdk}")
    run(cmake_args)
    run(["cmake", "--build", build, "--config", "Release"])

    if args.package:
        run([sys.executable, pathlib.Path(__file__).with_name("xyris-package.py"), str(project), str(build)])
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
