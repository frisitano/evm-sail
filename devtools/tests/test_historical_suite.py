from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from devtools.harness.ssz_builder import EELS_REVISION
from devtools.tests import historical_suite
from devtools.tests.historical_suite import verify_audit


class HistoricalSuiteAuditTests(unittest.TestCase):
    def test_cached_fixtures_still_rebuild_and_replay_both_backends(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            repo = root / "repo"
            for directory in ("devtools/harness", "devtools/tests", "config/corpora"):
                (repo / directory).mkdir(parents=True)
            (repo / "devtools/tests/full_fork_selection.py").write_text("# selection\n")
            limits: dict[str, object] = {"eels_revision": EELS_REVISION}
            for group in ("main", "extra"):
                limits[group] = {"source_cases": 1, "conversion_limits": []}
            (repo / "config/corpora/historical-conversion-limits.json").write_text(
                json.dumps(limits)
            )
            cache = root / "cache"
            (cache / "execution-specs").mkdir(parents=True)
            commands: list[list[str]] = []

            def command(argv: list[str], *, cwd: Path, env: dict[str, str], log: Path) -> int:
                commands.append(argv)
                if log.name == "fill.log":
                    source = log.parent / "source/state_tests"
                    source.mkdir(parents=True)
                    (source / "fixture.json").write_text("{}")
                elif log.name == "export.log":
                    exports = log.parent / "ssz"
                    exports.mkdir()
                    (exports / "fixture.jsonl").write_text("{}\n")
                elif log.name == "audit.log":
                    (log.parent / "audit.json").write_text(
                        json.dumps(
                            {
                                "expected_cases": 1,
                                "exported_rows": 1,
                                "missing": [],
                                "failures": [],
                                "duplicates": [],
                            }
                        )
                    )
                return 0

            with (
                patch.object(historical_suite, "REPO_ROOT", repo),
                patch.object(historical_suite, "run", side_effect=command),
                patch(
                    "devtools.tests.historical_suite.subprocess.check_output",
                    side_effect=[EELS_REVISION, ""] * 2,
                ),
                patch.object(sys, "argv", ["historical_suite", "--directory", str(cache)]),
            ):
                historical_suite.main()
                commands.clear()
                historical_suite.main()
            rebuilds = [c for c in commands if "-c" in c and "load_guest(rebuild=True" in c[2]]
            self.assertEqual([c[-1] for c in rebuilds], ["standard", "optimized"])
            self.assertFalse(any("--save-synthesized" in c or "--output" in c for c in commands))
            replays = [c for c in commands if "devtools.harness.cli" in c]
            self.assertEqual(
                [c[c.index("--build") + 1] for c in replays], ["standard", "optimized"] * 2
            )

    def test_only_documented_missing_conversions_are_accepted(self) -> None:
        limits: dict[str, object] = {
            "main": {
                "source_cases": 3,
                "conversion_limits": [{"file": "blocked.json", "case": "blocked"}],
            }
        }
        report: dict[str, object] = {
            "expected_cases": 3,
            "exported_rows": 2,
            "missing": [["/fixtures/blocked.json", "blocked"]],
            "failures": [],
            "duplicates": [],
        }
        verify_audit(report, "main", limits)
        changes: tuple[dict[str, object], ...] = (
            {"missing": [["/fixtures/other.json", "blocked"]]},
            {"missing": []},
            {"failures": [{"errors": ["original-receipt-root"]}]},
            {"duplicates": [["/fixtures/one.json", "one"]]},
            {"expected_cases": 4},
            {"exported_rows": 1},
        )
        for changed in changes:
            with self.subTest(changed=changed), self.assertRaises(RuntimeError):
                verify_audit({**report, **changed}, "main", limits)


if __name__ == "__main__":
    unittest.main()
