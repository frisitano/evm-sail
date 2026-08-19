#!/usr/bin/env python3
"""Check the stable source-quality contract of optimized C extraction."""

from __future__ import annotations

import argparse
import re
import tomllib
from dataclasses import dataclass
from pathlib import Path
from typing import cast

from devtools.build_support import BuildSupportError
from devtools.paths import REPO_ROOT
from devtools.toolchains import SHA_RE, load_toolchains

ROOT = REPO_ROOT
DEFAULT_GENERATED = ROOT / "build/c-optimised/generated"
DEFAULT_TOOLCHAINS = ROOT / "config/toolchains.toml"
DEFAULT_WAIVERS = ROOT / "config/optimised-c-conformance-waivers.toml"

SCALAR_DECLARATION = re.compile(
    r"^(?P<indent>\s+)(?:bool|u?int(?:8|16|32|64)_t|u128|u256|u320|"
    r"word|bytes20|bytes32|unit)\s+(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*;$"
)
SCALAR_INITIALIZER = re.compile(
    r"^(?P<indent>\s+)(?:bool|u?int(?:8|16|32|64)_t|u128|u256|u320|"
    r"word|bytes20|bytes32|unit)\s+(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*=.*;$"
)
GAS_DEFINITION = re.compile(
    r"^(?P<qualifier>const\s+)?(?:bool|u?int(?:8|16|32|64)_t|u128|u256|u320)\s+"
    r"G_[A-Za-z0-9_]+\s*="
)
GAS_DECLARATION = re.compile(
    r"^(?P<qualifier>extern\s+const\s+)?"
    r"(?:bool|u?int(?:8|16|32|64)_t|u128|u256|u320)\s+G_[A-Za-z0-9_]+\s*;"
)
FUNCTION_START = re.compile(r"^u256 stateless_input_slice_load\(Bytes s, uint32_t off\)$")
FATAL_MATCH_FAILURE = re.compile(r"fatal_error\([^;]+\);\s*sail_match_failure\(", re.MULTILINE)


@dataclass(frozen=True, order=True)
class WaiverKey:
    rule: str
    path: str
    name: str


@dataclass(frozen=True)
class ScalarFinding:
    key: WaiverKey
    message: str


WAIVABLE_RULES = frozenset(
    {
        "adjacent-scalar-initializer",
        "immediate-scalar-return",
    }
)


def load_waivers(path: Path, toolchains_path: Path) -> set[WaiverKey]:
    data = cast(dict[str, object], tomllib.loads(path.read_text(encoding="utf-8")))
    if data.get("schema_version") != 1:
        raise BuildSupportError(f"{path}: unsupported conformance-waiver schema")

    revision = data.get("compiler_revision")
    if not isinstance(revision, str) or not SHA_RE.fullmatch(revision):
        raise BuildSupportError(f"{path}: compiler_revision must be a full Git SHA")
    toolchains = load_toolchains(toolchains_path)
    sail = toolchains.get("sail")
    assert isinstance(sail, dict)
    pinned_revision = sail["commit"]
    if revision != pinned_revision:
        raise BuildSupportError(
            f"{path}: compiler_revision {revision} does not match pinned Sail "
            f"revision {pinned_revision}"
        )

    entries = data.get("waiver")
    if not isinstance(entries, list):
        raise BuildSupportError(f"{path}: waiver must be an array of tables")
    waivers: set[WaiverKey] = set()
    for index, raw_entry in enumerate(entries, 1):
        if not isinstance(raw_entry, dict):
            raise BuildSupportError(f"{path}: waiver {index} must be a table")
        if set(raw_entry) != {"rule", "path", "name"}:
            raise BuildSupportError(
                f"{path}: waiver {index} must contain only rule, path, and name"
            )
        rule = raw_entry["rule"]
        source_path = raw_entry["path"]
        name = raw_entry["name"]
        if not isinstance(rule, str) or rule not in WAIVABLE_RULES:
            raise BuildSupportError(f"{path}: waiver {index} has unknown rule {rule!r}")
        if not isinstance(source_path, str):
            raise BuildSupportError(f"{path}: waiver {index} path must be a string")
        parsed_path = Path(source_path)
        if (
            parsed_path.is_absolute()
            or ".." in parsed_path.parts
            or not source_path.startswith("src/spec/")
        ):
            raise BuildSupportError(f"{path}: waiver {index} path must be beneath src/spec")
        if not isinstance(name, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name):
            raise BuildSupportError(f"{path}: waiver {index} name is not a C identifier")
        key = WaiverKey(rule, source_path, name)
        if key in waivers:
            raise BuildSupportError(f"{path}: duplicate waiver {key}")
        waivers.add(key)
    return waivers


def reconcile_waivers(
    findings: list[ScalarFinding], waivers: set[WaiverKey]
) -> tuple[list[ScalarFinding], list[WaiverKey], int]:
    observed_keys = {finding.key for finding in findings}
    unwaived = [finding for finding in findings if finding.key not in waivers]
    stale = sorted(waivers - observed_keys)
    return unwaived, stale, len(observed_keys & waivers)


def source_files(generated: Path) -> list[Path]:
    source_root = generated / "src/spec"
    manifest = source_root / "sources.list"
    if not manifest.is_file():
        raise ValueError(f"missing generated source manifest: {manifest}")

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


def adjacent_split_initializers(path: Path, text: str) -> list[ScalarFinding]:
    lines = text.splitlines()
    findings: list[ScalarFinding] = []
    for index, line in enumerate(lines):
        match = SCALAR_DECLARATION.match(line)
        if match is None:
            continue
        following = index + 1
        while following < len(lines) and not lines[following].strip():
            following += 1
        if following >= len(lines):
            continue
        assignment = re.match(
            rf"^{re.escape(match.group('indent'))}{re.escape(match.group('name'))}\s*=",
            lines[following],
        )
        if assignment:
            name = match.group("name")
            findings.append(
                ScalarFinding(
                    WaiverKey("adjacent-scalar-initializer", path.as_posix(), name),
                    f"{path}:{index + 1}: adjacent scalar declaration and assignment for {name}",
                )
            )
    return findings


def immediate_scalar_return_temporaries(path: Path, text: str) -> list[ScalarFinding]:
    lines = text.splitlines()
    findings: list[ScalarFinding] = []
    for index, line in enumerate(lines[:-1]):
        match = SCALAR_INITIALIZER.match(line)
        if match is None:
            continue
        following = index + 1
        while following < len(lines) and not lines[following].strip():
            following += 1
        if following >= len(lines):
            continue
        returned = re.match(
            rf"^{re.escape(match.group('indent'))}return\s+"
            rf"{re.escape(match.group('name'))}\s*;$",
            lines[following],
        )
        if returned:
            name = match.group("name")
            findings.append(
                ScalarFinding(
                    WaiverKey("immediate-scalar-return", path.as_posix(), name),
                    f"{path}:{index + 1}: scalar {name} is initialized only to be "
                    "returned immediately",
                )
            )
    return findings


def function_body(lines: list[str], start: re.Pattern[str]) -> str | None:
    for index, line in enumerate(lines):
        if start.match(line):
            depth = 0
            body: list[str] = []
            for body_line in lines[index:]:
                body.append(body_line)
                depth += body_line.count("{") - body_line.count("}")
                if depth == 0 and "}" in body_line:
                    return "\n".join(body)
    return None


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "generated",
        nargs="?",
        type=Path,
        default=DEFAULT_GENERATED,
        help="optimized generated-C package directory",
    )
    parser.add_argument("--toolchains", type=Path, default=DEFAULT_TOOLCHAINS)
    parser.add_argument("--waivers", type=Path, default=DEFAULT_WAIVERS)
    args = parser.parse_args()
    generated = args.generated.resolve()

    try:
        sources = source_files(generated)
        waivers = load_waivers(args.waivers, args.toolchains)
    except (BuildSupportError, OSError, ValueError, tomllib.TOMLDecodeError) as error:
        print(f"optimized C conformance: {error}")
        return 1

    errors: list[str] = []
    scalar_findings: list[ScalarFinding] = []
    texts: dict[Path, str] = {}
    for path in sources:
        text = path.read_text(errors="replace")
        texts[path] = text
        relative_path = path.relative_to(generated)
        scalar_findings.extend(adjacent_split_initializers(relative_path, text))
        scalar_findings.extend(immediate_scalar_return_temporaries(relative_path, text))
        if FATAL_MATCH_FAILURE.search(text):
            errors.append(f"{path}: fatal_error retains an unreachable Sail exit bridge")
        for legacy_exception_state in ("have_exception", "current_exception"):
            if re.search(rf"\b{legacy_exception_state}\b", text):
                errors.append(
                    f"{path}: generated C retains legacy exception state {legacy_exception_state}"
                )
        for number, line in enumerate(text.splitlines(), 1):
            gas_definition = GAS_DEFINITION.match(line)
            if gas_definition and gas_definition.group("qualifier") is None:
                errors.append(f"{path}:{number}: literal gas storage is not const: {line.strip()}")

    unwaived_findings, stale_waivers, waived_count = reconcile_waivers(scalar_findings, waivers)
    errors.extend(finding.message for finding in unwaived_findings)
    errors.extend(f"stale optimized C conformance waiver: {waiver}" for waiver in stale_waivers)

    include_root = generated / "include"
    headers = list(include_root.rglob("*.h"))
    for path in headers:
        header_text = path.read_text(errors="replace")
        for legacy_exception_state in ("have_exception", "current_exception"):
            if re.search(rf"\b{legacy_exception_state}\b", header_text):
                errors.append(
                    f"{path}: generated header retains legacy exception state "
                    f"{legacy_exception_state}"
                )
        for number, line in enumerate(header_text.splitlines(), 1):
            gas_declaration = GAS_DECLARATION.match(line)
            if gas_declaration and gas_declaration.group("qualifier") is None:
                errors.append(
                    f"{path}:{number}: gas declaration is not extern const: {line.strip()}"
                )

    all_headers = "\n".join(path.read_text(errors="replace") for path in headers)
    for required in ("u128", "u256", "u320", "bytes20", "bytes32"):
        if not re.search(rf"\b{required}\b", all_headers):
            errors.append(f"generated headers do not expose {required}")
    for legacy in ("sail_u128", "sail_u256", "sail_u320", "fixed_bytes_20", "fixed_bytes_32"):
        if re.search(rf"\b{legacy}\b", all_headers):
            errors.append(f"generated headers expose legacy representation name {legacy}")

    region_path = next(
        (path for path in sources if path.as_posix().endswith("/host/region_access.c")),
        None,
    )
    region_body = (
        function_body(texts[region_path].splitlines(), FUNCTION_START)
        if region_path is not None
        else None
    )
    if region_body is None:
        errors.append("generated sources do not define stateless_input_slice_load")
    else:
        for artifact in ("tmp_", "result_", "goto "):
            if artifact in region_body:
                errors.append(
                    "stateless_input_slice_load retains lowering artifact " + artifact.strip()
                )
        if region_body.count("return ") != 2:
            errors.append("stateless_input_slice_load is not a direct two-branch return")

    if errors:
        for violation in errors:
            print(violation)
        print(f"optimized C conformance: FAILED ({len(errors)} violations)")
        return 1

    print(
        "optimized C conformance: clean "
        f"({len(sources)} translation units; initialized locals, direct returns, "
        f"const gas ABI, named fixed types; {waived_count} exact compiler findings waived)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
