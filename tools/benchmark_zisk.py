#!/usr/bin/env python3
"""Benchmark stateless-validator ZisK guests with the local CPU emulator.

Guest invocations are deliberately sequential, while each emulator process is
free to use its normal Rayon parallelism. ZisK steps are the primary
comparison; wall time includes process startup and is reported only as a
secondary host-side measurement.
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
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_TMP_ROOT = Path(
    os.environ.get("AGENT_TMPDIR", REPO_ROOT / ".agent-tmp")
)
DEFAULT_ZISKEMU = Path.home() / ".zisk" / "bin" / "ziskemu"
DEFAULT_ZISK_LOCK = REPO_ROOT / "zkvm" / "zisk" / "Cargo.lock"
STEP_PATTERNS = (
    re.compile(r"(?i)\bsteps?\b\s*[:=]\s*([0-9][0-9,_]*)"),
    re.compile(r"(?i)\b([0-9][0-9,_]*)\s+steps?\b"),
)


@dataclass(frozen=True)
class FixtureCase:
    fixture: Path
    fixture_sha256: str
    name: str
    block_index: int
    payload: bytes
    expected: bytes

    @property
    def identity(self) -> str:
        digest = hashlib.sha256(self.payload).hexdigest()[:12]
        safe_name = re.sub(r"[^A-Za-z0-9_.-]+", "-", self.name).strip("-")
        return f"{safe_name[-96:]}-block-{self.block_index}-{digest}"


@dataclass(frozen=True)
class Guest:
    name: str
    elf: Path


def decode_hex(value: str) -> bytes:
    if value.startswith(("0x", "0X")):
        value = value[2:]
    return bytes.fromhex(value)


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


def frame_input(payload: bytes) -> bytes:
    framed = len(payload).to_bytes(8, "little") + payload
    return framed + bytes((-len(framed)) % 8)


def output_matches(actual: bytes, expected: bytes) -> bool:
    return actual[: len(expected)] == expected and not any(actual[len(expected) :])


def parse_steps(output: str) -> int:
    for pattern in STEP_PATTERNS:
        match = pattern.search(output)
        if match:
            return int(match.group(1).replace(",", "").replace("_", ""))
    raise RuntimeError(f"could not parse ZisK step count:\n{output[-4000:]}")


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
        raise RuntimeError(
            f"ziskemu {actual} is incompatible with locked ziskos {required}"
        )
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
    input_path.write_bytes(frame_input(case.payload))
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
) -> dict[str, str]:
    profile_dir = run_dir / "profiles" / guest.name
    profile_dir.mkdir(parents=True, exist_ok=True)
    stem = profile_dir / case.identity
    input_path = stem.with_suffix(".input.bin")
    output_path = stem.with_suffix(".output.bin")
    report_path = stem.with_suffix(f".{mode}.txt")
    input_path.write_bytes(frame_input(case.payload))
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
    artifacts: dict[str, str] = {"report": str(report_path)}
    if mode == "sdk":
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
        profiler_path = stem.with_suffix(".profile.json.gz")
        disassembly_path = stem.with_suffix(".disassembly.txt")
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
        artifacts.update(
            profiler=str(profiler_path), disassembly=str(disassembly_path)
        )

    result = subprocess.run(
        command,
        capture_output=True,
        text=True,
        timeout=timeout,
    )
    report_path.write_text(result.stdout + result.stderr)
    actual = output_path.read_bytes() if output_path.exists() else b""
    if result.returncode != 0 or not output_matches(actual, case.expected):
        raise RuntimeError(
            f"{guest.name} {mode} profile failed {case.identity}; "
            f"see {report_path}"
        )
    return artifacts


def percentile(values: list[int], probability: float) -> int:
    ordered = sorted(values)
    index = max(0, math.ceil(probability * len(ordered)) - 1)
    return ordered[index]


def summarize_guest(cases: list[dict[str, object]]) -> dict[str, object]:
    total_steps = 0
    median_elapsed_ns = 0
    p95_elapsed_ns = 0
    for case in cases:
        measurements = case["measurements"]
        assert isinstance(measurements, list)
        steps = {int(measurement["steps"]) for measurement in measurements}
        if len(steps) != 1:
            raise RuntimeError(f"non-deterministic step counts: {sorted(steps)}")
        elapsed = [int(measurement["elapsed_ns"]) for measurement in measurements]
        total_steps += steps.pop()
        median_elapsed_ns += round(statistics.median(elapsed))
        p95_elapsed_ns += percentile(elapsed, 0.95)
    return {
        "case_count": len(cases),
        "total_steps": total_steps,
        "sum_case_median_elapsed_ns": median_elapsed_ns,
        "sum_case_p95_elapsed_ns": p95_elapsed_ns,
        "all_outputs_matched": True,
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
        lines.append(
            f"| {name} | {int(elf['size_bytes']) / (1024 * 1024):.2f} | "
            f"{steps:,} | {steps / baseline_steps:.2f}x | "
            f"{int(summary['sum_case_median_elapsed_ns']) / 1_000_000:.2f} | "
            f"{int(summary['sum_case_p95_elapsed_ns']) / 1_000_000:.2f} | exact |"
        )
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
    parser.add_argument("fixtures", nargs="+", type=Path)
    parser.add_argument(
        "--guest",
        action="append",
        type=parse_guest,
        required=True,
        help="named guest ELF as NAME=PATH (repeatable)",
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
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.warmups < 0 or args.repetitions < 1:
        raise ValueError("warmups must be non-negative and repetitions positive")
    guests: list[Guest] = args.guest
    guest_names = [guest.name for guest in guests]
    if len(guest_names) != len(set(guest_names)):
        raise ValueError("guest names must be unique")
    baseline = args.baseline or guest_names[0]
    if baseline not in guest_names:
        raise ValueError(f"unknown baseline guest: {baseline}")
    unknown_profile_guests = set(args.profile_guest) - set(guest_names)
    if unknown_profile_guests:
        raise ValueError(
            f"unknown profile guests: {', '.join(sorted(unknown_profile_guests))}"
        )

    ziskemu = args.ziskemu.expanduser().resolve()
    if not ziskemu.is_file():
        raise ValueError(f"ziskemu does not exist: {ziskemu}")
    if args.skip_version_check:
        emulator_version = ziskemu_version(ziskemu)
        locked_version = None
    else:
        emulator_version, locked_version = check_ziskemu_version(
            ziskemu, args.zisk_lock.resolve()
        )

    cases = load_cases(args.fixtures)
    if args.limit is not None:
        cases = cases[: args.limit]
    if not cases:
        raise ValueError("no embedded stateless fixture cases found")

    run_dir = args.output_dir.expanduser().resolve()
    run_dir.mkdir(parents=True, exist_ok=True)
    metadata = dict(parse_assignment(value, "metadata") for value in args.metadata)
    result: dict[str, object] = {
        "format_version": 1,
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
        "warmups": args.warmups,
        "repetitions": args.repetitions,
        "baseline": baseline,
        "fixtures": [
            {
                "path": str(case.fixture),
                "sha256": case.fixture_sha256,
                "case": case.name,
                "block_index": case.block_index,
                "input_sha256": hashlib.sha256(case.payload).hexdigest(),
                "input_bytes": len(case.payload),
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
        for warmup in range(args.warmups):
            print(f"  warmup {warmup + 1}/{args.warmups}", flush=True)
            run_emulator(ziskemu, guest, cases[0], run_dir, args.timeout)

        measured_cases: list[dict[str, object]] = []
        for case_index, case in enumerate(cases, 1):
            measurements = []
            for repetition in range(args.repetitions):
                print(
                    f"  case {case_index}/{len(cases)} repetition "
                    f"{repetition + 1}/{args.repetitions}",
                    flush=True,
                )
                measurements.append(
                    run_emulator(ziskemu, guest, case, run_dir, args.timeout)
                )
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
            "cases": measured_cases,
            "summary": summarize_guest(measured_cases),
        }
        if args.profile != "none" and (
            not args.profile_guest or guest.name in args.profile_guest
        ):
            print(f"  {args.profile} profile", flush=True)
            guest_result["profile"] = run_profile(
                ziskemu,
                guest,
                cases[0],
                run_dir,
                args.timeout,
                args.profile,
            )
        guest_results[guest.name] = guest_result

        (run_dir / "results.json").write_text(json.dumps(result, indent=2) + "\n")

    report = markdown_report(result, baseline)
    (run_dir / "report.md").write_text(report)
    print(report)
    print(f"results: {run_dir / 'results.json'}")
    print(f"report:  {run_dir / 'report.md'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
