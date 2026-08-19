#!/usr/bin/env python3
"""Tested helpers used by the repository's Make targets.

Keep policy and filesystem manipulation here so the Makefile remains a small,
portable target graph (including on the GNU Make 3.81 shipped by macOS).
"""

from __future__ import annotations

import argparse
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
from collections.abc import Callable, Iterable, Sequence
from pathlib import Path, PurePosixPath
from typing import cast

DIAGNOSTIC_RE = re.compile(r"\b(?:warning|error)\b", re.IGNORECASE)
LEAN_PREFIX_RE = re.compile(r"(?<![A-Za-z0-9_])prefix(?![A-Za-z0-9_])")
LEAN_REDUNDANT_SIGMA_PACK_RE = re.compile(r"⟨_, ⟨_, (dependentValue[0-9]+)⟩⟩")
LEAN_REDUNDANT_SIGMA_BIND_RE = re.compile(r"let ⟨_, ⟨_, (carried_(?:code|memory|returndata))⟩⟩ :")
LEAN_RESULT_STATUS_PROJECTION_RE = re.compile(r"\(\(\(result\)\.2\)\.2\)")
LEAN_REDUNDANT_OUTPUT_BIND_RE = re.compile(
    r"let ⟨_, ⟨_, output_after⟩⟩ :"
    r"((?:(?!\n[ \t]*\(pure ).)*?)(?=\n[ \t]*\(pure \(\)\))",
    re.DOTALL,
)
LEAN_INTERPRETER_SIGMA_PACKS = 19
LEAN_INTERPRETER_SIGMA_BIND_CANDIDATES = 22
LEAN_INTERPRETER_SIGMA_BINDS = 21
LEAN_INTERPRETER_RESULT_PROJECTIONS = 1


class BuildSupportError(ValueError):
    """A deterministic build-input or artifact validation failure."""


def read_ordered_manifest(manifest: Path, root: Path) -> list[Path]:
    """Read a relative-path manifest, preserving order and validating entries."""

    root = root.resolve()
    entries: list[Path] = []
    seen: set[PurePosixPath] = set()
    for line_number, raw_line in enumerate(
        manifest.read_text(encoding="utf-8").splitlines(), start=1
    ):
        entry = raw_line.strip()
        if not entry or entry.startswith("#"):
            continue
        relative = PurePosixPath(entry)
        if relative.is_absolute() or ".." in relative.parts:
            raise BuildSupportError(
                f"{manifest}:{line_number}: path must stay below {root}: {entry!r}"
            )
        if relative in seen:
            raise BuildSupportError(
                f"{manifest}:{line_number}: duplicate manifest entry: {entry!r}"
            )
        candidate = (root / Path(*relative.parts)).resolve()
        try:
            candidate.relative_to(root)
        except ValueError as error:
            raise BuildSupportError(
                f"{manifest}:{line_number}: path escapes {root}: {entry!r}"
            ) from error
        if not candidate.is_file():
            raise BuildSupportError(
                f"{manifest}:{line_number}: manifest file does not exist: {entry!r}"
            )
        seen.add(relative)
        entries.append(candidate)
    if not entries:
        raise BuildSupportError(f"{manifest}: manifest contains no files")
    return entries


def render_make_words(paths: Iterable[Path], prefix: str = "") -> str:
    """Render paths as shell words suitable for a Make recipe."""

    words: list[str] = []
    for path in paths:
        if prefix:
            words.append(shlex.quote(prefix))
        words.append(shlex.quote(os.fspath(path)))
    return " ".join(words)


def write_canonical_manifest(manifest: Path, root: Path, output: Path) -> int:
    """Validate a source manifest and write normalized root-relative entries."""

    resolved_root = root.resolve()
    paths = read_ordered_manifest(manifest, resolved_root)
    entries = [path.relative_to(resolved_root).as_posix() for path in paths]
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = output.with_name(f".{output.name}.new")
    temporary.write_text("".join(f"{entry}\n" for entry in entries), encoding="utf-8")
    temporary.replace(output)
    return len(entries)


def run_logged_command(
    command: Sequence[str],
    log_path: Path,
    *,
    excerpt_lines: int = 20,
    fail_on_diagnostics: bool = True,
    failure_label: str = "lint",
) -> int:
    """Run a command without losing its status and retain its complete output."""

    log_path.parent.mkdir(parents=True, exist_ok=True)
    completed = subprocess.run(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )
    output = completed.stdout or ""
    log_path.write_text(output, encoding="utf-8")
    diagnostics = [line for line in output.splitlines() if DIAGNOSTIC_RE.search(line)]
    failed = completed.returncode != 0 or (fail_on_diagnostics and bool(diagnostics))
    if not failed:
        return 0

    excerpt = diagnostics if diagnostics else output.splitlines()[-excerpt_lines:]
    for line in excerpt[:excerpt_lines]:
        print(line, file=sys.stderr)
    reason = (
        f"command exited with status {completed.returncode}"
        if completed.returncode
        else "warning/error diagnostics were emitted"
    )
    print(
        f"{failure_label}: FAILED ({reason}; complete log: {log_path})",
        file=sys.stderr,
    )
    return completed.returncode or 1


def comment_box_errors(paths: Iterable[Path]) -> list[str]:
    """Report misaligned one-line C-style banner comment boxes."""

    errors: list[str] = []
    divider = re.compile(r"^/\* =+ \*/$")
    comment = re.compile(r"^/\*.*\*/$")
    for path in paths:
        block: list[tuple[int, str]] = []

        def check_block(current_block: list[tuple[int, str]], current_path: Path) -> None:
            widths = [len(line) for _, line in current_block if divider.match(line)]
            if not widths:
                return
            expected = widths[0]
            for line_number, line in current_block:
                if len(line) != expected:
                    errors.append(
                        f"{current_path}:{line_number}: comment box width {len(line)} != {expected}"
                    )

        for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
            if comment.match(line):
                block.append((line_number, line))
            else:
                check_block(block, path)
                block = []
        check_block(block, path)
    return errors


def tree_differences(expected: Path, actual: Path, *, limit: int = 40) -> list[str]:
    """Return bounded, deterministic path/content differences for two trees."""

    def files(root: Path) -> dict[str, Path]:
        if not root.is_dir():
            return {}
        return {
            path.relative_to(root).as_posix(): path
            for path in root.rglob("*")
            if path.is_file() and not path.is_symlink()
        }

    expected_files = files(expected)
    actual_files = files(actual)
    differences: list[str] = []
    for relative in sorted(expected_files.keys() - actual_files.keys()):
        differences.append(f"missing from generated tree: {relative}")
    for relative in sorted(actual_files.keys() - expected_files.keys()):
        differences.append(f"unexpected generated file: {relative}")
    for relative in sorted(expected_files.keys() & actual_files.keys()):
        left = expected_files[relative].read_bytes()
        right = actual_files[relative].read_bytes()
        if left != right:
            differences.append(f"content differs: {relative}")
    if len(differences) > limit:
        omitted = len(differences) - limit
        differences = [*differences[:limit], f"... {omitted} more difference(s)"]
    return differences


def publish_tree(staged: Path, destination: Path) -> None:
    """Replace a committed artifact tree from validated staging."""

    if not staged.is_dir():
        raise BuildSupportError(f"staged tree does not exist: {staged}")
    destination.parent.mkdir(parents=True, exist_ok=True)
    temporary = Path(
        tempfile.mkdtemp(prefix=f".{destination.name}.publish-", dir=destination.parent)
    )
    backup = destination.with_name(f".{destination.name}.previous")
    try:
        shutil.copytree(
            staged,
            temporary / destination.name,
            ignore=shutil.ignore_patterns(".lake", "__pycache__", "*.pyc"),
        )
        if backup.exists():
            shutil.rmtree(backup)
        if destination.exists():
            destination.rename(backup)
        (temporary / destination.name).rename(destination)
        if backup.exists():
            shutil.rmtree(backup)
    except Exception:
        if not destination.exists() and backup.exists():
            backup.rename(destination)
        raise
    finally:
        shutil.rmtree(temporary, ignore_errors=True)


def normalize_lean_tree(root: Path) -> int:
    """Apply deterministic workarounds for known generated Lean defects."""

    replacements = 0
    for path in sorted(root.rglob("*.lean")):
        original = path.read_text(encoding="utf-8")
        normalized, count = LEAN_PREFIX_RE.subn("evm_prefix", original)
        if count:
            path.write_text(normalized, encoding="utf-8")
            replacements += count

    interpreter = root / "Evm" / "Evm" / "Interpreter.lean"
    if interpreter.is_file():
        original = interpreter.read_text(encoding="utf-8")
        normalized, pack_count = LEAN_REDUNDANT_SIGMA_PACK_RE.subn(r"\1", original)
        bind_candidates = list(LEAN_REDUNDANT_SIGMA_BIND_RE.finditer(normalized))
        bind_parts: list[str] = []
        bind_count = 0
        skipped_bind_count = 0
        previous_end = 0
        for candidate in bind_candidates:
            bind_parts.append(normalized[previous_end : candidate.start()])
            following = normalized[candidate.end() :]
            next_binding = re.search(r"\n[ \t]*let ", following)
            binding_body = following[
                : next_binding.start() if next_binding is not None else len(following)
            ]
            # This one generated binding deliberately exposes the memory fields
            # before repacking them in the enclosing dependent result. The other
            # candidates incorrectly discard a complete Sigma value before use.
            if candidate.group(1) == "carried_memory" and "(tup__6 :" in binding_body:
                bind_parts.append(candidate.group(0))
                skipped_bind_count += 1
            else:
                bind_parts.append(f"let {candidate.group(1)} :")
                bind_count += 1
            previous_end = candidate.end()
        bind_parts.append(normalized[previous_end:])
        normalized = "".join(bind_parts)
        normalized, projection_count = LEAN_RESULT_STATUS_PROJECTION_RE.subn("result", normalized)
        if (
            pack_count != LEAN_INTERPRETER_SIGMA_PACKS
            or len(bind_candidates) != LEAN_INTERPRETER_SIGMA_BIND_CANDIDATES
            or bind_count != LEAN_INTERPRETER_SIGMA_BINDS
            or skipped_bind_count != 1
            or projection_count != LEAN_INTERPRETER_RESULT_PROJECTIONS
        ):
            raise BuildSupportError(
                f"{interpreter}: generated dependent-tuple shape changed "
                f"(packs={pack_count}, bind candidates={len(bind_candidates)}, "
                f"binds={bind_count}, skipped binds={skipped_bind_count}, "
                f"result projections={projection_count}; expected "
                f"{LEAN_INTERPRETER_SIGMA_PACKS}/"
                f"{LEAN_INTERPRETER_SIGMA_BIND_CANDIDATES}/"
                f"{LEAN_INTERPRETER_SIGMA_BINDS}/1/"
                f"{LEAN_INTERPRETER_RESULT_PROJECTIONS})"
            )
        interpreter.write_text(normalized, encoding="utf-8")
        replacements += pack_count + bind_count + projection_count

    transaction = root / "Evm" / "Evm" / "Transaction.lean"
    if transaction.is_file():
        original = transaction.read_text(encoding="utf-8")
        normalized, output_bind_count = LEAN_REDUNDANT_OUTPUT_BIND_RE.subn(
            r"let output_after :\1", original
        )
        if output_bind_count != 1:
            raise BuildSupportError(
                f"{transaction}: generated create-output shape changed "
                f"(bindings={output_bind_count}; expected 1)"
            )
        transaction.write_text(normalized, encoding="utf-8")
        replacements += output_bind_count
    if replacements == 0:
        raise BuildSupportError(f"{root}: expected at least one generated Lean rewrite")
    return replacements


def _command_manifest(args: argparse.Namespace) -> int:
    paths = read_ordered_manifest(args.manifest, args.root)
    print(render_make_words(paths, args.prefix))
    return 0


def _command_canonicalize_manifest(args: argparse.Namespace) -> int:
    count = write_canonical_manifest(args.manifest, args.root, args.output)
    print(f"canonicalized {count} manifest entr{'y' if count == 1 else 'ies'}")
    return 0


def _command_lint(args: argparse.Namespace) -> int:
    command = args.command
    if command and command[0] == "--":
        command = command[1:]
    if not command:
        raise BuildSupportError("lint-command requires a command after --")
    return run_logged_command(command, args.log, excerpt_lines=args.excerpt_lines)


def _command_run(args: argparse.Namespace) -> int:
    command = args.command
    if command and command[0] == "--":
        command = command[1:]
    if not command:
        raise BuildSupportError("run-command requires a command after --")
    return run_logged_command(
        command,
        args.log,
        excerpt_lines=args.excerpt_lines,
        fail_on_diagnostics=False,
        failure_label="command",
    )


def _command_comment_boxes(args: argparse.Namespace) -> int:
    errors = comment_box_errors(args.paths)
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    return 0


def _command_compare(args: argparse.Namespace) -> int:
    differences = tree_differences(args.committed, args.generated, limit=args.limit)
    if differences:
        print("generated artifact drift:", file=sys.stderr)
        print("\n".join(f"  {item}" for item in differences), file=sys.stderr)
        return 1
    print(f"generated artifact matches {args.committed}")
    return 0


def _command_publish(args: argparse.Namespace) -> int:
    publish_tree(args.staged, args.destination)
    print(f"published {args.staged} -> {args.destination}")
    return 0


def _command_normalize_lean(args: argparse.Namespace) -> int:
    count = normalize_lean_tree(args.root)
    print(f"normalized {count} generated Lean identifier occurrence(s)")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command_name", required=True)

    manifest = subparsers.add_parser("manifest-words")
    manifest.add_argument("--root", type=Path, required=True)
    manifest.add_argument("--prefix", default="")
    manifest.add_argument("manifest", type=Path)
    manifest.set_defaults(function=_command_manifest)

    canonicalize = subparsers.add_parser("canonicalize-manifest")
    canonicalize.add_argument("--root", type=Path, required=True)
    canonicalize.add_argument("--output", type=Path, required=True)
    canonicalize.add_argument("manifest", type=Path)
    canonicalize.set_defaults(function=_command_canonicalize_manifest)

    lint = subparsers.add_parser("lint-command")
    lint.add_argument("--log", type=Path, required=True)
    lint.add_argument("--excerpt-lines", type=int, default=20)
    lint.add_argument("command", nargs=argparse.REMAINDER)
    lint.set_defaults(function=_command_lint)

    run = subparsers.add_parser("run-command")
    run.add_argument("--log", type=Path, required=True)
    run.add_argument("--excerpt-lines", type=int, default=40)
    run.add_argument("command", nargs=argparse.REMAINDER)
    run.set_defaults(function=_command_run)

    boxes = subparsers.add_parser("comment-boxes")
    boxes.add_argument("paths", nargs="+", type=Path)
    boxes.set_defaults(function=_command_comment_boxes)

    compare = subparsers.add_parser("compare-trees")
    compare.add_argument("committed", type=Path)
    compare.add_argument("generated", type=Path)
    compare.add_argument("--limit", type=int, default=40)
    compare.set_defaults(function=_command_compare)

    publish = subparsers.add_parser("publish-tree")
    publish.add_argument("staged", type=Path)
    publish.add_argument("destination", type=Path)
    publish.set_defaults(function=_command_publish)

    normalize = subparsers.add_parser("normalize-lean")
    normalize.add_argument("root", type=Path)
    normalize.set_defaults(function=_command_normalize_lean)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        function = cast(Callable[[argparse.Namespace], int], args.function)
        return function(args)
    except (BuildSupportError, OSError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    raise SystemExit(main())
