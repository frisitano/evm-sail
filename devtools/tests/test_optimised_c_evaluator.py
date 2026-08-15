from __future__ import annotations

import os
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from devtools.optimised_c.build import (
    MODEL_CFLAGS,
    MODEL_DEFINES,
    ROOT,
    manifest_entries,
    package_makefile,
)
from devtools.optimised_c.evaluate import (
    SAMPLE_STRATA,
    SCHEMA_VERSION,
    display_path,
    gate,
    package_build_gate,
    representative_sample,
    source_metrics,
    strip_c_comments_and_literals,
    validate_record,
)

TMP_ROOT = Path(os.environ.get("AGENT_TMPDIR", ROOT / ".agent-tmp"))


class OptimisedCEvaluatorTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        TMP_ROOT.mkdir(parents=True, exist_ok=True)

    def test_c_lexer_masks_comments_and_literals(self) -> None:
        source = (
            'goto live; /* goto hidden; tmp_comment */ const char *s = "goto tmp_string";\n'
            "// result_comment goto\nresult_live = tmp_live;\n"
        )
        masked = strip_c_comments_and_literals(source)
        self.assertIn("goto live", masked)
        self.assertIn("result_live = tmp_live", masked)
        self.assertNotIn("hidden", masked)
        self.assertNotIn("tmp_string", masked)
        self.assertEqual(masked.count("\n"), source.count("\n"))

    def test_metrics_use_distinct_identifiers_and_lexer_aware_goto(self) -> None:
        with tempfile.TemporaryDirectory(dir=TMP_ROOT) as directory:
            source = Path(directory) / "sample.c"
            source.write_text(
                "int tmp_one;\n\n"
                "int ztmp_two = tmp_one;\n"
                "int result_value = tmp_one; /* goto result_hidden */\n"
                'const char *text = "goto tmp_three";\n'
                "goto done;\n"
            )
            self.assertEqual(
                source_metrics([source]),
                {
                    "manifest_translation_units": 1,
                    "generated_physical_lines": 6,
                    "generated_nonblank_lines": 5,
                    "distinct_temporary_identifiers": 2,
                    "distinct_result_identifiers": 1,
                    "goto_keyword_tokens": 1,
                },
            )

    def test_manifest_rejects_duplicates_and_parent_traversal(self) -> None:
        with tempfile.TemporaryDirectory(dir=TMP_ROOT) as directory:
            manifest = Path(directory) / "sources.list"
            manifest.write_text("a.c\na.c\n")
            with self.assertRaisesRegex(ValueError, "duplicate"):
                manifest_entries(manifest)
            manifest.write_text("../a.c\n")
            with self.assertRaisesRegex(ValueError, "escapes"):
                manifest_entries(manifest)

    def test_package_makefile_uses_shared_flags(self) -> None:
        rendered = package_makefile()
        for flag in (*MODEL_DEFINES, *MODEL_CFLAGS):
            self.assertIn(flag.replace('"', '\\"'), rendered)

    def test_make_sail_source_override_reaches_evaluator_command(self) -> None:
        common = [
            "make",
            "--no-print-directory",
            "-n",
            "c-optimised-evaluate",
            "SAIL=/review/compiler/bin/sail",
        ]
        with_source = subprocess.run(
            [*common, "SAIL_SOURCE=/review/compiler/source"],
            cwd=ROOT,
            check=False,
            capture_output=True,
            text=True,
        )
        self.assertEqual(with_source.returncode, 0, with_source.stderr)
        evaluator_command = with_source.stdout.split(
            "-m devtools.optimised_c.evaluate", maxsplit=1
        )[1]
        self.assertIn("--sail /review/compiler/bin/sail", evaluator_command)
        self.assertIn('--sail-source "/review/compiler/source"', evaluator_command)

        without_source = subprocess.run(
            [*common, "SAIL_SOURCE="],
            cwd=ROOT,
            check=False,
            capture_output=True,
            text=True,
        )
        self.assertEqual(without_source.returncode, 0, without_source.stderr)
        evaluator_command = without_source.stdout.split(
            "-m devtools.optimised_c.evaluate", maxsplit=1
        )[1]
        self.assertNotIn("--sail-source", evaluator_command)

    def test_representative_sample_requires_a_real_anchor(self) -> None:
        with tempfile.TemporaryDirectory(dir=TMP_ROOT) as directory:
            source = Path(directory) / "sample.c"
            source.write_text("int actual_anchor(void) { return 1; }\n")
            sample = representative_sample(source, "test stratum", "actual_anchor")
            self.assertEqual(sample["anchor_symbol"], "actual_anchor")
            with self.assertRaisesRegex(ValueError, "anchor is absent"):
                representative_sample(source, "test stratum", "comment_only_anchor")

    def test_accepted_sample_anchors_exist_in_tracked_extraction(self) -> None:
        generated = ROOT / "extractions/c/optimised/src/src/spec"
        for relative, stratum, anchor in SAMPLE_STRATA:
            sample = representative_sample(generated / relative, stratum, anchor)
            self.assertEqual(sample["stratum"], stratum)

    def test_skipped_gate_has_contract_shaped_evidence(self) -> None:
        record = gate(
            "fixture_smoke",
            "correctness",
            "evm-sail",
            "T1",
            [],
            "skipped",
            "fixture is byte-exact",
            [],
            skip_reason="fixture not selected",
        )
        self.assertIsNone(record["elapsed_seconds"])
        self.assertEqual(record["evidence"], [])
        self.assertEqual(record["owner"], "evm-sail")

    def test_failed_package_build_is_recorded_with_log_evidence(self) -> None:
        with tempfile.TemporaryDirectory(dir=TMP_ROOT) as directory:
            root = Path(directory)
            log = root / "quality/build.log"
            with patch(
                "devtools.optimised_c.evaluate.run",
                return_value=subprocess.CompletedProcess(
                    ["make"], returncode=1, stdout="compile failed\n"
                ),
            ):
                record, written_log = package_build_gate(root, "clang", root / "libevmsail.a", log)
            self.assertEqual(record["status"], "fail")
            self.assertEqual(record["evidence"], [display_path(log)])
            self.assertEqual(written_log, log)
            self.assertEqual(log.read_text(), "compile failed\n")

    def test_record_validator_rejects_non_list_performance(self) -> None:
        digest = "a" * 64
        commit = "b" * 40
        record = {
            "schema_version": SCHEMA_VERSION,
            "run": {
                "environment": {
                    "os": "test",
                    "architecture": "test",
                    "clang_version": None,
                    "clang_format_version": None,
                    "clang_tidy_version": None,
                }
            },
            "repositories": {
                "evm_sail": {"commit": commit},
                "sail": {"commit": commit},
            },
            "compiler": {"binary_sha256": digest},
            "extraction": {"manifest_sha256": digest},
            "artifacts": [],
            "samples": [],
            "gates": [],
            "metrics": [],
            "findings": [],
            "human_review": {},
            "proof_impact": {
                "shared_lowering_changed": "unknown",
                "artifact_hashes_before": [],
                "artifact_hashes_after": [],
                "conclusion": "not_run",
            },
            "fixtures": [],
            "performance": [],
        }
        validate_record(record)
        record["performance"] = {"runs": []}
        with self.assertRaisesRegex(ValueError, "must be a list"):
            validate_record(record)


if __name__ == "__main__":
    unittest.main()
