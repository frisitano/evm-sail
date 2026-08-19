#!/usr/bin/env python3
"""Normalize known identifier collisions in Sail's generated Rocq patterns."""

from __future__ import annotations

import argparse
import re
from collections.abc import Sequence
from pathlib import Path

EXISTT_BINDER_RE = re.compile(r"(?P<prefix>@existT _ _ )(?P<name>len|off)\b")


def normalize_line(line: str) -> tuple[str, int]:
    """Give repeated generated ``len``/``off`` binders unique pattern names."""

    seen: dict[str, int] = {}
    changes = 0

    def replace(match: re.Match[str]) -> str:
        nonlocal changes
        name = match.group("name")
        occurrence = seen.get(name, 0) + 1
        seen[name] = occurrence
        if occurrence == 1:
            return match.group(0)
        changes += 1
        return f"{match.group('prefix')}{name}__{occurrence}"

    return EXISTT_BINDER_RE.sub(replace, line), changes


def normalize_file(path: Path) -> int:
    """Normalize a generated Rocq file and return the number of renamed binders."""

    original = path.read_text(encoding="utf-8")
    changed_lines: list[str] = []
    changes = 0
    for line in original.splitlines(keepends=True):
        normalized, line_changes = normalize_line(line)
        changed_lines.append(normalized)
        changes += line_changes
    if changes:
        path.write_text("".join(changed_lines), encoding="utf-8")
    return changes


def normalize_tree(root: Path) -> int:
    """Normalize all generated Rocq source files below ``root``."""

    return sum(normalize_file(path) for path in sorted(root.rglob("*.v")))


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", type=Path)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    changes = normalize_tree(args.root)
    print(f"rocq normalization: renamed {changes} colliding existential binders")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
