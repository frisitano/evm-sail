from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from devtools.rocq import normalize_line, normalize_tree


class RocqNormalizationTests(unittest.TestCase):
    def test_repeated_existential_binders_are_unique(self) -> None:
        source = (
            "let '(@existT _ _ len (@existT _ _ off left), "
            "@existT _ _ len (@existT _ _ off right)) := value in\n"
        )
        normalized, changes = normalize_line(source)
        self.assertEqual(changes, 2)
        self.assertIn("@existT _ _ len__2", normalized)
        self.assertIn("@existT _ _ off__2", normalized)

    def test_distinct_binders_and_other_lines_are_unchanged(self) -> None:
        source = "@existT _ _ len (@existT _ _ off (@existT _ _ _ value))\n"
        self.assertEqual(normalize_line(source), (source, 0))

    def test_tree_normalization_is_idempotent(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "model.v"
            path.write_text(
                "@existT _ _ len x, @existT _ _ len y\n",
                encoding="utf-8",
            )
            self.assertEqual(normalize_tree(path.parent), 1)
            self.assertEqual(normalize_tree(path.parent), 0)


if __name__ == "__main__":
    unittest.main()
