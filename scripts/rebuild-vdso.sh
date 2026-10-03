#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Maintainer operation: regenerate the guest ELF and refresh checked-in pins.
set -eu
project=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
cd "$project/vdso/arm64"
temporary=$(mktemp ./prebuilt/.vdso.XXXXXX)
trap 'rm -f "$temporary"' EXIT
"${LINPAD_VDSO_CLANG:-clang}" -target aarch64-linux-gnu -fuse-ld=lld \
    -o "$temporary" vdso.S vdso.c -nostdlib -Wl,-T,vdso.lds \
    -Wl,--hash-style,sysv -shared -fPIC -ffreestanding
python3 - "$temporary" "${LINPAD_VDSO_CLANG:-clang}" <<'PY'
import hashlib, json, pathlib, struct, subprocess, sys
root = pathlib.Path('.')
data = pathlib.Path(sys.argv[1]).read_bytes()
assert data[:7] == b'\x7fELF\x02\x01\x01'
assert struct.unpack_from('<HH', data, 16) == (3, 183) and len(data) <= 8192
manifest = json.loads((root/'prebuilt/manifest.json').read_text())
manifest['sha256'] = hashlib.sha256(data).hexdigest()
manifest['size'] = len(data)
manifest['inputs'] = {name: hashlib.sha256((root/name).read_bytes()).hexdigest() for name in manifest['inputs']}
manifest['compiler'] = subprocess.check_output([sys.argv[2], '--version'], text=True).splitlines()[0]
(root/'prebuilt/libvdso.so.elf').write_bytes(data)
(root/'prebuilt/manifest.json').write_text(json.dumps(manifest, indent=2)+'\n')
PY
python3 prepare-prebuilt.py "$temporary"
echo 'Regenerated VDSO and input pins; review and commit both before distribution.'
