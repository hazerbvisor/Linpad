#!/usr/bin/env python3
"""Validate source/payload pins before embedding the guest-only ELF VDSO."""
# SPDX-License-Identifier: GPL-3.0-only
import hashlib
import json
from pathlib import Path
import struct
import sys


def validate(root):
    root = Path(root)
    manifest = json.loads((root / "prebuilt/manifest.json").read_text())
    for name, expected in manifest["inputs"].items():
        actual = hashlib.sha256((root / name).read_bytes()).hexdigest()
        if actual != expected:
            raise ValueError("Stale prebuilt VDSO input: " + name + "; run scripts/rebuild-vdso.sh")
    data = (root / "prebuilt/libvdso.so.elf").read_bytes()
    if len(data) != manifest["size"] or hashlib.sha256(data).hexdigest() != manifest["sha256"]:
        raise ValueError("Prebuilt VDSO size/SHA-256 mismatch")
    if len(data) < 64 or data[:7] != b"\x7fELF\x02\x01\x01":
        raise ValueError("Expected ELF64 little-endian VDSO")
    if struct.unpack_from("<HH", data, 16) != (3, 183):
        raise ValueError("Expected AArch64 ET_DYN VDSO")
    if len(data) > 8192:
        raise ValueError("VDSO exceeds the two-page runtime allocation")
    for name in ("__kernel_rt_sigreturn", "__kernel_clock_gettime", "__kernel_clock_getres", "__kernel_gettimeofday"):
        if name.encode() + b"\0" not in data:
            raise ValueError("VDSO symbol missing: " + name)
    return data


if __name__ == "__main__":
    data = validate(Path(__file__).resolve().parent)
    output = Path(sys.argv[1])
    if not output.exists() or output.read_bytes() != data:
        output.write_bytes(data)
