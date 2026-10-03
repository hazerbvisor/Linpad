#!/bin/sh

# Use portable tool discovery shared by local and CI builds.
. "$(dirname "$0")/build-tools.sh"

ninja "$@"
