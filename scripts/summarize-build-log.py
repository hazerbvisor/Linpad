#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Extract compiler/linker failures with context from the complete build log."""
import argparse
from pathlib import Path
import re

ANSI = re.compile(r"\x1b\[[0-?]*[ -/]*[@-~]")
ERROR = re.compile(r"(?:fatal )?error:|\bERROR:|^FAILED:|Undefined symbols for architecture|^ld:|^ninja: build stopped|^\*\* BUILD FAILED \*\*", re.IGNORECASE)


def summarize(log):
    lines = ANSI.sub("", log).splitlines()
    spans = []
    for index, line in enumerate(lines):
        if not ERROR.search(line):
            continue
        start, end = max(0, index - 3), min(len(lines), index + 5)
        if spans and start <= spans[-1][1]:
            spans[-1] = (spans[-1][0], max(end, spans[-1][1]))
        else:
            spans.append((start, end))
    if not spans:
        return "No compiler/linker failure marker found; consult the complete xcodebuild.log.\n"
    return "\n".join("Lines {}-{}:\n{}\n".format(start + 1, end, "\n".join(lines[start:end]))
                     for start, end in spans)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    args = parser.parse_args()
    print(summarize(args.log.read_text(errors="replace")), end="")
