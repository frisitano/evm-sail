#!/usr/bin/env python3
"""Normalize the known split-module cycle in generated Python extraction."""

from __future__ import annotations

import argparse
import re
from collections.abc import Sequence
from pathlib import Path

TARGET = Path("evm/lib/mpt/updates.py")
MODULE_IMPORT = "from evm.lib import state_trie\n"
CALLBACK_FUNCTIONS = ("trie_updates_begin", "trie_updates_pop")


class PythonExtractionError(RuntimeError):
    """Raised when generated structure no longer matches the known rewrite."""


def normalize_updates(path: Path) -> int:
    """Move the state-trie import into the two reducer callback functions."""

    source = path.read_text(encoding="utf-8")
    local_import = f"    {MODULE_IMPORT}"
    lines = source.splitlines(keepends=True)
    top_level_count = lines.count(MODULE_IMPORT)
    local_count = lines.count(local_import)
    if top_level_count == 0:
        if local_count == len(CALLBACK_FUNCTIONS):
            return 0
        raise PythonExtractionError(f"expected generated import missing from {path}")

    if top_level_count != 1 or local_count != 0:
        raise PythonExtractionError(f"expected one generated state_trie import in {path}")
    if source.count("state_trie.trie_update_source_next(") != 2:
        raise PythonExtractionError(f"unexpected state-trie callback count in {path}")

    normalized = source.replace(MODULE_IMPORT, "", 1)
    for function in CALLBACK_FUNCTIONS:
        declaration = re.compile(rf"^(def {function}\([^\n]*:\n)", re.MULTILINE)
        normalized, replacements = declaration.subn(
            rf"\1{local_import}\n",
            normalized,
            count=1,
        )
        if replacements != 1:
            raise PythonExtractionError(f"expected one generated {function} declaration in {path}")

    path.write_text(normalized, encoding="utf-8")
    return len(CALLBACK_FUNCTIONS)


def normalize_tree(root: Path) -> int:
    """Normalize the generated Python package rooted at ``root``."""

    target = root / TARGET
    if not target.is_file():
        raise PythonExtractionError(f"generated Python module does not exist: {target}")
    return normalize_updates(target)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", type=Path)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    changes = normalize_tree(args.root)
    print(f"python extraction normalization: localized {changes} callback imports")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
