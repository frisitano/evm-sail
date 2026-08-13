#!/usr/bin/env python3
"""Stage generated optimized C and its handwritten FFI as one source package."""

from __future__ import annotations

import argparse
import shutil
import sys
from pathlib import Path

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.optimised_c_build import manifest_entries, package_makefile


ROOT = Path(__file__).resolve().parents[1]
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


def package(generated: Path, ffi_root: Path) -> None:
    generated = generated.resolve()
    ffi_root = ffi_root.resolve()
    generated_manifest = generated / "src/spec/sources.list"
    generated_entries = manifest_entries(generated_manifest)
    ffi_entries = manifest_entries(ffi_root / "optimised/contract/sources.list")

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
    shutil.copy2(ffi_root / "optimised/contract/sources.list", staged_ffi / "sources.list")

    combined = generated / "src/sources.list"
    combined.write_text(
        "# Complete optimized evm-sail source package, relative to src/.\n"
        "# Generated Sail modules precede their optimized host definitions.\n"
        + "".join(f"spec/{entry}\n" for entry in generated_entries)
        + "".join(f"ffi/{entry}\n" for entry in ffi_entries)
        + "".join(f"runtime/{entry}\n" for entry in RUNTIME_SOURCES)
    )
    shutil.copy2(ROOT / "LICENSE", generated / "LICENSE")
    (generated / "Makefile").write_text(package_makefile())
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
    args = parser.parse_args()
    try:
        package(args.generated, args.ffi_root)
    except (OSError, ValueError) as error:
        print(f"optimized C package: {error}")
        return 2
    print(f"optimized C package: staged at {args.generated.resolve()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
