from __future__ import annotations

import json
from pathlib import Path

from devtools.optimised_c.lint import Finding, finding_key, read_baseline


def test_finding_key_normalizes_host_typedef_and_generated_worker_id() -> None:
    macos = Finding(
        path="build/c-optimised/generated/src/spec/evm/transaction.c",
        line=303,
        column=13,
        check="clang-analyzer-deadcode.DeadStores,-warnings-as-errors",
        message="Value stored to '_8_1364_8_1661' of type 'uint64_t' (aka 'unsigned long long') is never read",
    )
    linux = Finding(
        path="build/c-optimised/generated/src/spec/evm/transaction.c",
        line=303,
        column=13,
        check="clang-analyzer-deadcode.DeadStores,-warnings-as-errors",
        message="Value stored to '_6_1364_6_1661' of type 'uint64_t' (aka 'unsigned long') is never read",
    )

    assert finding_key(macos) == finding_key(linux)


def test_read_baseline_normalizes_existing_keys(tmp_path: Path) -> None:
    baseline_path = tmp_path / "baseline.json"
    key = "\0".join(
        (
            "generated.c",
            "1",
            "2",
            "-Wreserved-identifier",
            "identifier '__sail_c_repr_u256_8_1793' has type 'uint64_t' (aka 'unsigned long long')",
        )
    )
    baseline_path.write_text(json.dumps([key]))

    assert read_baseline(baseline_path) == {
        key.replace("-Wreserved-identifier", "reserved-identifier")
        .replace("_8_", "_worker_")
        .replace(" (aka 'unsigned long long')", "")
    }


def test_reserved_identifier_checks_are_portable(tmp_path: Path) -> None:
    baseline_path = tmp_path / "baseline.json"
    message = (
        "declaration uses identifier '__sail_c_repr_u256_8_1793', which is a reserved identifier"
    )
    baseline_path.write_text(
        json.dumps(
            [
                "\0".join(
                    (
                        "generated.c",
                        "340",
                        "10",
                        "bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp,-warnings-as-errors",
                        message,
                    )
                )
            ]
        )
    )
    compiler_finding = Finding(
        path="generated.c",
        line=340,
        column=10,
        check="-Wreserved-identifier",
        message=("identifier '__sail_c_repr_u256_6_1793' is reserved because it starts with '__'"),
    )

    assert finding_key(compiler_finding) in read_baseline(baseline_path)
