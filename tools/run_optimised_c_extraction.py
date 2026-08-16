#!/usr/bin/env python3
"""Run optimized-C extraction with a compiler identity bound to its output."""

from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.evaluate_optimised_c import (
    EXTRACTION_PROVENANCE_SCHEMA,
    compiler_library_identity,
    compiler_plugin_identity,
    generated_source_tree_identity,
    resolve_compiler_provenance,
    sha256,
)
from tools.optimised_c_build import ROOT

PRE_EXTRACTION_SCHEMA = "evm-sail-optimised-c-pre-extraction-provenance/v1"


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(f".{path.name}.{os.getpid()}.tmp")
    temporary.write_text(json.dumps(value, indent=2) + "\n")
    temporary.replace(path)


def ensure_compiler_unchanged(before: dict[str, Any], after: dict[str, Any]) -> None:
    if before != after:
        raise ValueError("Sail compiler identity changed during extraction")


def compiler_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--sail", required=True)
    parser.add_argument("--sail-source", type=Path)
    parser.add_argument("--sail-effective-binary", type=Path)


def resolved(args: argparse.Namespace) -> tuple[Path, Path, Path, dict[str, Any]]:
    return resolve_compiler_provenance(
        args.sail, args.sail_source, args.sail_effective_binary
    )


def run_extraction(args: argparse.Namespace) -> int:
    state = args.state.resolve()
    state.unlink(missing_ok=True)
    launcher, effective, source, before = resolved(args)
    state.parent.mkdir(parents=True, exist_ok=True)

    compiler_args = args.compiler_args
    if compiler_args[:1] == ["--"]:
        compiler_args = compiler_args[1:]
    if not compiler_args:
        raise ValueError("missing Sail extraction arguments")

    captured_at = datetime.now(timezone.utc).isoformat()
    pre_extraction = {
        "schema_version": PRE_EXTRACTION_SCHEMA,
        "status": "started",
        "captured_at": captured_at,
        "compiler": before,
        "source": str(source),
        "working_directory": str(args.cwd.resolve()),
        "requested_command": [str(launcher), *compiler_args],
        "executed_snapshot_sha256": before["effective_binary_sha256"],
    }
    write_json_atomic(state, pre_extraction)

    with tempfile.TemporaryDirectory(
        prefix="sail-compiler-snapshot-", dir=state.parent
    ) as directory:
        snapshot_root = Path(directory)
        snapshot = snapshot_root / "_build/install/default/bin/sail"
        snapshot.parent.mkdir(parents=True)
        shutil.copy2(effective, snapshot)
        snapshot.chmod(snapshot.stat().st_mode | 0o100)
        if sha256(snapshot) != before["effective_binary_sha256"]:
            raise ValueError("Sail compiler snapshot does not match effective binary")
        installed_plugins = Path(before["plugin_directory"])
        snapshot_plugins = (
            snapshot_root / "_build/install/default/share/libsail/plugins"
        )
        shutil.copytree(installed_plugins, snapshot_plugins, symlinks=False)
        snapshot_plugin_identity = compiler_plugin_identity(snapshot_plugins)
        if (
            snapshot_plugin_identity["plugin_tree_sha256"]
            != before["plugin_tree_sha256"]
            or snapshot_plugin_identity["plugin_file_count"]
            != before["plugin_file_count"]
        ):
            raise ValueError("Sail plugin snapshot does not match installed plugins")
        installed_library = Path(before["library_directory"])
        snapshot_library = snapshot_root / "lib"
        shutil.copytree(installed_library, snapshot_library, symlinks=False)
        snapshot_library_identity = compiler_library_identity(snapshot_library)
        if (
            snapshot_library_identity["library_tree_sha256"]
            != before["library_tree_sha256"]
            or snapshot_library_identity["library_file_count"]
            != before["library_file_count"]
        ):
            raise ValueError("Sail library snapshot does not match source library")
        command = [str(snapshot), *compiler_args]
        environment = os.environ.copy()
        environment["SAIL_DIR"] = str(snapshot_root)
        environment["DUNE_DIR_LOCATIONS"] = (
            f"libsail:share:{snapshot_root}/_build/install/default/share/libsail"
        )
        result = subprocess.run(
            command, cwd=args.cwd.resolve(), env=environment, check=False
        )

    _, _, _, after = resolved(args)
    ensure_compiler_unchanged(before, after)
    if result.returncode != 0:
        return result.returncode

    write_json_atomic(
        state,
        {
            **pre_extraction,
            "status": "complete",
            "completed_at": datetime.now(timezone.utc).isoformat(),
        },
    )
    return 0


def finalize_extraction(args: argparse.Namespace) -> int:
    state_path = args.state.resolve()
    if not state_path.is_file():
        raise ValueError(f"missing pre-extraction provenance: {state_path}")
    state = json.loads(state_path.read_text())
    if state.get("schema_version") != PRE_EXTRACTION_SCHEMA:
        raise ValueError("unsupported pre-extraction provenance schema")
    if state.get("status") != "complete" or not state.get("completed_at"):
        raise ValueError("extraction provenance does not record successful completion")

    _, _, source, current = resolved(args)
    ensure_compiler_unchanged(state.get("compiler", {}), current)
    if state.get("source") != str(source):
        raise ValueError("Sail source changed between extraction and finalization")
    if state.get("executed_snapshot_sha256") != current["effective_binary_sha256"]:
        raise ValueError("executed compiler snapshot does not match current binary")

    generated = args.generated.resolve()
    output = args.output.resolve()
    stamp = {
        "schema_version": EXTRACTION_PROVENANCE_SCHEMA,
        "captured_at": state["captured_at"],
        "extraction_completed_at": state["completed_at"],
        "finalized_at": datetime.now(timezone.utc).isoformat(),
        "compiler": current,
        "source": str(source),
        "working_directory": state["working_directory"],
        "requested_command": state["requested_command"],
        "executed_snapshot_sha256": state["executed_snapshot_sha256"],
        "generated_source_tree": generated_source_tree_identity(generated),
    }
    write_json_atomic(output, stamp)
    state_path.unlink()
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="action", required=True)

    run_parser = subparsers.add_parser("run")
    compiler_arguments(run_parser)
    run_parser.add_argument("--state", type=Path, required=True)
    run_parser.add_argument("--cwd", type=Path, default=ROOT)
    run_parser.add_argument("compiler_args", nargs=argparse.REMAINDER)

    finalize_parser = subparsers.add_parser("finalize")
    compiler_arguments(finalize_parser)
    finalize_parser.add_argument("--state", type=Path, required=True)
    finalize_parser.add_argument("--generated", type=Path, required=True)
    finalize_parser.add_argument("--output", type=Path, required=True)

    args = parser.parse_args()
    try:
        if args.action == "run":
            return run_extraction(args)
        return finalize_extraction(args)
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"optimized C extraction provenance: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
