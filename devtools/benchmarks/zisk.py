#!/usr/bin/env python3
"""Benchmark stateless-validator ZisK guests with the local CPU emulator.

Guest invocations are deliberately sequential, while each emulator process is
free to use its normal Rayon parallelism. ZisK steps provide a deterministic
execution-size comparison; SDK profiles additionally report proving cost.
Wall time includes process startup and is only a secondary host-side measure.

results.json schema notes (format_version 2):

- ``guests.{name}.build`` records what each guest ELF was built from, resolved
  at benchmark time: ``{"commit": str | null, "version": str | null,
  "source": "cli" | "sidecar" | "repository"}``. Resolution order is a
  ``--guest-build NAME=VERSION`` argument, then an optional
  ``<elf>.build.json`` sidecar next to the ELF (with ``commit`` and/or
  ``version`` keys, written when the guest is staged), then — for the
  ``evm-sail`` guest only — the repository HEAD commit (suffixed ``-dirty``
  when the worktree has local changes). ``null`` means the provenance is
  unknown; the dashboard renders it as such rather than guessing.
- ``guests.{name}.cases[].measurements[].phases`` is an OPTIONAL ordered list
  of coarse per-phase step attributions:
  ``[{"name": str, "steps": int}, ...]``. The canonical phases are
  ``input-decode``, ``witness-indexing``, ``execution``, ``state-root``, and
  ``receipts-commitments``; each is read from the aligned cross-guest profile
  tag of the same phase (``decode_input``, ``index_witness``,
  ``execute_block``, ``state_root``, ``receipts_root`` — see
  ``PHASE_SCOPES``). Those five tags are emitted as mutually non-overlapping
  scopes, so the phase steps are exclusive and sum to at most the
  measurement's total ``steps``; any remainder is unattributed guest
  overhead. Guests without phase instrumentation omit the key entirely —
  absence means "not instrumented", never zero.

``--regenerate-dashboard results.json`` re-exports the dashboard dataset from
an existing benchmark result without running any emulator.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import platform
import re
import statistics
import subprocess
import sys
import time
from collections.abc import Iterable, Sequence
from dataclasses import dataclass
from pathlib import Path
from typing import Any, cast

from devtools.paths import REPO_ROOT, temporary_root

DEFAULT_TMP_ROOT = temporary_root()
DEFAULT_ZISKEMU = Path.home() / ".zisk" / "bin" / "ziskemu"
DEFAULT_ZISK_LOCK = REPO_ROOT / "zkvm" / "zisk" / "Cargo.lock"
DEFAULT_GUEST_DIR = REPO_ROOT / "devtools" / "benchmarks" / "zisk-guests"
DEFAULT_GUESTS = (
    ("evm-sail", "stateless-validator-evm-sail-zisk.elf"),
    ("reth", "stateless-validator-reth-zisk.elf"),
    ("ethrex", "stateless-validator-ethrex-zisk.elf"),
)
STEP_PATTERNS = (
    re.compile(r"(?i)\bsteps?\b\s*[:=]\s*([0-9][0-9,_]*)"),
    re.compile(r"(?i)\b([0-9][0-9,_]*)\s+steps?\b"),
    re.compile(r"(?im)^[║│]\s*STEPS\s+([0-9][0-9,_]*)\s+[║│]\s*$"),
    re.compile(r"(?im)^STEPS\s+([0-9][0-9,_]*)\s*$"),
)
COST_PATTERNS = (
    re.compile(r"(?i)\bcost\b\s*[:=]\s*([0-9][0-9,_]*)"),
    re.compile(r"(?im)^[║│]\s*COST\s+([0-9][0-9,_]*)\s+[║│]\s*$"),
    re.compile(r"(?im)^TOTAL\s+([0-9][0-9,_]*)\s+100(?:\.0+)?%\s*$"),
)
PROFILE_TAG_ROW = re.compile(
    r"^[║│]\s{2}([a-z][a-z0-9_]*)\s+.*?"
    r"([0-9][0-9,]*)\s+([0-9]+(?:\.[0-9]+)?)%\s+[║│]\s*$"
)
FULL_PROFILE_TAG_ROW = re.compile(
    r"^\s*([0-9][0-9,]*)\s+[0-9]+(?:\.[0-9]+)?%\s+"
    r"[0-9][0-9,]*\s+[0-9][0-9,]*\s+[0-9][0-9,]*\s+"
    r"[0-9][0-9,]*\s+([a-z][a-z0-9_]*)\s*$"
)
TOP_COST_FUNCTION_ROW = re.compile(
    r"^[║│]\s+\d+\s+(.+?)\s+[█▓▒░]+\s+"
    r"([0-9][0-9,]*)\s+([0-9]+(?:\.[0-9]+)?)%\s+[║│]\s*$"
)
FULL_TOP_COST_FUNCTION_ROW = re.compile(
    r"^\s*([0-9][0-9,]*)\s+([0-9]+(?:\.[0-9]+)?)%\s+"
    r"[0-9][0-9,]*\s+(.+?)\s*$"
)
SDK_OPCODE_COST_ROW = re.compile(
    r"^[║│]\s{2}([a-z][a-z0-9_]*)\s+[█▓▒░]+\s+"
    r"([0-9][0-9,]*)\s+([0-9]+(?:\.[0-9]+)?)%\s+[║│]"
)
FULL_OPCODE_COST_ROW = re.compile(
    r"^OP ([a-z][a-z0-9_]*)\s+"
    r"([0-9][0-9,]*)\s+([0-9]+(?:\.[0-9]+)?)%\s+"
    r"([0-9][0-9,]*)\s+([0-9]+(?:\.[0-9]+)?)%"
)
FULL_FROP_COST_ROW = re.compile(
    r"^FROP ([a-z][a-z0-9_]*)\s+"
    r"([0-9][0-9,]*)\s+([0-9]+(?:\.[0-9]+)?)%\s+"
    r"([0-9][0-9,]*)\s+([0-9]+(?:\.[0-9]+)?)%"
)
DISASSEMBLY_SYMBOL_ROW = re.compile(r"^[0-9a-f]+ <(.+)>:$")
DISASSEMBLY_INSTRUCTION_ROW = re.compile(r"^\s+[0-9a-f]+:\s+([0-9][0-9,]*)\s+")
FIXTURE_SUITES = frozenset(("blockchain_tests", "blockchain_tests_engine"))
PHASE_SCOPES = (
    ("input-decode", "decode_input"),
    ("witness-indexing", "index_witness"),
    ("execution", "execute_block"),
    ("state-root", "state_root"),
    ("receipts-commitments", "receipts_root"),
)
PHASE_NAMES = tuple(phase for phase, _ in PHASE_SCOPES)
DASHBOARD_FORMAT_VERSION = 6


def case_identity(name: str, block_index: int, input_sha256: str) -> str:
    safe_name = re.sub(r"[^A-Za-z0-9_.-]+", "-", name).strip("-")
    return f"{safe_name[-96:]}-block-{block_index}-{input_sha256[:12]}"


@dataclass(frozen=True)
class FixtureCase:
    fixture: Path
    fixture_sha256: str
    name: str
    block_index: int
    payload: bytes
    expected: bytes
    gas_used: int | None = None

    @property
    def input_bytes(self) -> int:
        return len(self.payload)

    @property
    def input_sha256(self) -> str:
        return hashlib.sha256(self.payload).hexdigest()

    @property
    def identity(self) -> str:
        return case_identity(self.name, self.block_index, self.input_sha256)


@dataclass(frozen=True)
class RecordedCase:
    """A fixture case reconstructed from an existing results.json record."""

    fixture: Path
    fixture_sha256: str
    name: str
    block_index: int
    input_sha256: str
    input_bytes: int
    gas_used: int | None

    @property
    def identity(self) -> str:
        return case_identity(self.name, self.block_index, self.input_sha256)


@dataclass(frozen=True)
class Guest:
    name: str
    elf: Path


def decode_hex(value: str) -> bytes:
    if value.startswith(("0x", "0X")):
        value = value[2:]
    return bytes.fromhex(value)


def decode_quantity(value: object) -> int | None:
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        return int(value, 0)
    return None


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def fixture_files(paths: Iterable[Path]) -> list[Path]:
    files: list[Path] = []
    for path in paths:
        path = path.resolve()
        if path.is_dir():
            files.extend(sorted(path.rglob("*.json")))
        elif path.is_file():
            files.append(path)
        else:
            raise ValueError(f"fixture path does not exist: {path}")
    return files


def load_cases(paths: Iterable[Path]) -> list[FixtureCase]:
    cases: list[FixtureCase] = []
    for path in fixture_files(paths):
        raw = path.read_bytes()
        try:
            document = json.loads(raw)
        except json.JSONDecodeError:
            continue
        if not isinstance(document, dict):
            continue
        fixture_sha256 = hashlib.sha256(raw).hexdigest()
        for name, fixture in document.items():
            if not isinstance(fixture, dict):
                continue
            blocks = fixture.get("blocks", [])
            if not isinstance(blocks, list):
                continue
            for block_index, block in enumerate(blocks):
                if not isinstance(block, dict):
                    continue
                input_hex = block.get("statelessInputBytes")
                output_hex = block.get("statelessOutputBytes")
                if not input_hex or output_hex is None:
                    continue
                cases.append(
                    FixtureCase(
                        fixture=path,
                        fixture_sha256=fixture_sha256,
                        name=name,
                        block_index=block_index,
                        payload=decode_hex(input_hex),
                        expected=decode_hex(output_hex),
                        gas_used=decode_quantity((block.get("blockHeader") or {}).get("gasUsed")),
                    )
                )
    return cases


def parse_assignment(value: str, kind: str) -> tuple[str, str]:
    if "=" not in value:
        raise argparse.ArgumentTypeError(f"{kind} must be NAME=VALUE: {value}")
    name, assigned = value.split("=", 1)
    if not name or not assigned:
        raise argparse.ArgumentTypeError(f"{kind} must be NAME=VALUE: {value}")
    return name, assigned


def parse_guest(value: str) -> Guest:
    name, raw_path = parse_assignment(value, "guest")
    elf = Path(raw_path).expanduser().resolve()
    if not elf.is_file():
        raise argparse.ArgumentTypeError(f"guest ELF does not exist: {elf}")
    return Guest(name=name, elf=elf)


def default_guests() -> list[Guest]:
    guests = [
        Guest(name=name, elf=(DEFAULT_GUEST_DIR / filename).resolve())
        for name, filename in DEFAULT_GUESTS
    ]
    missing = [str(guest.elf) for guest in guests if not guest.elf.is_file()]
    if missing:
        raise ValueError(
            "standard guest ELF(s) are missing:\n  "
            + "\n  ".join(missing)
            + "\nstage them with devtools/benchmarks/stage_zisk_guests.sh or pass --guest"
        )
    return guests


def repo_head_commit() -> str | None:
    """Return the repository HEAD commit, suffixed -dirty for local changes."""
    try:
        head = subprocess.run(
            ["git", "-C", str(REPO_ROOT), "rev-parse", "HEAD"],
            capture_output=True,
            check=True,
            text=True,
            timeout=10,
        ).stdout.strip()
        status = subprocess.run(
            ["git", "-C", str(REPO_ROOT), "status", "--porcelain"],
            capture_output=True,
            check=True,
            text=True,
            timeout=10,
        ).stdout.strip()
    except (OSError, subprocess.SubprocessError):
        return None
    return f"{head}-dirty" if status else head


def guest_build_provenance(guest: Guest, override: str | None) -> dict[str, str | None] | None:
    """Resolve what a guest ELF was built from, at benchmark time.

    Resolution order: an explicit --guest-build value, an `<elf>.build.json`
    sidecar next to the ELF, then the repository HEAD commit for the evm-sail
    guest. Returns None when nothing credible is known.
    """
    if override:
        return {"commit": None, "version": override, "source": "cli"}
    sidecar = guest.elf.with_name(f"{guest.elf.name}.build.json")
    if sidecar.is_file():
        recorded = json.loads(sidecar.read_text())
        if not isinstance(recorded, dict):
            raise ValueError(f"build sidecar is not an object: {sidecar}")
        commit = recorded.get("commit")
        version = recorded.get("version")
        if commit is None and version is None:
            raise ValueError(f"build sidecar has neither commit nor version: {sidecar}")
        return {"commit": commit, "version": version, "source": "sidecar"}
    if guest.name == "evm-sail":
        commit = repo_head_commit()
        if commit:
            return {"commit": commit, "version": None, "source": "repository"}
    return None


def frame_input(payload: bytes) -> bytes:
    framed = len(payload).to_bytes(8, "little") + payload
    return framed + bytes((-len(framed)) % 8)


def prepare_guest_payload(guest: Guest, payload: bytes) -> bytes:
    """Return the canonical stateless input unchanged for every guest."""
    return payload


def frame_guest_input(guest: Guest, payload: bytes) -> bytes:
    return frame_input(prepare_guest_payload(guest, payload))


def output_matches(actual: bytes, expected: bytes) -> bool:
    return actual[: len(expected)] == expected and not any(actual[len(expected) :])


def parse_steps(output: str) -> int:
    for pattern in STEP_PATTERNS:
        match = pattern.search(output)
        if match:
            return int(match.group(1).replace(",", "").replace("_", ""))
    raise RuntimeError(f"could not parse ZisK step count:\n{output[-4000:]}")


def parse_cost(output: str) -> int:
    for pattern in COST_PATTERNS:
        match = pattern.search(output)
        if match:
            return int(match.group(1).replace(",", "").replace("_", ""))
    raise RuntimeError(f"could not parse ZisK proving cost:\n{output[-4000:]}")


def parse_profile_scope_metric(output: str, metric: str) -> dict[str, int]:
    """Parse inclusive scope totals from a ziskemu profile-tag table."""
    sdk_marker = f"{metric.upper()} PROFILE TAGS"
    full_marker = f"PROFILE TAGS {metric.upper()}"
    if sdk_marker in output:
        section = output.split(sdk_marker, 1)[1]
        row_pattern = PROFILE_TAG_ROW
        name_group = 1
        value_group = 2
    elif full_marker in output:
        section = output.split(full_marker, 1)[1]
        row_pattern = FULL_PROFILE_TAG_ROW
        name_group = 2
        value_group = 1
    else:
        raise RuntimeError(f"ZisK profile report has no {metric} profile-tag section")

    scopes: dict[str, int] = {}
    for line in section.splitlines():
        match = row_pattern.match(line)
        if match:
            scopes[match.group(name_group)] = int(match.group(value_group).replace(",", ""))
        elif scopes and (not line.strip() or line.startswith(("╚", "└"))):
            break
    if not scopes:
        raise RuntimeError(f"ZisK {metric} profile-tag section contains no scope rows")
    return scopes


def parse_profile_scope_steps(output: str) -> dict[str, int]:
    return parse_profile_scope_metric(output, "steps")


def parse_profile_scope_costs(output: str) -> dict[str, int]:
    return parse_profile_scope_metric(output, "cost")


def phase_steps(scope_steps: dict[str, int], total_steps: int) -> list[dict[str, object]]:
    """Attribute a run's steps to the aligned cross-guest phases.

    The five phase tags are emitted as mutually non-overlapping scopes, so
    their inclusive totals are also exclusive phase totals. A guest that
    emits none of them is not phase-instrumented and gets an empty list.
    """
    phases = [
        {"name": phase, "steps": scope_steps[tag]}
        for phase, tag in PHASE_SCOPES
        if tag in scope_steps
    ]
    attributed = sum(scope_steps[tag] for _, tag in PHASE_SCOPES if tag in scope_steps)
    if attributed > total_steps:
        raise RuntimeError(
            f"phase steps {attributed:,} exceed the run's {total_steps:,} "
            f"total steps; the phase tags are not mutually exclusive"
        )
    return phases


def parse_top_cost_functions(output: str) -> list[dict[str, object]]:
    """Parse the SDK or full-statistics top-cost function table."""
    marker = "TOP COST FUNCTIONS"
    if marker not in output:
        return []
    section = output.split(marker, 1)[1]
    functions: list[dict[str, object]] = []
    for line in section.splitlines():
        match = TOP_COST_FUNCTION_ROW.match(line)
        if match:
            name, cost, share = match.group(1), match.group(2), match.group(3)
        else:
            match = FULL_TOP_COST_FUNCTION_ROW.match(line)
            if match:
                cost, share, name = match.group(1), match.group(2), match.group(3)
        if match:
            functions.append(
                {
                    "name": name.rstrip(),
                    "cost": int(cost.replace(",", "")),
                    "share_percent": float(share),
                }
            )
        elif functions and (not line.strip() or line.startswith(("╚", "└"))):
            break
    return functions


def parse_opcode_costs(output: str) -> list[dict[str, object]]:
    """Parse the SDK's truncated ZisK operation cost-distribution table."""
    marker = "COST DISTRIBUTION BY OPCODE"
    if marker not in output:
        return []
    section = output.split(marker, 1)[1]
    operations: list[dict[str, object]] = []
    for line in section.splitlines():
        match = SDK_OPCODE_COST_ROW.match(line)
        if match:
            operations.append(
                {
                    "name": match.group(1),
                    "cost": int(match.group(2).replace(",", "")),
                    "share_percent": float(match.group(3)),
                }
            )
        elif operations and line.startswith(("╚", "└")):
            break
    return operations


def parse_comprehensive_opcode_costs(output: str) -> list[dict[str, object]]:
    """Parse every used ZisK operation from a full statistics report."""
    total_cost = parse_cost(output)
    operations: dict[str, dict[str, object]] = {}
    for line in output.splitlines():
        match = FULL_OPCODE_COST_ROW.match(line)
        kind = "op"
        if match is None:
            match = FULL_FROP_COST_ROW.match(line)
            kind = "frop"
        if match is None:
            continue
        name = match.group(1)
        entry = operations.setdefault(
            name,
            {
                "name": name,
                "op_count": 0,
                "frop_count": 0,
                "op_cost": 0,
                "frop_cost": 0,
            },
        )
        entry[f"{kind}_count"] = int(match.group(2).replace(",", ""))
        entry[f"{kind}_cost"] = int(match.group(4).replace(",", ""))

    result = []
    for entry in operations.values():
        cost = cast(int, entry["op_cost"]) + cast(int, entry["frop_cost"])
        result.append(
            {
                **entry,
                "cost": cost,
                "share_percent": 100.0 * cost / total_cost,
            }
        )
    return sorted(
        result,
        key=lambda entry: (-cast(int, entry["cost"]), cast(str, entry["name"])),
    )


def parse_executed_functions(disassembly: str, total_steps: int) -> list[dict[str, object]]:
    """Aggregate exclusive executed steps under every non-local ELF symbol."""
    current_symbol = "<guest startup>"
    steps_by_symbol: dict[str, int] = {}
    for line in disassembly.splitlines():
        symbol_match = DISASSEMBLY_SYMBOL_ROW.match(line)
        if symbol_match:
            symbol = symbol_match.group(1)
            if not symbol.startswith(".L"):
                current_symbol = symbol
            continue
        instruction_match = DISASSEMBLY_INSTRUCTION_ROW.match(line)
        if instruction_match:
            count = int(instruction_match.group(1).replace(",", ""))
            steps_by_symbol[current_symbol] = steps_by_symbol.get(current_symbol, 0) + count

    accounted_steps = sum(steps_by_symbol.values())
    if accounted_steps != total_steps:
        raise RuntimeError(
            "disassembly execution counts do not match the ZisK step total: "
            f"{accounted_steps} != {total_steps}"
        )
    functions = [
        {
            "name": name,
            "exclusive_steps": steps,
            "share_percent": 100.0 * steps / total_steps,
        }
        for name, steps in steps_by_symbol.items()
        if steps
    ]
    return sorted(
        functions,
        key=lambda entry: (
            -cast(int, entry["exclusive_steps"]),
            cast(str, entry["name"]),
        ),
    )


def ziskemu_version(ziskemu: Path) -> str:
    result = subprocess.run(
        [str(ziskemu), "--version"],
        capture_output=True,
        check=True,
        text=True,
        timeout=10,
    )
    return (result.stdout + result.stderr).strip()


def locked_zisk_version(lock_path: Path) -> str:
    match = re.search(
        r'\[\[package\]\]\s+name = "ziskos"\s+version = "([^"]+)"',
        lock_path.read_text(),
    )
    if not match:
        raise RuntimeError(f"cannot find ziskos in {lock_path}")
    return match.group(1)


def check_ziskemu_version(ziskemu: Path, lock_path: Path) -> tuple[str, str]:
    actual_output = ziskemu_version(ziskemu)
    actual_match = re.search(r"\bziskemu\s+([^\s]+)", actual_output)
    if not actual_match:
        raise RuntimeError(f"cannot parse ziskemu version: {actual_output}")
    required = locked_zisk_version(lock_path)
    actual = actual_match.group(1)
    if actual != required:
        raise RuntimeError(f"ziskemu {actual} is incompatible with locked ziskos {required}")
    return actual_output, required


def run_emulator(
    ziskemu: Path,
    guest: Guest,
    case: FixtureCase,
    run_dir: Path,
    timeout: float,
) -> dict[str, object]:
    case_dir = run_dir / "cases" / guest.name / case.identity
    case_dir.mkdir(parents=True, exist_ok=True)
    input_path = case_dir / "input.bin"
    output_path = case_dir / "output.bin"
    input_path.write_bytes(frame_guest_input(guest, case.payload))
    output_path.unlink(missing_ok=True)

    command = [
        str(ziskemu),
        "--elf",
        str(guest.elf),
        "--inputs",
        str(input_path),
        "--output",
        str(output_path),
        "--steps",
    ]
    started = time.perf_counter_ns()
    result = subprocess.run(
        command,
        capture_output=True,
        text=True,
        timeout=timeout,
    )
    elapsed_ns = time.perf_counter_ns() - started
    diagnostic = result.stdout + result.stderr
    actual = output_path.read_bytes() if output_path.exists() else b""
    matched = result.returncode == 0 and output_matches(actual, case.expected)
    if result.returncode != 0 or not matched:
        raise RuntimeError(
            f"{guest.name} failed {case.identity}: rc={result.returncode}, "
            f"output_matched={matched}\n{diagnostic[-4000:]}"
        )
    return {
        "steps": parse_steps(diagnostic),
        "elapsed_ns": elapsed_ns,
        "actual_output_bytes": len(actual),
        "output_matched": True,
    }


def run_profile(
    ziskemu: Path,
    guest: Guest,
    case: FixtureCase,
    run_dir: Path,
    timeout: float,
    mode: str,
    keep_report: bool = True,
    comprehensive: bool = False,
) -> dict[str, object]:
    profile_dir = run_dir / "profiles" / guest.name
    profile_dir.mkdir(parents=True, exist_ok=True)
    stem = profile_dir / case.identity
    input_path = Path(f"{stem}.input.bin")
    output_path = Path(f"{stem}.output.bin")
    report_path = Path(f"{stem}.{mode}.txt")
    input_path.write_bytes(frame_guest_input(guest, case.payload))
    output_path.unlink(missing_ok=True)

    command = [
        str(ziskemu),
        "--elf",
        str(guest.elf),
        "--inputs",
        str(input_path),
        "--output",
        str(output_path),
    ]
    artifacts: dict[str, object] = {}
    if keep_report:
        artifacts["report"] = str(report_path)
    if mode == "sdk":
        if comprehensive:
            disassembly_path = Path(f"{stem}.disassembly.txt")
            command.extend(
                [
                    "--stats",
                    "--read-symbols",
                    "--disasm",
                    str(disassembly_path),
                ]
            )
            artifacts["disassembly"] = str(disassembly_path)
        else:
            command.extend(
                [
                    "--stats",
                    "--sdk",
                    "--opcodes",
                    "--profile-tags",
                    "--top-functions",
                ]
            )
    else:
        profiler_path = Path(f"{stem}.profile.json.gz")
        disassembly_path = Path(f"{stem}.disassembly.txt")
        command.extend(
            [
                "--stats",
                "--read-symbols",
                "--top-roi",
                "50",
                "--top-histogram",
                "50",
                "--top-roi-detail",
                "--compact-names=120",
                "--profiler-output",
                str(profiler_path),
                "--disasm",
                str(disassembly_path),
            ]
        )
        artifacts.update(profiler=str(profiler_path), disassembly=str(disassembly_path))

    started = time.perf_counter_ns()
    result = subprocess.run(
        command,
        capture_output=True,
        text=True,
        timeout=timeout,
    )
    elapsed_ns = time.perf_counter_ns() - started
    diagnostic = result.stdout + result.stderr
    report_path.write_text(diagnostic)
    actual = output_path.read_bytes() if output_path.exists() else b""
    if result.returncode != 0 or not output_matches(actual, case.expected):
        raise RuntimeError(f"{guest.name} {mode} profile failed {case.identity}; see {report_path}")
    steps = parse_steps(diagnostic)
    artifacts["steps"] = steps
    artifacts["elapsed_ns"] = elapsed_ns
    artifacts["actual_output_bytes"] = len(actual)
    if mode == "sdk":
        artifacts["cost"] = parse_cost(diagnostic)
        artifacts["top_cost_functions"] = parse_top_cost_functions(diagnostic)
        artifacts["function_profile_status"] = (
            "stack_mismatch"
            if "STACK MISMATCH DETECTED" in diagnostic
            else ("ok" if artifacts["top_cost_functions"] else "no_function_rows")
        )
        has_scope_steps = "STEPS PROFILE TAGS" in diagnostic or "PROFILE TAGS STEPS" in diagnostic
        has_scope_costs = "COST PROFILE TAGS" in diagnostic or "PROFILE TAGS COST" in diagnostic
        scope_steps = parse_profile_scope_steps(diagnostic) if has_scope_steps else {}
        artifacts["scope_steps"] = scope_steps
        artifacts["scope_costs"] = parse_profile_scope_costs(diagnostic) if has_scope_costs else {}
        phases = phase_steps(scope_steps, steps)
        if phases:
            artifacts["phases"] = phases
        if comprehensive:
            artifacts["opcode_costs"] = parse_comprehensive_opcode_costs(diagnostic)
            artifacts["opcode_profile_completeness"] = "all_used_operations"
            artifacts["executed_functions"] = parse_executed_functions(
                disassembly_path.read_text(), steps
            )
            artifacts["function_inventory_status"] = "ok"
            if not keep_report:
                disassembly_path.unlink()
        else:
            artifacts["opcode_costs"] = parse_opcode_costs(diagnostic)
    if not keep_report:
        report_path.unlink()
        input_path.unlink()
        output_path.unlink()
    return artifacts


def unavailable_profile(
    error: BaseException,
    guest: Guest,
    case: FixtureCase,
    run_dir: Path,
    mode: str,
) -> dict[str, str]:
    """Describe a guest/profile failure without treating it as a measurement."""
    report_path = run_dir / "profiles" / guest.name / f"{case.identity}.{mode}.txt"
    diagnostic = report_path.read_text() if report_path.exists() else ""
    if "OOM limit of heap with bump allocator" in diagnostic:
        return {
            "code": "guest_heap_oom",
            "message": "Guest unavailable: ZisK bump-allocator heap exhausted",
            "report": str(report_path),
        }
    if isinstance(error, subprocess.TimeoutExpired):
        return {
            "code": "timeout",
            "message": "Guest unavailable: profile timed out",
            "report": str(report_path),
        }
    return {
        "code": "guest_failure",
        "message": "Guest unavailable: execution failed",
        "report": str(report_path),
    }


def fixture_taxonomy(path: Path) -> dict[str, str]:
    """Map a retained fixture path to the dashboard's cascading groups."""
    parts = path.parts
    suite_index = next(
        (index for index, part in enumerate(parts) if part in FIXTURE_SUITES),
        None,
    )
    if suite_index is None:
        return {
            "corpus": path.parent.name or "fixtures",
            "suite": "fixtures",
            "target": "unspecified",
            "fork": "unspecified",
            "feature": "other",
            "category": "",
            "fixture": path.stem,
            "path": str(path),
        }

    relative = parts[suite_index:]

    def component(index: int, fallback: str) -> str:
        return relative[index] if len(relative) > index else fallback

    return {
        "corpus": parts[suite_index - 1] if suite_index else "fixtures",
        "suite": component(0, "fixtures"),
        "target": component(1, "unspecified"),
        "fork": component(2, "unspecified"),
        "feature": component(3, "other"),
        "category": "/".join(relative[4:-1]),
        "fixture": path.stem,
        "path": "/".join(relative),
    }


def measurement_phases(
    measurements: list[dict[str, Any]], guest: Guest, identity: str
) -> list[dict[str, object]] | None:
    """Validate and return a measurement's optional per-phase step list."""
    phases = measurements[0].get("phases") if measurements else None
    if phases is None:
        return None
    if not isinstance(phases, list) or not phases:
        raise ValueError(f"invalid phase breakdown for {guest.name} {identity}")
    for phase in phases:
        if (
            not isinstance(phase, dict)
            or not isinstance(phase.get("name"), str)
            or not isinstance(phase.get("steps"), int)
        ):
            raise ValueError(f"invalid phase entry for {guest.name} {identity}: {phase}")
    return [{"name": phase["name"], "steps": phase["steps"]} for phase in phases]


def export_dashboard_data(
    dashboard_dir: Path,
    cases: Sequence[FixtureCase | RecordedCase],
    guests: list[Guest],
    guest_results: dict[str, object],
    benchmark_result: dict[str, object],
) -> None:
    """Write one shared catalog with comparative per-fixture guest shards.

    Each catalog entry carries ``guest_total_steps`` and ``guest_total_cost``:
    the per-guest sums over the fixture's embedded cases, or ``None`` when any
    case is unmeasured. The dashboard's measure toggle ranks fixtures by
    whichever of the two the reader selected, so both totals live in the
    catalog and no shard has to be fetched to build a ranking.
    """
    results_by_guest: dict[
        str, tuple[dict[str, Any], list[dict[str, Any]], list[dict[str, Any]]]
    ] = {}
    for guest in guests:
        guest_result = guest_results.get(guest.name)
        if not isinstance(guest_result, dict):
            raise ValueError(f"dashboard result is missing guest {guest.name}")
        measured = guest_result.get("cases")
        profiles = guest_result.get("profiles")
        if not isinstance(measured, list) or not isinstance(profiles, list):
            raise ValueError(f"dashboard export requires measured SDK profiles for {guest.name}")
        if len(measured) != len(cases) or len(profiles) != len(cases):
            raise ValueError(
                f"dashboard profile count does not match fixture cases for {guest.name}"
            )
        results_by_guest[guest.name] = (
            cast(dict[str, Any], guest_result),
            cast(list[dict[str, Any]], measured),
            cast(list[dict[str, Any]], profiles),
        )

    dashboard_dir.mkdir(parents=True, exist_ok=True)
    shards_dir = dashboard_dir / "fixtures"
    shards_dir.mkdir(parents=True, exist_ok=True)
    grouped: dict[Path, list[tuple[int, FixtureCase | RecordedCase]]] = {}
    for case_index, case in enumerate(cases):
        grouped.setdefault(case.fixture, []).append((case_index, case))

    catalog: list[dict[str, Any]] = []
    for fixture, fixture_cases in grouped.items():
        taxonomy = fixture_taxonomy(fixture)
        fixture_id = hashlib.sha256(
            f"{taxonomy['path']}\0{fixture_cases[0][1].fixture_sha256}".encode()
        ).hexdigest()[:20]
        shard_name = f"fixtures/{fixture_id}.json"
        shard_cases: list[dict[str, Any]] = []
        for case_index, case in fixture_cases:
            case_guests: dict[str, dict[str, Any]] = {}
            for guest in guests:
                _, measured, profiles = results_by_guest[guest.name]
                measurement = measured[case_index]
                profile = profiles[case_index]
                measurements = measurement.get("measurements")
                artifacts = profile.get("artifacts")
                if not isinstance(artifacts, dict):
                    raise ValueError(f"missing profile artifacts for {guest.name} {case.identity}")
                unavailable = artifacts.get("unavailable")
                if isinstance(unavailable, dict):
                    case_guests[guest.name] = {"unavailable": unavailable}
                    continue
                if not isinstance(measurements, list) or not measurements:
                    raise ValueError(f"missing measurements for {guest.name} {case.identity}")
                scope_steps = artifacts.get("scope_steps", {})
                scope_costs = artifacts.get("scope_costs", {})
                total_cost = artifacts.get("cost")
                opcode_costs = artifacts.get("opcode_costs", [])
                functions = artifacts.get("top_cost_functions", [])
                executed_functions = artifacts.get("executed_functions", [])
                function_profile_status = artifacts.get("function_profile_status", "unknown")
                function_inventory_status = artifacts.get("function_inventory_status", "unknown")
                opcode_profile_completeness = artifacts.get(
                    "opcode_profile_completeness", "sdk_top_operations"
                )
                if not isinstance(scope_steps, dict) or not isinstance(scope_costs, dict):
                    raise ValueError(f"invalid SDK scope totals for {guest.name} {case.identity}")
                if not isinstance(total_cost, int):
                    raise ValueError(f"missing SDK total cost for {guest.name} {case.identity}")
                if not isinstance(functions, list):
                    raise ValueError(
                        f"invalid SDK function totals for {guest.name} {case.identity}"
                    )
                if not isinstance(executed_functions, list):
                    raise ValueError(
                        f"invalid executed-function inventory for {guest.name} {case.identity}"
                    )
                if not isinstance(opcode_costs, list):
                    raise ValueError(
                        f"invalid SDK operation totals for {guest.name} {case.identity}"
                    )
                if not isinstance(function_profile_status, str):
                    raise ValueError(
                        f"invalid function profile status for {guest.name} {case.identity}"
                    )
                if not isinstance(function_inventory_status, str):
                    raise ValueError(
                        f"invalid function inventory status for {guest.name} {case.identity}"
                    )
                if not isinstance(opcode_profile_completeness, str):
                    raise ValueError(
                        f"invalid operation profile completeness for {guest.name} {case.identity}"
                    )
                case_guests[guest.name] = {
                    "total_steps": int(artifacts["steps"]),
                    "total_cost": total_cost,
                    "scope_steps": scope_steps,
                    "scope_costs": scope_costs,
                    "opcode_costs": opcode_costs,
                    "opcode_profile_completeness": (opcode_profile_completeness),
                    "top_cost_functions": functions,
                    "function_profile_status": function_profile_status,
                    "executed_functions": executed_functions,
                    "function_inventory_status": function_inventory_status,
                }
                phases = measurement_phases(measurements, guest, case.identity)
                if phases is not None:
                    case_guests[guest.name]["phases"] = phases
            shard_cases.append(
                {
                    "id": case.identity,
                    "name": case.name,
                    "block_index": case.block_index,
                    "input_bytes": case.input_bytes,
                    "gas_used": case.gas_used,
                    "guests": case_guests,
                }
            )
        shard = {
            "format_version": DASHBOARD_FORMAT_VERSION,
            "fixture": {**taxonomy, "id": fixture_id},
            "cases": shard_cases,
        }
        (dashboard_dir / shard_name).write_text(json.dumps(shard, separators=(",", ":")) + "\n")
        guest_total_steps: dict[str, int | None] = {}
        guest_total_cost: dict[str, int | None] = {}
        for guest in guests:
            for field, totals_by_guest in (
                ("total_steps", guest_total_steps),
                ("total_cost", guest_total_cost),
            ):
                totals = [
                    shard_case["guests"].get(guest.name, {}).get(field)
                    for shard_case in shard_cases
                ]
                totals_by_guest[guest.name] = (
                    sum(totals) if all(total is not None for total in totals) else None
                )
        catalog.append(
            {
                **taxonomy,
                "id": fixture_id,
                "shard": shard_name,
                "case_count": len(shard_cases),
                "max_gas_used": max(
                    (case.gas_used for _, case in fixture_cases if case.gas_used is not None),
                    default=None,
                ),
                "total_gas_used": sum(
                    case.gas_used for _, case in fixture_cases if case.gas_used is not None
                )
                if any(case.gas_used is not None for _, case in fixture_cases)
                else None,
                "guest_total_steps": guest_total_steps,
                "guest_total_cost": guest_total_cost,
            }
        )

    guest_catalog = []
    for guest in guests:
        guest_result, _, _ = results_by_guest[guest.name]
        elf = guest_result["elf"]
        assert isinstance(elf, dict)
        guest_catalog.append(
            {
                "name": guest.name,
                "elf_sha256": elf["sha256"],
                "elf_size_bytes": elf["size_bytes"],
                "build": guest_result.get("build"),
            }
        )
    catalog.sort(
        key=lambda item: (
            item["suite"],
            item["target"],
            item["fork"],
            item["feature"],
            item["category"],
            item["fixture"],
        )
    )
    payload = {
        "format_version": DASHBOARD_FORMAT_VERSION,
        "generated_at_unix_ns": str(benchmark_result["generated_at_unix_ns"]),
        "guests": guest_catalog,
        "ziskemu": benchmark_result["ziskemu"],
        "metadata": benchmark_result["metadata"],
        "fixture_count": len(catalog),
        "case_count": len(cases),
        "fixtures": catalog,
    }
    (dashboard_dir / "catalog.json").write_text(json.dumps(payload, separators=(",", ":")) + "\n")


def regenerate_dashboard(
    results_path: Path,
    dashboard_dir: Path,
    guest_builds: dict[str, str],
) -> None:
    """Re-export the dashboard dataset from an existing results.json."""
    result = json.loads(results_path.read_text())
    fixtures = result.get("fixtures")
    guest_results = result.get("guests")
    if not isinstance(fixtures, list) or not isinstance(guest_results, dict):
        raise ValueError(f"not a benchmark results file: {results_path}")
    cases = [
        RecordedCase(
            fixture=Path(record["path"]),
            fixture_sha256=record["sha256"],
            name=record["case"],
            block_index=record["block_index"],
            input_sha256=record["input_sha256"],
            input_bytes=record["input_bytes"],
            gas_used=record.get("gas_used"),
        )
        for record in fixtures
    ]
    guests = []
    for name, guest_result in guest_results.items():
        guests.append(Guest(name=name, elf=Path(guest_result["elf"]["path"])))
        if guest_result.get("build") is None:
            override = guest_builds.get(name)
            guest_result["build"] = (
                {"commit": None, "version": override, "source": "cli"} if override else None
            )
    unknown_builds = set(guest_builds) - set(guest_results)
    if unknown_builds:
        raise ValueError(f"unknown --guest-build guests: {', '.join(sorted(unknown_builds))}")
    export_dashboard_data(dashboard_dir, cases, guests, guest_results, result)


def percentile(values: list[int], probability: float) -> int:
    ordered = sorted(values)
    index = max(0, math.ceil(probability * len(ordered)) - 1)
    return ordered[index]


def summarize_guest(cases: list[dict[str, object]]) -> dict[str, object]:
    total_steps = 0
    median_elapsed_ns = 0
    p95_elapsed_ns = 0
    unavailable_count = 0
    for case in cases:
        measurements = case["measurements"]
        assert isinstance(measurements, list)
        if not measurements:
            unavailable_count += 1
            continue
        steps = {int(measurement["steps"]) for measurement in measurements}
        if len(steps) != 1:
            raise RuntimeError(f"non-deterministic step counts: {sorted(steps)}")
        elapsed = [int(measurement["elapsed_ns"]) for measurement in measurements]
        total_steps += steps.pop()
        median_elapsed_ns += round(statistics.median(elapsed))
        p95_elapsed_ns += percentile(elapsed, 0.95)
    return {
        "case_count": len(cases),
        "measured_case_count": len(cases) - unavailable_count,
        "unavailable_case_count": unavailable_count,
        "total_steps": total_steps,
        "sum_case_median_elapsed_ns": median_elapsed_ns,
        "sum_case_p95_elapsed_ns": p95_elapsed_ns,
        "all_outputs_matched": unavailable_count == 0,
    }


def markdown_report(result: dict[str, object], baseline: str) -> str:
    guests = result["guests"]
    assert isinstance(guests, dict)
    baseline_steps = int(guests[baseline]["summary"]["total_steps"])
    lines = [
        "# ZisK CPU benchmark",
        "",
        f"Cases: {result['case_count']}; repetitions: {result['repetitions']}; "
        "guest invocations are sequential.",
        "",
        "| Guest | ELF MiB | ZisK steps | vs baseline | median wall ms | p95 wall ms | Output |",
        "| --- | ---: | ---: | ---: | ---: | ---: | --- |",
    ]
    for name, guest in guests.items():
        summary = guest["summary"]
        elf = guest["elf"]
        steps = int(summary["total_steps"])
        unavailable = int(summary.get("unavailable_case_count", 0))
        comparable = unavailable == 0
        ratio = f"{steps / baseline_steps:.2f}x" if comparable else "n/a"
        output = (
            "exact"
            if comparable
            else f"{int(summary['measured_case_count'])}/{int(summary['case_count'])} exact; {unavailable} unavailable"
        )
        lines.append(
            f"| {name} | {int(elf['size_bytes']) / (1024 * 1024):.2f} | "
            f"{steps:,} | {ratio} | "
            f"{int(summary['sum_case_median_elapsed_ns']) / 1_000_000:.2f} | "
            f"{int(summary['sum_case_p95_elapsed_ns']) / 1_000_000:.2f} | {output} |"
        )
    metadata = result.get("metadata", {})
    if isinstance(metadata, dict) and metadata:
        lines.extend(["", "## Provenance", ""])
        for name, value in sorted(metadata.items()):
            rendered = f"[{value}]({value})" if name == "source_url" else value
            lines.append(f"- `{name}`: {rendered}")
    lines.extend(
        [
            "",
            "ZisK steps are deterministic for a fixed ELF and input. Wall time includes",
            "ELF loading and emulator process startup and is therefore secondary. The",
            "emulator uses its normal Rayon parallelism unless RAYON_NUM_THREADS is set",
            "by the caller.",
            "",
        ]
    )
    return "\n".join(lines)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("fixtures", nargs="*", type=Path)
    parser.add_argument(
        "--guest",
        action="append",
        type=parse_guest,
        help=(
            "named guest ELF as NAME=PATH (repeatable); defaults to the three "
            "ELFs under devtools/benchmarks/zisk-guests"
        ),
    )
    parser.add_argument(
        "--guest-build",
        action="append",
        default=[],
        help=(
            "record what a guest ELF was built from, as NAME=VERSION "
            "(repeatable); overrides the <elf>.build.json sidecar and the "
            "evm-sail repository-commit detection"
        ),
    )
    parser.add_argument(
        "--regenerate-dashboard",
        type=Path,
        metavar="RESULTS_JSON",
        help=(
            "re-export the dashboard dataset from an existing results.json "
            "into --dashboard-dir without running any emulator"
        ),
    )
    parser.add_argument("--baseline", help="guest name used for step ratios")
    parser.add_argument("--ziskemu", type=Path, default=DEFAULT_ZISKEMU)
    parser.add_argument("--zisk-lock", type=Path, default=DEFAULT_ZISK_LOCK)
    parser.add_argument("--skip-version-check", action="store_true")
    parser.add_argument("--warmups", type=int, default=1)
    parser.add_argument("--repetitions", type=int, default=5)
    parser.add_argument("--limit", type=int)
    parser.add_argument("--timeout", type=float, default=900.0)
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=DEFAULT_TMP_ROOT / "zisk-benchmark",
    )
    parser.add_argument(
        "--metadata",
        action="append",
        default=[],
        help="provenance entry as NAME=VALUE (repeatable)",
    )
    parser.add_argument("--profile", choices=("none", "sdk", "full"), default="none")
    parser.add_argument(
        "--profile-guest",
        action="append",
        default=[],
        help="profile only the named guest (repeatable; default: all guests)",
    )
    parser.add_argument(
        "--dashboard-dir",
        type=Path,
        help=(
            "export a lazy-loaded MkDocs dashboard dataset; requires "
            "--profile sdk and profiles every selected guest"
        ),
    )
    parser.add_argument(
        "--dashboard-only",
        action="store_true",
        help=(
            "use each full statistics profile invocation as the benchmark "
            "measurement and dashboard source"
        ),
    )
    parser.add_argument(
        "--continue-on-error",
        action="store_true",
        help=(
            "record failed guest profiles as unavailable and continue; "
            "intended for comparative dashboard runs"
        ),
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    guest_builds = dict(parse_assignment(value, "guest build") for value in args.guest_build)
    if args.regenerate_dashboard:
        if not args.dashboard_dir:
            raise ValueError("--regenerate-dashboard requires --dashboard-dir")
        if args.fixtures:
            raise ValueError(
                "--regenerate-dashboard reads fixtures from the recorded "
                "results; do not pass fixture paths"
            )
        dashboard_dir = args.dashboard_dir.expanduser().resolve()
        regenerate_dashboard(
            args.regenerate_dashboard.expanduser().resolve(),
            dashboard_dir,
            guest_builds,
        )
        print(f"dashboard: {dashboard_dir / 'catalog.json'}")
        return 0
    if not args.fixtures:
        raise ValueError("at least one fixture path is required")
    if args.warmups < 0 or args.repetitions < 1:
        raise ValueError("warmups must be non-negative and repetitions positive")
    guests: list[Guest] = args.guest or default_guests()
    guest_names = [guest.name for guest in guests]
    if len(guest_names) != len(set(guest_names)):
        raise ValueError("guest names must be unique")
    baseline = args.baseline or ("reth" if "reth" in guest_names else guest_names[0])
    if baseline not in guest_names:
        raise ValueError(f"unknown baseline guest: {baseline}")
    unknown_profile_guests = set(args.profile_guest) - set(guest_names)
    if unknown_profile_guests:
        raise ValueError(f"unknown profile guests: {', '.join(sorted(unknown_profile_guests))}")
    unknown_build_guests = set(guest_builds) - set(guest_names)
    if unknown_build_guests:
        raise ValueError(f"unknown --guest-build guests: {', '.join(sorted(unknown_build_guests))}")
    dashboard_guest_names = args.profile_guest or guest_names
    if args.dashboard_dir and (
        args.profile != "sdk" or set(dashboard_guest_names) != set(guest_names)
    ):
        raise ValueError(
            "--dashboard-dir requires --profile sdk and profiles for every selected guest"
        )
    if args.dashboard_only and (
        not args.dashboard_dir or set(dashboard_guest_names) != set(guest_names)
    ):
        raise ValueError(
            "--dashboard-only requires --dashboard-dir and profiles for every selected guest"
        )

    ziskemu = args.ziskemu.expanduser().resolve()
    if not ziskemu.is_file():
        raise ValueError(f"ziskemu does not exist: {ziskemu}")
    if args.skip_version_check:
        emulator_version = ziskemu_version(ziskemu)
        locked_version = None
    else:
        emulator_version, locked_version = check_ziskemu_version(ziskemu, args.zisk_lock.resolve())

    cases = load_cases(args.fixtures)
    if args.limit is not None:
        cases = cases[: args.limit]
    if not cases:
        raise ValueError("no embedded stateless fixture cases found")

    run_dir = args.output_dir.expanduser().resolve()
    run_dir.mkdir(parents=True, exist_ok=True)
    metadata = dict(parse_assignment(value, "metadata") for value in args.metadata)
    result: dict[str, object] = {
        "format_version": 2,
        "generated_at_unix_ns": time.time_ns(),
        "host": {
            "platform": platform.platform(),
            "machine": platform.machine(),
            "python": sys.version.split()[0],
            "rayon_num_threads": os.environ.get("RAYON_NUM_THREADS", "default"),
        },
        "ziskemu": {
            "path": str(ziskemu),
            "version": emulator_version,
            "locked_ziskos_version": locked_version,
        },
        "metadata": metadata,
        "case_count": len(cases),
        "warmups": 0 if args.dashboard_only else args.warmups,
        "repetitions": 1 if args.dashboard_only else args.repetitions,
        "baseline": baseline,
        "fixtures": [
            {
                "path": str(case.fixture),
                "sha256": case.fixture_sha256,
                "case": case.name,
                "block_index": case.block_index,
                "input_sha256": hashlib.sha256(case.payload).hexdigest(),
                "input_bytes": len(case.payload),
                "gas_used": case.gas_used,
                "expected_output_bytes": len(case.expected),
            }
            for case in cases
        ],
        "guests": {},
    }

    guest_results = result["guests"]
    assert isinstance(guest_results, dict)
    for guest in guests:
        print(f"benchmarking {guest.name}", flush=True)
        for warmup in range(0 if args.dashboard_only else args.warmups):
            print(f"  warmup {warmup + 1}/{args.warmups}", flush=True)
            run_emulator(ziskemu, guest, cases[0], run_dir, args.timeout)

        profiles: list[dict[str, object]] = []
        if args.profile != "none" and (not args.profile_guest or guest.name in args.profile_guest):
            for case_index, case in enumerate(cases, 1):
                print(
                    f"  {args.profile} profile {case_index}/{len(cases)}",
                    flush=True,
                )
                try:
                    artifacts = run_profile(
                        ziskemu,
                        guest,
                        case,
                        run_dir,
                        args.timeout,
                        args.profile,
                        keep_report=not args.dashboard_only,
                        comprehensive=bool(args.dashboard_dir),
                    )
                except (RuntimeError, subprocess.TimeoutExpired) as error:
                    if not args.continue_on_error:
                        raise
                    unavailable = unavailable_profile(error, guest, case, run_dir, args.profile)
                    print(f"    {unavailable['message']}", flush=True)
                    artifacts = {"unavailable": unavailable}
                profiles.append(
                    {
                        "identity": case.identity,
                        "name": case.name,
                        "block_index": case.block_index,
                        "artifacts": artifacts,
                    }
                )

        measured_cases: list[dict[str, object]] = []
        if args.dashboard_only:
            for case, profile in zip(cases, profiles):
                profile_artifacts = profile["artifacts"]
                assert isinstance(profile_artifacts, dict)
                profile_unavailable = profile_artifacts.get("unavailable")
                if isinstance(profile_unavailable, dict):
                    measured_cases.append(
                        {
                            "identity": case.identity,
                            "name": case.name,
                            "block_index": case.block_index,
                            "measurements": [],
                            "unavailable": profile_unavailable,
                        }
                    )
                    continue
                measurement = {
                    "steps": profile_artifacts["steps"],
                    "elapsed_ns": profile_artifacts["elapsed_ns"],
                    "actual_output_bytes": profile_artifacts["actual_output_bytes"],
                    "output_matched": True,
                }
                if "phases" in profile_artifacts:
                    measurement["phases"] = profile_artifacts["phases"]
                measured_cases.append(
                    {
                        "identity": case.identity,
                        "name": case.name,
                        "block_index": case.block_index,
                        "measurements": [measurement],
                    }
                )
        else:
            for case_index, case in enumerate(cases, 1):
                measurements = []
                for repetition in range(args.repetitions):
                    print(
                        f"  case {case_index}/{len(cases)} repetition "
                        f"{repetition + 1}/{args.repetitions}",
                        flush=True,
                    )
                    measurements.append(run_emulator(ziskemu, guest, case, run_dir, args.timeout))
                measured_cases.append(
                    {
                        "identity": case.identity,
                        "name": case.name,
                        "block_index": case.block_index,
                        "measurements": measurements,
                    }
                )

        guest_result: dict[str, object] = {
            "elf": {
                "path": str(guest.elf),
                "size_bytes": guest.elf.stat().st_size,
                "sha256": sha256_file(guest.elf),
            },
            "build": guest_build_provenance(guest, guest_builds.get(guest.name)),
            "cases": measured_cases,
            "summary": summarize_guest(measured_cases),
        }
        if profiles:
            guest_result["profiles"] = profiles
        guest_results[guest.name] = guest_result

        (run_dir / "results.json").write_text(json.dumps(result, indent=2) + "\n")

    if args.dashboard_dir:
        export_dashboard_data(
            args.dashboard_dir.expanduser().resolve(),
            cases,
            guests,
            guest_results,
            result,
        )

    report = markdown_report(result, baseline)
    (run_dir / "report.md").write_text(report)
    print(report)
    print(f"results: {run_dir / 'results.json'}")
    print(f"report:  {run_dir / 'report.md'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
