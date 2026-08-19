import tempfile
import tomllib
import unittest
from pathlib import Path

from devtools.build_support import BuildSupportError
from devtools.toolchains import flatten, load_toolchains


class ToolchainManifestTests(unittest.TestCase):
    def test_repository_manifest_has_full_source_revisions(self) -> None:
        values = flatten(load_toolchains(Path("config/toolchains.toml")))
        self.assertEqual(len(values["SAIL_COMMIT"]), 40)
        self.assertEqual(len(values["SAIL_LSP_COMMIT"]), 40)
        self.assertEqual(len(values["LEAN_SAIL_COMMIT"]), 40)
        self.assertEqual(values["SOLVER_Z3_SUPPORTED_MAJOR"], "4")
        self.assertEqual(values["LLVM_MAJOR"], "18")
        project = tomllib.loads(Path("pyproject.toml").read_text(encoding="utf-8"))
        self.assertEqual(project["project"]["requires-python"], "==3.12.*")
        self.assertEqual(project["tool"]["uv"]["python-preference"], "only-managed")

    def test_rejects_branch_in_place_of_commit(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "toolchains.toml"
            path.write_text(
                """
schema_version = 1
[sail]
commit = "main"
[sail_lsp]
commit = "0000000000000000000000000000000000000000"
[lean]
sail_commit = "0000000000000000000000000000000000000000"
[solver]
z3_supported_major = 4
z3_memo_schema = 1
[llvm]
major = 18
""",
                encoding="utf-8",
            )
            with self.assertRaises(BuildSupportError):
                load_toolchains(path)

    def test_rejects_nonpositive_tool_versions(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "toolchains.toml"
            path.write_text(
                Path("config/toolchains.toml").read_text(encoding="utf-8"), encoding="utf-8"
            )
            path.write_text(
                path.read_text(encoding="utf-8").replace(
                    "z3_supported_major = 4", "z3_supported_major = 0"
                ),
                encoding="utf-8",
            )
            with self.assertRaises(BuildSupportError):
                load_toolchains(path)


if __name__ == "__main__":
    unittest.main()
