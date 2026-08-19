from __future__ import annotations

import io
import json
import tarfile
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from devtools.build_support import BuildSupportError
from devtools.corpus import (
    DOWNLOAD_TIMEOUT_SECONDS,
    CorpusManifest,
    _download,
    extract_archive,
    inventory_corpus,
    load_manifest,
    provision_corpus,
    verify_corpus,
)


def manifest_for(inventory_sha: str, files: int = 1, cases: int = 1) -> CorpusManifest:
    return CorpusManifest(
        schema_version=1,
        extractor_schema=1,
        name="test",
        version="v1",
        source_tag="tag",
        source_commit="0" * 40,
        t8n_version="1",
        generation_args="fill",
        archive_url="https://example.invalid/fixtures.tar.gz",
        archive_sha256="0" * 64,
        archive_root=".",
        include_directories=("fixtures",),
        tree_sha256=inventory_sha,
        expected_json_files=files,
        expected_cases=cases,
    )


class CorpusTests(unittest.TestCase):
    def test_download_uses_a_bounded_network_timeout(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            destination = Path(temporary) / "archive.tar.gz"
            response = io.BytesIO(b"archive")
            with patch("devtools.corpus.urlopen", return_value=response) as mocked:
                _download("https://example.invalid/archive.tar.gz", destination)
            mocked.assert_called_once_with(
                "https://example.invalid/archive.tar.gz",
                timeout=DOWNLOAD_TIMEOUT_SECONDS,
            )
            self.assertEqual(destination.read_bytes(), b"archive")

    def test_inventory_is_deterministic_and_counts_cases(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            fixtures = root / "fixtures"
            fixtures.mkdir()
            (fixtures / "case.json").write_text(
                json.dumps(
                    {"tests": [{"statelessInputBytes": "00", "statelessOutputBytes": "01"}]}
                ),
                encoding="utf-8",
            )
            inventory = inventory_corpus(root, ("fixtures",))
            self.assertEqual(inventory.json_files, 1)
            self.assertEqual(inventory.embedded_cases, 1)
            self.assertEqual(verify_corpus(root, manifest_for(inventory.tree_sha256)), inventory)

    def test_verify_reports_all_inventory_mismatches(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "fixtures").mkdir()
            (root / "fixtures/case.json").write_text("{}", encoding="utf-8")
            with self.assertRaises(BuildSupportError) as raised:
                verify_corpus(root, manifest_for("f" * 64, files=2, cases=2))
            message = str(raised.exception)
            self.assertIn("tree SHA-256", message)
            self.assertIn("JSON files", message)
            self.assertIn("embedded cases", message)

    def test_safe_extraction_rejects_parent_traversal(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            archive_path = root / "bad.tar.gz"
            with tarfile.open(archive_path, "w:gz") as archive:
                info = tarfile.TarInfo("../escape")
                contents = b"bad"
                info.size = len(contents)
                archive.addfile(info, io.BytesIO(contents))
            with self.assertRaises(BuildSupportError):
                extract_archive(archive_path, root / "output")
            self.assertFalse((root.parent / "escape").exists())

    def test_manifest_rejects_unknown_schema(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "manifest.toml"
            path.write_text(
                """
schema_version = 2
extractor_schema = 1
[corpus]
name = "x"
version = "v1"
[source]
tag = "tag"
commit = "commit"
t8n_version = "1"
generation_args = "fill"
[archive]
url = "https://example.invalid/a.tar.gz"
sha256 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
root = "."
include_directories = ["fixtures"]
[inventory]
tree_sha256 = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
json_files = 1
embedded_cases = 1
""",
                encoding="utf-8",
            )
            with self.assertRaisesRegex(BuildSupportError, "unsupported manifest schema"):
                load_manifest(path)

    def test_provision_refuses_to_replace_an_invalid_existing_tree(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            destination = root / "corpus"
            fixtures = destination / "fixtures"
            fixtures.mkdir(parents=True)
            marker = fixtures / "keep-me.json"
            marker.write_text("{}", encoding="utf-8")

            with self.assertRaisesRegex(
                BuildSupportError, "refusing to replace invalid corpus directory"
            ):
                provision_corpus(manifest_for("f" * 64), destination, root / "cache")

            self.assertEqual(marker.read_text(encoding="utf-8"), "{}")


if __name__ == "__main__":
    unittest.main()
