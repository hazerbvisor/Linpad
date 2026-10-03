#!/bin/sh
set -eu

# Use portable tool discovery shared by local and CI builds.
. "$(dirname "$0")/build-tools.sh"

# Compile all independent targets so one run reports every compiler failure.
ninja -k 0 "$@"

# The ARM64 app builds all three runtime archives in this directory.
if [ -f libish_emu.a ]; then
    python3 "$SRCROOT/scripts/check-runtime-symbols.py" libish_emu.a --nm "$(xcrun --find nm)"
fi
