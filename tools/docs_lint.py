#!/usr/bin/env python3
"""Enforce docs/SPEC-STYLE.md over the specification comments.

Checks every sail/**/*.sail file:
- the first /*md block opens with a `# Title` heading;
- every `##` section name is `Constants`, `Types`, `Helpers`, or `The …`;
- no heading (any level) carries an EIP or Yellow Paper citation;
- every /*! doc comment's first sentence ends with a period;
- banned vocabulary never appears in /*md or /*! prose;
- references to other executable specification codebases never appear in Sail;
- host-interface pages carry the Non-normative banner;
- aggregate member documentation precedes the member declaration;
- coverage: types, registers, mappings, substantial functions, and vals
  (including the host interface's extern vals) carry /*! docs.

Strict: exits non-zero listing every violation.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

SECTION = re.compile(r"^(Constants|Types|Helpers|The .+)$")
AGGREGATE = re.compile(r"^(enum|union|struct) (\w+)[^=\n]*=\s*\{\n(.*?)^\}", re.M | re.S)
ENUM_UNION = re.compile(r"^(enum|union) (\w+)[^=\n]*=\s*\{\n(.*?)^\}", re.M | re.S)
HEADING = re.compile(r"^(#{1,6}) (.+)$", re.M)
CITED_HEADING = re.compile(r"EIP-\d|Yellow Paper|YP\b")
BANNED = re.compile(
    r"C-side|C-backed|\bthis site\b|\bdoc comments?\b|\bmkdocs\b|\bhover\b",
    re.IGNORECASE,
)
FOREIGN_SPEC = re.compile(r"\bEELS\b|execution-specs", re.IGNORECASE)

MD_BLOCK = re.compile(r"/\*md\b(.*?)\*/", re.S)
DOC_BLOCK = re.compile(r"/\*!(.*?)\*/", re.S)


def is_book_source(path: Path, root: Path) -> bool:
    """Whether a Sail file belongs to the canonical specification book."""
    rel = path.relative_to(root)
    return not (len(rel.parts) >= 2 and rel.parts[:2] == ("sail", "optimised"))


def lint_file(path: Path, rel: str) -> list[str]:
    problems = []
    text = path.read_text()
    for match in FOREIGN_SPEC.finditer(text):
        line = text[: match.start()].count("\n") + 1
        problems.append(f"{rel}:{line}: foreign specification reference {match.group(0)!r}")
    md_blocks = MD_BLOCK.findall(text)
    doc_blocks = DOC_BLOCK.findall(text)
    if not md_blocks and not doc_blocks:
        # a file with definitions but no documentation at all is a page
        # with a filename title and no prose — never acceptable
        if any(line.startswith(DEF_KEYWORDS) for line in text.split("\n")):
            problems.append(f"{rel}: no documentation comments at all")
        return problems

    if md_blocks:
        first = md_blocks[0].strip()
        if not first.startswith("# "):
            problems.append(f"{rel}: first /*md block must open with a '# Title' heading")
        else:
            title = first.split("\n", 1)[0][2:].strip()
            if len(title) > 40:
                problems.append(f"{rel}: page title too long for the nav ({len(title)} chars): {title!r}")

    for block in md_blocks:
        for level, heading in HEADING.findall(block):
            if CITED_HEADING.search(heading):
                problems.append(f"{rel}: heading carries a citation: {heading!r}")
            if level == "##" and not SECTION.match(heading.strip()):
                problems.append(f"{rel}: section name outside the vocabulary: {heading!r}")

    for block in md_blocks + doc_blocks:
        for match in BANNED.finditer(block):
            line = block[: match.start()].count("\n") + 1
            problems.append(f"{rel}: banned term {match.group(0)!r} (block line {line})")

    for block in doc_blocks:
        first_para = block.strip().split("\n\n")[0]
        flat = " ".join(first_para.split())
        if flat and "." not in flat:
            problems.append(f"{rel}: /*! first sentence lacks a period: {flat[:60]!r}")

    is_host = "/host/" in rel
    if is_host and md_blocks and "Non-normative" not in md_blocks[0]:
        problems.append(f"{rel}: host-interface page missing the Non-normative banner")

    if md_blocks:
        intro = MD_BLOCK.search(text).group(1)
        if not any(line.strip() and not line.lstrip().startswith("#") for line in intro.splitlines()):
            problems.append(f"{rel}: page introduction has no prose")

    problems.extend(coverage(text, rel))

    return problems


DEF_KEYWORDS = ("function ", "val ", "register ", "type ", "enum ", "struct ", "union ", "mapping ", "let ")
ALWAYS_DOCUMENTED = ("register ", "type ", "enum ", "struct ", "union ", "mapping ")


def coverage(text: str, rel: str) -> list[str]:
    """What must carry a /*! doc comment (normative modules only)."""
    problems = []
    lines = text.split("\n")
    documented_functions = set()
    defs = []  # (line index, keyword, name)
    for i, line in enumerate(lines):
        for kw in DEF_KEYWORDS:
            if line.startswith(kw):
                name = line[len(kw):].split("(")[0].split(":")[0].split("=")[0].split(" forall")[0].strip()
                defs.append((i, kw, name))
                break

    def has_doc(i: int) -> bool:
        j = i - 1
        while j >= 0 and not lines[j].strip():
            j -= 1
        if j < 0 or not lines[j].rstrip().endswith("*/"):
            return False
        while j >= 0 and "/*" not in lines[j]:
            j -= 1
        return j >= 0 and lines[j].lstrip().startswith("/*!")

    def def_length(i: int, next_i: int) -> int:
        # a definition ends at the next column-0 comment or definition;
        # comments between definitions never count toward its length
        end = i + 1
        while end < next_i and not lines[end].startswith("/*"):
            end += 1
        while end > i + 1 and not lines[end - 1].strip():
            end -= 1
        return end - i

    caps_lets = 0
    for n, (i, kw, name) in enumerate(defs):
        next_i = defs[n + 1][0] if n + 1 < len(defs) else len(lines)
        doc = has_doc(i)
        if not doc and kw == "function " and n > 0:
            previous_i, previous_kw, previous_name = defs[n - 1]
            doc = (
                previous_kw == "val "
                and previous_name == name
                and has_doc(previous_i)
            )
        if kw == "function " and doc:
            documented_functions.add(name)
        if kw in ALWAYS_DOCUMENTED and not doc:
            problems.append(f"{rel}:{i + 1}: undocumented {kw.strip()} `{name}`")
        elif kw == "function " and not doc and def_length(i, next_i) >= 4:
            problems.append(f"{rel}:{i + 1}: undocumented substantial function `{name}`")
        if kw == "let " and name[:1].isupper():
            caps_lets += 1

    for i, kw, name in defs:
        if kw == "val " and not has_doc(i) and name not in ALL_FUNCTIONS:
            # a val is covered by its function definition (any file)
            problems.append(f"{rel}:{i + 1}: undocumented val `{name}`")

    for m in AGGREGATE.finditer(text):
        kind, name, body = m.groups()
        base = text[: m.start(3)].count("\n")
        for i, line in enumerate(body.split("\n")):
            comment = line.find("/*")
            if comment >= 0 and line[:comment].strip():
                problems.append(
                    f"{rel}:{base + i + 1}: comment must precede its member in {kind} `{name}`"
                )

    for m in ENUM_UNION.finditer(text):
        kind, name, body = m.groups()
        base = text[: m.start(3)].count("\n")
        covered = False
        for i, line in enumerate(body.split("\n")):
            s = line.strip()
            if not s:
                covered = False  # a blank line ends a comment-headed group
                continue
            if s.startswith("/*") or s.startswith("*"):
                covered = True
                continue
            if not covered:
                problems.append(
                    f"{rel}:{base + i + 1}: uncommented member line in {kind} `{name}`: {s.split(',')[0].strip()!r}"
                )

    if caps_lets >= 4:
        md = "\n".join(MD_BLOCK.findall(text))
        if "## Constants" not in md and "| --" not in md and "|---" not in md and "| -" not in md:
            problems.append(f"{rel}: {caps_lets} constants but no Constants section or summary table")

    return problems


ALL_FUNCTIONS: set[str] = set()


def collect_functions(root: Path) -> None:
    for path in (root / "sail").rglob("*.sail"):
        if not is_book_source(path, root):
            continue
        for line in path.read_text().split("\n"):
            if line.startswith("function "):
                name = line[9:].split("(")[0].split(" forall")[0].strip()
                ALL_FUNCTIONS.add(name)


def lint_mod(path: Path, rel: str) -> list[str]:
    """A directory's mod.md section overview: title, prose, clean terms."""
    problems = []
    text = path.read_text()
    if not text.startswith("# "):
        problems.append(f"{rel}: must open with a '# Title' heading")
    else:
        title = text.split("\n", 1)[0][2:].strip()
        if len(title) > 40:
            problems.append(f"{rel}: title too long for the nav ({len(title)} chars): {title!r}")
    for level, heading in HEADING.findall(text):
        if CITED_HEADING.search(heading):
            problems.append(f"{rel}: heading carries a citation: {heading!r}")
    for match in BANNED.finditer(text):
        line = text[: match.start()].count("\n") + 1
        problems.append(f"{rel}:{line}: banned term {match.group(0)!r}")
    if not any(line.strip() and not line.lstrip().startswith(("#", "-", "!!!", " ")) for line in text.splitlines()):
        problems.append(f"{rel}: overview has no prose")
    is_host = "/host/" in f"/{rel}"
    if is_host and "Non-normative" not in text:
        problems.append(f"{rel}: host-interface overview missing the Non-normative banner")
    return problems


def main() -> int:
    root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(".")
    collect_functions(root)
    problems = []
    for path in sorted((root / "sail").rglob("*.sail")):
        if not is_book_source(path, root):
            continue
        problems.extend(lint_file(path, str(path.relative_to(root))))
    # every definition-bearing subdirectory carries a mod.md section overview
    for directory in sorted(
        {p.parent for p in (root / "sail").rglob("*.sail") if is_book_source(p, root)}
    ):
        if directory == root / "sail":
            continue
        mod = directory / "mod.md"
        rel = str(mod.relative_to(root))
        if not mod.exists():
            problems.append(f"{rel}: missing section overview (mod.md)")
        else:
            problems.extend(lint_mod(mod, rel))
    if problems:
        print(f"docs-lint: {len(problems)} violation(s)")
        for p in problems:
            print("  " + p)
        return 1
    print("docs-lint: clean")
    return 0


if __name__ == "__main__":
    sys.exit(main())
