#!/usr/bin/env python3
import json
import pathlib
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[2]
CLI = ROOT / "tools/xyris-build/xyris-dev.py"

def run(*args):
    return subprocess.run([sys.executable, str(CLI), *args], cwd=ROOT, text=True, capture_output=True)

def main():
    with tempfile.TemporaryDirectory() as td:
        root = pathlib.Path(td)
        project = root / "hello"
        r = run("init", str(project), "--language", "c")
        assert r.returncode == 0, r.stderr
        assert (project / "CMakeLists.txt").exists()
        assert (project / "main.c").exists()
        metadata = json.loads((project / ".xyris-project").read_text())
        assert metadata == {"format": "xyris-project", "version": 1, "name": "hello", "language": "c"}
        r = run("info", "--json")
        assert r.returncode == 0, r.stderr
        info = json.loads(r.stdout)
        assert info["abi"] == "xyris-abi-v0.1"
        assert set(("c", "cpp", "rust")) == set(info["languages"])
        r = run("doctor", "--json")
        assert r.returncode in (0, 1)
        result = json.loads(r.stdout)
        assert "checks" in result and "sdk" in result
    print("xyris-dev CLI tests passed")

if __name__ == "__main__":
    main()
