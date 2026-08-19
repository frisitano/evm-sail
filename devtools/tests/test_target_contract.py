from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from devtools.target_contract import validate_target_contract


class TargetContractTests(unittest.TestCase):
    def test_reports_missing_forbidden_and_help_errors(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            makefile = root / "Makefile"
            makefile.write_text(
                '.PHONY: present legacy\npresent:\nlegacy:\n\t@echo "legacy"\n',
                encoding="utf-8",
            )
            config = root / "targets.toml"
            config.write_text(
                'schema_version = 1\nrequired = ["present", "missing"]\n'
                'forbidden = ["legacy"]\nhelp = ["present"]\n',
                encoding="utf-8",
            )
            errors = validate_target_contract(makefile, config)
            self.assertEqual(len(errors), 3)

    def test_accepts_matching_contract(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            makefile = root / "Makefile"
            makefile.write_text(
                'present:\n\t@echo "  make present - documented"\n',
                encoding="utf-8",
            )
            config = root / "targets.toml"
            config.write_text(
                'schema_version = 1\nrequired = ["present"]\n'
                'forbidden = ["legacy"]\nhelp = ["present"]\n',
                encoding="utf-8",
            )
            self.assertEqual(validate_target_contract(makefile, config), [])


if __name__ == "__main__":
    unittest.main()
