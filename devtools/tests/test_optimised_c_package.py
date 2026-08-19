from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from devtools.optimised_c import package as optimised_c_package


class OptimisedCPackageTests(unittest.TestCase):
    def test_filters_manifest_and_serializes_make_sources(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            generated = root / "generated"
            spec = generated / "src/spec"
            spec.mkdir(parents=True)
            (spec / "model file.c").write_text("", encoding="utf-8")
            (spec / "sources.list").write_text("model file.c\n", encoding="utf-8")

            ffi = root / "ffi"
            ffi_src = ffi / "optimised/contract/src"
            ffi_include = ffi / "optimised/contract/include"
            ffi_src.mkdir(parents=True)
            ffi_include.mkdir(parents=True)
            (ffi_src / "keep.c").write_text("", encoding="utf-8")
            (ffi_src / "skip.c").write_text("", encoding="utf-8")
            (ffi_src.parent / "sources.list").write_text("keep.c\nskip.c\n", encoding="utf-8")
            (ffi_include / "ffi.h").write_text("", encoding="utf-8")
            for name in ("zkvm_accelerators.h", "zkvm_bigint.h", "zkvm_io.h"):
                (ffi / name).write_text("", encoding="utf-8")

            runtime = root / "runtime"
            runtime.mkdir()
            for name in optimised_c_package.RUNTIME_HEADERS:
                (runtime / name).write_text("", encoding="utf-8")
            for name in optimised_c_package.RUNTIME_SOURCES:
                (runtime / name).write_text("", encoding="utf-8")
            (root / "LICENSE").write_text("license\n", encoding="utf-8")

            with (
                patch.object(optimised_c_package, "ROOT", root),
                patch.object(optimised_c_package, "RUNTIME_ROOT", runtime),
            ):
                optimised_c_package.package(generated, ffi, excluded_ffi_sources=("skip.c",))

            combined = (generated / "src/sources.list").read_text(encoding="utf-8")
            self.assertIn("spec/model file.c", combined)
            self.assertIn("ffi/keep.c", combined)
            self.assertNotIn("ffi/skip.c", combined)
            makefile = (generated / "Makefile").read_text(encoding="utf-8")
            self.assertIn(r"spec/model\ file.c", makefile)
            self.assertIn("ffi/keep.c", makefile)
            self.assertNotIn("ffi/skip.c", makefile)

    def test_rejects_unknown_exclusion(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            generated = root / "generated"
            spec = generated / "src/spec"
            spec.mkdir(parents=True)
            (spec / "model.c").write_text("", encoding="utf-8")
            (spec / "sources.list").write_text("model.c\n", encoding="utf-8")
            ffi = root / "ffi/optimised/contract/src"
            ffi.mkdir(parents=True)
            (ffi / "keep.c").write_text("", encoding="utf-8")
            (ffi.parent / "sources.list").write_text("keep.c\n", encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "not in the manifest"):
                optimised_c_package.package(
                    generated,
                    root / "ffi",
                    excluded_ffi_sources=("unknown.c",),
                )


if __name__ == "__main__":
    unittest.main()
