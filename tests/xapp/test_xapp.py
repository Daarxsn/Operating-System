#!/usr/bin/env python3
import hashlib
import json
import pathlib
import subprocess
import sys
import tempfile
import zipfile

ROOT = pathlib.Path(__file__).resolve().parents[2]
PACKAGER = ROOT / "tools/xyris-build/xyris-package.py"


def run(*args):
    return subprocess.run([sys.executable, str(PACKAGER), *map(str, args)], check=True, capture_output=True, text=True)


def main():
    with tempfile.TemporaryDirectory() as td:
        root = pathlib.Path(td)
        project = root / "demo"
        build = project / "build"
        build.mkdir(parents=True)
        payload = b"ELF-XyrisOS-7.6-test"
        (build / "demo.elf").write_bytes(payload)
        package = project / "demo.xapp"
        run("build", project, build, "--version", "1.2.3")
        run("validate", package)
        with zipfile.ZipFile(package) as z:
            assert set(z.namelist()) == {"manifest.json", "app.elf"}
            manifest = json.loads(z.read("manifest.json"))
            assert manifest["format"] == "xyris-xapp"
            assert manifest["version"] == 1
            assert manifest["app_version"] == "1.2.3"
            assert manifest["sha256"] == hashlib.sha256(payload).hexdigest()
            assert manifest["size"] == len(payload)
            assert z.read("app.elf") == payload

        tampered = root / "tampered.xapp"
        with zipfile.ZipFile(package) as src, zipfile.ZipFile(tampered, "w") as dst:
            for item in src.infolist():
                data = src.read(item.filename)
                if item.filename == "app.elf":
                    data += b"tamper"
                dst.writestr(item, data)
        failed = subprocess.run([sys.executable, str(PACKAGER), "validate", tampered], capture_output=True, text=True)
        assert failed.returncode != 0
        assert "manifest" in failed.stderr
    print("xapp packaging tests: PASS")


if __name__ == "__main__":
    main()
