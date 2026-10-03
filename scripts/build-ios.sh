#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Unsigned compilation by default. Explicit trailing xcodebuild settings override.
set -eu
project=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
if [ "$(uname -s)" != Darwin ] || ! command -v xcodebuild >/dev/null 2>&1; then
    echo "An Apple Silicon Mac with Xcode is required; this script cannot build an IPA on Linux." >&2
    exit 1
fi
if [ "$(uname -m)" != arm64 ]; then
    echo "Use an Apple Silicon Mac and an ARM64 device/simulator for the Asbestos backend." >&2
    exit 1
fi
exec xcodebuild -project "$project/iSH.xcodeproj" -scheme iSH-ARM64 \
    -configuration Release -destination 'generic/platform=iOS' \
    -derivedDataPath "${LINPAD_DERIVED_DATA:-$project/build-ios}" \
    ARCHS=arm64 ONLY_ACTIVE_ARCH=YES CODE_SIGNING_ALLOWED=NO \
    "$@" build
