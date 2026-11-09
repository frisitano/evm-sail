from __future__ import annotations

import contextlib
import io
import subprocess
import sys
import tempfile
import unittest
from argparse import Namespace
from pathlib import Path
from unittest.mock import patch

from devtools.build_support import BuildSupportError
from devtools.harness import cli
from devtools.harness.known_failures import (
    FailureKey,
    compare_known_failures,
    format_comparison,
    known_failure_keys,
    load_known_failures,
    parse_failure,
)

ROOT = Path(__file__).resolve().parents[2]
CORPUS = "corpus"
FILE_A = "blockchain_tests/a.json"
FILE_B = "blockchain_tests_engine/b.json"
CASE_A = "test_a[fork_Amsterdam-blockchain_test-x]#0"
CASE_B = "test_b[fork_Amsterdam-blockchain_test_engine-y]#engine0"
SHARED = "test_shared[fork_Amsterdam-z]#0"


def entry(file: str, case: str, failure: str = "diff") -> str:
    return f'  {{ file = "{file}", case = "{case}", failure = "{failure}" }},\n'


def write_list(directory: str, *entries: str, header: str = "schema_version = 1\n") -> Path:
    path = Path(directory) / "known.toml"
    path.write_text(f"{header}cases = [\n{''.join(entries)}]\n", encoding="utf-8")
    return path


def fail_line(file: str, case: str, category: str = "diff") -> str:
    return f"FAIL {case} [{CORPUS}/{file}] {category}"


def shard_result(*lines: str, passed: int, total: int) -> subprocess.CompletedProcess[str]:
    output = "\n".join((*lines, f"=== {passed}/{total} passed (optimized guest, byte-exact) ==="))
    return subprocess.CompletedProcess(args=[], returncode=1, stdout=output, stderr="")


class KnownFailureListTests(unittest.TestCase):
    def test_loads_entries_with_their_categories(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = write_list(
                temporary,
                entry(FILE_A, CASE_A),
                entry(FILE_B, CASE_B, "runerr"),
                entry(FILE_B, SHARED),
            )
            self.assertEqual(
                load_known_failures(path),
                {
                    FailureKey(FILE_A, CASE_A): "diff",
                    FailureKey(FILE_B, CASE_B): "runerr",
                    FailureKey(FILE_B, SHARED): "diff",
                },
            )

    def test_an_empty_list_is_valid(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            self.assertEqual(load_known_failures(write_list(temporary)), {})

    def test_rejects_malformed_lists(self) -> None:
        bad_lists = {
            "schema": write_list_args(header="schema_version = 2\n"),
            "unknown category": write_list_args(entry(FILE_A, CASE_A, "crash")),
            "absolute path": write_list_args(entry("/etc/a.json", CASE_A)),
            "parent path": write_list_args(entry("../a.json", CASE_A)),
            "duplicate": write_list_args(entry(FILE_A, CASE_A), entry(FILE_A, CASE_A)),
            "missing field": write_list_args(f'  {{ file = "{FILE_A}", case = "{CASE_A}" }},\n'),
            "extra field": write_list_args(
                f'  {{ file = "{FILE_A}", case = "{CASE_A}", failure = "diff", note = "x" }},\n'
            ),
            "empty case": write_list_args(entry(FILE_A, "")),
        }
        for name, (header, entries) in bad_lists.items():
            with (
                self.subTest(name),
                tempfile.TemporaryDirectory() as temporary,
                self.assertRaises(BuildSupportError),
            ):
                load_known_failures(write_list(temporary, *entries, header=header))

    def test_rejects_invalid_toml(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "known.toml"
            path.write_text("schema_version = [\n", encoding="utf-8")
            with self.assertRaises(BuildSupportError):
                load_known_failures(path)

    def test_the_committed_list_is_valid(self) -> None:
        path = ROOT / "config" / "corpora" / "tests-zkevm-known-failures.toml"
        self.assertIsInstance(load_known_failures(path), dict)

    def test_parses_failures_from_harness_lines(self) -> None:
        self.assertEqual(
            parse_failure(f"FAIL {CASE_A} [/c/{FILE_A}] diff"), (f"/c/{FILE_A}", CASE_A, "diff")
        )
        self.assertEqual(
            parse_failure(f"FAIL {CASE_B} [/c/{FILE_B}] runerr: boom: it broke"),
            (f"/c/{FILE_B}", CASE_B, "runerr"),
        )
        self.assertEqual(
            parse_failure("FAIL x[y z]#0 [dir/f.jsonl] builderr: no input"),
            ("dir/f.jsonl", "x[y z]#0", "builderr"),
        )
        for line in (
            f"PASS {CASE_A}",
            f"FAIL {CASE_A} [/c/{FILE_A}]",
            f"FAIL {CASE_A} [/c/{FILE_A}] exploded",
            "      got_len=1 exp_len=2",
            "=== 1/2 passed (optimized guest, byte-exact) ===",
        ):
            self.assertIsNone(parse_failure(line))

    def test_a_case_that_fails_twice_is_ambiguous(self) -> None:
        with self.assertRaises(BuildSupportError):
            known_failure_keys([(FILE_A, CASE_A, "diff"), (FILE_A, CASE_A, "diff")])

    def test_comparison_names_regressions_fixed_and_changed_cases(self) -> None:
        key_a, key_b, key_c = (
            FailureKey(FILE_A, CASE_A),
            FailureKey(FILE_B, CASE_B),
            FailureKey(FILE_B, SHARED),
        )
        exact = compare_known_failures(
            {key_a: "diff", key_b: "runerr"}, {key_b: "runerr", key_a: "diff"}
        )
        self.assertTrue(exact.matches)
        mixed = compare_known_failures(
            {key_a: "diff", key_c: "diff", key_b: "runerr"},
            {key_a: "diff", key_b: "diff", FailureKey("x", "y"): "diff"},
        )
        self.assertEqual(mixed.regressions, ((key_c, "diff"),))
        self.assertEqual(mixed.fixed, ((FailureKey("x", "y"), "diff"),))
        self.assertEqual(mixed.changed, ((key_b, "diff", "runerr"),))
        report = format_comparison(mixed, "known.toml", 3)
        for expected in (
            "REGRESSION",
            "FIXED",
            "CHANGED",
            f"{FILE_B}::{SHARED}",
            "listed diff, now runerr",
        ):
            self.assertIn(expected, report)


def write_list_args(
    *entries: str, header: str = "schema_version = 1\n"
) -> tuple[str, tuple[str, ...]]:
    return header, entries


class CorpusVerdictTests(unittest.TestCase):
    def run_sharded(
        self,
        listed: list[str],
        shards: list[subprocess.CompletedProcess[str]],
        expected_cases: int | None = None,
    ) -> tuple[int, str]:
        with tempfile.TemporaryDirectory() as temporary:
            path = write_list(temporary, *listed)
            args = Namespace(
                jobs=len(shards),
                quiet=True,
                verbose=False,
                profile=False,
                lean=False,
                python=False,
                build_mode="optimized",
                fork=None,
                limit=0,
                timeout=None,
                fail_limit=0,
                known_failures=str(path),
                known_failure_cases=load_known_failures(path),
                corpus_manifest="manifest.toml",
                expected_cases=expected_cases,
                corpus_roots=[CORPUS],
            )
            files = [f"{CORPUS}/f{index}.json" for index in range(len(shards))]
            output = io.StringIO()
            with (
                patch("devtools.harness.cli.subprocess.run", side_effect=shards),
                contextlib.redirect_stdout(output),
            ):
                status = cli.run_sharded(files, args)
        return status, output.getvalue()

    def shards(self, *failures: tuple[str, str, str]) -> list[subprocess.CompletedProcess[str]]:
        lines = (fail_line(*failure) for failure in failures)
        return [
            shard_result(*lines, passed=8, total=8 + len(failures)),
            shard_result(passed=5, total=5),
        ]

    def test_passes_when_the_failures_equal_the_list(self) -> None:
        listed = [entry(FILE_A, CASE_A), entry(FILE_B, CASE_B, "runerr")]
        shards = self.shards((FILE_A, CASE_A, "diff"), (FILE_B, CASE_B, "runerr"))
        status, output = self.run_sharded(listed, shards)
        self.assertEqual(status, 0, output)
        self.assertIn("match the list exactly", output)

    def test_fails_naming_a_regression(self) -> None:
        status, output = self.run_sharded(
            [entry(FILE_A, CASE_A)], self.shards((FILE_A, CASE_A, "diff"), (FILE_B, CASE_B, "diff"))
        )
        self.assertEqual(status, 1)
        self.assertIn("REGRESSION", output)
        self.assertIn(f"{FILE_B}::{CASE_B}", output)

    def test_a_twin_case_in_another_file_is_a_regression(self) -> None:
        # The same case id in two fixture files is two cases.
        status, output = self.run_sharded(
            [entry(FILE_A, SHARED)], self.shards((FILE_A, SHARED, "diff"), (FILE_B, SHARED, "diff"))
        )
        self.assertEqual(status, 1)
        self.assertIn(f"{FILE_B}::{SHARED}", output)

    def test_fails_naming_a_listed_case_that_passes(self) -> None:
        status, output = self.run_sharded(
            [entry(FILE_A, CASE_A), entry(FILE_B, CASE_B)], self.shards((FILE_A, CASE_A, "diff"))
        )
        self.assertEqual(status, 1)
        self.assertIn("FIXED", output)
        self.assertIn(f"{FILE_B}::{CASE_B}", output)

    def test_fails_when_a_listed_case_changes_category(self) -> None:
        status, output = self.run_sharded(
            [entry(FILE_A, CASE_A, "diff")], self.shards((FILE_A, CASE_A, "runerr"))
        )
        self.assertEqual(status, 1)
        self.assertIn("CHANGED", output)

    def test_an_empty_list_passes_a_clean_run_and_fails_any_failure(self) -> None:
        clean = [shard_result(passed=8, total=8), shard_result(passed=5, total=5)]
        self.assertEqual(self.run_sharded([], clean)[0], 0)
        self.assertEqual(self.run_sharded([], self.shards((FILE_A, CASE_A, "diff")))[0], 1)

    def test_an_unattributable_failure_fails_the_run(self) -> None:
        shards = [
            shard_result("FAIL <unparsable>", passed=8, total=9),
            shard_result(passed=5, total=5),
        ]
        status, output = self.run_sharded([entry(FILE_A, CASE_A)], shards)
        self.assertEqual(status, 1)
        self.assertIn("FAIL lines parsed", output)

    def test_the_case_count_must_match_the_pinned_corpus(self) -> None:
        clean = [shard_result(passed=8, total=8), shard_result(passed=5, total=5)]
        self.assertEqual(self.run_sharded([], clean, expected_cases=13)[0], 0)
        status, output = self.run_sharded([], clean, expected_cases=14)
        self.assertEqual(status, 1)
        self.assertIn("ran 13", output)
        self.assertIn("pins 14", output)


class EarlyValidationTests(unittest.TestCase):
    def main_error(self, *argv: str) -> str:
        stderr = io.StringIO()
        with (
            patch.object(sys, "argv", ["cli.py", *argv]),
            contextlib.redirect_stderr(stderr),
            self.assertRaises(SystemExit) as raised,
        ):
            cli.main()
        self.assertEqual(raised.exception.code, 2)
        return stderr.getvalue()

    def test_known_failures_needs_a_corpus_directory(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = write_list(temporary)
            error = self.main_error("--known-failures", str(path), "one-fixture.json")
        self.assertIn("corpus directory", error)

    def test_a_malformed_list_fails_before_the_run(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = write_list(temporary, entry(FILE_A, CASE_A, "crash"))
            error = self.main_error("--known-failures", str(path), temporary)
        self.assertIn("unknown failure category", error)

    def test_a_malformed_manifest_fails_before_the_run(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            manifest = Path(temporary) / "manifest.toml"
            manifest.write_text("schema_version = 1\n", encoding="utf-8")
            error = self.main_error("--corpus-manifest", str(manifest), temporary)
        self.assertIn("invalid corpus manifest", error)


if __name__ == "__main__":
    unittest.main()
