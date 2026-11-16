"""The corpus gate rejects partial inventories and failed worker processes."""

from __future__ import annotations

import contextlib
import io
import subprocess
import unittest
from argparse import Namespace
from unittest.mock import patch

from devtools.harness import cli


class CorpusVerdictTests(unittest.TestCase):
    def run_shards(
        self, results: list[subprocess.CompletedProcess[str]], expected: int = 13
    ) -> int:
        args = Namespace(
            jobs=len(results),
            quiet=True,
            verbose=False,
            profile=False,
            lean=False,
            python=False,
            build_mode="optimized",
            save_synthesized=None,
            fork=None,
            limit=0,
            timeout=None,
            fail_limit=0,
            corpus_manifest="manifest.toml",
            expected_cases=expected,
        )
        with (
            patch("devtools.harness.cli.subprocess.run", side_effect=results),
            contextlib.redirect_stdout(io.StringIO()),
        ):
            return cli.run_sharded([f"fixture-{i}.json" for i in range(len(results))], args)

    @staticmethod
    def shard(passed: int, total: int, status: int = 0) -> subprocess.CompletedProcess[str]:
        return subprocess.CompletedProcess(
            [], status, f"=== {passed}/{total} passed (optimized guest, byte-exact) ===", ""
        )

    def test_complete_success(self) -> None:
        self.assertEqual(self.run_shards([self.shard(8, 8), self.shard(5, 5)]), 0)

    def test_any_case_failure(self) -> None:
        self.assertEqual(self.run_shards([self.shard(7, 8, 1), self.shard(5, 5)]), 1)

    def test_missing_case(self) -> None:
        self.assertEqual(self.run_shards([self.shard(8, 8), self.shard(4, 4)]), 1)

    def test_failed_worker_with_success_summary(self) -> None:
        self.assertEqual(self.run_shards([self.shard(8, 8, 1), self.shard(5, 5)]), 1)

    def test_worker_dies_without_summary(self) -> None:
        self.assertEqual(
            self.run_shards([subprocess.CompletedProcess([], -9, "", ""), self.shard(5, 5)]),
            1,
        )

    def test_empty_run(self) -> None:
        self.assertEqual(self.run_shards([self.shard(0, 0)], expected=0), 1)


if __name__ == "__main__":
    unittest.main()
