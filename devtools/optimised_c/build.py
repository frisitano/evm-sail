#!/usr/bin/env python3
"""Authoritative source and flag model for the optimized C package."""

from __future__ import annotations

import subprocess
from dataclasses import dataclass
from pathlib import Path

from devtools.paths import REPO_ROOT

ROOT = REPO_ROOT
FFI_ROOT = ROOT / "extractions/c"
EDITABLE_FFI_SOURCE = FFI_ROOT / "optimised/contract/src"
EDITABLE_FFI_MANIFEST = FFI_ROOT / "optimised/contract/sources.list"

LANGUAGE_FLAGS = ("-std=c11",)
MODEL_DEFINES = (
    '-DEVMSAIL_MODEL_H="evmsail/spec.h"',
    "-DEVMSAIL_OPTIMIZED_FFI",
)
MODEL_CFLAGS = ("-ffunction-sections", "-fdata-sections")
PACKAGE_DEFAULT_CFLAGS = ("-O2", *LANGUAGE_FLAGS, "-DNDEBUG")


@dataclass(frozen=True)
class CompilationLayout:
    generated: Path
    generated_source: Path
    generated_manifest: Path
    include_root: Path
    ffi_source: Path
    ffi_manifest: Path
    packaged: bool


def manifest_entries(path: Path) -> list[str]:
    """Read a relative, ordered source manifest and reject ambiguous entries."""
    if not path.is_file():
        raise ValueError(f"missing source manifest: {path}")
    entries: list[str] = []
    for line in path.read_text().splitlines():
        entry = line.strip()
        if not entry or entry.startswith("#"):
            continue
        entry_path = Path(entry)
        if entry_path.is_absolute() or ".." in entry_path.parts:
            raise ValueError(f"manifest entry escapes its source tree: {entry}")
        entries.append(entry)
    if not entries:
        raise ValueError(f"empty source manifest: {path}")
    duplicates = sorted({entry for entry in entries if entries.count(entry) > 1})
    if duplicates:
        raise ValueError(f"duplicate source manifest entries: {', '.join(duplicates)}")
    return entries


def manifest_paths(path: Path, source_root: Path) -> list[Path]:
    paths: list[Path] = []
    for entry in manifest_entries(path):
        source = source_root / entry
        if not source.is_file():
            raise ValueError(f"manifest source does not exist: {source}")
        paths.append(source.resolve())
    return paths


def compilation_layout(generated: Path, *, editable_ffi: bool) -> CompilationLayout:
    generated = generated.resolve()
    generated_source = generated / "src/spec"
    packaged_ffi_source = generated / "src/ffi"
    packaged_ffi_manifest = packaged_ffi_source / "sources.list"
    packaged = packaged_ffi_manifest.is_file() and not editable_ffi
    return CompilationLayout(
        generated=generated,
        generated_source=generated_source,
        generated_manifest=generated_source / "sources.list",
        include_root=generated / "include",
        ffi_source=packaged_ffi_source if packaged else EDITABLE_FFI_SOURCE,
        ffi_manifest=packaged_ffi_manifest if packaged else EDITABLE_FFI_MANIFEST,
        packaged=packaged,
    )


def compilation_sources(layout: CompilationLayout) -> tuple[list[Path], list[Path]]:
    generated = manifest_paths(layout.generated_manifest, layout.generated_source)
    ffi = manifest_paths(layout.ffi_manifest, layout.ffi_source)
    return generated, ffi


def resolve_sail_lib(sail: str) -> Path:
    result = subprocess.run(
        [sail, "--dir"],
        cwd=ROOT,
        text=True,
        capture_output=True,
        check=False,
    )
    if result.returncode != 0:
        raise ValueError("cannot resolve the custom Sail library directory")
    sail_lib = Path(result.stdout.strip()) / "lib"
    if not (sail_lib / "sail.h").is_file():
        raise ValueError(f"missing Sail C runtime headers under {sail_lib}")
    return sail_lib.resolve()


def compilation_flags(layout: CompilationLayout, *, sail: str) -> list[str]:
    """Return flags shared by diagnostics and root editor metadata."""
    flags = [
        *LANGUAGE_FLAGS,
        *MODEL_DEFINES,
        *MODEL_CFLAGS,
        f"-I{layout.include_root}",
        f"-I{layout.ffi_source}",
    ]
    if not layout.packaged:
        flags.extend(
            [
                f"-I{ROOT / 'zkvm/runtime/sail256'}",
                f"-I{ROOT / 'zkvm/runtime'}",
                f"-I{resolve_sail_lib(sail)}",
                f"-I{FFI_ROOT / 'optimised/contract/include'}",
                f"-I{FFI_ROOT}",
            ]
        )
    return flags


def _make_words(words: tuple[str, ...]) -> str:
    return " ".join(word.replace('"', '\\"') for word in words)


def _make_source_word(word: str) -> str:
    """Escape a source path for a simply-expanded GNU Make assignment."""

    return word.replace("$", "$$").replace("#", r"\#").replace(" ", r"\ ")


def package_makefile(sources: tuple[str, ...] | None = None) -> str:
    """Render the relocatable package build from the shared flag constants."""
    default_cflags = _make_words(PACKAGE_DEFAULT_CFLAGS)
    model_cppflags = _make_words(MODEL_DEFINES)
    model_cflags = _make_words(MODEL_CFLAGS)
    source_assignment = (
        " ".join(_make_source_word(source) for source in sources)
        if sources is not None
        else "$(shell sed -e '/^[[:space:]]*\\#/d' -e '/^[[:space:]]*$$/d' src/sources.list)"
    )
    return f"""\
.DEFAULT_GOAL := all

CC ?= cc
AR ?= ar
CFLAGS ?= {default_cflags}
CPPFLAGS ?=

SOURCES := {source_assignment}
OBJECTS := $(patsubst %.c,build/%.o,$(SOURCES))
MODEL_CPPFLAGS := {model_cppflags}
MODEL_CFLAGS := {model_cflags}

.PHONY: all clean

all: libevmsail.a

libevmsail.a: $(OBJECTS)
\t$(AR) crs $@ $(OBJECTS)

build/%.o: src/%.c
\t@mkdir -p $(dir $@)
\t$(CC) $(CPPFLAGS) $(MODEL_CPPFLAGS) $(CFLAGS) $(MODEL_CFLAGS) \\
\t\t-Iinclude -Isrc/ffi -c $< -o $@

clean:
\trm -rf build libevmsail.a
"""
