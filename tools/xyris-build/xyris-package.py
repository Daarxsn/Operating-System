#!/usr/bin/env python3
"""Build and validate XyrisOS .xapp application packages."""
from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import re
import tempfile
import zipfile

FORMAT = "xyris-xapp"
FORMAT_VERSION = 1
ABI = "xyris-abi-v0.1"
ARCH = "x86_64"
REQUIRED_FILES = {"manifest.json", "app.elf"}
NAME_RE = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$")


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def manifest_for(name: str, app_version: str, payload: bytes) -> dict[str, object]:
    return {
        "abi": ABI,
        "arch": ARCH,
        "entry": "app.elf",
        "format": FORMAT,
        "name": name,
        "sha256": sha256(payload),
        "size": len(payload),
        "version": FORMAT_VERSION,
        "app_version": app_version,
    }


def load_manifest(z: zipfile.ZipFile) -> dict[str, object]:
    try:
        raw = z.read("manifest.json")
    except KeyError as exc:
        raise ValueError("missing manifest.json") from exc
    try:
        manifest = json.loads(raw.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ValueError("manifest.json is not valid UTF-8 JSON") from exc
    if not isinstance(manifest, dict):
        raise ValueError("manifest.json must contain a JSON object")
    return manifest


def validate_package(path: pathlib.Path) -> dict[str, object]:
    if not path.is_file():
        raise ValueError(f"package not found: {path}")
    try:
        with zipfile.ZipFile(path, "r") as z:
            if z.testzip() is not None:
                raise ValueError("package contains a corrupt ZIP member")
            names = set(z.namelist())
            if names != REQUIRED_FILES:
                extra = sorted(names - REQUIRED_FILES)
                missing = sorted(REQUIRED_FILES - names)
                detail = []
                if missing:
                    detail.append("missing=" + ",".join(missing))
                if extra:
                    detail.append("unexpected=" + ",".join(extra))
                raise ValueError("invalid package members (" + "; ".join(detail) + ")")
            manifest = load_manifest(z)
            expected = {
                "format": FORMAT,
                "version": FORMAT_VERSION,
                "entry": "app.elf",
                "arch": ARCH,
                "abi": ABI,
            }
            for key, value in expected.items():
                if manifest.get(key) != value:
                    raise ValueError(f"manifest field {key!r} must be {value!r}")
            name = manifest.get("name")
            if not isinstance(name, str) or not NAME_RE.fullmatch(name):
                raise ValueError("manifest name is invalid")
            app_version = manifest.get("app_version")
            if not isinstance(app_version, str) or not app_version.strip():
                raise ValueError("manifest app_version must be a non-empty string")
            payload = z.read("app.elf")
            if manifest.get("size") != len(payload):
                raise ValueError("manifest size does not match app.elf")
            if manifest.get("sha256") != sha256(payload):
                raise ValueError("manifest sha256 does not match app.elf")
            return manifest
    except zipfile.BadZipFile as exc:
        raise ValueError("not a valid ZIP/XAPP archive") from exc


def build_package(project: pathlib.Path, build: pathlib.Path, output: pathlib.Path,
                  name: str | None, app_version: str) -> pathlib.Path:
    candidates = sorted(
        x for x in build.rglob("*")
        if x.is_file() and x.suffix.lower() in {".elf", ".bin"}
    )
    if not candidates:
        raise ValueError("no ELF/bin artifact found")
    elf = next((x for x in candidates if x.suffix.lower() == ".elf"), candidates[0])
    package_name = name or project.name
    if not NAME_RE.fullmatch(package_name):
        raise ValueError("application name must match [A-Za-z0-9][A-Za-z0-9._-]{0,63}")
    payload = elf.read_bytes()
    manifest = manifest_for(package_name, app_version, payload)

    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=output.parent, prefix=output.name + ".", delete=False) as tmp:
        temp_path = pathlib.Path(tmp.name)
    try:
        with zipfile.ZipFile(temp_path, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
            for member, data in (
                ("manifest.json", json.dumps(manifest, indent=2, sort_keys=True).encode("utf-8") + b"\n"),
                ("app.elf", payload),
            ):
                info = zipfile.ZipInfo(member, date_time=(1980, 1, 1, 0, 0, 0))
                info.compress_type = zipfile.ZIP_DEFLATED
                info.external_attr = 0o100644 << 16
                z.writestr(info, data)
        validate_package(temp_path)
        temp_path.replace(output)
    finally:
        temp_path.unlink(missing_ok=True)
    return output


def main() -> int:
    parser = argparse.ArgumentParser(prog="xyris-package")
    sub = parser.add_subparsers(dest="command", required=True)

    build = sub.add_parser("build", help="create a .xapp from a built application")
    build.add_argument("project")
    build.add_argument("build_dir")
    build.add_argument("-o", "--output")
    build.add_argument("--name")
    build.add_argument("--version", default="0.1.0")

    check = sub.add_parser("validate", help="validate an existing .xapp package")
    check.add_argument("package")

    args = parser.parse_args()
    try:
        if args.command == "build":
            project = pathlib.Path(args.project).resolve()
            build_dir = pathlib.Path(args.build_dir).resolve()
            output = pathlib.Path(args.output).resolve() if args.output else project / f"{args.name or project.name}.xapp"
            result = build_package(project, build_dir, output, args.name, args.version)
            print(f"created {result}")
        else:
            manifest = validate_package(pathlib.Path(args.package).resolve())
            print(f"valid {args.package}: {manifest['name']} {manifest['app_version']}")
    except ValueError as exc:
        parser.error(str(exc))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
