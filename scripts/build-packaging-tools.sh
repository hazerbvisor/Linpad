#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Builds fakefsify for the build machine, without the ARM64 execution engine.
set -eu
project=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
output=${1:-$project/build-packaging}
mkdir -p "$output"
output=$(CDPATH= cd -- "$output" && pwd)
cd "$project"
${CC:-cc} -std=gnu11 -O2 -DGUEST_ARM64=1 -I. \
    $(pkg-config --cflags sqlite3 libarchive) \
    tools/fakefsify.c tools/fakefs.c tools/packaging-log.c util/fchdir.c \
    fs/fake-db.c fs/fake-migrate.c fs/fake-rebuild.c \
    $(pkg-config --libs sqlite3 libarchive) -o "$output/fakefsify"
ln -sf fakefsify "$output/unfakefsify"
