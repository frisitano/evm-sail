#!/usr/bin/env python3
"""Generate a source-aligned Sail C extraction without changing its semantics.

Sail's C backend emits one implementation and one header. This tool keeps the
header intact and replaces the implementation's source-definition run with
unity includes whose paths mirror the active files in the Sail project. A
second, temporary Sail run appends preserved marker functions to copies of the
source files; the markers reveal file boundaries without modifying the
unmarked C run that is checked in.
"""

from __future__ import annotations

import argparse
import json
import re
import shlex
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Sequence


FUNCTION_DEFINITION = re.compile(
    r"(?m)^(?P<header>[A-Za-z_][^\n#;{}=]*\([^;\n{}]*\))[ \t]*(?:\n[ \t]*)?\{"
)
MARKER_PREFIX = "evmsail_c_source_marker_"
UNITY_BUILD_MACRO = "EVMSAIL_C_UNITY_BUILD"


@dataclass(frozen=True)
class CSpan:
    """One generated top-level C function definition."""

    name: str
    start: int
    end: int
    is_static: bool = False


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--sail-command",
        required=True,
        help="Sail executable or shell-style command prefix",
    )
    parser.add_argument("--project", required=True, type=Path)
    parser.add_argument("--module", required=True)
    parser.add_argument("--source-root", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--work-dir", required=True, type=Path)
    parser.add_argument(
        "--cc",
        default="cc",
        help="C compiler command recorded in the editor compilation database",
    )
    parser.add_argument(
        "--compile-flag",
        action="append",
        default=[],
        help="C flag recorded in the editor compilation database; may be repeated",
    )
    parser.add_argument(
        "--variable",
        action="append",
        default=[],
        metavar="NAME=VALUE",
        help="Sail project variable; may be repeated",
    )
    parser.add_argument(
        "c_args",
        nargs=argparse.REMAINDER,
        help="C backend arguments following --",
    )
    args = parser.parse_args()
    if args.c_args[:1] == ["--"]:
        args.c_args = args.c_args[1:]
    if not args.c_args:
        parser.error("pass the Sail C backend arguments after --")
    return args


def within(path: Path, root: Path) -> bool:
    try:
        path.relative_to(root)
    except ValueError:
        return False
    return True


def checked_path(path: Path, repo_root: Path, label: str) -> Path:
    resolved = (repo_root / path).resolve() if not path.is_absolute() else path.resolve()
    if resolved == repo_root or not within(resolved, repo_root):
        raise ValueError(f"{label} must be a non-root path inside {repo_root}: {resolved}")
    return resolved


def run(command: Sequence[str], repo_root: Path) -> None:
    try:
        subprocess.run(command, cwd=repo_root, check=True)
    except subprocess.CalledProcessError as error:
        print(f"failed command: {shlex.join(command)}", file=sys.stderr)
        raise SystemExit(error.returncode) from error


def capture(command: Sequence[str], repo_root: Path) -> str:
    try:
        result = subprocess.run(
            command,
            cwd=repo_root,
            check=True,
            stdout=subprocess.PIPE,
            text=True,
        )
    except subprocess.CalledProcessError as error:
        print(f"failed command: {shlex.join(command)}", file=sys.stderr)
        raise SystemExit(error.returncode) from error
    return result.stdout


def matching_open_paren(text: str, close: int) -> int:
    depth = 0
    for index in range(close, -1, -1):
        char = text[index]
        if char == ")":
            depth += 1
        elif char == "(":
            depth -= 1
            if depth == 0:
                return index
    raise ValueError(f"unbalanced function header: {text!r}")


def function_name(header: str) -> str:
    """Extract a generated function name, including CREATE(type)-style names."""
    header = header.rstrip()
    if not header.endswith(")"):
        raise ValueError(f"function header does not end in ')': {header!r}")
    arguments_open = matching_open_paren(header, len(header) - 1)
    prefix = header[:arguments_open].rstrip()
    if prefix.endswith(")"):
        macro_open = matching_open_paren(prefix, len(prefix) - 1)
        identifier = re.search(r"[A-Za-z_][A-Za-z0-9_]*$", prefix[:macro_open])
        if identifier is None:
            raise ValueError(f"cannot parse macro function name: {header!r}")
        return prefix[identifier.start() :]
    identifier = re.search(r"[A-Za-z_][A-Za-z0-9_]*$", prefix)
    if identifier is None:
        raise ValueError(f"cannot parse function name: {header!r}")
    return identifier.group(0)


def definition_end(text: str, opening_brace: int) -> int:
    """Find the closing brace while ignoring comments and quoted literals."""
    depth = 0
    state = "code"
    index = opening_brace
    while index < len(text):
        char = text[index]
        following = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and following == "*":
                state = "block-comment"
                index += 2
                continue
            if char == "/" and following == "/":
                state = "line-comment"
                index += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return index + 1
        elif state == "block-comment":
            if char == "*" and following == "/":
                state = "code"
                index += 2
                continue
        elif state == "line-comment":
            if char == "\n":
                state = "code"
        elif state in ("string", "character"):
            if char == "\\":
                index += 2
                continue
            if (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ):
                state = "code"
        index += 1
    raise ValueError("unterminated top-level C definition")


def definition_spans(text: str) -> list[CSpan]:
    spans = []
    cursor = 0
    while match := FUNCTION_DEFINITION.search(text, cursor):
        header = match.group("header")
        opening_brace = match.end() - 1
        end = definition_end(text, opening_brace)
        spans.append(
            CSpan(
                name=function_name(header),
                start=match.start(),
                end=end,
                is_static=header.lstrip().startswith("static "),
            )
        )
        cursor = end
    return spans


def groups_from_markers(
    spans: Sequence[CSpan], marker_names: Sequence[str], *, skip_static: bool
) -> list[list[str]]:
    groups: list[list[str]] = []
    current: list[str] = []
    marker_index = 0
    for span in spans:
        if skip_static and span.is_static:
            continue
        if (
            marker_index < len(marker_names)
            and span.name == f"z{marker_names[marker_index]}"
        ):
            groups.append(current)
            current = []
            marker_index += 1
        elif marker_index < len(marker_names):
            current.append(span.name)
    if marker_index != len(marker_names):
        expected = marker_names[marker_index]
        raise ValueError(f"generated marker function not found in order: {expected}")
    return groups


def split_unmarked(
    text: str,
    spans: Sequence[CSpan],
    groups: Sequence[Sequence[str]],
    *,
    skip_static: bool,
    artifact: str,
) -> tuple[str, list[str], str]:
    relevant = [span for span in spans if not (skip_static and span.is_static)]
    expected = [name for group in groups for name in group]
    actual = [span.name for span in relevant[: len(expected)]]
    if actual != expected:
        mismatch = next(
            (
                index
                for index, (wanted, found) in enumerate(zip(expected, actual))
                if wanted != found
            ),
            min(len(expected), len(actual)),
        )
        wanted = expected[mismatch] if mismatch < len(expected) else "<end>"
        found = actual[mismatch] if mismatch < len(actual) else "<end>"
        raise ValueError(
            f"{artifact} marker run diverged from unmarked output at definition "
            f"{mismatch}: expected {wanted}, found {found}"
        )
    if not expected:
        raise ValueError(f"no source definitions found in unmarked {artifact}")

    prefix = text[: relevant[0].start]
    payloads: list[str] = []
    cursor = 0
    for group in groups:
        count = len(group)
        if count == 0:
            payloads.append("")
            continue
        start = relevant[cursor].start
        cursor += count
        end = relevant[cursor].start if cursor < len(relevant) else len(text)
        payloads.append(text[start:end])

    suffix_start = relevant[len(expected)].start if len(expected) < len(relevant) else len(text)
    suffix = text[suffix_start:]
    if prefix + "".join(payloads) + suffix != text:
        raise AssertionError(f"{artifact} split did not reconstruct unmarked output byte-for-byte")
    return prefix, payloads, suffix


def source_fragment_path(source: Path, source_root: Path, suffix: str) -> Path:
    source_resolved = source.resolve()
    try:
        relative = source_resolved.relative_to(source_root.resolve())
    except ValueError as error:
        raise ValueError(f"active source is outside --source-root: {source}") from error
    if relative.suffix != ".sail":
        raise ValueError(f"active source is not a .sail file: {source}")
    return relative.with_suffix(suffix)


def fragment_text(
    source_label: str, umbrella: str, context_include: str, payload: str
) -> str:
    return (
        f"/* Generated from {source_label}. Included by {umbrella}; "
        "do not compile separately. */\n"
        f"#ifndef {UNITY_BUILD_MACRO}\n"
        f'#include "{context_include}"\n'
        "#endif\n"
        f"{payload}"
    )


def context_text(prefix: str) -> str:
    return (
        "/* Generated unity-build context. Public consumers should include evm.h. */\n"
        "#pragma once\n"
        + prefix
    )


def umbrella_text(fragment_paths: Sequence[Path], suffix: str) -> str:
    includes = "".join(f'#include "{path.as_posix()}"\n' for path in fragment_paths)
    if suffix and not suffix.startswith("\n"):
        suffix = "\n" + suffix
    return (
        f"#ifndef {UNITY_BUILD_MACRO}\n"
        f"#define {UNITY_BUILD_MACRO} 1\n"
        "#endif\n"
        '#include "evm_internal.h"\n'
        + includes
        + suffix
    )


def umbrella_include_path(fragment: Path, umbrella: str) -> str:
    return ("../" * len(fragment.parent.parts)) + umbrella


def write_compile_commands(
    destination: Path,
    repo_root: Path,
    compiler: str,
    flags: Sequence[str],
    sources: Sequence[Path],
) -> None:
    compiler_command = shlex.split(compiler)
    if not compiler_command:
        raise ValueError("--cc is empty")
    entries = []
    for source in sources:
        resolved = source.resolve()
        entries.append(
            {
                "directory": str(repo_root),
                "file": str(resolved),
                "arguments": compiler_command
                + list(flags)
                + ["-fsyntax-only", str(resolved)],
            }
        )
    destination.write_text(json.dumps(entries, indent=2) + "\n", encoding="utf-8")


def synchronize(staging: Path, output: Path) -> None:
    expected = {
        path.relative_to(staging)
        for path in staging.rglob("*")
        if path.is_file()
    }
    output.mkdir(parents=True, exist_ok=True)
    for path in sorted(output.rglob("*"), key=lambda item: len(item.parts), reverse=True):
        if path.is_file() and path.relative_to(output) not in expected:
            path.unlink()
        elif path.is_dir() and not any(path.iterdir()):
            path.rmdir()
    for relative in sorted(expected):
        source = staging / relative
        destination = output / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source, destination)


def main() -> int:
    args = parse_args()
    repo_root = Path.cwd().resolve()
    project = checked_path(args.project, repo_root, "--project")
    source_root = checked_path(args.source_root, repo_root, "--source-root")
    output_dir = checked_path(args.output_dir, repo_root, "--output-dir")
    work_dir = checked_path(args.work_dir, repo_root, "--work-dir")
    project_argument = project.relative_to(repo_root).as_posix()
    sail_command = shlex.split(args.sail_command)
    if not sail_command:
        raise ValueError("--sail-command is empty")

    variable_args = [item for variable in args.variable for item in ("--variable", variable)]
    listed = capture(
        sail_command
        + ["--list-files", project_argument, args.module]
        + variable_args,
        repo_root,
    )
    source_tokens = shlex.split(listed)
    if not source_tokens:
        raise ValueError("Sail project has no active source files")
    sources = [
        (repo_root / token).resolve() if not Path(token).is_absolute() else Path(token).resolve()
        for token in source_tokens
    ]
    if len(sources) != len(set(sources)):
        raise ValueError("Sail project lists an active source file more than once")

    if work_dir.exists():
        shutil.rmtree(work_dir)
    raw_dir = work_dir / "raw"
    marked_root = work_dir / "marked"
    staging = work_dir / "staging"
    raw_dir.mkdir(parents=True)
    marked_root.mkdir(parents=True)
    staging.mkdir(parents=True)

    unmarked_prefix = raw_dir / "evm"
    run(
        sail_command
        + args.c_args
        + variable_args
        + ["-o", str(unmarked_prefix), project_argument, args.module],
        repo_root,
    )

    marker_names = [f"{MARKER_PREFIX}{index:03d}" for index in range(len(sources))]
    marked_sources = []
    for source, marker_name in zip(sources, marker_names):
        relative = source_fragment_path(source, source_root, ".sail")
        destination = marked_root / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        source_text = source.read_text(encoding="utf-8")
        if MARKER_PREFIX in source_text:
            raise ValueError(f"reserved extraction marker prefix occurs in {source}")
        destination.write_text(
            source_text.rstrip("\n")
            + "\n\n"
            + f"val {marker_name} : unit -> unit\n"
            + f"function {marker_name}() = ()\n",
            encoding="utf-8",
        )
        marked_sources.append(destination)

    marked_prefix = raw_dir / "marked-evm"
    marker_preserves = [item for name in marker_names for item in ("--c-preserve", name)]
    run(
        sail_command
        + args.c_args
        + marker_preserves
        + ["-o", str(marked_prefix)]
        + [str(path) for path in marked_sources],
        repo_root,
    )

    unmarked_c = unmarked_prefix.with_suffix(".c").read_text(encoding="utf-8")
    unmarked_h = unmarked_prefix.with_suffix(".h").read_text(encoding="utf-8")
    marked_c = marked_prefix.with_suffix(".c").read_text(encoding="utf-8")

    c_groups = groups_from_markers(
        definition_spans(marked_c), marker_names, skip_static=True
    )
    c_prefix, c_payloads, c_suffix = split_unmarked(
        unmarked_c,
        definition_spans(unmarked_c),
        c_groups,
        skip_static=True,
        artifact="C implementation",
    )

    c_fragments = [source_fragment_path(source, source_root, ".c") for source in sources]
    source_labels = [source.relative_to(repo_root).as_posix() for source in sources]
    for label, relative, payload in zip(source_labels, c_fragments, c_payloads):
        destination = staging / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_text(
            fragment_text(
                label,
                "evm.c",
                umbrella_include_path(relative, "evm_internal.h"),
                payload,
            ),
            encoding="utf-8",
        )
    (staging / "evm.c").write_text(
        umbrella_text(c_fragments, c_suffix), encoding="utf-8"
    )
    (staging / "evm.h").write_text(unmarked_h, encoding="utf-8")
    (staging / "evm_internal.h").write_text(
        context_text(c_prefix), encoding="utf-8"
    )

    repo_path = str(repo_root)
    for generated in staging.rglob("*"):
        if generated.is_file() and repo_path in generated.read_text(encoding="utf-8"):
            raise ValueError(f"generated extraction contains an absolute workspace path: {generated}")

    synchronize(staging, output_dir)
    sail_dir = Path(capture(sail_command + ["--dir"], repo_root).strip()).resolve()
    sail_runtime = sail_dir / "lib"
    if not (sail_runtime / "sail.h").is_file():
        raise ValueError(f"Sail runtime header not found under {sail_runtime}")
    editor_sources = [output_dir / "evm.c"] + [output_dir / path for path in c_fragments]
    write_compile_commands(
        output_dir / "compile_commands.json",
        repo_root,
        args.cc,
        list(args.compile_flag) + [f"-I{sail_runtime}"],
        editor_sources,
    )
    print(
        f"extract-c: generated {len(sources)} source-aligned C fragments "
        f"and editor metadata under {output_dir.relative_to(repo_root)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
