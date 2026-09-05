#!/usr/bin/env python3
"""Create a deterministic XyrisOS .xapp package from a built application."""
from __future__ import annotations
import argparse, hashlib, json, pathlib, zipfile

def main():
    p=argparse.ArgumentParser(prog="xyris-package")
    p.add_argument("project"); p.add_argument("build")
    a=p.parse_args(); project=pathlib.Path(a.project).resolve(); build=pathlib.Path(a.build).resolve()
    candidates=sorted(x for x in build.rglob("*") if x.is_file() and x.suffix in {".elf",".bin"})
    if not candidates: raise SystemExit("xyris-package: no ELF/bin artifact found")
    elf=next((x for x in candidates if x.suffix==".elf"), candidates[0])
    out=project/(project.name+".xapp")
    digest=hashlib.sha256(elf.read_bytes()).hexdigest()
    manifest={"format":"xyris-xapp","version":1,"name":project.name,"entry":"app.elf","arch":"x86_64","abi":"xyris-abi-v0.1","sha256":digest}
    with zipfile.ZipFile(out,"w",compression=zipfile.ZIP_DEFLATED) as z:
        info=zipfile.ZipInfo("manifest.json", date_time=(1980,1,1,0,0,0))
        info.compress_type=zipfile.ZIP_DEFLATED
        z.writestr(info, json.dumps(manifest,indent=2,sort_keys=True)+"\n")
        info=zipfile.ZipInfo("app.elf", date_time=(1980,1,1,0,0,0))
        info.compress_type=zipfile.ZIP_DEFLATED
        z.writestr(info, elf.read_bytes())
    print(f"created {out}")
    return 0
if __name__=="__main__": raise SystemExit(main())
