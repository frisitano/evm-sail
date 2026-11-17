"""Opt-in EELS selection of the explicitly requested Glacier/BPO profiles.

Load with ``fill -p devtools.tests.full_fork_selection --fork ...``. The pinned
framework normally hides Glacier forks and excludes BPO forks unless a test
has its blob-parameter marker. This plugin removes those selection shortcuts
and gives the difficulty-only Glacier profiles London's EVM feature ordering.
Test bodies, assertions, and reference execution remain unchanged. It is never
loaded by the normal repository test suite.
"""

from __future__ import annotations

from typing import Any


def pytest_load_initial_conftests(early_config: Any, parser: Any, args: list[str]) -> None:
    """Enable full explicitly selected profile coverage before collection."""
    from execution_testing import forks
    from execution_testing.cli.pytest_commands.plugins.forks import forks as plugin
    from execution_testing.forks import helpers

    extra = frozenset((forks.ArrowGlacier, forks.GrayGlacier))
    for module in (forks, helpers, plugin):
        module.ALL_FORKS = module.ALL_FORKS | extra
        module.ALL_FORKS_WITH_TRANSITIONS = module.ALL_FORKS_WITH_TRANSITIONS | extra

    def no_default_bpo_exclusion(self: Any) -> set[Any]:
        return set()

    plugin.ValidForBPOForks._process_with_marker_args = no_default_bpo_exclusion
    meta = type(forks.London)
    for name in ("__lt__", "__le__", "__gt__", "__ge__"):
        original = getattr(meta, name)

        def compare(left: Any, right: Any, operation: Any = original) -> bool:
            if left is forks.ArrowGlacier or left is forks.GrayGlacier:
                left = forks.London
            if right is forks.ArrowGlacier or right is forks.GrayGlacier:
                right = forks.London
            return bool(operation(left, right))

        setattr(meta, name, compare)
