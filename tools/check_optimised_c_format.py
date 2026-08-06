#!/usr/bin/env python3
"""Report whether optimized C matches the repository clang-format policy.

The checker never rewrites generated output. Formatting defects in generated C
belong in the Sail/JIB C emitter; handwritten optimized FFI can be formatted in
place separately after its semantic changes have settled.
"""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import xml.etree.ElementTree as ET
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_GENERATED = ROOT / "build/c-optimised/generated"


@dataclass(frozen=True)
class FormatResult:
    path: Path
    owner: str
    replacements: int
    error: str | None = None


def manifest_paths(manifest: Path, source_root: Path) -> list[Path]:
    paths: list[Path] = []
    for entry in manifest.read_text().splitlines():
        entry = entry.strip()
        if not entry or entry.startswith("#"):
            continue
        path = source_root / entry
        if not path.is_file():
            raise ValueError(f"manifest source does not exist: {path}")
        paths.append(path)
    return paths


def optimized_files(generated: Path, scope: str) -> list[tuple[Path, str]]:
    files: list[tuple[Path, str]] = []
    if scope in ("all", "generated"):
        generated_source = generated / "src/spec"
        generated_manifest = generated_source / "sources.list"
        if not generated_manifest.is_file():
            raise ValueError(f"missing generated manifest: {generated_manifest}")
        files.extend(
            (path, "generated compiler output")
            for path in manifest_paths(generated_manifest, generated_source)
        )
        files.extend(
            (path, "generated compiler output")
            for path in sorted((generated / "include").rglob("*.h"))
        )

    if scope in ("all", "ffi"):
        ffi_source = ROOT / "ffi/optimized/src"
        ffi_manifest = ROOT / "ffi/optimized/sources.list"
        files.extend(
            (path, "optimized FFI")
            for path in manifest_paths(ffi_manifest, ffi_source)
        )
        files.extend(
            (path, "optimized FFI")
            for path in sorted(ffi_source.rglob("*.h"))
        )
        files.extend(
            (path, "optimized FFI")
            for path in sorted((ROOT / "ffi/optimized/include").rglob("*.h"))
        )

    deduplicated: dict[Path, str] = {}
    for path, owner in files:
        deduplicated[path.resolve()] = owner
    return sorted(deduplicated.items())


def check_file(
    clang_format: str, style: Path, item: tuple[Path, str]
) -> FormatResult:
    path, owner = item
    result = subprocess.run(
        [
            clang_format,
            f"--style=file:{style}",
            "--output-replacements-xml",
            str(path),
        ],
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if result.returncode != 0:
        return FormatResult(path, owner, 0, result.stderr.strip() or result.stdout.strip())
    try:
        replacements = len(ET.fromstring(result.stdout).findall("replacement"))
    except ET.ParseError as error:
        return FormatResult(path, owner, 0, f"invalid clang-format XML: {error}")
    return FormatResult(path, owner, replacements)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("generated", nargs="?", type=Path, default=DEFAULT_GENERATED)
    parser.add_argument(
        "--clang-format",
        default=os.environ.get("CLANG_FORMAT", "clang-format"),
    )
    parser.add_argument("--scope", choices=("all", "generated", "ffi"), default="all")
    parser.add_argument("--strict", action="store_true")
    parser.add_argument("--jobs", type=int, default=min(8, os.cpu_count() or 1))
    parser.add_argument("--max-files", type=int, default=50)
    args = parser.parse_args()

    clang_format = shutil.which(args.clang_format)
    if clang_format is None:
        print(f"optimized C format: clang-format not found: {args.clang_format}")
        return 2
    style = ROOT / ".clang-format"
    if not style.is_file():
        print(f"optimized C format: missing policy: {style}")
        return 2

    try:
        files = optimized_files(args.generated.resolve(), args.scope)
    except ValueError as error:
        print(f"optimized C format: {error}")
        return 2

    with ThreadPoolExecutor(max_workers=max(1, args.jobs)) as executor:
        results = list(
            executor.map(
                lambda item: check_file(clang_format, style, item),
                files,
            )
        )

    errors = [result for result in results if result.error is not None]
    if errors:
        for result in errors:
            print(f"{result.path}: {result.error}")
        print(f"optimized C format: FAILED ({len(errors)} tool errors)")
        return 2

    drift = [result for result in results if result.replacements]
    if drift:
        print("optimized C clang-format queue:")
        displayed = drift if args.max_files == 0 else drift[: args.max_files]
        for result in displayed:
            relative = result.path.relative_to(ROOT)
            print(f"  {relative}: {result.replacements} replacements ({result.owner})")
        if args.max_files and len(drift) > args.max_files:
            print(f"  ... {len(drift) - args.max_files} more files")

    owner_counts: dict[str, tuple[int, int]] = {}
    for result in results:
        files_with_drift, replacements = owner_counts.get(result.owner, (0, 0))
        owner_counts[result.owner] = (
            files_with_drift + (1 if result.replacements else 0),
            replacements + result.replacements,
        )
    summary = ", ".join(
        f"{owner}: {files_with_drift} files/{replacements} replacements"
        for owner, (files_with_drift, replacements) in sorted(owner_counts.items())
    )
    print(
        f"optimized C format: {len(drift)} of {len(results)} files need formatting; "
        f"{summary or 'no files'}"
    )
    return 1 if args.strict and drift else 0


if __name__ == "__main__":
    raise SystemExit(main())
