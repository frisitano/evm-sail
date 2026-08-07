#!/usr/bin/env python3
"""Summarize opt-in Sail source and post-Jib readability diagnostics."""

from __future__ import annotations

import argparse
from collections import Counter
import json
from pathlib import Path
import re


DIAGNOSTIC = re.compile(
    r"^Warning: (?P<phase>Jib )?[Rr]eadability lint \[(?P<rule>[^]]+)]"
    r"(?: (?P<location>.*))?$"
)


def parse(path: Path, phase: str) -> list[tuple[str, str, str]]:
    findings: list[tuple[str, str, str]] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        match = DIAGNOSTIC.match(line)
        if match is None:
            continue
        reported_phase = "jib" if match.group("phase") else "source"
        if reported_phase == phase:
            findings.append((phase, match.group("rule"), match.group("location") or "<no source location>"))
    return findings


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-log", type=Path, required=True)
    parser.add_argument("--jib-log", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--json-output", type=Path)
    args = parser.parse_args()

    findings = parse(args.source_log, "source") + parse(args.jib_log, "jib")
    counts = Counter((phase, rule) for phase, rule, _ in findings)
    source_count = sum(count for (phase, _), count in counts.items() if phase == "source")
    jib_count = sum(count for (phase, _), count in counts.items() if phase == "jib")

    lines = [
        "Sail readability lint report",
        f"source log: {args.source_log}",
        f"post-Jib log: {args.jib_log}",
        "",
        f"Sail source findings: {source_count}",
        f"post-Jib findings: {jib_count}",
        f"total findings: {len(findings)}",
        "",
        "rule counts:",
    ]
    if counts:
        lines.extend(
            f"  {phase}:{rule}: {count}"
            for (phase, rule), count in sorted(counts.items())
        )
    else:
        lines.append("  none")
    lines.extend(
        (
            "",
            "Complete finding locations are retained in the JSON report and raw logs.",
        )
    )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text("\n".join(lines) + "\n", encoding="utf-8")

    if args.json_output is not None:
        document = {
            "schema_version": 1,
            "logs": {
                "source": str(args.source_log),
                "jib": str(args.jib_log),
            },
            "summary": {
                "source_findings": source_count,
                "jib_findings": jib_count,
                "total_findings": len(findings),
            },
            "rule_counts": [
                {"phase": phase, "rule": rule, "count": count}
                for (phase, rule), count in sorted(counts.items())
            ],
            "findings": [
                {"phase": phase, "rule": rule, "location": location}
                for phase, rule, location in findings
            ],
        }
        args.json_output.parent.mkdir(parents=True, exist_ok=True)
        args.json_output.write_text(
            json.dumps(document, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
