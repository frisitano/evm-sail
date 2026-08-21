#!/usr/bin/env python3
"""Run Clang diagnostics and order findings by optimized-C cleanup pass.

This is deliberately a diagnostic driver, not a source rewriter.  Generated C
is rebuilt from Sail after compiler changes; applying clang-tidy fixes directly
would sever that relationship.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
from pathlib import Path

from devtools.optimised_c.build import (
    compilation_flags,
    compilation_layout,
    compilation_sources,
)
from devtools.paths import REPO_ROOT

ROOT = REPO_ROOT
DEFAULT_GENERATED = ROOT / "build/c-optimised/generated"
DIAGNOSTIC = re.compile(
    r"^(?P<path>.*?):(?P<line>\d+):(?P<column>\d+): "
    r"(?P<severity>warning|error): (?P<message>.*?)(?: \[(?P<check>[^]]+)\])?$"
)
TYPEDEF_ALIAS = re.compile(r" \(aka '[^']+'\)")
GENERATED_WORKER_ID = re.compile(r"_\d+_")


@dataclass(frozen=True, order=True)
class Finding:
    path: str
    line: int
    column: int
    check: str
    message: str


PASS_ORDER = (
    "unit erasure",
    "fatal and exception lowering",
    "lifetime and initialization",
    "dead temporary and value elimination",
    "boolean simplification",
    "control-flow and scope cleanup",
    "type and const correctness",
    "undefined behavior and security",
    "portability",
    "API and maintainability",
    "general C correctness",
)


def cleanup_pass(finding: Finding) -> str:
    check = finding.check.lower()
    message = finding.message.lower()
    if "unused-parameter" in check and "unit" in message:
        return PASS_ORDER[0]
    if "noreturn" in check or "noreturn" in message or "exception" in message:
        return PASS_ORDER[1]
    if any(marker in check for marker in ("uninitialized", "lifetime", "dangling")):
        return PASS_ORDER[2]
    if any(
        marker in check
        for marker in (
            "deadstores",
            "unused-variable",
            "unused-but-set-variable",
            "unused-value",
            "unused-function",
        )
    ):
        return PASS_ORDER[3]
    if any(marker in check for marker in ("simplify-boolean", "branch-clone")):
        return PASS_ORDER[4]
    if any(
        marker in check
        for marker in (
            "unused-label",
            "else-after-return",
            "redundant-control-flow",
            "switch-default",
        )
    ):
        return PASS_ORDER[5]
    if "qualifier" in message or "incompatible-pointer" in check:
        return PASS_ORDER[6]
    if any(marker in check for marker in ("security", "cert-", "undefined", "divide-zero", "null")):
        return PASS_ORDER[7]
    if "portability" in check:
        return PASS_ORDER[8]
    if any(marker in check for marker in ("readability", "misc-", "performance-", "bugprone-")):
        return PASS_ORDER[9]
    return PASS_ORDER[10]


def normalize_message(message: str) -> str:
    """Remove host- and worker-specific spelling from generated diagnostics."""
    message = TYPEDEF_ALIAS.sub("", message)
    identifier = re.search(r"identifier '([^']+)'", message)
    if identifier is not None and (
        "reserved identifier" in message or "is reserved because" in message
    ):
        message = f"identifier '{identifier.group(1)}' is reserved"
    return GENERATED_WORKER_ID.sub("_worker_", message)


def normalize_check(check: str, message: str) -> str:
    """Canonicalize diagnostics emitted under equivalent Clang check names."""
    if check == "-Wreserved-identifier" or "reserved-identifier" in check:
        return "reserved-identifier"
    return check


def normalize_finding_key(key: str) -> str:
    fields = key.split("\0")
    if len(fields) == 5:
        fields[-2] = normalize_check(fields[-2], fields[-1])
        fields[-1] = normalize_message(fields[-1])
    return "\0".join(fields)


def finding_key(finding: Finding) -> str:
    return "\0".join(
        (
            finding.path,
            str(finding.line),
            str(finding.column),
            normalize_check(finding.check, finding.message),
            normalize_message(finding.message),
        )
    )


def read_baseline(path: Path | None) -> set[str]:
    if path is None or not path.is_file():
        return set()
    value = json.loads(path.read_text())
    if not isinstance(value, list) or not all(isinstance(item, str) for item in value):
        raise ValueError(f"lint baseline must be a JSON array of strings: {path}")
    return {normalize_finding_key(item) for item in value}


def stable_analyzer_checkers(clang: str) -> list[str]:
    """Return every non-alpha checker advertised by the installed Clang."""
    result = subprocess.run(
        [clang, "-cc1", "-analyzer-checker-help"],
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError("cannot enumerate Clang static-analyzer checkers:\n" + result.stdout)
    # Annex K's bounds-checking interfaces are optional in C11 and unavailable
    # in the guest toolchains. The corresponding analyzer checker recommends
    # fprintf_s for fatal diagnostics, which is less portable than fprintf and
    # does not improve a fixed format string.
    excluded = {"security.insecureAPI.DeprecatedOrUnsafeBufferHandling"}
    checkers = []
    for line in result.stdout.splitlines():
        match = re.match(r"^  (\S+)(?:\s|$)", line)
        if match is not None and "." in match.group(1) and match.group(1) not in excluded:
            checkers.append(match.group(1))
    if not checkers:
        raise RuntimeError("Clang reported no stable static-analyzer checkers")
    return checkers


def finding_owner(finding: Finding) -> str:
    if finding.path.startswith("build/c-optimised/generated/src/ffi/"):
        return "optimized FFI"
    if finding.path.startswith("build/c-optimised/generated/"):
        return "generated compiler output"
    if finding.path.startswith("extractions/c/optimised/contract/"):
        return "optimized FFI"
    return "included runtime/support code"


def parse_findings(output: str) -> set[Finding]:
    findings: set[Finding] = set()
    for line in output.splitlines():
        match = DIAGNOSTIC.match(line)
        if match is None:
            continue
        path = Path(match.group("path"))
        try:
            display_path = str(path.resolve().relative_to(ROOT))
        except ValueError:
            display_path = str(path)
        findings.add(
            Finding(
                display_path,
                int(match.group("line")),
                int(match.group("column")),
                match.group("check") or "clang",
                match.group("message"),
            )
        )
    return findings


def run(command: list[str]) -> set[Finding]:
    result = subprocess.run(
        command,
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    findings = parse_findings(result.stdout)
    if result.returncode != 0 and not findings:
        rendered = " ".join(command)
        raise RuntimeError(
            f"diagnostic command failed without a parseable finding ({result.returncode}):\n"
            f"{rendered}\n{result.stdout}"
        )
    return findings


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("generated", nargs="?", type=Path, default=DEFAULT_GENERATED)
    parser.add_argument("--clang", default=os.environ.get("CLANG", "clang"))
    parser.add_argument(
        "--sail",
        default=os.environ.get("SAIL", "sail"),
        help="custom Sail executable used to locate its C runtime headers",
    )
    parser.add_argument(
        "--clang-tidy",
        default=os.environ.get("CLANG_TIDY", "clang-tidy"),
        help="clang-tidy executable, or 'none' to run the built-in Clang checks only",
    )
    parser.add_argument("--require-clang-tidy", action="store_true")
    parser.add_argument("--strict", action="store_true", help="fail when diagnostics remain")
    parser.add_argument(
        "--profile",
        choices=("focused", "comprehensive"),
        default="comprehensive",
        help="diagnostic breadth (default: comprehensive)",
    )
    parser.add_argument(
        "--baseline",
        type=Path,
        help="JSON baseline; strict mode fails only on diagnostics not listed there",
    )
    parser.add_argument(
        "--write-baseline",
        type=Path,
        help="write the current stable diagnostic keys after analysis",
    )
    parser.add_argument("--jobs", type=int, default=min(8, os.cpu_count() or 1))
    parser.add_argument(
        "--max-findings-per-pass",
        type=int,
        default=50,
        help="maximum detailed findings printed per cleanup pass; 0 prints all",
    )
    args = parser.parse_args()

    generated = args.generated.resolve()
    layout = compilation_layout(generated, editable_ffi=False)
    if not layout.generated_manifest.is_file():
        print(f"optimized C lint: missing generated manifest: {layout.generated_manifest}")
        return 2

    clang = shutil.which(args.clang)
    if clang is None:
        print(f"optimized C lint: clang not found: {args.clang}")
        return 2
    tidy = None if args.clang_tidy == "none" else shutil.which(args.clang_tidy)
    if args.require_clang_tidy and tidy is None:
        print(f"optimized C lint: clang-tidy not found: {args.clang_tidy}")
        return 2

    try:
        generated_sources, ffi_sources = compilation_sources(layout)
        sources = [*generated_sources, *ffi_sources]
        flags = compilation_flags(layout, sail=shutil.which(args.sail) or args.sail)
    except ValueError as error:
        print(f"optimized C lint: {error}")
        return 2
    focused_warnings = [
        "-Werror=unused-label",
        "-Werror=unused-variable",
        "-Werror=unused-but-set-variable",
        "-Werror=unused-value",
        "-Werror=uninitialized",
        "-Werror=return-type",
        "-Werror=incompatible-pointer-types-discards-qualifiers",
    ]
    comprehensive_warnings = [
        "-Weverything",
        # The optimized backend targets C11. Keep modern declaration placement
        # and _Noreturn visible without treating their absence from C90/C99 as
        # defects. Fixed-layout ABI records deliberately trade padding for
        # stable field offsets. System include paths are selected by the
        # toolchain rather than by generated code. Exhaustive enum switches
        # deliberately omit a default so -Wswitch-enum diagnoses a newly added,
        # unhandled variant; -Wcovered-switch-default remains enabled to reject
        # redundant defaults. Everything else remains visible.
        "-Wno-declaration-after-statement",
        "-Wno-padded",
        "-Wno-poison-system-directories",
        "-Wno-pre-c11-compat",
        "-Wno-switch-default",
        # This low-level C ABI intentionally exposes checked pointer-backed
        # tables and slices. Clang's unsafe-buffer migration warning is aimed
        # at replacing C arrays with C++ view types that are unavailable here.
        "-Wno-unsafe-buffer-usage",
        *focused_warnings,
    ]
    syntax_warnings = (
        comprehensive_warnings if args.profile == "comprehensive" else focused_warnings
    )
    try:
        analyzer_checkers = (
            stable_analyzer_checkers(clang)
            if args.profile == "comprehensive"
            else ["deadcode.DeadStores"]
        )
    except RuntimeError as error:
        print(f"optimized C lint: {error}")
        return 2

    commands: list[list[str]] = []
    for source in sources:
        commands.append([clang, "-fsyntax-only", *syntax_warnings, *flags, str(source)])
        commands.append(
            [
                clang,
                "--analyze",
                "-o",
                os.devnull,
                "-Xclang",
                "-analyzer-disable-all-checks",
                "-Xclang",
                f"-analyzer-checker={','.join(analyzer_checkers)}",
                *flags,
                str(source),
            ]
        )
        if tidy is not None:
            # Both handwritten FFI modules and generated modules intentionally
            # include the generated package umbrella.  Asking include-cleaner
            # to replace that stable ABI boundary with direct, generator-owned
            # module headers makes the FFI depend on extraction partitioning.
            disabled_tidy_checks = ["-misc-include-cleaner"]
            if source.is_relative_to(generated):
                # Generated modules include their package umbrella by design;
                # recursive generated functions preserve explicit recursive
                # Sail equations. Recursion policy belongs at the semantic
                # source boundary, not in a C style checker that cannot
                # distinguish bounded structural recursion.
                disabled_tidy_checks.append("-misc-no-recursion")
            tidy_checks = [f"--checks={','.join(disabled_tidy_checks)}"]
            commands.append(
                [
                    tidy,
                    "--quiet",
                    f"--config-file={ROOT / '.clang-tidy'}",
                    *tidy_checks,
                    str(source),
                    "--",
                    *flags,
                ]
            )

    findings: set[Finding] = set()
    try:
        with ThreadPoolExecutor(max_workers=max(1, args.jobs)) as executor:
            for result in executor.map(run, commands):
                findings.update(result)
    except RuntimeError as error:
        print(f"optimized C lint: {error}")
        return 2

    try:
        baseline = read_baseline(args.baseline)
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"optimized C lint: cannot read baseline: {error}")
        return 2
    new_findings = {finding for finding in findings if finding_key(finding) not in baseline}
    if args.write_baseline is not None:
        args.write_baseline.parent.mkdir(parents=True, exist_ok=True)
        args.write_baseline.write_text(
            json.dumps(sorted(finding_key(finding) for finding in findings), indent=2) + "\n"
        )

    grouped: dict[str, list[Finding]] = {name: [] for name in PASS_ORDER}
    for finding in new_findings:
        grouped[cleanup_pass(finding)].append(finding)

    if new_findings:
        print(f"optimized C cleanup queue ({args.profile} profile, compiler pass order):")
        for pass_name in PASS_ORDER:
            pass_findings = sorted(grouped[pass_name])
            if not pass_findings:
                continue
            print(f"\n{pass_name} ({len(pass_findings)})")
            limit = args.max_findings_per_pass
            displayed = pass_findings if limit == 0 else pass_findings[:limit]
            for finding in displayed:
                print(
                    f"  {finding.path}:{finding.line}:{finding.column}: "
                    f"{finding.message} [{finding.check}]"
                )
            if limit != 0 and len(pass_findings) > limit:
                print(f"  ... {len(pass_findings) - limit} more in this pass")
    else:
        print("optimized C lint: no new actionable Clang diagnostics")

    owner_counts: dict[str, int] = {}
    for finding in findings:
        owner = finding_owner(finding)
        owner_counts[owner] = owner_counts.get(owner, 0) + 1
    owner_summary = (
        ", ".join(f"{owner}: {count}" for owner, count in sorted(owner_counts.items()))
        or "no findings"
    )
    check_counts = Counter(finding.check for finding in findings)
    check_summary = (
        ", ".join(f"{check}: {count}" for check, count in check_counts.most_common(12))
        or "no findings"
    )
    current_keys = {finding_key(finding) for finding in findings}
    tidy_status = "enabled (broad C profile)" if tidy is not None else "not installed"
    print(
        f"\noptimized C lint: {len(findings)} total, {len(new_findings)} new, "
        f"{len(baseline & current_keys)} baselined diagnostics across {len(sources)} "
        f"translation units; {len(analyzer_checkers)} static-analyzer checkers; "
        f"clang-tidy {tidy_status}"
    )
    print(f"optimized C lint ownership: {owner_summary}")
    print(f"optimized C lint leading checks: {check_summary}")
    return 1 if args.strict and new_findings else 0


if __name__ == "__main__":
    sys.exit(main())
