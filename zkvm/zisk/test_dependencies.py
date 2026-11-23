"""Prepared dependency integrity checks; no network or compiler required."""

import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import dependencies


class PreparedArtifactsTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.archive = self.root / "runtime with spaces.a"
        self.linker = self.root / "upstream.ld"
        self.archive.write_bytes(b"prepared runtime")
        self.linker.write_bytes(b"pinned upstream linker")
        self.linker_hash = dependencies.digest(self.linker)
        self.mock_hash = patch.object(dependencies, "LINKER_SHA256", self.linker_hash)
        self.mock_hash.start()
        self.addCleanup(self.mock_hash.stop)
        url, revision, version = dependencies.pin()
        self.record = {
            "schema_version": 1,
            "source": {"repository": url, "revision": revision},
            "version": version,
            "target": dependencies.TARGET,
            "staticlib": {"path": str(self.archive), "sha256": dependencies.digest(self.archive)},
            "linker": {"path": str(self.linker), "sha256": self.linker_hash},
        }
        self.manifest = self.root / "artifacts.json"
        self.save()

    def save(self):
        self.manifest.write_text(json.dumps(self.record))

    def test_prepared_paths_do_not_depend_on_upstream_directory_layout(self):
        result = dependencies.read_artifacts(self.manifest)
        self.assertEqual(result["staticlib"]["path"], str(self.archive))
        self.assertEqual(result["linker"]["path"], str(self.linker))

    def test_changed_cached_archive_is_rejected(self):
        self.archive.write_bytes(b"stale or replaced archive")
        with self.assertRaisesRegex(ValueError, "staticlib changed"):
            dependencies.read_artifacts(self.manifest)

    def test_changed_linker_cannot_be_accepted_by_editing_its_receipt_hash(self):
        self.linker.write_bytes(b"locally maintained divergent script")
        self.record["linker"]["sha256"] = dependencies.digest(self.linker)
        self.save()
        with self.assertRaisesRegex(ValueError, "pinned upstream bytes"):
            dependencies.read_artifacts(self.manifest)

    def test_stale_release_identity_is_rejected(self):
        self.record["source"]["revision"] = "0" * 40
        self.save()
        with self.assertRaisesRegex(ValueError, "identity differs"):
            dependencies.read_artifacts(self.manifest)

    def test_missing_preparation_is_actionable(self):
        self.manifest.unlink()
        with self.assertRaisesRegex(ValueError, "make zisk-deps"):
            dependencies.read_artifacts(self.manifest)

    def test_symlinked_linker_is_rejected(self):
        original = self.root / "original.ld"
        self.linker.rename(original)
        self.linker.symlink_to(original)
        with self.assertRaisesRegex(ValueError, "linker changed"):
            dependencies.read_artifacts(self.manifest)


if __name__ == "__main__":
    unittest.main()
