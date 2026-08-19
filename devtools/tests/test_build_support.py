from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path

from devtools.build_support import (
    BuildSupportError,
    comment_box_errors,
    normalize_lean_tree,
    publish_tree,
    read_ordered_manifest,
    run_logged_command,
    tree_differences,
    write_canonical_manifest,
)


class ManifestTests(unittest.TestCase):
    def test_preserves_order_comments_and_spaces(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "second file.sail").write_text("second", encoding="utf-8")
            (root / "first.sail").write_text("first", encoding="utf-8")
            manifest = root / "manifest"
            manifest.write_text("# ordered\nsecond file.sail\n\nfirst.sail\n", encoding="utf-8")
            self.assertEqual(
                read_ordered_manifest(manifest, root),
                [(root / "second file.sail").resolve(), (root / "first.sail").resolve()],
            )

    def test_rejects_duplicates_escape_and_missing_files(self) -> None:
        cases = ("file.sail\nfile.sail\n", "../file.sail\n", "missing.sail\n")
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "file.sail").write_text("", encoding="utf-8")
            manifest = root / "manifest"
            for contents in cases:
                with self.subTest(contents=contents):
                    manifest.write_text(contents, encoding="utf-8")
                    with self.assertRaises(BuildSupportError):
                        read_ordered_manifest(manifest, root)

    def test_canonicalizes_a_validated_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "nested").mkdir()
            (root / "nested" / "first.c").write_text("", encoding="utf-8")
            (root / "second.c").write_text("", encoding="utf-8")
            manifest = root / "sources.list"
            manifest.write_text("# compiler output\nnested/first.c\n\nsecond.c\n", encoding="utf-8")
            output = root / "build" / "canonical.list"
            self.assertEqual(write_canonical_manifest(manifest, root, output), 2)
            self.assertEqual(
                output.read_text(encoding="utf-8"),
                "nested/first.c\nsecond.c\n",
            )


class LintRunnerTests(unittest.TestCase):
    def test_nonzero_without_error_keyword_still_fails(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            log = Path(temporary) / "lint.log"
            status = run_logged_command(
                [sys.executable, "-c", "print('plain failure'); raise SystemExit(7)"],
                log,
            )
            self.assertEqual(status, 7)
            self.assertEqual(log.read_text(encoding="utf-8"), "plain failure\n")

    def test_warning_with_zero_status_fails(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            status = run_logged_command(
                [sys.executable, "-c", "print('warning: something')"],
                Path(temporary) / "lint.log",
            )
            self.assertEqual(status, 1)

    def test_generic_logged_command_preserves_success_with_error_word(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            status = run_logged_command(
                [sys.executable, "-c", "print('zero errors')"],
                Path(temporary) / "command.log",
                fail_on_diagnostics=False,
            )
            self.assertEqual(status, 0)


class ArtifactTests(unittest.TestCase):
    def test_publish_tree_excludes_local_build_caches(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            staged = root / "staged"
            destination = root / "published"
            (staged / ".lake").mkdir(parents=True)
            (staged / ".lake" / "cache").write_text("cache", encoding="utf-8")
            (staged / "__pycache__").mkdir()
            (staged / "__pycache__" / "model.pyc").write_bytes(b"cache")
            (staged / "Model.lean").write_text("def model := 1\n", encoding="utf-8")

            publish_tree(staged, destination)

            self.assertEqual(
                (destination / "Model.lean").read_text(encoding="utf-8"),
                "def model := 1\n",
            )
            self.assertFalse((destination / ".lake").exists())
            self.assertFalse((destination / "__pycache__").exists())

    def test_tree_difference_reports_paths_and_content(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            expected, actual = root / "expected", root / "actual"
            expected.mkdir()
            actual.mkdir()
            (expected / "changed").write_text("old", encoding="utf-8")
            (actual / "changed").write_text("new", encoding="utf-8")
            (expected / "missing").write_text("x", encoding="utf-8")
            (actual / "extra").write_text("x", encoding="utf-8")
            self.assertEqual(
                tree_differences(expected, actual),
                [
                    "missing from generated tree: missing",
                    "unexpected generated file: extra",
                    "content differs: changed",
                ],
            )

    def test_tree_difference_and_publish_preserve_symlinks(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            expected, actual = root / "expected", root / "actual"
            expected.mkdir()
            actual.mkdir()
            (expected / "target").write_text("target", encoding="utf-8")
            (actual / "target").write_text("target", encoding="utf-8")
            (expected / "link").symlink_to("target")
            (actual / "link").symlink_to("different")
            self.assertEqual(
                tree_differences(expected, actual),
                ["symlink target differs: link"],
            )

            published = root / "published"
            publish_tree(expected, published)
            self.assertTrue((published / "link").is_symlink())
            self.assertEqual((published / "link").readlink(), Path("target"))

    def test_lean_normalization_is_identifier_aware(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            source = root / "Model.lean"
            source.write_text(
                "def prefix := 1\ndef prefix_sum := prefix + 1\n-- prefixing\n\n",
                encoding="utf-8",
            )
            self.assertEqual(normalize_lean_tree(root), 2)
            self.assertEqual(
                source.read_text(encoding="utf-8"),
                "def evm_prefix := 1\ndef prefix_sum := evm_prefix + 1\n-- prefixing\n",
            )

    def test_lean_normalization_repairs_known_interpreter_sigma_shape(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            interpreter = root / "Evm" / "Evm" / "Interpreter.lean"
            interpreter.parent.mkdir(parents=True)
            interpreter.write_text(
                "\n".join(
                    ["def prefix := 1"]
                    + ["let ⟨_, ⟨_, carried_memory⟩⟩ : SigmaType :=\n  (tup__7 : SigmaType)"] * 14
                    + [
                        "(pure ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma\n"
                        "  fun (initial_code_dependentWitness0 : Nat) => SigmaType))))"
                    ]
                    * 7
                    + [
                        "(pure ((⟨_, ⟨_, carried_returndata⟩⟩ : (Sigma\n"
                        "  fun (initial_code_dependentWitness0 : Nat) => SigmaType))))"
                    ]
                    * 7
                )
                + "\n",
                encoding="utf-8",
            )
            self.assertEqual(normalize_lean_tree(root), 29)
            normalized = interpreter.read_text(encoding="utf-8")
            self.assertEqual(normalized.count("let carried_memory : SigmaType"), 14)
            self.assertEqual(
                normalized.count("let ⟨_, ⟨_, carried_memory⟩⟩ : SigmaType"),
                0,
            )
            self.assertIn("let carried_memory : SigmaType", normalized)
            self.assertEqual(normalized.count("⟨_, ⟨_, carried_code⟩⟩"), 0)
            self.assertEqual(normalized.count("⟨_, ⟨_, carried_returndata⟩⟩"), 0)

    def test_lean_normalization_repairs_known_transaction_output_shape(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            transaction = root / "Evm" / "Evm" / "Transaction.lean"
            transaction.parent.mkdir(parents=True)
            transaction.write_text(
                "let ⟨_, ⟨_, output_after⟩⟩ : SigmaType :=\n  (tup__5 : SigmaType)\n(pure ())\n",
                encoding="utf-8",
            )
            self.assertEqual(normalize_lean_tree(root), 1)
            self.assertEqual(
                transaction.read_text(encoding="utf-8"),
                "let output_after : SigmaType :=\n  (tup__5 : SigmaType)\n(pure ())\n",
            )

    def test_comment_box_reports_every_bad_line(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            source = Path(temporary) / "model.sail"
            source.write_text("/* ===== */\n/* x */\n/* too long */\n\n", encoding="utf-8")
            errors = comment_box_errors([source])
            self.assertEqual(len(errors), 2)
            self.assertIn(":2:", errors[0])
            self.assertIn(":3:", errors[1])


if __name__ == "__main__":
    unittest.main()
