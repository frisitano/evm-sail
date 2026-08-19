#!/usr/bin/env python3
"""Emit the versioned optimized-C extraction quality record."""

from __future__ import annotations

import argparse
import getpass
import hashlib
import json
import os
import platform
import re
import shutil
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, cast

from devtools.optimised_c.build import ROOT, compilation_layout, compilation_sources
from devtools.optimised_c.compdb import check_database, database_entries

SCHEMA_VERSION = "evm-sail-extraction-quality/v1"
EXTRACTION_PROVENANCE_SCHEMA = "evm-sail-optimised-c-extraction-provenance/v1"
DEFAULT_GENERATED = ROOT / "build/c-optimised/generated"
DEFAULT_OUTPUT = ROOT / "build/extraction-quality/record.json"
DEFAULT_SUMMARY = ROOT / "build/extraction-quality/summary.md"
BASELINE_METRICS = {
    "manifest_translation_units": 74,
    "generated_physical_lines": 11769,
    "generated_nonblank_lines": 10389,
    "distinct_temporary_identifiers": 200,
    "distinct_result_identifiers": 150,
    "distinct_intermediate_tuple_identifiers": 310,
    "goto_keyword_tokens": 3,
}

TERMINAL_ZERO_METRICS = (
    "distinct_temporary_identifiers",
    "distinct_result_identifiers",
    "distinct_intermediate_tuple_identifiers",
)
SAMPLE_STRATA = (
    ("prelude.c", "primitive lowering", "word_sub_word_u256_uint8_t_to_u256"),
    ("evm/gas.c", "arithmetic/protocol", "memory_access"),
    ("evm/transaction.c", "state transition", "process_transaction"),
    ("lib/mpt/updates.c", "recursive/data-heavy", "witness_subtree"),
    ("lib/rlp/decoding.c", "parser/failure path", "rlp_decode_word"),
    (
        "host/region_access.c",
        "generated/FFI boundary",
        "stateless_input_slice_load_Bytes_uint16_t_to_u256",
    ),
    ("kernel/environment.c", "known lowering finding", "k_blockhash"),
    ("executor/block.c", "high-level aggregation", "execute_block_body"),
)


def run(command: list[str], cwd: Path = ROOT) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        command,
        cwd=cwd,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )


def tool_version(executable: str) -> str | None:
    resolved = shutil.which(executable)
    if resolved is None:
        return None
    result = run([resolved, "--version"])
    if result.returncode != 0 or not result.stdout.strip():
        return None
    return result.stdout.splitlines()[0]


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def display_path(path: Path) -> str:
    resolved = path.resolve()
    try:
        return str(resolved.relative_to(ROOT))
    except ValueError:
        return str(resolved)


def git_repository(path: Path) -> dict[str, Any]:
    top = run(["git", "rev-parse", "--show-toplevel"], path)
    if top.returncode != 0:
        raise ValueError(f"not a Git repository: {path}")
    root = Path(top.stdout.strip()).resolve()
    commit = run(["git", "rev-parse", "HEAD"], root)
    branch = run(["git", "branch", "--show-current"], root)
    status = run(["git", "status", "--porcelain=v1", "--untracked-files=all"], root)
    if commit.returncode != 0 or branch.returncode != 0 or status.returncode != 0:
        raise ValueError(f"cannot inspect Git repository: {root}")
    lines = status.stdout.splitlines()
    return {
        "path": str(root),
        "commit": commit.stdout.strip(),
        "branch": branch.stdout.strip() or None,
        "tracked_dirty": any(not line.startswith("??") for line in lines),
        "untracked_count": sum(line.startswith("??") for line in lines),
    }


def discover_sail_source(executable: Path, explicit: Path | None) -> Path:
    selected_source = explicit
    if selected_source is None and os.environ.get("SAIL_SOURCE"):
        selected_source = Path(os.environ["SAIL_SOURCE"])
    if selected_source is not None:
        probe = run(["git", "rev-parse", "--show-toplevel"], selected_source)
        if probe.returncode != 0:
            raise ValueError(f"explicit Sail source is not a Git checkout: {selected_source}")
        return Path(probe.stdout.strip()).resolve()

    candidates: list[Path] = []
    # OPAM switches retain the exact source tree from which the installed
    # compiler was built at <switch>/.opam-switch/sources/sail.
    if executable.parent.name == "bin":
        candidates.append(executable.parent.parent / ".opam-switch/sources/sail")
    candidates.append(executable.parent)
    for candidate in candidates:
        probe = run(["git", "rev-parse", "--show-toplevel"], candidate)
        if probe.returncode == 0:
            return Path(probe.stdout.strip()).resolve()
    raise ValueError("cannot discover the Sail compiler source checkout; pass --sail-source")


def discover_effective_sail_executable(launcher: Path, source: Path, explicit: Path | None) -> Path:
    """Resolve the compiled executable behind a source-tree Sail launcher."""
    with launcher.open("rb") as stream:
        is_script = stream.read(2) == b"#!"
    if is_script:
        source_build = source / "_build/install/default/bin/sail"
        launcher_text = launcher.read_text(errors="replace")
        executable_lines = [
            line.strip()
            for line in launcher_text.splitlines()
            if line.strip() and not line.lstrip().startswith("#")
        ]
        recognized_source_launcher = (
            launcher.parent == source
            and executable_lines
            and executable_lines[-1] == 'exec "$SAIL_DIR/_build/install/default/bin/sail" "$@"'
            and 'export DUNE_DIR_LOCATIONS="libsail:share:$SAIL_DIR/_build/install/default/share/libsail"'
            in executable_lines
            and sum(line.startswith("exec ") for line in executable_lines) == 1
        )
        if not recognized_source_launcher:
            raise ValueError("cannot prove the compiled Sail executable behind the launcher")
        effective = source_build.resolve()
    else:
        effective = launcher

    if not effective.is_file() or not os.access(effective, os.X_OK):
        raise ValueError(f"Sail effective executable is not executable: {effective}")
    with effective.open("rb") as stream:
        if stream.read(2) == b"#!":
            raise ValueError(
                f"Sail effective executable is a script, not a compiled binary: {effective}"
            )
    if explicit is not None and explicit.expanduser().resolve() != effective.resolve():
        raise ValueError(
            "explicit Sail effective executable is not the executable proven "
            "to be used by the launcher"
        )
    return effective.resolve()


def compiler_identity(launcher: Path, effective: Path) -> dict[str, str]:
    effective_hash = sha256(effective)
    return {
        "executable": str(launcher),
        "launcher_sha256": sha256(launcher),
        "effective_executable": str(effective),
        "effective_binary_sha256": effective_hash,
        # Retain the v1 field with corrected semantics for record consumers.
        "binary_sha256": effective_hash,
    }


def directory_tree_identity(directory: Path) -> dict[str, Any]:
    if not directory.is_dir():
        raise ValueError(f"compiler resource directory is missing: {directory}")
    files = sorted(path for path in directory.rglob("*") if path.is_file())
    if not files:
        raise ValueError(f"compiler resource directory is empty: {directory}")
    digest = hashlib.sha256()
    for path in files:
        digest.update(path.relative_to(directory).as_posix().encode())
        digest.update(b"\0")
        digest.update(bytes.fromhex(sha256(path)))
    return {"sha256": digest.hexdigest(), "file_count": len(files)}


def compiler_plugin_identity(plugin_directory: Path) -> dict[str, Any]:
    tree = directory_tree_identity(plugin_directory)
    return {
        "plugin_directory": str(plugin_directory.resolve()),
        "plugin_tree_sha256": tree["sha256"],
        "plugin_file_count": tree["file_count"],
    }


def compiler_library_identity(library_directory: Path) -> dict[str, Any]:
    tree = directory_tree_identity(library_directory)
    return {
        "library_directory": str(library_directory.resolve()),
        "library_tree_sha256": tree["sha256"],
        "library_file_count": tree["file_count"],
    }


def version_source_commit(version: str) -> str:
    commits = re.findall(r"\b[0-9a-f]{40}\b", version)
    if len(commits) != 1:
        raise ValueError("Sail compiler version does not identify exactly one full source commit")
    return cast(str, commits[0])


def resolve_compiler_provenance(
    sail: str, explicit_source: Path | None, explicit_effective: Path | None
) -> tuple[Path, Path, Path, dict[str, Any]]:
    executable_name = shutil.which(sail)
    if executable_name is None:
        raise ValueError(f"Sail not found: {sail}")
    launcher = Path(executable_name).resolve()
    source = discover_sail_source(launcher, explicit_source)
    repository = git_repository(source)
    effective = discover_effective_sail_executable(launcher, source, explicit_effective)
    launcher_version = run([str(launcher), "--version"])
    if launcher_version.returncode != 0:
        raise ValueError("cannot read Sail compiler launcher version")
    effective_version = run([str(effective), "--version"])
    if effective_version.returncode != 0:
        raise ValueError("cannot read effective Sail compiler version")
    reported_version = launcher_version.stdout.strip()
    effective_reported_version = effective_version.stdout.strip()
    if reported_version != effective_reported_version:
        raise ValueError("Sail launcher and effective executable report different versions")
    source_commit = version_source_commit(effective_reported_version)
    if source_commit != repository["commit"]:
        raise ValueError("Sail compiler build metadata does not match the selected source commit")
    identity = {
        **compiler_identity(launcher, effective),
        **compiler_plugin_identity(source / "_build/install/default/share/libsail/plugins"),
        **compiler_library_identity(source / "lib"),
        "reported_version": reported_version,
        "effective_reported_version": effective_reported_version,
        "source_commit": source_commit,
    }
    return launcher, effective, source, identity


def generated_source_tree_identity(generated: Path) -> dict[str, Any]:
    files = sorted(
        {
            *generated.glob("Makefile"),
            *generated.glob("include/**/*.h"),
            *generated.glob("src/**/*.c"),
            *generated.glob("src/**/*.h"),
            *generated.glob("src/**/sources.list"),
        }
    )
    files = [path for path in files if path.is_file()]
    if not files:
        raise ValueError(f"optimized C extraction has no source files: {generated}")
    digest = hashlib.sha256()
    for path in files:
        relative = path.relative_to(generated).as_posix()
        digest.update(relative.encode())
        digest.update(b"\0")
        digest.update(bytes.fromhex(sha256(path)))
    return {"sha256": digest.hexdigest(), "file_count": len(files)}


def load_extraction_provenance_stamp(
    stamp_path: Path,
    generated: Path,
    compiler: dict[str, Any],
    source: Path | None = None,
) -> dict[str, Any]:
    if not stamp_path.is_file():
        raise ValueError(f"missing extraction provenance stamp: {stamp_path}")
    stamp = json.loads(stamp_path.read_text())
    if stamp.get("schema_version") != EXTRACTION_PROVENANCE_SCHEMA:
        raise ValueError("unsupported extraction provenance stamp schema")
    if stamp.get("compiler") != compiler:
        raise ValueError("extraction provenance stamp compiler identity mismatch")
    if source is not None and stamp.get("source") != str(source.resolve()):
        raise ValueError("extraction provenance stamp Sail source mismatch")
    if stamp.get("executed_snapshot_sha256") != compiler["effective_binary_sha256"]:
        raise ValueError("extraction provenance stamp compiler snapshot mismatch")
    command = stamp.get("requested_command")
    if (
        not isinstance(command, list)
        or not command
        or any(not isinstance(item, str) for item in command)
        or command[0] != compiler["executable"]
        or "--c-optimized-model" not in command
    ):
        raise ValueError("extraction provenance stamp command is invalid")
    working_directory = stamp.get("working_directory")
    if not isinstance(working_directory, str):
        raise ValueError("extraction provenance stamp working directory is invalid")
    if Path(working_directory).resolve() != ROOT:
        raise ValueError("extraction provenance stamp working directory mismatch")
    try:
        output_index = command.index("--c-output-dir") + 1
        stamped_output = Path(command[output_index])
    except (ValueError, IndexError):
        raise ValueError("extraction provenance stamp omits its output directory") from None
    if not stamped_output.is_absolute():
        stamped_output = Path(working_directory) / stamped_output
    if stamped_output.resolve() != generated.resolve():
        raise ValueError("extraction provenance stamp output directory mismatch")
    try:
        captured_at = datetime.fromisoformat(stamp["captured_at"])
        completed_at = datetime.fromisoformat(stamp["extraction_completed_at"])
        finalized_at = datetime.fromisoformat(stamp["finalized_at"])
    except (KeyError, TypeError, ValueError):
        raise ValueError("extraction provenance stamp timestamps are invalid") from None
    if any(timestamp.tzinfo is None for timestamp in (captured_at, completed_at, finalized_at)):
        raise ValueError("extraction provenance stamp timestamps lack timezones")
    if not captured_at <= completed_at <= finalized_at:
        raise ValueError("extraction provenance stamp timestamps are out of order")
    current_tree = generated_source_tree_identity(generated)
    if stamp.get("generated_source_tree") != current_tree:
        raise ValueError("extraction provenance stamp generated-source mismatch")
    return cast(dict[str, Any], stamp)


def strip_c_comments_and_literals(text: str) -> str:
    """Mask comments and literals while retaining code positions and newlines."""
    output: list[str] = []
    state = "code"
    escaped = False
    index = 0
    while index < len(text):
        char = text[index]
        following = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and following == "/":
                output.extend("  ")
                index += 2
                state = "line_comment"
                continue
            if char == "/" and following == "*":
                output.extend("  ")
                index += 2
                state = "block_comment"
                continue
            if char == '"':
                output.append(" ")
                state = "string"
                escaped = False
            elif char == "'":
                output.append(" ")
                state = "char"
                escaped = False
            else:
                output.append(char)
        elif state == "line_comment":
            output.append("\n" if char == "\n" else " ")
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == "*" and following == "/":
                output.extend("  ")
                index += 2
                state = "code"
                continue
            output.append("\n" if char == "\n" else " ")
        else:
            output.append("\n" if char == "\n" else " ")
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        index += 1
    return "".join(output)


def intermediate_tuple_identifiers(text: str) -> set[str]:
    """Find generated tuple-valued locals without counting semantic tuple ABI."""
    code = strip_c_comments_and_literals(text)
    tokens = re.findall(r"[A-Za-z_][A-Za-z0-9_]*|[{}();,=*]", code)
    identifiers: set[str] = set()
    function_depth = 0

    def starts_function_body(brace_index: int) -> bool:
        if brace_index == 0 or tokens[brace_index - 1] != ")":
            return False
        paren_depth = 1
        cursor = brace_index - 2
        while cursor >= 0:
            if tokens[cursor] == ")":
                paren_depth += 1
            elif tokens[cursor] == "(":
                paren_depth -= 1
                if paren_depth == 0:
                    break
            cursor -= 1
        return (
            cursor > 0
            and re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", tokens[cursor - 1]) is not None
            and tokens[cursor - 1] not in {"if", "for", "switch", "while"}
        )

    for index, token in enumerate(tokens):
        if token == "{":
            if function_depth > 0:
                function_depth += 1
            elif starts_function_body(index):
                function_depth = 1
            continue
        if token == "}":
            function_depth = max(0, function_depth - 1)
            continue
        if token != "struct" or function_depth == 0 or index + 3 >= len(tokens):
            continue
        tuple_type, identifier, following = tokens[index + 1 : index + 4]
        if (
            tuple_type.startswith("tuple_")
            and re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*_[0-9]+_[0-9]+", identifier)
            and following in {"=", ";", ","}
        ):
            identifiers.add(identifier)
    return identifiers


def result_identifiers(text: str) -> set[str]:
    """Find generated result names before or after semantic-name recovery."""
    code = strip_c_comments_and_literals(text)
    return set(
        re.findall(
            r"\b(?:result_[A-Za-z0-9_]+|[A-Za-z_][A-Za-z0-9_]*_result_[0-9]+_[0-9]+)\b",
            code,
        )
    )


def source_metrics(sources: list[Path]) -> dict[str, int]:
    physical = 0
    nonblank = 0
    temporaries: set[str] = set()
    results: set[str] = set()
    intermediate_tuples: set[str] = set()
    gotos = 0
    for source in sources:
        text = source.read_text(errors="replace")
        lines = text.splitlines()
        physical += len(lines)
        nonblank += sum(bool(line.strip()) for line in lines)
        code = strip_c_comments_and_literals(text)
        temporaries.update(re.findall(r"\b(?:z?tmp)_[A-Za-z0-9_]+\b", code))
        results.update(result_identifiers(text))
        intermediate_tuples.update(intermediate_tuple_identifiers(text))
        gotos += len(re.findall(r"\bgoto\b", code))
    return {
        "manifest_translation_units": len(sources),
        "generated_physical_lines": physical,
        "generated_nonblank_lines": nonblank,
        "distinct_temporary_identifiers": len(temporaries),
        "distinct_result_identifiers": len(results),
        "distinct_intermediate_tuple_identifiers": len(intermediate_tuples),
        "goto_keyword_tokens": gotos,
    }


def metric_records(values: dict[str, int]) -> list[dict[str, Any]]:
    definitions = {
        "manifest_translation_units": "normalized non-comment entries in src/spec/sources.list",
        "generated_physical_lines": "physical lines across manifest generated sources",
        "generated_nonblank_lines": "nonblank lines across manifest generated sources",
        "distinct_temporary_identifiers": r"distinct tokens matching (?:z?tmp)_[A-Za-z0-9_]+",
        "distinct_result_identifiers": (
            "distinct tokens matching result_* or a generated *_result_<phase>_<serial> suffix"
        ),
        "distinct_intermediate_tuple_identifiers": (
            "distinct function-local struct tuple_* declarations whose identifier "
            "carries Sail's generated _<phase>_<serial> suffix"
        ),
        "goto_keyword_tokens": "lexer-aware goto tokens outside comments and literals",
    }
    return [
        {
            "id": metric_id,
            "definition": definitions[metric_id],
            "scope": "generated manifest translation units",
            "unit": "count",
            "value": value,
            "baseline_value": BASELINE_METRICS[metric_id],
            "delta": value - BASELINE_METRICS[metric_id],
            "status": "descriptive",
        }
        for metric_id, value in values.items()
    ]


def artifact(path: Path, classification: str, owner: str) -> dict[str, Any]:
    lines = 0
    if classification != "static_library":
        lines = len(path.read_text(errors="replace").splitlines())
    return {
        "path": display_path(path),
        "sha256": sha256(path),
        "bytes": path.stat().st_size,
        "lines": lines,
        "classification": classification,
        "owner": owner,
    }


def representative_sample(path: Path, stratum: str, anchor_symbol: str) -> dict[str, str]:
    source = strip_c_comments_and_literals(path.read_text(errors="replace"))
    if re.search(rf"\b{re.escape(anchor_symbol)}\s*\(", source) is None:
        raise ValueError(f"representative sample anchor is absent: {path}: {anchor_symbol}")
    return {
        "path": display_path(path),
        "sha256": sha256(path),
        "stratum": stratum,
        "anchor_symbol": anchor_symbol,
    }


def gate(
    gate_id: str,
    dimension: str,
    owner: str,
    tier: str,
    command: list[str],
    status: str,
    pass_criteria: str,
    evidence: list[str],
    *,
    skip_reason: str | None = None,
) -> dict[str, Any]:
    return {
        "id": gate_id,
        "dimension": dimension,
        "owner": owner,
        "tier": tier,
        "command": command,
        "status": status,
        "pass_criteria": pass_criteria,
        "elapsed_seconds": None if status in {"skipped", "unavailable"} else 0.0,
        "evidence": evidence,
        "skip_reason": skip_reason,
    }


def terminal_zero_metric_gate(values: dict[str, int], manifest: Path) -> dict[str, Any]:
    nonzero = {
        metric_id: values[metric_id]
        for metric_id in TERMINAL_ZERO_METRICS
        if values[metric_id] != 0
    }
    return gate(
        "terminal_zero_identifier_metrics",
        "readability",
        "readability evaluator",
        "T0",
        [],
        "fail" if nonzero else "pass",
        "distinct temporary, result, and intermediate-tuple identifiers are each exactly zero",
        [display_path(manifest)],
    )


def conformance_gate(generated: Path) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    command = [
        "python3",
        "-m",
        "devtools.optimised_c.check",
        display_path(generated),
    ]
    started = datetime.now(timezone.utc)
    result = run(command)
    elapsed = (datetime.now(timezone.utc) - started).total_seconds()
    output_lines = result.stdout.splitlines()
    finding_lines = [
        line for line in output_lines if line and not line.startswith("optimized C conformance:")
    ]
    findings = []
    for index, message in enumerate(finding_lines, 1):
        location = re.match(r"(?P<path>.*?\.c)(?::(?P<line>\d+))?: (?P<message>.*)", message)
        path = display_path(generated)
        line_number = None
        detail = message
        if location:
            path = display_path(Path(location.group("path")))
            line_number = int(location.group("line")) if location.group("line") else None
            detail = location.group("message")
        fingerprint = hashlib.sha256(message.encode()).hexdigest()
        findings.append(
            {
                "id": f"generated-conformance-{index:03d}",
                "gate_id": "deterministic_generated_c_conformance",
                "tool": "devtools.optimised_c.check",
                "check": "optimized-generated-c-contract",
                "severity": "error",
                "path": path,
                "line": line_number,
                "owner": "Sail C backend",
                "message": detail,
                "fingerprint": fingerprint,
            }
        )
    record = gate(
        "deterministic_generated_c_conformance",
        "c_hygiene",
        "Sail C backend",
        "T0",
        command,
        "pass" if result.returncode == 0 else "fail",
        "the deterministic generated-C contract reports zero violations",
        [display_path(generated)],
    )
    record["elapsed_seconds"] = round(elapsed, 3)
    return record, findings


def package_build_gate(
    generated: Path, clang: str, built_library: Path | None, log_path: Path
) -> tuple[dict[str, Any], Path | None]:
    if built_library is None:
        return (
            gate(
                "optimized_package_build",
                "correctness",
                "optimized C package",
                "T1",
                [],
                "skipped",
                "optimized package archive exists after a successful build",
                [],
                skip_reason="no --built-library evidence was supplied",
            ),
            None,
        )

    command = [
        "make",
        "--no-print-directory",
        "-C",
        display_path(generated),
        f"CC={clang}",
    ]
    started = datetime.now(timezone.utc)
    result = run(command)
    elapsed = (datetime.now(timezone.utc) - started).total_seconds()
    log_path.parent.mkdir(parents=True, exist_ok=True)
    log_path.write_text(result.stdout)
    passed = result.returncode == 0 and built_library.is_file()
    record = gate(
        "optimized_package_build",
        "correctness",
        "optimized C package",
        "T1",
        command,
        "pass" if passed else "fail",
        "optimized package archive exists after a successful build",
        [display_path(log_path)] + ([display_path(built_library)] if passed else []),
    )
    record["elapsed_seconds"] = round(elapsed, 3)
    return record, log_path


def skipped_gates() -> list[dict[str, Any]]:
    return [
        gate(
            "clang_inventory",
            "c_hygiene",
            "Sail C backend",
            "T1",
            [],
            "skipped",
            "focused Clang inventory completes",
            [],
            skip_reason="not requested; compilation-database smoke is the bounded Clang check",
        ),
        gate(
            "format_inventory",
            "readability",
            "readability review",
            "T1",
            [],
            "skipped",
            "format inventory completes",
            [],
            skip_reason="not requested in this evaluator run",
        ),
        gate(
            "fixture_smoke",
            "correctness",
            "evm-sail",
            "T1",
            [],
            "skipped",
            "pinned fixture output is byte-exact",
            [],
            skip_reason="no fixture was requested or executed",
        ),
        gate(
            "human_readability_review",
            "readability",
            "readability review",
            "T1",
            [],
            "skipped",
            "no zero rubric score and total score at least 7",
            [],
            skip_reason="human review is advisory during experiments and no scores were supplied",
        ),
        gate(
            "full_fixture_corpus",
            "correctness",
            "evm-sail",
            "T2",
            [],
            "skipped",
            "complete pinned corpus is byte-exact",
            [],
            skip_reason="full-corpus execution is outside this focused worker brief",
        ),
        gate(
            "proof_extraction_diff",
            "proof_impact",
            "Sail proof extraction",
            "T2",
            [],
            "skipped",
            "proof artifacts are unchanged or reviewed",
            [],
            skip_reason="no generated semantic source was changed; proof extraction was not run",
        ),
        gate(
            "zisk_performance",
            "performance",
            "performance benchmarking",
            "T3",
            [],
            "skipped",
            "pinned repeated performance run completes",
            [],
            skip_reason="performance refresh is scheduled only after correctness gates pass",
        ),
    ]


def validate_record(record: dict[str, Any]) -> None:
    """Enforce the accepted v1 contract before publishing any evidence."""
    required = {
        "schema_version",
        "run",
        "repositories",
        "compiler",
        "extraction",
        "artifacts",
        "samples",
        "gates",
        "metrics",
        "findings",
        "human_review",
        "proof_impact",
        "fixtures",
        "performance",
    }
    missing = sorted(required - record.keys())
    if missing:
        raise ValueError(f"quality record is missing fields: {', '.join(missing)}")
    if record["schema_version"] != SCHEMA_VERSION:
        raise ValueError(f"unsupported quality-record schema: {record['schema_version']}")

    environment = record["run"]["environment"]
    environment_fields = {
        "os",
        "architecture",
        "clang_version",
        "clang_format_version",
        "clang_tidy_version",
    }
    if not environment_fields.issubset(environment):
        raise ValueError("quality record omits required environment versions")

    for repository in record["repositories"].values():
        if re.fullmatch(r"[0-9a-f]{40}", repository["commit"]) is None:
            raise ValueError("quality record repository commit is not a full Git SHA")
    compiler_fields = {
        "executable",
        "launcher_sha256",
        "effective_executable",
        "effective_binary_sha256",
        "binary_sha256",
        "reported_version",
        "effective_reported_version",
        "source_commit",
        "plugin_directory",
        "plugin_tree_sha256",
        "plugin_file_count",
        "library_directory",
        "library_tree_sha256",
        "library_file_count",
    }
    missing_compiler_fields = sorted(compiler_fields - record["compiler"].keys())
    if missing_compiler_fields:
        raise ValueError(
            "quality record omits compiler identity fields: " + ", ".join(missing_compiler_fields)
        )
    for field in (
        "launcher_sha256",
        "effective_binary_sha256",
        "binary_sha256",
        "plugin_tree_sha256",
        "library_tree_sha256",
    ):
        if re.fullmatch(r"[0-9a-f]{64}", record["compiler"][field]) is None:
            raise ValueError(f"quality record compiler {field} is not SHA-256")
    if record["compiler"]["binary_sha256"] != record["compiler"]["effective_binary_sha256"]:
        raise ValueError("quality record v1 binary hash is not the effective binary hash")
    if (
        not isinstance(record["compiler"]["plugin_file_count"], int)
        or record["compiler"]["plugin_file_count"] <= 0
    ):
        raise ValueError("quality record compiler plugin inventory is empty")
    if (
        not isinstance(record["compiler"]["library_file_count"], int)
        or record["compiler"]["library_file_count"] <= 0
    ):
        raise ValueError("quality record compiler library inventory is empty")
    if record["compiler"]["source_commit"] != record["repositories"]["sail"]["commit"]:
        raise ValueError("quality record compiler source commit does not match Sail repository")
    for field in (
        "executable",
        "effective_executable",
        "reported_version",
        "effective_reported_version",
    ):
        if not isinstance(record["compiler"][field], str) or not record["compiler"][field]:
            raise ValueError(f"quality record compiler {field} is empty")
    if re.fullmatch(r"[0-9a-f]{64}", record["extraction"]["manifest_sha256"]) is None:
        raise ValueError("quality record manifest hash is not SHA-256")
    extraction_fields = {
        "provenance_stamp_path",
        "provenance_stamp_sha256",
        "generated_source_tree",
    }
    if not extraction_fields.issubset(record["extraction"]):
        raise ValueError("quality record omits extraction provenance fields")
    if re.fullmatch(r"[0-9a-f]{64}", record["extraction"]["provenance_stamp_sha256"]) is None:
        raise ValueError("quality record provenance stamp hash is not SHA-256")
    tree = record["extraction"]["generated_source_tree"]
    if (
        not isinstance(tree, dict)
        or re.fullmatch(r"[0-9a-f]{64}", tree.get("sha256", "")) is None
        or not isinstance(tree.get("file_count"), int)
        or tree["file_count"] <= 0
    ):
        raise ValueError("quality record generated source tree identity is invalid")

    for item in record["gates"]:
        if not item["owner"]:
            raise ValueError(f"quality gate has no owner: {item['id']}")
        if not isinstance(item["evidence"], list):
            raise ValueError(f"quality gate evidence is not a list: {item['id']}")
        if item["status"] in {"skipped", "unavailable"} and (
            not item["skip_reason"] or item["elapsed_seconds"] is not None
        ):
            raise ValueError(
                f"non-executed quality gate lacks a reason or has elapsed time: {item['id']}"
            )
    for finding in record["findings"]:
        if not finding["owner"]:
            raise ValueError(f"quality finding has no owner: {finding['id']}")

    proof_impact = record["proof_impact"]
    proof_fields = {
        "shared_lowering_changed",
        "artifact_hashes_before",
        "artifact_hashes_after",
        "conclusion",
    }
    if set(proof_impact) != proof_fields:
        raise ValueError("proof-impact evidence does not match the v1 contract")
    if proof_impact["conclusion"] not in {
        "unchanged",
        "changed",
        "not_run",
        "not_applicable",
    }:
        raise ValueError("proof-impact conclusion is invalid")
    if not isinstance(record["performance"], list):
        raise ValueError("performance evidence must be a list")


def write_summary(path: Path, record: dict[str, Any]) -> None:
    gates = record["gates"]
    counts = {
        status: sum(item["status"] == status for item in gates)
        for status in ("pass", "fail", "skipped", "unavailable", "not_applicable")
    }
    metrics = {metric["id"]: metric for metric in record["metrics"]}
    lines = [
        "# Optimized C extraction quality",
        "",
        f"Schema: `{record['schema_version']}`  ",
        f"Run: `{record['run']['id']}`  ",
        f"Outcome: {counts['pass']} passed, {counts['fail']} failed, {counts['skipped']} skipped.",
        "",
        "## Provenance",
        "",
        f"- EVM Sail: `{record['repositories']['evm_sail']['commit']}`",
        f"- Sail compiler source: `{record['compiler']['source_commit']}`",
        f"- Sail launcher SHA-256: `{record['compiler']['launcher_sha256']}`",
        f"- Effective Sail binary: `{record['compiler']['effective_executable']}`",
        f"- Effective Sail binary SHA-256: `{record['compiler']['effective_binary_sha256']}`",
        f"- Effective Sail plugin tree SHA-256: `{record['compiler']['plugin_tree_sha256']}`",
        f"- Effective Sail library tree SHA-256: `{record['compiler']['library_tree_sha256']}`",
        f"- Generated manifest SHA-256: `{record['extraction']['manifest_sha256']}`",
        "",
        "## Objective metrics",
        "",
        "| Metric | Current | Accepted baseline | Delta |",
        "|---|---:|---:|---:|",
    ]
    for metric_id in BASELINE_METRICS:
        item = metrics[metric_id]
        lines.append(
            f"| `{metric_id}` | {item['value']} | {item['baseline_value']} | {item['delta']:+d} |"
        )
    lines.extend(["", "## Gates", "", "| Tier | Gate | Status |", "|---|---|---|"])
    for item in gates:
        lines.append(f"| {item['tier']} | `{item['id']}` | {item['status']} |")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("generated", nargs="?", type=Path, default=DEFAULT_GENERATED)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--summary", type=Path, default=DEFAULT_SUMMARY)
    parser.add_argument(
        "--purpose", choices=("baseline", "experiment", "release"), default="experiment"
    )
    parser.add_argument("--operator", default=getpass.getuser())
    parser.add_argument("--sail", default=os.environ.get("SAIL", "sail"))
    parser.add_argument("--sail-source", type=Path)
    parser.add_argument("--sail-effective-binary", type=Path)
    parser.add_argument("--provenance-stamp", type=Path, required=True)
    parser.add_argument("--clang", default=os.environ.get("CLANG", "clang"))
    parser.add_argument("--compdb", type=Path, default=ROOT / "compile_commands.json")
    parser.add_argument("--built-library", type=Path)
    parser.add_argument("--require-pass", action="store_true")
    args = parser.parse_args()

    clang = shutil.which(args.clang)
    if clang is None:
        print(f"optimized C evaluator: clang not found: {args.clang}")
        return 2
    generated = args.generated.resolve()
    started_at = datetime.now(timezone.utc)

    try:
        (
            sail_executable,
            effective_sail_executable,
            sail_source,
            compiler,
        ) = resolve_compiler_provenance(args.sail, args.sail_source, args.sail_effective_binary)
        provenance_stamp_path = args.provenance_stamp.resolve()
        provenance_stamp = load_extraction_provenance_stamp(
            provenance_stamp_path, generated, compiler, sail_source
        )
        layout = compilation_layout(generated, editable_ffi=True)
        generated_sources, _ffi_sources = compilation_sources(layout)
        evm_repository = git_repository(ROOT)
        sail_repository = git_repository(sail_source)
        metric_values = source_metrics(generated_sources)
        extraction_command = provenance_stamp["requested_command"]
        conformance, findings = conformance_gate(generated)

        expected_compdb, generated_count = database_entries(
            generated, clang=clang, sail=str(sail_executable)
        )
        check_started = datetime.now(timezone.utc)
        check_database(args.compdb.resolve(), expected_compdb, generated_count)
        check_elapsed = (datetime.now(timezone.utc) - check_started).total_seconds()
        built_library = args.built_library.resolve() if args.built_library is not None else None
        build_gate, build_log = package_build_gate(
            generated,
            clang,
            built_library,
            args.output.resolve().parent / "optimized-package-build.log",
        )
    except (OSError, RuntimeError, ValueError, json.JSONDecodeError) as error:
        print(f"optimized C evaluator: {error}")
        return 2

    artifacts = [
        artifact(provenance_stamp_path, "extraction_provenance", "build tooling"),
        artifact(layout.generated_manifest, "source_manifest", "Sail C backend"),
    ]
    package_manifest = layout.generated / "src/sources.list"
    if not package_manifest.is_file():
        print(f"optimized C evaluator: missing package manifest: {package_manifest}")
        return 2
    artifacts.append(artifact(package_manifest, "package_manifest", "optimized C package"))
    artifacts.extend(
        artifact(source, "generated_translation_unit", "Sail C backend")
        for source in generated_sources
    )
    artifacts.extend(
        artifact(header, "generated_header", "Sail C backend")
        for header in sorted(layout.include_root.rglob("*.h"))
    )
    artifacts.append(artifact(args.compdb.resolve(), "compilation_database", "build tooling"))
    if build_log is not None:
        artifacts.append(artifact(build_log, "build_log", "optimized C package"))
    if build_gate["status"] == "pass" and built_library is not None:
        artifacts.append(artifact(built_library, "static_library", "optimized C package"))

    samples = []
    for relative, stratum, anchor in SAMPLE_STRATA:
        path = layout.generated_source / relative
        if not path.is_file():
            print(f"optimized C evaluator: missing representative sample: {path}")
            return 2
        try:
            samples.append(representative_sample(path, stratum, anchor))
        except ValueError as error:
            print(f"optimized C evaluator: {error}")
            return 2

    manifest_gate = gate(
        "manifest_membership",
        "provenance",
        "build tooling",
        "T0",
        [],
        "pass",
        "all manifest entries resolve uniquely and source classes match exactly",
        [display_path(layout.generated_manifest), display_path(layout.ffi_manifest)],
    )
    provenance_gate = gate(
        "identity_and_provenance",
        "provenance",
        "extraction evaluator",
        "T0",
        [],
        "pass",
        "extraction-time stamp matches the current launcher, effective binary, source commit, and generated source tree",
        [
            str(ROOT),
            str(sail_source),
            str(sail_executable),
            str(effective_sail_executable),
            display_path(provenance_stamp_path),
            display_path(layout.generated_manifest),
        ],
    )
    metric_gate = gate(
        "objective_source_metrics",
        "readability",
        "readability evaluator",
        "T0",
        [],
        "pass",
        "all fixed metric definitions produce values; metrics are descriptive",
        [display_path(layout.generated_manifest)],
    )
    terminal_metric_gate = terminal_zero_metric_gate(metric_values, layout.generated_manifest)
    compdb_gate = gate(
        "compilation_database_smoke",
        "c_hygiene",
        "build tooling",
        "T1",
        [
            "python3",
            "-m",
            "devtools.optimised_c.compdb",
            "--sail",
            str(sail_executable),
            "--clang",
            clang,
            "--output",
            display_path(args.compdb),
            "--check",
            display_path(generated),
        ],
        "pass",
        "database membership equals generated plus editable FFI manifests and one generated entry passes clang -fsyntax-only",
        [display_path(args.compdb)],
    )
    compdb_gate["elapsed_seconds"] = round(check_elapsed, 3)
    now = datetime.now(timezone.utc)
    run_id = f"{now.strftime('%Y%m%dT%H%M%SZ')}-{evm_repository['commit'][:12]}"
    record = {
        "schema_version": SCHEMA_VERSION,
        "run": {
            "id": run_id,
            "started_at": started_at.isoformat(),
            "purpose": args.purpose,
            "operator": args.operator,
            "environment": {
                "os": platform.system(),
                "architecture": platform.machine(),
                "clang_version": tool_version(clang),
                "clang_format_version": tool_version("clang-format"),
                "clang_tidy_version": tool_version("clang-tidy"),
            },
        },
        "repositories": {"evm_sail": evm_repository, "sail": sail_repository},
        "compiler": compiler,
        "extraction": {
            "profile": "optimised",
            "command": extraction_command,
            "flags": extraction_command[1:],
            "output_root": display_path(generated),
            "manifest_path": display_path(layout.generated_manifest),
            "manifest_sha256": sha256(layout.generated_manifest),
            "provenance_stamp_path": display_path(provenance_stamp_path),
            "provenance_stamp_sha256": sha256(provenance_stamp_path),
            "generated_source_tree": provenance_stamp["generated_source_tree"],
        },
        "artifacts": artifacts,
        "samples": samples,
        "gates": [
            provenance_gate,
            manifest_gate,
            metric_gate,
            terminal_metric_gate,
            conformance,
            compdb_gate,
            build_gate,
            *skipped_gates(),
        ],
        "metrics": metric_records(metric_values),
        "findings": findings,
        "human_review": {
            "rubric_version": "evm-sail-generated-c-readability/v1",
            "reviewers": [],
            "scores": [],
            "disagreements": [],
        },
        "proof_impact": {
            "shared_lowering_changed": "unknown",
            "artifact_hashes_before": [],
            "artifact_hashes_after": [],
            "conclusion": "not_run",
        },
        "fixtures": [],
        "performance": [],
    }

    try:
        validate_record(record)
    except ValueError as error:
        print(f"optimized C evaluator: {error}")
        return 2
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(record, indent=2) + "\n")
    write_summary(args.summary, record)
    gates = cast(list[dict[str, Any]], record["gates"])
    failed = [item["id"] for item in gates if item["status"] == "fail"]
    print(
        f"optimized C evaluator: wrote {args.output.resolve()} and {args.summary.resolve()} "
        f"({len(generated_sources)} generated units; {len(failed)} failing gates)"
    )
    return 1 if args.require_pass and failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
