import tempfile
import unittest
from pathlib import Path

from devtools.build_support import BuildSupportError
from devtools.optimised_c.check import (
    ScalarFinding,
    WaiverKey,
    load_waivers,
    reconcile_waivers,
)

REVISION = "7132a90c173997183dfe114f11dfa90459493879"


class OptimisedCConformanceWaiverTests(unittest.TestCase):
    def write_toolchains(self, directory: Path, revision: str = REVISION) -> Path:
        path = directory / "toolchains.toml"
        path.write_text(
            f"""
schema_version = 1
[sail]
commit = "{revision}"
[sail_lsp]
commit = "0000000000000000000000000000000000000000"
[lean]
sail_commit = "0000000000000000000000000000000000000000"
""",
            encoding="utf-8",
        )
        return path

    def write_waivers(self, directory: Path, revision: str = REVISION) -> Path:
        path = directory / "waivers.toml"
        path.write_text(
            f"""
schema_version = 1
compiler_revision = "{revision}"
[[waiver]]
rule = "immediate-scalar-return"
path = "src/spec/prelude.c"
name = "result_1"
""",
            encoding="utf-8",
        )
        return path

    def test_exact_finding_is_waived(self) -> None:
        key = WaiverKey("immediate-scalar-return", "src/spec/prelude.c", "result_1")
        finding = ScalarFinding(key, "finding")
        unwaived, stale, waived_count = reconcile_waivers([finding], {key})
        self.assertEqual(unwaived, [])
        self.assertEqual(stale, [])
        self.assertEqual(waived_count, 1)

    def test_new_finding_and_stale_waiver_both_fail_reconciliation(self) -> None:
        observed = WaiverKey("immediate-scalar-return", "src/spec/prelude.c", "result_2")
        waived = WaiverKey("immediate-scalar-return", "src/spec/prelude.c", "result_1")
        finding = ScalarFinding(observed, "new finding")
        unwaived, stale, waived_count = reconcile_waivers([finding], {waived})
        self.assertEqual(unwaived, [finding])
        self.assertEqual(stale, [waived])
        self.assertEqual(waived_count, 0)

    def test_waivers_are_bound_to_pinned_compiler(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            toolchains = self.write_toolchains(directory)
            waivers = self.write_waivers(directory, "1111111111111111111111111111111111111111")
            with self.assertRaisesRegex(BuildSupportError, "does not match pinned Sail"):
                load_waivers(waivers, toolchains)

    def test_repository_waivers_are_valid(self) -> None:
        waivers = load_waivers(
            Path("config/optimised-c-conformance-waivers.toml"),
            Path("config/toolchains.toml"),
        )
        self.assertEqual(len(waivers), 0)


if __name__ == "__main__":
    unittest.main()
