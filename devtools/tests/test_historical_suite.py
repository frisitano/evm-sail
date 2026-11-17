from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from devtools.harness.ssz_builder import EELS_REVISION
from devtools.tests import historical_suite
from devtools.tests.historical_suite import verify_audit, verify_export


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
            fail_export = True

            def command(argv: list[str], *, cwd: Path, env: dict[str, str], log: Path) -> int:
                nonlocal fail_export
                commands.append(argv)
                if log.name == "fill.log":
                    source = log.parent / "source/state_tests"
                    source.mkdir(parents=True)
                    (source / "fixture.json").write_text("{}")
                elif log.name == "export.log":
                    exports = log.parent / "ssz"
                    exports.mkdir()
                    (exports / "fixture.jsonl").write_text("{}\n")
                    if fail_export:
                        fail_export = False
                        log.write_text(
                            "=== 0/1 passed (optimized guest, byte-exact) ===\nfail categories: {'diff': 1}\n"
                        )
                        return 1
                    log.write_text("=== 1/1 passed (optimized guest, byte-exact) === [4 jobs]\n")
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
                    side_effect=[EELS_REVISION, ""] * 3,
                ),
                patch.object(sys, "argv", ["historical_suite", "--directory", str(cache)]),
            ):
                with self.assertRaises(RuntimeError):
                    historical_suite.main()
                commands.clear()
                historical_suite.main()
                # The failed model export does not erase completed reference
                # generation: only the extra group still needs a fill.
                self.assertEqual(sum("--output" in c for c in commands), 1)
                commands.clear()
                historical_suite.main()
            rebuilds = [c for c in commands if "-c" in c and "load_guest(rebuild=True" in c[2]]
            self.assertEqual([c[-1] for c in rebuilds], ["standard", "optimized"])
            self.assertFalse(any("--save-synthesized" in c or "--output" in c for c in commands))
            replays = [c for c in commands if "devtools.harness.cli" in c]
            self.assertEqual(
                [c[c.index("--build") + 1] for c in replays], ["standard", "optimized"] * 2
            )

    def test_export_rejects_model_failures_and_abnormal_workers(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            log = Path(temporary) / "export.log"
            good = "=== 2/3 passed (optimized guest, byte-exact) === [4 jobs]\nfail categories: {'builderr': 1}\n"
            log.write_text(good)
            verify_export(log, 1, 3, 1)
            for transcript, status in (
                (good, 2),
                (good, -9),
                (good, 0),
                (good.replace("'builderr'", "'diff'"), 1),
                (good.replace("'builderr'", "'runerr'"), 1),
                (good.replace("'builderr'", "'noref'"), 1),
                (good.replace("2/3", "1/3"), 1),
                (good + good, 1),
                ("Traceback: worker startup failed", 1),
            ):
                log.write_text(transcript)
                with (
                    self.subTest(status=status, transcript=transcript),
                    self.assertRaises(RuntimeError),
                ):
                    verify_export(log, status, 3, 1)

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
