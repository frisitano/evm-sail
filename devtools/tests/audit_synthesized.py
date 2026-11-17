"""Audit exported SSZ pairs against the original state fixtures' receipts and bytes."""

from __future__ import annotations

import argparse
import json
from functools import lru_cache
from pathlib import Path
from typing import Any, cast

from devtools.harness.cli import collect, fixture_items


def audit(directory: Path, sources: Path, report_path: Path) -> int:
    from ethereum.forks.amsterdam.stateless import ProtocolFork
    from ethereum.merkle_patricia_trie import EMPTY_TRIE_ROOT
    from execution_testing.fixtures.common import FixtureTransactionReceipt

    @lru_cache(maxsize=8)
    def source_cases(path: str) -> dict[str, Any]:
        values = cast(Any, collect)(path, None, 0)
        indexed = {case["cid"]: case for case in values}
        if len(indexed) != len(values):
            raise ValueError(f"ambiguous state case ids in {path}")
        return indexed

    seen: set[tuple[str, str]] = set()
    failures = []
    duplicates = []
    rows = 0
    for shard in sorted(directory.glob("*.jsonl")):
        for _, fixture in cast(Any, fixture_items)(str(shard)):
            rows += 1
            info = fixture["_info"]
            source = Path(info["source"]).resolve()
            if not source.is_relative_to(sources.resolve()):
                raise ValueError(f"source is outside the selected fixture tree: {source}")
            key = (str(source), info["case"])
            if key in seen:
                duplicates.append(key)
            seen.add(key)
            case = source_cases(str(source))[info["case"]]
            block = fixture["blocks"][0]
            inp = bytes.fromhex(block["statelessInputBytes"].removeprefix("0x"))
            out = bytes.fromhex(block["statelessOutputBytes"].removeprefix("0x"))
            npr = 2 + int.from_bytes(inp[2:6], "little")
            payload = npr + int.from_bytes(inp[npr : npr + 4], "little")
            errors = []
            schema = (int(ProtocolFork[case["fork"]]) << 8) | 1
            if (
                info["fork"] != case["fork"]
                or int.from_bytes(inp[:2], "big") != schema
                or int.from_bytes(out[-2:], "little") != schema
            ):
                errors.append("original-fork-schema")
            if len(out) != 43 or out[32] != int(not bool(case["expected_exception"])):
                errors.append("validity")
            receipt = case.get("receipt")
            root = (
                FixtureTransactionReceipt.list_root(
                    [FixtureTransactionReceipt.model_validate(receipt)]
                )
                if receipt is not None
                else EMPTY_TRIE_ROOT
            )
            if inp[payload + 84 : payload + 116] != bytes(root):
                errors.append("original-receipt-root")
            tx_start = payload + int.from_bytes(inp[payload + 504 : payload + 508], "little")
            tx_end = payload + int.from_bytes(inp[payload + 508 : payload + 512], "little")
            expected_tx = bytes.fromhex(case["txbytes"].removeprefix("0x"))
            if (
                int.from_bytes(inp[tx_start : tx_start + 4], "little") != 4
                or inp[tx_start + 4 : tx_end] != expected_tx
            ):
                errors.append("original-signed-envelope")
            if errors:
                failures.append({"source": key[0], "case": key[1], "errors": errors})
    expected: set[tuple[str, str]] = set()
    for path in sorted(sources.rglob("*.json")):
        expected.update((str(path.resolve()), cid) for cid in source_cases(str(path)))
    missing = sorted(expected - seen)
    report = {
        "exported_rows": rows,
        "unique_cases": len(seen),
        "expected_cases": len(expected),
        "duplicates": duplicates,
        "missing": missing,
        "failures": failures,
    }
    report_path.write_text(json.dumps(report, indent=2) + "\n")
    print(
        f"SSZ audit: {len(seen)}/{len(expected)} cases, {len(failures)} mismatches, "
        f"{len(duplicates)} duplicates, {len(missing)} missing",
        flush=True,
    )
    return int(bool(failures or duplicates or missing))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path)
    parser.add_argument("--sources", type=Path, required=True)
    parser.add_argument("--report", type=Path, required=True)
    args = parser.parse_args()
    return audit(args.directory, args.sources, args.report)


if __name__ == "__main__":
    raise SystemExit(main())
