from __future__ import annotations

import json
import os
import subprocess
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

from tools.evaluate_optimised_c import (
    EXTRACTION_PROVENANCE_SCHEMA,
    SAMPLE_STRATA,
    SCHEMA_VERSION,
    compiler_identity,
    compiler_library_identity,
    compiler_plugin_identity,
    discover_effective_sail_executable,
    discover_sail_source,
    display_path,
    gate,
    generated_source_tree_identity,
    intermediate_tuple_identifiers,
    load_extraction_provenance_stamp,
    package_build_gate,
    representative_sample,
    result_identifiers,
    resolve_compiler_provenance,
    source_metrics,
    strip_c_comments_and_literals,
    terminal_zero_metric_gate,
    validate_record,
)
from tools.optimised_c_build import (
    MODEL_CFLAGS,
    MODEL_DEFINES,
    ROOT,
    manifest_entries,
    package_makefile,
)
from tools.run_optimised_c_extraction import (
    PRE_EXTRACTION_SCHEMA,
    ensure_compiler_unchanged,
    finalize_extraction,
    run_extraction,
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
                    "distinct_intermediate_tuple_identifiers": 0,
                    "goto_keyword_tokens": 1,
                },
            )

    def test_result_metric_catches_semantic_prefix_without_comments_or_literals(self) -> None:
        source = (
            "struct OpcodeOutcome Continue_result_2_1752 = Continue(UNIT);\n"
            "struct LogTopics LogTopics0_result_2_1745 = LogTopics0(UNIT);\n"
            "uint64_t result_value = 0;\n"
            "uint64_t domain_result = 0;\n"
            'const char *hidden = "Failed_result_2_1753 result_hidden";\n'
            "// Exceptional_result_2_1999\n"
        )
        self.assertEqual(
            result_identifiers(source),
            {
                "Continue_result_2_1752",
                "LogTopics0_result_2_1745",
                "result_value",
            },
        )

    def test_intermediate_tuple_metric_excludes_semantic_tuple_abi(self) -> None:
        source = (
            "struct tuple_bool_uint_64 api_result(uint64_t value);\n"
            "void consume(struct tuple_bool_uint_64 parameter);\n"
            "struct tuple_bool_uint_64 semantic_global;\n"
            "struct tuple_bool_uint_64 global_result_5_23;\n"
            "struct wrapper { struct tuple_bool_uint_64 field_6_24; };\n"
            "struct tuple_bool_uint_64 return_plumbing(void) {\n"
            "  struct tuple_bool_uint_64 return_result_9_20 = api_result(3);\n"
            "  return return_result_9_20;\n"
            "}\n"
            "void evaluate(void) {\n"
            "  struct tuple_bool_uint_64 call_result_2_17 = api_result(1);\n"
            "  struct /* stable across comments */ "
            "tuple_bool_uint_64 unpack_result_3_18;\n"
            "  bool unpacked = unpack_result_3_18.tup0;\n"
            "  struct tuple_bool_uint_64 assignment_result_8_19;\n"
            "  assignment_result_8_19 = api_result(2);\n"
            "  struct tuple_bool_uint_64 semantic_state;\n"
            '  const char *hidden = "struct tuple_bool_uint_64 tmp_8_99;";\n'
            "  // struct tuple_bool_uint_64 result_8_88;\n"
            "  semantic_state = (struct tuple_bool_uint_64){.tup0 = true};\n"
            "  call_result_2_17 = semantic_state;\n"
            "  if (unpacked) consume(semantic_state);\n"
            "}\n"
        )
        self.assertEqual(
            intermediate_tuple_identifiers(source),
            {
                "assignment_result_8_19",
                "call_result_2_17",
                "return_result_9_20",
                "unpack_result_3_18",
            },
        )

    def test_terminal_zero_metric_gate_fails_each_nonzero_metric(self) -> None:
        metric_ids = (
            "distinct_temporary_identifiers",
            "distinct_result_identifiers",
            "distinct_intermediate_tuple_identifiers",
        )
        for nonzero_metric in metric_ids:
            with self.subTest(nonzero_metric=nonzero_metric):
                values = {metric_id: 0 for metric_id in metric_ids}
                values[nonzero_metric] = 1
                record = terminal_zero_metric_gate(values, Path("sources.list"))
                self.assertEqual(record["id"], "terminal_zero_identifier_metrics")
                self.assertEqual(record["status"], "fail")

    def test_terminal_zero_metric_gate_passes_only_all_zero(self) -> None:
        record = terminal_zero_metric_gate(
            {
                "distinct_temporary_identifiers": 0,
                "distinct_result_identifiers": 0,
                "distinct_intermediate_tuple_identifiers": 0,
            },
            Path("sources.list"),
        )
        self.assertEqual(record["id"], "terminal_zero_identifier_metrics")
        self.assertEqual(record["status"], "pass")

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
            "tools/evaluate_optimised_c.py", maxsplit=1
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
            "tools/evaluate_optimised_c.py", maxsplit=1
        )[1]
        self.assertNotIn("--sail-source", evaluator_command)

    def test_make_effective_binary_override_reaches_evaluator_command(self) -> None:
        command = subprocess.run(
            [
                "make",
                "--no-print-directory",
                "-n",
                "c-optimised-evaluate",
                "SAIL=/review/compiler/sail",
                "SAIL_SOURCE=/review/compiler/source",
                "SAIL_EFFECTIVE_BINARY=/review/compiler/bin/sail",
            ],
            cwd=ROOT,
            check=False,
            capture_output=True,
            text=True,
        )
        self.assertEqual(command.returncode, 0, command.stderr)
        evaluator_command = command.stdout.split(
            "tools/evaluate_optimised_c.py", maxsplit=1
        )[1]
        self.assertIn(
            '--sail-effective-binary "/review/compiler/bin/sail"',
            evaluator_command,
        )
        self.assertIn("--provenance-stamp", evaluator_command)
        self.assertIn("tools/run_optimised_c_extraction.py run", command.stdout)
        self.assertIn("tools/run_optimised_c_extraction.py finalize", command.stdout)

    def test_compiler_identity_tracks_rebuilt_binary_under_stable_launcher(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory(dir=TMP_ROOT) as directory:
            source = Path(directory)
            launcher = source / "sail"
            effective = source / "_build/install/default/bin/sail"
            effective.parent.mkdir(parents=True)
            launcher.write_text(
                '#!/bin/sh\nSAIL_DIR=$(dirname "$0")\n'
                'export DUNE_DIR_LOCATIONS="libsail:share:$SAIL_DIR/_build/install/default/share/libsail"\n'
                'exec "$SAIL_DIR/_build/install/default/bin/sail" "$@"\n'
            )
            launcher.chmod(0o755)
            effective.write_bytes(b"compiled-codegen-v1")
            effective.chmod(0o755)

            resolved = discover_effective_sail_executable(launcher, source, None)
            before = compiler_identity(launcher, resolved)
            effective.write_bytes(b"compiled-codegen-v2")
            after = compiler_identity(launcher, resolved)

            self.assertEqual(before["launcher_sha256"], after["launcher_sha256"])
            self.assertNotEqual(
                before["effective_binary_sha256"],
                after["effective_binary_sha256"],
            )
            self.assertEqual(before["binary_sha256"], before["effective_binary_sha256"])
            self.assertNotEqual(before["binary_sha256"], after["binary_sha256"])

    def test_compiler_change_during_extraction_is_rejected(self) -> None:
        before = {"effective_binary_sha256": "a" * 64}
        after = {"effective_binary_sha256": "b" * 64}
        with self.assertRaisesRegex(ValueError, "changed during extraction"):
            ensure_compiler_unchanged(before, after)

    def test_extraction_runner_persists_start_before_rejecting_binary_change(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory(dir=TMP_ROOT) as directory:
            root = Path(directory)
            launcher = root / "launcher"
            effective = root / "_build/install/default/bin/sail"
            source = root / "source"
            state = root / "compiler-provenance.pre.json"
            launcher.write_bytes(b"launcher")
            effective.parent.mkdir(parents=True)
            effective.write_bytes(b"compiled-codegen")
            effective.chmod(0o755)
            plugins = root / "_build/install/default/share/libsail/plugins"
            plugins.mkdir(parents=True)
            (plugins / "sail_plugin_c.cmxs").write_bytes(b"compiled-plugin")
            library = root / "lib"
            library.mkdir()
            (library / "prelude.sail").write_text("/* prelude */\n")
            before = compiler_identity(launcher, effective)
            before.update(compiler_plugin_identity(plugins))
            before.update(compiler_library_identity(library))
            after = {**before, "effective_binary_sha256": "b" * 64}
            args = SimpleNamespace(
                state=state,
                compiler_args=["--", "--version"],
                cwd=root,
            )
            with (
                patch(
                    "tools.run_optimised_c_extraction.resolved",
                    side_effect=[
                        (launcher, effective, source, before),
                        (launcher, effective, source, after),
                    ],
                ),
                patch(
                    "tools.run_optimised_c_extraction.subprocess.run",
                    return_value=subprocess.CompletedProcess([], 0),
                ),
            ):
                with self.assertRaisesRegex(ValueError, "changed during extraction"):
                    run_extraction(args)
            persisted = json.loads(state.read_text())
            self.assertEqual(persisted["status"], "started")
            self.assertNotIn("completed_at", persisted)

    def test_extraction_finalizer_rejects_source_mismatch(self) -> None:
        with tempfile.TemporaryDirectory(dir=TMP_ROOT) as directory:
            root = Path(directory)
            state = root / "compiler-provenance.pre.json"
            compiler = {"effective_binary_sha256": "a" * 64}
            state.write_text(
                json.dumps(
                    {
                        "schema_version": PRE_EXTRACTION_SCHEMA,
                        "status": "complete",
                        "captured_at": "2026-08-17T00:00:00+00:00",
                        "completed_at": "2026-08-17T00:00:01+00:00",
                        "compiler": compiler,
                        "source": str(root / "before"),
                        "requested_command": ["sail", "-c"],
                        "executed_snapshot_sha256": "a" * 64,
                    }
                )
            )
            args = SimpleNamespace(
                state=state,
                generated=root / "generated",
                output=root / "stamp.json",
            )
            with patch(
                "tools.run_optimised_c_extraction.resolved",
                return_value=(
                    root / "launcher",
                    root / "effective",
                    root / "after",
                    compiler,
                ),
            ):
                with self.assertRaisesRegex(ValueError, "Sail source changed"):
                    finalize_extraction(args)

    def test_unrelated_explicit_effective_binary_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory(dir=TMP_ROOT) as directory:
            source = Path(directory)
            launcher = source / "sail"
            effective = source / "_build/install/default/bin/sail"
            unrelated = source / "other/sail"
            effective.parent.mkdir(parents=True)
            unrelated.parent.mkdir(parents=True)
            launcher.write_text(
                '#!/bin/sh\nSAIL_DIR=$(dirname "$0")\n'
                'export DUNE_DIR_LOCATIONS="libsail:share:$SAIL_DIR/_build/install/default/share/libsail"\n'
                'exec "$SAIL_DIR/_build/install/default/bin/sail" "$@"\n'
            )
            launcher.chmod(0o755)
            effective.write_bytes(b"compiled-codegen")
            effective.chmod(0o755)
            unrelated.write_bytes(b"compiled-codegen")
            unrelated.chmod(0o755)
            with self.assertRaisesRegex(ValueError, "not the executable proven"):
                discover_effective_sail_executable(launcher, source, unrelated)

    def test_unrecognized_and_non_executable_launchers_are_rejected(self) -> None:
        with tempfile.TemporaryDirectory(dir=TMP_ROOT) as directory:
            source = Path(directory)
            launcher = source / "sail"
            launcher.write_text('#!/bin/sh\nexec /some/other/sail "$@"\n')
            launcher.chmod(0o755)
            with self.assertRaisesRegex(ValueError, "cannot prove"):
                discover_effective_sail_executable(launcher, source, None)

            launcher.write_text(
                '#!/bin/sh\nSAIL_DIR=$(dirname "$0")\n'
                'export DUNE_DIR_LOCATIONS="libsail:share:$SAIL_DIR/_build/install/default/share/libsail"\n'
                'exec "$SAIL_DIR/_build/install/default/bin/sail" "$@"\n'
            )
            effective = source / "_build/install/default/bin/sail"
            effective.parent.mkdir(parents=True)
            effective.write_bytes(b"compiled-codegen")
            effective.chmod(0o644)
            with self.assertRaisesRegex(ValueError, "not executable"):
                discover_effective_sail_executable(launcher, source, None)
            effective.write_text("#!/bin/sh\nexit 0\n")
            effective.chmod(0o755)
            with self.assertRaisesRegex(ValueError, "script, not a compiled binary"):
                discover_effective_sail_executable(launcher, source, None)

    def test_invalid_explicit_source_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory(dir=TMP_ROOT) as directory:
            invalid = Path(directory) / "not-a-checkout"
            invalid.mkdir()
            with patch(
                "tools.evaluate_optimised_c.run",
                return_value=subprocess.CompletedProcess([], 128, "not a repository"),
            ):
                with self.assertRaisesRegex(ValueError, "explicit Sail source"):
                    discover_sail_source(Path("/compiler/sail"), invalid)

    def test_version_and_source_mismatches_are_rejected(self) -> None:
        launcher = Path("/compiler/sail")
        effective = Path("/compiler/sail.bin")
        source = Path("/compiler/source")
        commit = "a" * 40
        repository = {"commit": commit}
        same_version = f"Sail test (branch @ {commit})\n"
        with (
            patch(
                "tools.evaluate_optimised_c.shutil.which", return_value=str(launcher)
            ),
            patch(
                "tools.evaluate_optimised_c.discover_sail_source",
                return_value=source,
            ),
            patch(
                "tools.evaluate_optimised_c.discover_effective_sail_executable",
                return_value=effective,
            ),
            patch("tools.evaluate_optimised_c.git_repository", return_value=repository),
            patch(
                "tools.evaluate_optimised_c.run",
                side_effect=[
                    subprocess.CompletedProcess([], 0, same_version),
                    subprocess.CompletedProcess([], 0, "Sail other\n"),
                ],
            ),
        ):
            with self.assertRaisesRegex(ValueError, "different versions"):
                resolve_compiler_provenance("sail", source, effective)

        wrong_version = f"Sail test (branch @ {'b' * 40})\n"
        with (
            patch(
                "tools.evaluate_optimised_c.shutil.which", return_value=str(launcher)
            ),
            patch(
                "tools.evaluate_optimised_c.discover_sail_source",
                return_value=source,
            ),
            patch(
                "tools.evaluate_optimised_c.discover_effective_sail_executable",
                return_value=effective,
            ),
            patch("tools.evaluate_optimised_c.git_repository", return_value=repository),
            patch(
                "tools.evaluate_optimised_c.run",
                side_effect=[
                    subprocess.CompletedProcess([], 0, wrong_version),
                    subprocess.CompletedProcess([], 0, wrong_version),
                ],
            ),
        ):
            with self.assertRaisesRegex(ValueError, "build metadata"):
                resolve_compiler_provenance("sail", source, effective)

    def test_extraction_stamp_rejects_generated_source_change(self) -> None:
        with tempfile.TemporaryDirectory(dir=TMP_ROOT) as directory:
            generated = Path(directory) / "generated"
            source = generated / "src/spec/model.c"
            source.parent.mkdir(parents=True)
            source.write_text("int model(void) { return 1; }\n")
            compiler = {
                "executable": "/compiler/sail",
                "effective_binary_sha256": "a" * 64,
            }
            stamp_path = generated / "extraction-provenance.json"
            stamp_path.write_text(
                json.dumps(
                    {
                        "schema_version": EXTRACTION_PROVENANCE_SCHEMA,
                        "captured_at": "2026-08-17T00:00:00+00:00",
                        "extraction_completed_at": "2026-08-17T00:00:01+00:00",
                        "finalized_at": "2026-08-17T00:00:02+00:00",
                        "compiler": compiler,
                        "working_directory": str(ROOT),
                        "requested_command": [
                            "/compiler/sail",
                            "--c-optimized-model",
                            "--c-output-dir",
                            str(generated),
                        ],
                        "executed_snapshot_sha256": "a" * 64,
                        "generated_source_tree": generated_source_tree_identity(
                            generated
                        ),
                    }
                )
            )
            load_extraction_provenance_stamp(stamp_path, generated, compiler)
            with self.assertRaisesRegex(ValueError, "compiler identity mismatch"):
                load_extraction_provenance_stamp(
                    stamp_path,
                    generated,
                    {"effective_binary_sha256": "b" * 64},
                )
            source.write_text("int model(void) { return 2; }\n")
            with self.assertRaisesRegex(ValueError, "generated-source mismatch"):
                load_extraction_provenance_stamp(stamp_path, generated, compiler)

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
                "tools.evaluate_optimised_c.run",
                return_value=subprocess.CompletedProcess(
                    ["make"], returncode=1, stdout="compile failed\n"
                ),
            ):
                record, written_log = package_build_gate(
                    root, "clang", root / "libevmsail.a", log
                )
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
            "compiler": {
                "executable": "/compiler/sail",
                "launcher_sha256": digest,
                "effective_executable": "/compiler/_build/bin/sail",
                "effective_binary_sha256": digest,
                "binary_sha256": digest,
                "reported_version": "Sail test",
                "effective_reported_version": "Sail test",
                "source_commit": commit,
                "plugin_directory": "/compiler/share/libsail/plugins",
                "plugin_tree_sha256": digest,
                "plugin_file_count": 1,
                "library_directory": "/compiler/lib",
                "library_tree_sha256": digest,
                "library_file_count": 1,
            },
            "extraction": {
                "manifest_sha256": digest,
                "provenance_stamp_path": "extraction-provenance.json",
                "provenance_stamp_sha256": digest,
                "generated_source_tree": {"sha256": digest, "file_count": 1},
            },
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
