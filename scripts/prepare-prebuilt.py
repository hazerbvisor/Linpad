#!/usr/bin/env python3
"""Verify/cache pinned Apple libarchive binaries; never execute downloaded code."""
# SPDX-License-Identifier: GPL-3.0-only
import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import subprocess
import tempfile
import zipfile

PROJECT = Path(__file__).resolve().parent.parent


def digest(path):
    with open(path, "rb") as stream:
        hasher = hashlib.sha256()
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            hasher.update(block)
        return hasher.hexdigest()


def matches(path, entry):
    return path.is_file() and path.stat().st_size == entry["size"] and digest(path) == entry["sha256"]


def prepare(cache, manifest, archive=None):
    cache = Path(cache).resolve()
    cache.mkdir(parents=True, exist_ok=True)
    destination = cache / ("libarchive-" + manifest["version"])
    # Only the known, pinned members can become output paths. Never extractall.
    for entry in manifest["files"]:
        path = PurePosixPath(entry["path"])
        if path.is_absolute() or ".." in path.parts or not path.parts or "\\" in entry["path"]:
            raise ValueError("Unsafe artifact member path")
    if all(matches(destination / e["path"], e) for e in manifest["files"]):
        print("Verified cached prebuilt libarchive " + manifest["version"])
        return destination
    archive = Path(archive) if archive else cache / ("libarchive-" + manifest["version"] + ".zip")
    if not archive.exists():
        # curl uses the builder's standard proxy/TLS trust and fails on HTTP errors.
        with tempfile.TemporaryDirectory(prefix=".download-", dir=cache) as temp:
            downloaded = Path(temp) / "artifact.zip"
            subprocess.run(["curl", "--fail", "--location", "--retry", "2",
                            manifest["url"], "--output", str(downloaded)], check=True)
            if not matches(downloaded, manifest):
                raise ValueError("libarchive archive size/SHA-256 mismatch")
            os.replace(downloaded, archive)
    if not matches(archive, manifest):
        raise ValueError("libarchive archive size/SHA-256 mismatch; remove the invalid cached ZIP and retry")
    # Validate every required member before touching a previously usable cache.
    with zipfile.ZipFile(archive) as bundle:
        members = {}
        for entry in manifest["files"]:
            info = bundle.getinfo(entry["path"])
            if info.file_size != entry["size"]:
                raise ValueError("Unexpected member size: " + entry["path"])
            data = bundle.read(info)
            if hashlib.sha256(data).hexdigest() != entry["sha256"]:
                raise ValueError("Member SHA-256 mismatch: " + entry["path"])
            members[entry["path"]] = data
        for name, data in members.items():
            output = destination / name
            output.parent.mkdir(parents=True, exist_ok=True)
            with tempfile.NamedTemporaryFile(dir=output.parent, delete=False) as stream:
                temporary = Path(stream.name)
                try:
                    stream.write(data)
                    stream.flush()
                    os.replace(temporary, output)
                finally:
                    temporary.unlink(missing_ok=True)
    print("Prepared verified iOS device/simulator libarchive " + manifest["version"])
    return destination


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cache", type=Path, default=PROJECT / "build-data/prebuilt")
    parser.add_argument("--archive", type=Path, help="Use an existing pinned ZIP (offline cache priming)")
    args = parser.parse_args()
    manifest = json.loads((PROJECT / "config/prebuilt-libarchive.json").read_text())
    prepare(args.cache, manifest, args.archive)


if __name__ == "__main__":
    main()
