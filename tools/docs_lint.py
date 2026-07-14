#!/usr/bin/env python3
"""Enforce docs/SPEC-STYLE.md over the specification comments.

Checks every sail/**/*.sail file:
- the first /*md block opens with a `# Title` heading;
- every `##` section name is `Constants`, `Types`, `Helpers`, or `The …`;
- no heading (any level) carries an EIP or Yellow Paper citation;
- every /*! doc comment's first sentence ends with a period;
- banned vocabulary never appears in /*md or /*! prose;
- host-interface pages carry the Non-normative banner.

Strict: exits non-zero listing every violation.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

SECTION = re.compile(r"^(Constants|Types|Helpers|The .+)$")
HEADING = re.compile(r"^(#{1,6}) (.+)$", re.M)
CITED_HEADING = re.compile(r"EIP-\d|Yellow Paper|YP\b")
BANNED = re.compile(
    r"\bEELS\b|execution-specs|C-side|C-backed|\bthis site\b|\bdoc comments?\b|\bmkdocs\b|\bhover\b",
    re.IGNORECASE,
)

MD_BLOCK = re.compile(r"/\*md\b(.*?)\*/", re.S)
DOC_BLOCK = re.compile(r"/\*!(.*?)\*/", re.S)


def lint_file(path: Path, rel: str) -> list[str]:
    problems = []
    text = path.read_text()
    md_blocks = MD_BLOCK.findall(text)
    doc_blocks = DOC_BLOCK.findall(text)
    if not md_blocks and not doc_blocks:
        return problems

    if md_blocks:
        first = md_blocks[0].strip()
        if not first.startswith("# "):
            problems.append(f"{rel}: first /*md block must open with a '# Title' heading")

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

    is_host = "/host/" in rel and "/host/kernel/" not in rel
    if is_host and md_blocks and "Non-normative" not in md_blocks[0]:
        problems.append(f"{rel}: host-interface page missing the Non-normative banner")

    return problems


def main() -> int:
    root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(".")
    problems = []
    for path in sorted((root / "sail").rglob("*.sail")):
        problems.extend(lint_file(path, str(path.relative_to(root))))
    if problems:
        print(f"docs-lint: {len(problems)} violation(s)")
        for p in problems:
            print("  " + p)
        return 1
    print("docs-lint: clean")
    return 0


if __name__ == "__main__":
    sys.exit(main())
