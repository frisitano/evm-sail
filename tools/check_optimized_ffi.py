#!/usr/bin/env python3
"""Enforce the production contract of the allocation-free optimized C FFI."""

from __future__ import annotations

import argparse
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
FFI_ROOT = ROOT / "ffi/optimized"
SOURCE_ROOT = ROOT / "ffi/optimized/src"
INCLUDE_ROOT = ROOT / "ffi/optimized/include"
PRELUDE = INCLUDE_ROOT / "evmsail/prelude.h"
SAIL_ROOT = ROOT / "sail/optimised"
MANIFEST = ROOT / "ffi/optimized/sources.list"

FORBIDDEN_CALLS = re.compile(
    r"\b(?:malloc|calloc|realloc|free|qsort|"
    r"sail_(?:new|create|allocate|reallocate|free)[A-Za-z0-9_]*)\s*\("
)
FORBIDDEN_STDIO = re.compile(
    r"\b(?:printf|fprintf|sprintf|snprintf|puts|fputs|putchar|fputc|fwrite)\s*\("
)
FORBIDDEN_MODULE_BACKING_ARRAY = re.compile(
    r"\bstatic\b[^;{}]*\[\s*GUEST_[A-Z0-9_]+\s*\][^;]*;",
    re.DOTALL,
)
FORBIDDEN_HOST_PREFIX = re.compile(r"\b(?:evmsail|optimized)_[A-Za-z0-9_]*")
GENERATED_PACKAGE_SYMBOLS = {"evmsail_model_init"}
FORBIDDEN_INDIRECT_FUNCTION = re.compile(
    r"\(\s*\*\s*[A-Za-z_][A-Za-z0-9_]*\s*\)\s*\("
)
# These tags select protocol behavior in Sail and are deliberately erased at
# optimized-C boundaries in favour of their source-defined data descriptors.
# Rejecting the generated tag names prevents a later C fast path from silently
# reintroducing a second semantic classifier.
FORBIDDEN_C_SEMANTIC_TAG = re.compile(
    r"\b(?:zCallKind|zCreateKind|"
    r"zDeepStackOperation|zTxSignatureScheme)\b"
)
RAW_MODEL_TYPE = re.compile(
    r"\b(?:sail_fixed_bytes_(?:u64_lanes_20|u64_lanes_32|256)|sail_u256)\b"
)
FORBIDDEN_MPT_STATUS = re.compile(r"\b(?:mpt_status|mpt_fail)\b")
SAIL_C_BINDING = re.compile(r'\bc\s*:\s*"([^"]+)"')


def manifest_sources() -> list[str]:
    entries = [
        line.strip()
        for line in MANIFEST.read_text().splitlines()
        if line.strip() and not line.lstrip().startswith("#")
    ]
    duplicates = sorted({entry for entry in entries if entries.count(entry) > 1})
    if duplicates:
        raise ValueError(f"duplicate manifest entries: {', '.join(duplicates)}")
    return entries


def strip_comments_and_literals(source: str) -> str:
    """Preserve newlines while removing C comments, strings, and characters."""
    result = list(source)
    index = 0
    state = "code"
    while index < len(result):
        char = result[index]
        nxt = result[index + 1] if index + 1 < len(result) else ""
        if state == "code":
            if char == "/" and nxt == "*":
                result[index] = result[index + 1] = " "
                state = "comment"
                index += 2
                continue
            if char == "/" and nxt == "/":
                result[index] = result[index + 1] = " "
                state = "line-comment"
                index += 2
                continue
            if char == '"':
                result[index] = " "
                state = "string"
            elif char == "'":
                result[index] = " "
                state = "character"
        elif state == "comment":
            if char == "*" and nxt == "/":
                result[index] = result[index + 1] = " "
                state = "code"
                index += 2
                continue
            if char != "\n":
                result[index] = " "
        elif state == "line-comment":
            if char == "\n":
                state = "code"
            else:
                result[index] = " "
        else:
            if char == "\\" and nxt:
                if char != "\n":
                    result[index] = " "
                if nxt != "\n":
                    result[index + 1] = " "
                index += 2
                continue
            if (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ):
                state = "code"
            if char != "\n":
                result[index] = " "
        index += 1
    return "".join(result)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--manifest-only",
        action="store_true",
        help="check source ownership without enforcing the zero-allocation policy",
    )
    args = parser.parse_args()

    errors: list[str] = []
    try:
        entries = manifest_sources()
    except ValueError as error:
        print(f"optimized FFI audit: {error}")
        return 1

    manifest_set = set(entries)
    production_set = {
        path.relative_to(SOURCE_ROOT).as_posix()
        for path in SOURCE_ROOT.rglob("*.c")
        if "test" not in path.relative_to(SOURCE_ROOT).parts
    }
    for missing in sorted(production_set - manifest_set):
        errors.append(f"unowned production source: {missing}")
    for stale in sorted(manifest_set - production_set):
        errors.append(f"missing production source: {stale}")

    if not args.manifest_only:
        for entry in entries:
            path = SOURCE_ROOT / entry
            if not path.is_file():
                continue
            source = strip_comments_and_literals(
                path.read_text(errors="replace")
            )
            for pattern, label in (
                (FORBIDDEN_CALLS, "forbidden allocation/sort call"),
                (FORBIDDEN_STDIO, "production stdio call"),
                (
                    FORBIDDEN_INDIRECT_FUNCTION,
                    "indirect function/callback; use a closed tag and dispatcher",
                ),
            ):
                for match in pattern.finditer(source):
                    line = source.count("\n", 0, match.start()) + 1
                    errors.append(f"{entry}:{line}: {label}: {match.group(0).strip()}")
            if entry != "workspace.c":
                for match in FORBIDDEN_MODULE_BACKING_ARRAY.finditer(source):
                    line = source.count("\n", 0, match.start()) + 1
                    errors.append(
                        f"{entry}:{line}: module-owned capacity backing array"
                    )

        for path in sorted(FFI_ROOT.rglob("*")):
            if path.suffix not in {".c", ".h"} or not path.is_file():
                continue
            source = strip_comments_and_literals(path.read_text(errors="replace"))
            relative = path.relative_to(ROOT)
            for match in FORBIDDEN_HOST_PREFIX.finditer(source):
                if match.group(0) in GENERATED_PACKAGE_SYMBOLS:
                    continue
                line = source.count("\n", 0, match.start()) + 1
                errors.append(
                    f"{relative}:{line}: redundant optimized-host prefix: "
                    f"{match.group(0)}"
                )
            if path != PRELUDE:
                for match in RAW_MODEL_TYPE.finditer(source):
                    line = source.count("\n", 0, match.start()) + 1
                    errors.append(
                        f"{relative}:{line}: raw generated ABI type outside prelude: "
                        f"{match.group(0)}"
                    )
            for match in FORBIDDEN_C_SEMANTIC_TAG.finditer(source):
                line = source.count("\n", 0, match.start()) + 1
                errors.append(
                    f"{relative}:{line}: source-owned semantic dispatch tag in C: "
                    f"{match.group(0)}"
                )
            for match in FORBIDDEN_MPT_STATUS.finditer(source):
                line = source.count("\n", 0, match.start()) + 1
                errors.append(
                    f"{relative}:{line}: deferred MPT fatal status: "
                    f"{match.group(0)}"
                )

        for path in sorted(SAIL_ROOT.rglob("*.sail")):
            source = path.read_text(errors="replace")
            relative = path.relative_to(ROOT)
            for match in SAIL_C_BINDING.finditer(source):
                symbol = match.group(1)
                if symbol.startswith(("evmsail_", "optimized_")):
                    line = source.count("\n", 0, match.start()) + 1
                    errors.append(
                        f"{relative}:{line}: prefixed optimized C binding: {symbol}"
                    )

    if errors:
        print("optimized FFI audit failed:")
        for error in errors:
            print(f"  {error}")
        return 1

    scope = "manifest" if args.manifest_only else "manifest and production policy"
    print(f"optimized FFI audit: {scope} clean ({len(entries)} sources)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
