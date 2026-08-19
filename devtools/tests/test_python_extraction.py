from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from devtools.python_extraction import PythonExtractionError, normalize_tree


class PythonExtractionNormalizationTests(unittest.TestCase):
    def _write_generated_module(self, root: Path, extra_callback: bool = False) -> Path:
        target = root / "evm/lib/mpt/updates.py"
        target.parent.mkdir(parents=True)
        extra = (
            "\ndef unexpected_callback():\n    return state_trie.trie_update_source_next(None)\n"
            if extra_callback
            else ""
        )
        target.write_text(
            "from evm.lib import state_trie\n\n"
            "def trie_updates_begin(source):\n"
            "    return state_trie.trie_update_source_next(source)\n\n"
            "def trie_updates_pop(updates):\n"
            "    return state_trie.trie_update_source_next(updates)\n"
            f"{extra}",
            encoding="utf-8",
        )
        return target

    def test_callback_imports_are_local_and_idempotent(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            target = self._write_generated_module(root)
            self.assertEqual(normalize_tree(root), 2)
            normalized = target.read_text(encoding="utf-8")
            self.assertNotIn(
                "from evm.lib import state_trie\n",
                normalized.splitlines(keepends=True),
            )
            self.assertEqual(normalized.count("    from evm.lib import state_trie\n"), 2)
            self.assertEqual(normalize_tree(root), 0)

    def test_unexpected_callback_shape_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            self._write_generated_module(root, extra_callback=True)
            with self.assertRaises(PythonExtractionError):
                normalize_tree(root)


if __name__ == "__main__":
    unittest.main()
