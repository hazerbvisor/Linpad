# SPDX-License-Identifier: GPL-3.0-only
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location("build_log", Path(__file__).resolve().parents[2] / "scripts/summarize-build-log.py")
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class BuildLogTests(unittest.TestCase):
    def test_earlier_error_survives_later_warning_noise(self):
        log = "[12/93] compile\nFAILED: platform_darwin.o\nclang -c platform/darwin.c\n" \
              "platform/darwin.c:19:12: error: unknown type name 'dispatch_once_t'\n" \
              + "warning: incompatible function type\n" * 200 + "ninja: build stopped: subcommand failed.\n"
        summary = module.summarize(log)
        self.assertIn("FAILED: platform_darwin.o", summary)
        self.assertIn("unknown type name 'dispatch_once_t'", summary)
        self.assertIn("clang -c platform/darwin.c", summary)
        self.assertLess(summary.count("warning:"), 10)

    def test_linker_symbol_context_and_ansi(self):
        log = "Undefined symbols for architecture arm64:\n  _sqlite3_open_v2 referenced from fake-db.o\n" \
              "\x1b[31mld: symbol(s) not found for architecture arm64\x1b[0m\n"
        summary = module.summarize(log)
        self.assertIn("_sqlite3_open_v2", summary)
        self.assertNotIn("\x1b", summary)
        self.assertEqual(summary.count("Lines "), 1)

    def test_warning_only_is_not_reported_as_failure(self):
        self.assertIn("No compiler/linker failure marker", module.summarize("warning: unused function\n** BUILD SUCCEEDED **\n"))

    def test_configuration_error_and_empty_log(self):
        self.assertIn("ERROR: missing SDK", module.summarize("ERROR: missing SDK\n"))
        self.assertIn("consult the complete", module.summarize(""))
