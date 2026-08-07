#!/usr/bin/env python3
"""Check the stable source-quality contract of optimized C extraction."""

from __future__ import annotations

import argparse
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_GENERATED = ROOT / "build/c-optimised/generated"

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
FUNCTION_START = re.compile(
    r"^u256 stateless_input_slice_load\(Bytes s, uint32_t off\)$"
)
FATAL_MATCH_FAILURE = re.compile(
    r"fatal_error\([^;]+\);\s*sail_match_failure\(", re.MULTILINE
)


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


def adjacent_split_initializers(path: Path, text: str) -> list[str]:
    lines = text.splitlines()
    errors: list[str] = []
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
            errors.append(
                f"{path}:{index + 1}: adjacent scalar declaration and assignment "
                f"for {match.group('name')}"
            )
    return errors


def immediate_scalar_return_temporaries(path: Path, text: str) -> list[str]:
    lines = text.splitlines()
    errors: list[str] = []
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
            errors.append(
                f"{path}:{index + 1}: scalar {match.group('name')} is initialized "
                "only to be returned immediately"
            )
    return errors


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
    args = parser.parse_args()
    generated = args.generated.resolve()

    try:
        sources = source_files(generated)
    except ValueError as error:
        print(f"optimized C conformance: {error}")
        return 1

    errors: list[str] = []
    texts: dict[Path, str] = {}
    for path in sources:
        text = path.read_text(errors="replace")
        texts[path] = text
        errors.extend(adjacent_split_initializers(path, text))
        errors.extend(immediate_scalar_return_temporaries(path, text))
        if FATAL_MATCH_FAILURE.search(text):
            errors.append(
                f"{path}: fatal_error retains an unreachable Sail exit bridge"
            )
        for legacy_exception_state in ("have_exception", "current_exception"):
            if re.search(rf"\b{legacy_exception_state}\b", text):
                errors.append(
                    f"{path}: generated C retains legacy exception state "
                    f"{legacy_exception_state}"
                )
        for number, line in enumerate(text.splitlines(), 1):
            gas_definition = GAS_DEFINITION.match(line)
            if gas_definition and gas_definition.group("qualifier") is None:
                errors.append(
                    f"{path}:{number}: literal gas storage is not const: {line.strip()}"
                )

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
        for error in errors:
            print(error)
        print(f"optimized C conformance: FAILED ({len(errors)} violations)")
        return 1

    print(
        "optimized C conformance: clean "
        f"({len(sources)} translation units; initialized locals, direct returns, "
        "const gas ABI, named fixed types)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
