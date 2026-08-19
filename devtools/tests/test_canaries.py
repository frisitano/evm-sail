from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from devtools.canaries import validate_canaries


class CanaryTests(unittest.TestCase):
    def test_reports_every_missing_and_forbidden_pattern(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "model.txt").write_text("bad marker\n", encoding="utf-8")
            config = root / "canaries.toml"
            config.write_text(
                """
schema_version = 1
[[required]]
path = "model.txt"
pattern = "first"
[[required]]
glob = "*.txt"
pattern = "second"
[[forbidden]]
path = "model.txt"
pattern = "bad"
""",
                encoding="utf-8",
            )
            errors = validate_canaries(root, config)
            self.assertEqual(len(errors), 3)
            self.assertIn("first", errors[0])
            self.assertIn("second", errors[1])
            self.assertIn("forbidden", errors[2])

    def test_accepts_required_and_absent_forbidden_patterns(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "model.txt").write_text("required\n", encoding="utf-8")
            config = root / "canaries.toml"
            config.write_text(
                """
schema_version = 1
[[required]]
path = "model.txt"
pattern = "^required$"
[[forbidden]]
glob = "*.txt"
pattern = "forbidden"
""",
                encoding="utf-8",
            )
            self.assertEqual(validate_canaries(root, config), [])


if __name__ == "__main__":
    unittest.main()
