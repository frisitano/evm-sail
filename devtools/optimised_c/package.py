#!/usr/bin/env python3
"""Stage generated optimized C and its handwritten FFI as one source package."""

from __future__ import annotations

import argparse
import shutil
from pathlib import Path

from devtools.optimised_c.build import manifest_entries, package_makefile
from devtools.paths import REPO_ROOT

ROOT = REPO_ROOT
DEFAULT_GENERATED = ROOT / "build/c-optimised/generated"
RUNTIME_ROOT = ROOT / "zkvm/runtime"
RUNTIME_SOURCES = ("sail.c", "cycle_scopes.c")
RUNTIME_HEADERS = ("sail.h", "sail_failure.h", "cycle_scopes.h")


def copy_header_tree(source: Path, destination: Path) -> None:
    for header in sorted(source.rglob("*.h")):
        relative = header.relative_to(source)
        target = destination / relative
        if target.exists() and target.read_bytes() != header.read_bytes():
            raise ValueError(f"optimized FFI header collides with generated header: {relative}")
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(header, target)


def package(generated: Path, ffi_root: Path, *, excluded_ffi_sources: tuple[str, ...] = ()) -> None:
    generated = generated.resolve()
    ffi_root = ffi_root.resolve()
    generated_manifest = generated / "src/spec/sources.list"
    generated_entries = manifest_entries(generated_manifest)
    all_ffi_entries = manifest_entries(ffi_root / "optimised/contract/sources.list")
    excluded = set(excluded_ffi_sources)
    unknown = sorted(excluded - set(all_ffi_entries))
    if unknown:
        raise ValueError(f"excluded FFI source is not in the manifest: {', '.join(unknown)}")
    ffi_entries = [entry for entry in all_ffi_entries if entry not in excluded]

    for entry in generated_entries:
        if not (generated / "src/spec" / entry).is_file():
            raise ValueError(f"generated manifest source does not exist: {entry}")
    for entry in ffi_entries:
        if not (ffi_root / "optimised/contract/src" / entry).is_file():
            raise ValueError(f"optimized FFI manifest source does not exist: {entry}")

    include_root = generated / "include"
    copy_header_tree(ffi_root / "optimised/contract/include", include_root)
    for contract in ("zkvm_accelerators.h", "zkvm_bigint.h", "zkvm_io.h"):
        shutil.copy2(ffi_root / contract, include_root / contract)

    runtime_source_root = RUNTIME_ROOT / "sail256"
    runtime_dir = generated / "src/runtime"
    if runtime_dir.exists():
        shutil.rmtree(runtime_dir)
    runtime_dir.mkdir(parents=True)
    for name in RUNTIME_HEADERS:
        source = RUNTIME_ROOT / name
        if not source.is_file():
            source = runtime_source_root / name
        if not source.is_file():
            raise ValueError(f"missing optimized runtime header: {name}")
        shutil.copy2(source, include_root / name)
    for name in RUNTIME_SOURCES:
        source = RUNTIME_ROOT / name
        if not source.is_file():
            source = runtime_source_root / name
        if not source.is_file():
            raise ValueError(f"missing optimized runtime source: {name}")
        shutil.copy2(source, runtime_dir / name)

    staged_ffi = generated / "src/ffi"
    # This path is valid only after the generated model manifest above has
    # established that `generated` is an optimized-C build directory.
    if staged_ffi.exists():
        shutil.rmtree(staged_ffi)
    shutil.copytree(ffi_root / "optimised/contract/src", staged_ffi)
    (staged_ffi / "sources.list").write_text(
        "# Ordered optimized FFI sources, relative to this directory.\n"
        + "".join(f"{entry}\n" for entry in ffi_entries)
    )

    combined = generated / "src/sources.list"
    combined.write_text(
        "# Complete optimized evm-sail source package, relative to src/.\n"
        "# Generated Sail modules precede their optimized host definitions.\n"
        + "".join(f"spec/{entry}\n" for entry in generated_entries)
        + "".join(f"ffi/{entry}\n" for entry in ffi_entries)
        + "".join(f"runtime/{entry}\n" for entry in RUNTIME_SOURCES)
    )
    shutil.copy2(ROOT / "LICENSE", generated / "LICENSE")
    package_sources = tuple(
        [f"spec/{entry}" for entry in generated_entries]
        + [f"ffi/{entry}" for entry in ffi_entries]
        + [f"runtime/{entry}" for entry in RUNTIME_SOURCES]
    )
    (generated / "Makefile").write_text(package_makefile(package_sources))
    (generated / "PACKAGE.md").write_text(
        "# Optimized evm-sail C package\n\n"
        "This directory is the self-contained production source package for "
        "the generated model, optimized FFI, and fixed-width Sail runtime. "
        "Run `make` here to build `libevmsail.a`; `src/sources.list` is the "
        "complete ordered source manifest used by that build.\n\n"
        "The final executable supplies the platform implementations declared "
        "by `include/zkvm_accelerators.h` and `include/zkvm_io.h`. A zkVM "
        "guest also supplies its platform startup, allocator, abort, and link "
        "layout when linking the archive.\n"
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("generated", nargs="?", type=Path, default=DEFAULT_GENERATED)
    parser.add_argument("--ffi-root", type=Path, default=ROOT / "extractions/c")
    parser.add_argument("--exclude-ffi-source", action="append", default=[])
    args = parser.parse_args()
    try:
        package(
            args.generated,
            args.ffi_root,
            excluded_ffi_sources=tuple(args.exclude_ffi_source),
        )
    except (OSError, ValueError) as error:
        print(f"optimized C package: {error}")
        return 2
    print(f"optimized C package: staged at {args.generated.resolve()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
