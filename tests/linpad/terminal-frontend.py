#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Bundled Canvas frontend controls. Requires Playwright; no Linux guest runs."""
import argparse
from functools import partial
import http.server
from pathlib import Path
import threading

from playwright.sync_api import sync_playwright


class QuietHandler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, *args):
        pass


NATIVE_BRIDGE = """window.__nativeMessages = [];
window.webkit = {messageHandlers: new Proxy({}, {
    get: (_, name) => ({postMessage: value => window.__nativeMessages.push([name, value])})
})};"""


def check(browser, address):
    page = browser.new_page(viewport={"width": 1280, "height": 850})
    errors = []
    page.on("pageerror", lambda error: errors.append(str(error)))
    page.add_init_script(NATIVE_BRIDGE)
    page.goto(address + "/xterm-term.html")
    page.wait_for_function("window.__nativeMessages.some(([name]) => name === 'load')")
    assert page.evaluate("exports.getSize().every(x => x > 0)")
    assert not errors, errors
    assert page.locator("canvas").count() > 0
    page.evaluate("exports.write('\\x1b[32mTerminal frontend control\\x1b[0m\\r\\n$ ')")
    page.wait_for_function("""Array.from(document.querySelectorAll('canvas')).some(canvas => {
        const context = canvas.getContext('2d');
        return context && context.getImageData(0, 0, canvas.width, canvas.height).data.some((x, i) => i % 4 !== 3 && x > 40);
    })""")
    page.evaluate("window.dispatchEvent(new ErrorEvent('error', {message: 'intentional error control'}))")
    assert page.evaluate("window.__nativeMessages.some(([name, value]) => name === 'frontendError' && value.includes('intentional error control'))")
    page.close()
    broken = browser.new_page()
    broken.add_init_script(NATIVE_BRIDGE)
    broken.route("**/xterm-classic.js", lambda route: route.abort())
    broken.goto(address + "/xterm-term.html")
    broken.wait_for_function("window.__nativeMessages.some(([name]) => name === 'frontendError')")
    assert not broken.evaluate("window.__nativeMessages.some(([name]) => name === 'load')")
    broken.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--chromium", help="Optional installed Chromium executable")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2] / "app/terminal"
    server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), partial(QuietHandler, directory=str(root)))
    threading.Thread(target=server.serve_forever, daemon=True).start()
    try:
        with sync_playwright() as playwright:
            browser = playwright.chromium.launch(executable_path=args.chromium)
            try:
                check(browser, "http://127.0.0.1:" + str(server.server_address[1]))
            finally:
                browser.close()
    finally:
        server.shutdown()
        server.server_close()
    print("Bundled frontend passed: Canvas pixels, dimensions, ready/error bridge, missing-resource path")
