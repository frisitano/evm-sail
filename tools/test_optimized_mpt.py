#!/usr/bin/env python3
"""Native differential tests for the optimized sequential MPT reducer."""

import ctypes
import os
import sys
import unittest


ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXT = "dylib" if sys.platform == "darwin" else "so"
LIB = os.path.join(
    ROOT,
    "zkvm",
    "native-runner",
    ".build-optimized",
    f"libevmsail_guest.{EXT}",
)

SCENARIOS = (
    "empty trie insertion",
    "single-leaf replacement",
    "branch insertion",
    "one branch-child replacement",
    "branch and extension collapse",
    "single-leaf deletion",
    "mixed deep update/insert/delete",
    "two branch-child replacements",
)


class OptimizedMptReducerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        if not os.path.exists(LIB):
            raise unittest.SkipTest(
                "optimized native library is absent; build it with "
                "`python3 harness/run.py --build optimized --rebuild --limit 1 ...`"
            )
        cls.lib = ctypes.CDLL(LIB)
        try:
            cls.selftest = cls.lib.mpt_selftest
        except AttributeError as exc:
            raise unittest.SkipTest(
                "optimized native library predates the MPT self-test; rebuild it"
            ) from exc
        cls.selftest.argtypes = [ctypes.c_uint64]
        cls.selftest.restype = ctypes.c_uint64

    def test_sequential_reducer_matches_canonical_rebuild(self) -> None:
        for index, name in enumerate(SCENARIOS):
            with self.subTest(scenario=name):
                self.assertEqual(self.selftest(index), 0)


if __name__ == "__main__":
    unittest.main()
