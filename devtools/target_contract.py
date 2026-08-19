#!/usr/bin/env python3
"""Validate the repository's public Make target surface."""

from __future__ import annotations

import argparse
import re
import sys
import tomllib
from collections.abc import Sequence
from pathlib import Path

from devtools.build_support import BuildSupportError

TARGET_RE = re.compile(r"^([A-Za-z0-9][A-Za-z0-9_.-]*)(?:\s+[^:]*)?:", re.MULTILINE)


def validate_target_contract(makefile: Path, config: Path) -> list[str]:
    source = makefile.read_text(encoding="utf-8")
    data = tomllib.loads(config.read_text(encoding="utf-8"))
    if data.get("schema_version") != 1:
        raise BuildSupportError(f"{config}: unsupported target-contract schema")
    targets = set(TARGET_RE.findall(source))
    errors: list[str] = []
    for target in data.get("required", []):
        if target not in targets:
            errors.append(f"missing required Make target: {target}")
    for target in data.get("forbidden", []):
        if target in targets:
            errors.append(f"forbidden legacy Make target exists: {target}")
    for target in data.get("help", []):
        pattern = rf'echo "[^"\n]*make {re.escape(target)}(?:\s|\")'
        if not re.search(pattern, source):
            errors.append(f"Make help omits public target: {target}")
    return errors


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--config", type=Path, required=True)
    parser.add_argument("--makefile", type=Path, default=Path("Makefile"))
    args = parser.parse_args(argv)
    try:
        errors = validate_target_contract(args.makefile, args.config)
        if errors:
            print("public target contract failed:", file=sys.stderr)
            print("\n".join(f"  {error}" for error in errors), file=sys.stderr)
            return 1
        print(f"public targets: {args.config} passed")
        return 0
    except (BuildSupportError, OSError, tomllib.TOMLDecodeError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    raise SystemExit(main())
