#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Reject duplicate gadget definitions even if archive extraction hides them."""
import argparse
from collections import Counter
import re
import subprocess

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive")
    parser.add_argument("--nm", default="nm")
    args = parser.parse_args()
    output = subprocess.check_output([args.nm, "-g", "-U", args.archive], text=True)
    definitions = Counter(re.findall(r"\b_?(gadget_[A-Za-z0-9_]+)$", output, re.MULTILINE))
    if not definitions:
        raise SystemExit("No gadget definitions found in runtime archive")
    duplicate = sorted(name for name, count in definitions.items() if count > 1)
    if duplicate:
        raise SystemExit("Duplicate runtime gadgets: " + ", ".join(duplicate))
    print("Verified {} unique runtime gadget definitions".format(len(definitions)))
