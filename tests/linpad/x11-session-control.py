#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Optional native Linux X11 control, NOT Linpad/Alpine guest execution.

Run with DISPLAY set by xvfb-run. Pass a compiled tests/linpad/display.c binary:
  xvfb-run -s '-screen 0 800x600x24 -nolisten tcp -extension MIT-SHM -extension GLX' \
    python3 tests/linpad/x11-session-control.py /tmp/display-check xterm
Requires native xclock/xeyes/xterm, xwd, xdotool, xwininfo and core fonts.
"""
import hashlib
import os
from pathlib import Path
import subprocess
import sys
import time
import uuid


def wait_for(predicate, seconds=20):
    end = time.monotonic() + seconds
    while time.monotonic() < end:
        if predicate():
            return
        time.sleep(0.1)
    raise AssertionError("Timed out waiting for real X11 control result")


def main():
    assert os.environ.get("DISPLAY"), "Run under a real X server / xvfb-run"
    decoder = str(Path(sys.argv[1]).resolve())
    app = sys.argv[2] if len(sys.argv) > 2 else "xclock"
    assert app in ("xclock", "xeyes", "xterm")
    repo = Path(__file__).resolve().parents[2]
    script = repo / "app/RootfsPatch.bundle/files/usr/share/linpad/x11-session.sh"
    identifier = uuid.uuid4().hex
    directory = Path("/tmp/linpad-x11-" + identifier)
    directory.mkdir(mode=0o700)
    mailbox = directory / "input"
    mailbox.write_text("")
    frame = directory / "frame.xwd"

    def send(text):
        with mailbox.open("a") as stream:
            stream.write(text)

    subprocess.run(["xdotool", "mousemove", "400", "300"], check=True)
    with (directory / "control.log").open("w") as log:
        session = subprocess.Popen(["sh", str(script), "--inside", identifier, app],
                                   stdout=log, stderr=subprocess.STDOUT)
        try:
            def ready():
                if session.poll() is not None:
                    raise AssertionError((directory / "client.log").read_text() +
                                         (directory / "control.log").read_text())
                return frame.exists()
            wait_for(ready)
            subprocess.run([decoder, str(frame)], check=True)
            initial = hashlib.sha256(frame.read_bytes()).digest()
            if app == "xterm":
                proof = directory / "keyboard-proof"
                command = "printf linpad-input-ok > " + str(proof)
                records = []
                for char in command:
                    key = char if char.isalnum() else ("space" if char == " " else f"U{ord(char):04X}")
                    records.extend((f"down {key}\n", f"up {key}\n"))
                send("".join(records) + "down Return\nup Return\n")
                wait_for(proof.exists)
                assert proof.read_text() == "linpad-input-ok"
            send("move 100 110\nclick 100 110 1\nrelease\n")
            wait_for(lambda: "X=100\nY=110" in subprocess.check_output(
                ["xdotool", "getmouselocation", "--shell"], text=True))
            wait_for(lambda: hashlib.sha256(frame.read_bytes()).digest() != initial)
            send("stop\n")
            assert session.wait(timeout=10) == 0
            subprocess.run([decoder, str(frame)], check=True)
            print(f"Native host {app}: mapped window, changing frames, pointer; " +
                  ("key down/up → xterm PTY → shell command; " if app == "xterm" else "") +
                  f"clean stop. Evidence: {directory}")
        finally:
            if session.poll() is None:
                session.terminate()
                session.wait(timeout=10)


if __name__ == "__main__":
    main()
