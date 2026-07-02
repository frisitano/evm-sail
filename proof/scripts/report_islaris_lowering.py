#!/usr/bin/env python3
"""Manage Islaris per-instruction lowering coverage for a full ELF dump."""

from __future__ import annotations

import argparse
import os
import re
import sys
from dataclasses import dataclass
from pathlib import Path


INSTR_RE = re.compile(r"^\s*([0-9a-fA-F]+):\s+([0-9a-fA-F]+)\s+(\S+)")


@dataclass(frozen=True)
class Instr:
    addr: int
    addr_hex: str
    opcode: str
    mnemonic: str

    @property
    def stem(self) -> str:
        return f"a{self.addr:x}"


@dataclass(frozen=True)
class InstrBlock:
    instr: Instr
    lines: tuple[str, ...]


def parse_instr_match(line: str) -> Instr | None:
    match = INSTR_RE.match(line)
    if match is None:
        return None
    addr_hex, opcode, mnemonic = match.groups()
    return Instr(int(addr_hex, 16), addr_hex.lower(), opcode.lower(), mnemonic)


def parse_dump(path: Path) -> list[Instr]:
    instrs: list[Instr] = []
    seen: set[int] = set()
    with path.open("r", encoding="utf-8") as f:
        for line in f:
            instr = parse_instr_match(line)
            if instr is None:
                continue
            if instr.addr in seen:
                raise SystemExit(
                    f"duplicate instruction address in dump: 0x{instr.addr:x}"
                )
            seen.add(instr.addr)
            instrs.append(instr)
    if not instrs:
        raise SystemExit(f"no instruction lines found in dump: {path}")
    return instrs


def parse_dump_blocks(path: Path) -> tuple[list[str], list[InstrBlock], str | None]:
    prefix: list[str] = []
    pending: list[str] = []
    blocks: list[InstrBlock] = []
    seen_instr = False
    isla_config: str | None = None

    with path.open("r", encoding="utf-8") as f:
        for line in f:
            stripped = line.strip()
            if stripped.startswith("//@isla-config:") and isla_config is None:
                isla_config = stripped.split(":", 1)[1].strip()

            instr = parse_instr_match(line)
            if instr is not None:
                blocks.append(InstrBlock(instr=instr, lines=tuple(pending + [line])))
                pending = []
                seen_instr = True
                continue

            if stripped.startswith("//@"):
                pending.append(line)
                continue

            if seen_instr:
                if stripped:
                    pending.append(line)
            else:
                prefix.append(line)

    if pending:
        raise SystemExit("dump ended with annotations or labels not attached to an instruction")
    if not blocks:
        raise SystemExit(f"no instruction lines found in dump: {path}")
    return prefix, blocks, isla_config


def nonempty(path: Path) -> bool:
    try:
        return path.stat().st_size > 0
    except FileNotFoundError:
        return False


def pct(part: int, total: int) -> str:
    if total == 0:
        return "0.0000%"
    return f"{(part / total) * 100:.4f}%"


def count_matching_files(path: Path, suffix: str) -> tuple[int, int]:
    total = 0
    empty = 0
    if not path.is_dir():
        return (0, 0)
    for entry in path.iterdir():
        if entry.name.endswith(suffix) and entry.is_file():
            total += 1
            if entry.stat().st_size == 0:
                empty += 1
    return (total, empty)


def generated_addr_set(out_dir: Path, instrs: list[Instr]) -> set[int]:
    generated: set[int] = set()
    for instr in instrs:
        if nonempty(out_dir / f"{instr.stem}.v"):
            generated.add(instr.addr)
    return generated


def find_extra_v_files(out_dir: Path, expected_stems: set[str]) -> list[str]:
    if not out_dir.is_dir():
        return []
    extras: list[str] = []
    for entry in out_dir.iterdir():
        if not entry.name.endswith(".v") or not entry.is_file():
            continue
        stem = entry.name[:-2]
        if stem == "instrs":
            continue
        if stem not in expected_stems:
            extras.append(entry.name)
    return sorted(extras)


def render_report(dump: Path, out_dir: Path, instrs: list[Instr]) -> str:
    expected = len(instrs)
    stems = {instr.stem for instr in instrs}
    generated = generated_addr_set(out_dir, instrs)
    generated_count = len(generated)
    v_total, v_empty = count_matching_files(out_dir, ".v")
    isla_total, isla_empty = count_matching_files(out_dir, ".isla")
    missing = [instr for instr in instrs if instr.addr not in generated]
    extras = find_extra_v_files(out_dir, stems)

    contiguous = 0
    for instr in instrs:
        if instr.addr not in generated:
            break
        contiguous += 1

    first = instrs[0]
    last = instrs[-1]
    first_missing = missing[0] if missing else None
    last_contiguous = instrs[contiguous - 1] if contiguous else None
    highest_generated = max(generated) if generated else None

    lines = [
        "# Reth ZisK Full Islaris Lowering Progress",
        "",
        f"- dump: `{dump}`",
        f"- output_dir: `{out_dir}`",
        f"- expected_instructions: {expected}",
        f"- dump_first_pc: 0x{first.addr:016x}",
        f"- dump_last_pc: 0x{last.addr:016x}",
        f"- generated_nonempty_coq_files: {generated_count} ({pct(generated_count, expected)})",
        f"- contiguous_prefix_coq_files: {contiguous} ({pct(contiguous, expected)})",
        f"- all_v_files: {v_total}",
        f"- empty_v_files: {v_empty}",
        f"- all_isla_files: {isla_total}",
        f"- empty_isla_files: {isla_empty}",
        f"- complete: {'yes' if generated_count == expected and not missing else 'no'}",
    ]

    if first_missing is not None:
        lines.append(f"- first_missing_pc: 0x{first_missing.addr:016x}")
        lines.append(f"- first_missing_file: `{first_missing.stem}.v`")
    if last_contiguous is not None:
        lines.append(f"- last_contiguous_pc: 0x{last_contiguous.addr:016x}")
    if highest_generated is not None:
        lines.append(f"- highest_generated_pc: 0x{highest_generated:016x}")
    if extras:
        lines.append(f"- extra_v_files_not_in_dump: {len(extras)}")
        lines.append(f"- first_extra_v_file: `{extras[0]}`")

    lines.extend(
        [
            "",
            "A non-empty per-instruction `.v` file is counted as lowered.",
            "Zero-byte `.isla` files are expected after an interrupted Islaris run",
            "because each worker opens the trace file before `isla-footprint`",
            "finishes.",
            "",
        ]
    )
    return "\n".join(lines)


def write_shard(
    dump: Path,
    out: Path,
    shard_index: int,
    shard_size: int,
) -> str:
    if shard_index < 0:
        raise SystemExit("--shard-index must be non-negative")
    if shard_size <= 0:
        raise SystemExit("--shard-size must be positive")

    prefix, blocks, isla_config = parse_dump_blocks(dump)
    start = shard_index * shard_size
    end = min(start + shard_size, len(blocks))
    if start >= len(blocks):
        raise SystemExit(
            f"shard {shard_index} starts at instruction {start}, "
            f"but dump only has {len(blocks)} instructions"
        )

    selected = blocks[start:end]
    first = selected[0].instr
    last = selected[-1].instr

    out.parent.mkdir(parents=True, exist_ok=True)
    with out.open("w", encoding="utf-8") as f:
        f.write(f"// Shard generated from {dump}\n")
        f.write(f"// Shard index: {shard_index}\n")
        f.write(f"// Shard size: {shard_size}\n")
        f.write(f"// Instruction range: [{start}, {end})\n")
        f.write(f"// PC range: 0x{first.addr:016x}..0x{last.addr:016x}\n")
        for line in prefix:
            f.write(line)
        first_lines = list(selected[0].lines)
        if (
            isla_config is not None
            and not any(line.strip().startswith("//@isla-config:") for line in first_lines)
        ):
            f.write(f"//@isla-config: {isla_config}\n")
        for block in selected:
            for line in block.lines:
                f.write(line)

    return (
        f"wrote shard {shard_index} with {len(selected)} instructions "
        f"(0x{first.addr:016x}..0x{last.addr:016x}) to {out}"
    )


def render_instrs(coq_namespace: str, instrs: list[Instr]) -> str:
    lines = ["Require Import isla.isla_lang."]
    lines.append("")
    for instr in instrs:
        lines.append(f"Require Export {coq_namespace}.{instr.stem}.")
    lines.append("")
    lines.append("Definition instr_map := [")
    for i, instr in enumerate(instrs):
        sep = ";" if i + 1 < len(instrs) else ""
        lines.append(
            f"  (0x{instr.addr:x}%Z, {instr.stem} (* {instr.mnemonic} *)){sep}"
        )
    lines.append("].")
    lines.append("")
    return "\n".join(lines)


def render_dune(coq_namespace: str) -> str:
    return "\n".join(
        [
            "; Generated by [evm-sail proof/scripts/report_islaris_lowering.py], do not edit.",
            "(coq.theory",
            f" (name {coq_namespace})",
            " (package islaris)",
            " (flags :standard -w -notation-overridden -w -redundant-canonical-projection)",
            ' (synopsis "Generated file")',
            " (theories isla isla.riscv64))",
            "",
        ]
    )


def write_instrs(out_dir: Path, coq_namespace: str, instrs: list[Instr]) -> str:
    if not coq_namespace:
        raise SystemExit("--coq-namespace is required with --write-instrs")
    out_dir.mkdir(parents=True, exist_ok=True)
    instrs_path = out_dir / "instrs.v"
    dune_path = out_dir / "dune"
    instrs_path.write_text(render_instrs(coq_namespace, instrs), encoding="utf-8")
    dune_path.write_text(render_dune(coq_namespace), encoding="utf-8")
    return f"wrote {instrs_path} and {dune_path}"


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dump", required=True, type=Path)
    parser.add_argument("--coq-dir", required=True, type=Path)
    parser.add_argument("--report", type=Path)
    parser.add_argument("--require-any", action="store_true")
    parser.add_argument("--require-complete", action="store_true")
    parser.add_argument("--write-shard", type=Path)
    parser.add_argument("--shard-index", type=int, default=0)
    parser.add_argument("--shard-size", type=int, default=1024)
    parser.add_argument("--write-instrs", action="store_true")
    parser.add_argument("--coq-namespace")
    args = parser.parse_args(argv)

    instrs = parse_dump(args.dump)
    generated = generated_addr_set(args.coq_dir, instrs)

    if args.write_shard is not None:
        print(
            write_shard(
                args.dump,
                args.write_shard,
                args.shard_index,
                args.shard_size,
            )
        )

    if args.require_any and not generated:
        print("error: no non-empty generated Coq instruction files", file=sys.stderr)
        return 1
    if args.require_complete and len(generated) != len(instrs):
        print(
            f"error: incomplete lowering: {len(generated)} of {len(instrs)} "
            "instruction files generated",
            file=sys.stderr,
        )
        return 1

    if args.write_instrs:
        print(write_instrs(args.coq_dir, args.coq_namespace or "", instrs))

    if args.write_shard is None and not args.write_instrs:
        report = render_report(args.dump, args.coq_dir, instrs)
        if args.report is not None:
            args.report.parent.mkdir(parents=True, exist_ok=True)
            args.report.write_text(report, encoding="utf-8")
        print(report)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
