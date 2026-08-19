#!/usr/bin/env python3
"""Validate declarative syntactic canaries with complete diagnostics."""

from __future__ import annotations

import argparse
import re
import sys
import tomllib
from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path, PurePosixPath

from devtools.build_support import BuildSupportError


@dataclass(frozen=True)
class Canary:
    pattern: str
    path: str | None = None
    glob: str | None = None


def load_canaries(path: Path) -> tuple[list[Canary], list[Canary]]:
    data = tomllib.loads(path.read_text(encoding="utf-8"))
    if data.get("schema_version") != 1:
        raise BuildSupportError(f"{path}: unsupported canary schema")

    def entries(kind: str) -> list[Canary]:
        result: list[Canary] = []
        for index, raw in enumerate(data.get(kind, []), start=1):
            if not isinstance(raw, dict) or not isinstance(raw.get("pattern"), str):
                raise BuildSupportError(f"{path}: invalid {kind} canary {index}")
            exact = raw.get("path")
            pattern_glob = raw.get("glob")
            if (exact is None) == (pattern_glob is None):
                raise BuildSupportError(
                    f"{path}: {kind} canary {index} needs exactly one of path/glob"
                )
            selector = exact if exact is not None else pattern_glob
            relative = PurePosixPath(str(selector))
            if relative.is_absolute() or ".." in relative.parts:
                raise BuildSupportError(
                    f"{path}: {kind} canary {index} has unsafe selector {selector!r}"
                )
            try:
                re.compile(raw["pattern"], re.MULTILINE)
            except re.error as error:
                raise BuildSupportError(
                    f"{path}: invalid regex in {kind} canary {index}: {error}"
                ) from error
            result.append(Canary(raw["pattern"], exact, pattern_glob))
        return result

    required, forbidden = entries("required"), entries("forbidden")
    if not required and not forbidden:
        raise BuildSupportError(f"{path}: no canaries declared")
    return required, forbidden


def _selected_files(root: Path, canary: Canary) -> list[Path]:
    if canary.path is not None:
        candidate = root / canary.path
        return [candidate] if candidate.is_file() else []
    assert canary.glob is not None
    return sorted(path for path in root.glob(canary.glob) if path.is_file())


def validate_canaries(root: Path, config: Path) -> list[str]:
    required, forbidden = load_canaries(config)
    errors: list[str] = []
    for kind, canaries in (("required", required), ("forbidden", forbidden)):
        for canary in canaries:
            selector = canary.path or canary.glob or ""
            files = _selected_files(root, canary)
            if not files:
                errors.append(f"{kind} selector matched no files: {selector}")
                continue
            regex = re.compile(canary.pattern, re.MULTILINE)
            matches = [
                path.relative_to(root).as_posix()
                for path in files
                if regex.search(path.read_text(encoding="utf-8"))
            ]
            if kind == "required" and not matches:
                errors.append(f"missing required pattern {canary.pattern!r} in {selector}")
            elif kind == "forbidden" and matches:
                errors.append(f"forbidden pattern {canary.pattern!r} in {', '.join(matches)}")
    return errors


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--config", type=Path, required=True)
    parser.add_argument("--root", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        errors = validate_canaries(args.root, args.config)
        if errors:
            print("canary validation failed:", file=sys.stderr)
            print("\n".join(f"  {error}" for error in errors), file=sys.stderr)
            return 1
        print(f"canaries: {args.config} passed")
        return 0
    except (BuildSupportError, OSError, tomllib.TOMLDecodeError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    raise SystemExit(main())
