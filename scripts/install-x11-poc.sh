#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Optional guest install; the same script is bundled in Linpad's rootfs overlay.
set -eu
project=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
exec sh "$project/app/RootfsPatch.bundle/files/usr/share/linpad/install-x11.sh"
