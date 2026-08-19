#!/usr/bin/env python3
"""Read the repository's pinned toolchain manifest."""

from __future__ import annotations

import argparse
import re
import sys
import tomllib
from collections.abc import Mapping, Sequence
from pathlib import Path
from typing import cast

from devtools.build_support import BuildSupportError

SHA_RE = re.compile(r"^[0-9a-f]{40}$")


def load_toolchains(path: Path) -> dict[str, object]:
    data = cast(dict[str, object], tomllib.loads(path.read_text(encoding="utf-8")))
    if data.get("schema_version") != 1:
        raise BuildSupportError(f"{path}: unsupported toolchain manifest schema")
    for section, key in (
        ("sail", "commit"),
        ("sail_lsp", "commit"),
        ("lean", "sail_commit"),
    ):
        section_data = data.get(section)
        if not isinstance(section_data, dict):
            raise BuildSupportError(f"{path}: {section} must be a table")
        value = section_data.get(key)
        if not isinstance(value, str) or not SHA_RE.fullmatch(value):
            raise BuildSupportError(f"{path}: {section}.{key} must be a full Git SHA")
    for section, key in (
        ("solver", "z3_supported_major"),
        ("solver", "z3_memo_schema"),
        ("llvm", "major"),
    ):
        section_data = data.get(section)
        if not isinstance(section_data, dict):
            raise BuildSupportError(f"{path}: {section} must be a table")
        value = section_data.get(key)
        if not isinstance(value, int) or isinstance(value, bool) or value <= 0:
            raise BuildSupportError(f"{path}: {section}.{key} must be a positive integer")
    return data


def flatten(data: Mapping[str, object]) -> dict[str, str]:
    result: dict[str, str] = {}
    for section, values in data.items():
        if section == "schema_version" or not isinstance(values, dict):
            continue
        for key, value in values.items():
            result[f"{section}_{key}".upper()] = str(value)
    return result


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, default=Path("config/toolchains.toml"))
    parser.add_argument("--github-env", action="store_true")
    parser.add_argument("key", nargs="?")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        values = flatten(load_toolchains(args.manifest))
        if args.github_env:
            for key, value in sorted(values.items()):
                print(f"{key}={value}")
        elif args.key:
            try:
                print(values[args.key.upper()])
            except KeyError as error:
                raise BuildSupportError(f"unknown toolchain key: {args.key}") from error
        else:
            for key, value in sorted(values.items()):
                print(f"{key}={value}")
        return 0
    except (BuildSupportError, OSError, tomllib.TOMLDecodeError) as error:
        print(f"toolchains: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
