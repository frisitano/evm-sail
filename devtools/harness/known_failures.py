"""Known corpus failures: an exact allow-list for the fixture harness.

A corpus run passes only if its failing cases equal the list in
``config/corpora/tests-zkevm-known-failures.toml``. An entry names the fixture
file (relative to the corpus directory), the case id within it, and how the
case fails. A failing case that is not listed is a regression, a listed case
that no longer fails must come off the list, and a listed case whose failure
category changed is reported as well, so a case cannot start failing for a
different reason unnoticed.
"""

from __future__ import annotations

import re
import tomllib
from collections.abc import Iterable, Mapping
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import NamedTuple

from devtools.build_support import BuildSupportError

SCHEMA_VERSION = 1

# How a case fails: the guest output differs from the reference (`diff`), the
# guest raised while running (`runerr`), the input could not be built
# (`builderr`), or the fixture has no reference output (`noref`).
CATEGORIES = ("builderr", "diff", "noref", "runerr")

# `FAIL <case id> [<fixture file>] <category>` with an optional `: <detail>`.
FAIL_LINE_RE = re.compile(
    r"^FAIL (?P<case>.+?) \[(?P<file>[^\[\]]+\.jsonl?)\] "
    rf"(?P<category>{'|'.join(CATEGORIES)})(?:: .*)?$"
)


class FailureKey(NamedTuple):
    """One corpus case: a fixture file (relative path) and a case id in it."""

    file: str
    case: str


@dataclass(frozen=True)
class KnownFailureComparison:
    """The difference between the failing cases and the allow-list."""

    regressions: tuple[tuple[FailureKey, str], ...]
    fixed: tuple[tuple[FailureKey, str], ...]
    changed: tuple[tuple[FailureKey, str, str], ...]

    @property
    def matches(self) -> bool:
        return not (self.regressions or self.fixed or self.changed)


def load_known_failures(path: Path) -> dict[FailureKey, str]:
    """Read and validate an allow-list: each failing case and its category."""

    try:
        data = tomllib.loads(path.read_text(encoding="utf-8"))
    except tomllib.TOMLDecodeError as error:
        raise BuildSupportError(f"{path}: invalid TOML: {error}") from error
    if data.get("schema_version") != SCHEMA_VERSION:
        raise BuildSupportError(f"{path}: unsupported schema {data.get('schema_version')!r}")
    entries = data.get("cases")
    if not isinstance(entries, list):
        raise BuildSupportError(f"{path}: cases must be an array of tables")
    known: dict[FailureKey, str] = {}
    for entry in entries:
        if not isinstance(entry, dict) or set(entry) != {"file", "case", "failure"}:
            raise BuildSupportError(f"{path}: each case needs exactly file, case and failure")
        file, case, failure = entry["file"], entry["case"], entry["failure"]
        if not all(isinstance(value, str) and value for value in (file, case, failure)):
            raise BuildSupportError(f"{path}: file, case and failure must be non-empty strings")
        relative = PurePosixPath(file)
        if relative.is_absolute() or ".." in relative.parts:
            raise BuildSupportError(f"{path}: fixture path must be relative: {file!r}")
        if failure not in CATEGORIES:
            raise BuildSupportError(f"{path}: unknown failure category {failure!r} for {case!r}")
        key = FailureKey(file, case)
        if key in known:
            raise BuildSupportError(f"{path}: duplicate case {file}::{case}")
        known[key] = failure
    return known


def parse_failure(line: str) -> tuple[str, str, str] | None:
    """Return (fixture file, case id, category) from a harness `FAIL` line."""

    match = FAIL_LINE_RE.match(line)
    if match is None:
        return None
    return match.group("file"), match.group("case"), match.group("category")


def compare_known_failures(
    failing: Mapping[FailureKey, str], known: Mapping[FailureKey, str]
) -> KnownFailureComparison:
    """Split the difference between failing cases and the allow-list."""

    return KnownFailureComparison(
        regressions=tuple(sorted((key, failing[key]) for key in failing.keys() - known.keys())),
        fixed=tuple(sorted((key, known[key]) for key in known.keys() - failing.keys())),
        changed=tuple(
            sorted(
                (key, known[key], failing[key])
                for key in failing.keys() & known.keys()
                if failing[key] != known[key]
            )
        ),
    )


def format_comparison(comparison: KnownFailureComparison, path: str, known_count: int) -> str:
    """Render the outcome of a comparison for the harness summary."""

    def label(key: FailureKey) -> str:
        return f"{key.file}::{key.case}"

    lines = [f"=== known failures: {known_count} listed in {path} ==="]
    if comparison.regressions:
        lines.append(f"REGRESSION: {len(comparison.regressions)} case(s) fail but are not listed:")
        lines.extend(f"  {label(key)} ({category})" for key, category in comparison.regressions)
    if comparison.fixed:
        lines.append(
            f"FIXED: {len(comparison.fixed)} listed case(s) no longer fail "
            "(or are absent from the corpus); remove them from the list:"
        )
        lines.extend(f"  {label(key)} ({category})" for key, category in comparison.fixed)
    if comparison.changed:
        lines.append(f"CHANGED: {len(comparison.changed)} listed case(s) fail differently:")
        lines.extend(
            f"  {label(key)} (listed {listed}, now {actual})"
            for key, listed, actual in comparison.changed
        )
    if comparison.matches:
        lines.append("failing cases match the list exactly")
    return "\n".join(lines)


def known_failure_keys(failures: Iterable[tuple[str, str, str]]) -> dict[FailureKey, str]:
    """Index (file, case, category) triples; a repeated case is an error."""

    indexed: dict[FailureKey, str] = {}
    for file, case, category in failures:
        key = FailureKey(file, case)
        if key in indexed:
            raise BuildSupportError(f"case failed twice: {file}::{case}")
        indexed[key] = category
    return indexed
