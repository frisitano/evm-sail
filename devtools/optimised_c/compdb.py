#!/usr/bin/env python3
"""Generate root clangd metadata for generated and editable optimized C."""

from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
from pathlib import Path
from typing import TypedDict

from devtools.optimised_c.build import (
    ROOT,
    compilation_flags,
    compilation_layout,
    compilation_sources,
)

DEFAULT_GENERATED = ROOT / "build/c-optimised/generated"
DEFAULT_OUTPUT = ROOT / "compile_commands.json"


class CompilationEntry(TypedDict):
    directory: str
    file: str
    arguments: list[str]


def database_entries(
    generated: Path, *, clang: str, sail: str
) -> tuple[list[CompilationEntry], int]:
    layout = compilation_layout(generated, editable_ffi=True)
    generated_sources, ffi_sources = compilation_sources(layout)
    flags = compilation_flags(layout, sail=sail)
    entries: list[CompilationEntry] = [
        {
            "directory": str(ROOT),
            "file": str(source),
            "arguments": [clang, *flags, "-c", str(source)],
        }
        for source in (*generated_sources, *ffi_sources)
    ]
    return entries, len(generated_sources)


def write_database(output: Path, entries: list[CompilationEntry]) -> None:
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(entries, indent=2) + "\n")


def check_database(output: Path, expected: list[CompilationEntry], generated_count: int) -> None:
    recorded = json.loads(output.read_text())
    expected_files = [entry["file"] for entry in expected]
    recorded_files = [entry.get("file") for entry in recorded]
    if recorded_files != expected_files:
        raise ValueError("compilation database membership or manifest order drifted")
    if len(recorded_files) != len(set(recorded_files)):
        raise ValueError("compilation database contains duplicate translation units")

    representative = next(
        (
            entry
            for entry in recorded
            if entry["file"].endswith("/host/region_access.c")
            and entry["file"] in set(expected_files[:generated_count])
        ),
        recorded[0] if recorded else None,
    )
    if representative is None:
        raise ValueError("compilation database is empty")
    command = [
        "-fsyntax-only" if argument == "-c" else argument
        for argument in representative["arguments"]
    ]
    result = subprocess.run(
        command,
        cwd=representative["directory"],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(
            "representative compilation-database syntax check failed:\n" + result.stdout
        )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("generated", nargs="?", type=Path, default=DEFAULT_GENERATED)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--clang", default=os.environ.get("CLANG", "clang"))
    parser.add_argument("--sail", default=os.environ.get("SAIL", "sail"))
    parser.add_argument(
        "--check",
        action="store_true",
        help="verify exact manifest membership and syntax-check one generated entry",
    )
    args = parser.parse_args()

    clang = shutil.which(args.clang)
    sail = shutil.which(args.sail)
    if clang is None:
        print(f"optimized C compdb: clang not found: {args.clang}")
        return 2
    if sail is None:
        print(f"optimized C compdb: Sail not found: {args.sail}")
        return 2
    try:
        entries, generated_count = database_entries(args.generated, clang=clang, sail=sail)
        write_database(args.output.resolve(), entries)
        if args.check:
            check_database(args.output.resolve(), entries, generated_count)
    except (OSError, RuntimeError, ValueError, json.JSONDecodeError) as error:
        print(f"optimized C compdb: {error}")
        return 1

    ffi_count = len(entries) - generated_count
    check_status = "; membership and representative syntax check passed" if args.check else ""
    print(
        f"optimized C compdb: wrote {args.output.resolve()} "
        f"({generated_count} generated + {ffi_count} editable FFI = {len(entries)} entries"
        f"{check_status})"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
