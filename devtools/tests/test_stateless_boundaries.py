"""Mutation checks with independent EELS commitments and native fatal reasons.

The producer runs in the pinned EELS environment; the runner uses the same
native guest interface as the fixture harness. Expected invalid verdicts alone
cannot satisfy the signature checks: the recorded fatal reason must match.
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
import tempfile
from collections.abc import Iterator
from dataclasses import replace
from pathlib import Path
from typing import Any, cast

from devtools.tests.test_memory_frames import ROOT, harness_environment

CORPUS = ROOT / "zkvm/.fixtures/current-v2106-full/blockchain_tests/for_amsterdam"
ORDER = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141


def produce() -> list[dict[str, Any]]:
    from ethereum.crypto.hash import Hash32, keccak256
    from ethereum.forks.amsterdam.execution_engine.validation_helpers import _payload_header
    from ethereum.forks.amsterdam.stateless_guest import (
        deserialize_stateless_input,
        run_stateless_guest,
    )
    from ethereum.forks.amsterdam.stateless_host import serialize_stateless_input
    from ethereum_rlp import rlp
    from ethereum_types.bytes import Bytes
    from ethereum_types.numeric import U64, U256

    cases = []

    def add(name: str, value: Any, reason: str | None = None, rehash: bool = True) -> None:
        request = value.new_payload_request
        if rehash:
            header = _payload_header(
                request.execution_payload,
                request.parent_beacon_block_root,
                request.execution_requests,
            )
            payload = replace(
                request.execution_payload, block_hash=Hash32(keccak256(rlp.encode(header)))
            )
            value = replace(value, new_payload_request=replace(request, execution_payload=payload))
        inp = serialize_stateless_input(value)
        out = run_stateless_guest(inp)
        if reason:
            assert out[32] == 0, name
        cases.append({"name": name, "input": inp.hex(), "output": out.hex(), "reason": reason})
        print(f"reference {name}: valid={bool(out[32])}", flush=True)

    def embedded(relative: str) -> Iterator[Any]:
        for entry in json.loads((CORPUS / relative).read_text()).values():
            for block in entry.get("blocks", []):
                if (
                    "statelessInputBytes" in block
                    and bytes.fromhex(block["statelessOutputBytes"][2:])[32]
                ):
                    yield deserialize_stateless_input(
                        Bytes.fromhex(block["statelessInputBytes"][2:])
                    )

    seen = set()
    blob_value = None
    source = "amsterdam/eip2780_reduce_intrinsic_tx_gas/value_moving_transactions/intrinsic_decomposition_across_tx_types.json"
    for value in embedded(source):
        request = value.new_payload_request
        payload = request.execution_payload
        for position, raw in enumerate(payload.transactions):
            kind = 0 if raw[0] >= 192 else raw[0]
            if kind in seen:
                continue
            seen.add(kind)
            if kind == 3:
                blob_value = value
            add(f"signature/type{kind}/control", value)
            add(
                f"signature/type{kind}/wrong-chain-config",
                replace(value, chain_id=U64(int(value.chain_id) + 1)),
                "InvalidSignature" if kind == 0 else "ExecutionInvalid",
            )
            fields = rlp.decode(Bytes(raw if kind == 0 else raw[1:]))
            for label, field, number in (
                ("r-zero", -2, 0),
                ("r-order", -2, ORDER),
                ("s-zero", -1, 0),
                ("s-order", -1, ORDER),
                ("s-high", -1, ORDER // 2 + 1),
                ("parity", -3, 29 if kind == 0 else 2),
            ):
                signature_fields = list(fields)
                signature_fields[field] = number.to_bytes((number.bit_length() + 7) // 8, "big")
                encoded = rlp.encode(signature_fields)
                mutated = Bytes(encoded if kind == 0 else bytes((kind,)) + encoded)
                txs = list(payload.transactions)
                txs[position] = mutated
                altered = replace(
                    value,
                    new_payload_request=replace(
                        request, execution_payload=replace(payload, transactions=tuple(txs))
                    ),
                )
                add(f"signature/type{kind}/{label}", altered, "InvalidSignature")
            domain_fields = list(fields)
            # Keep r/s fixed while changing the signing domain. Such a
            # signature may recover a different key; it is not necessarily
            # an intrinsically invalid signature.
            domain_index = -3 if kind == 0 else 0
            previous = int.from_bytes(domain_fields[domain_index], "big")
            different = previous + 2 if kind == 0 else previous + 1
            domain_fields[domain_index] = different.to_bytes(
                (different.bit_length() + 7) // 8, "big"
            )
            encoded = rlp.encode(domain_fields)
            transactions = list(payload.transactions)
            transactions[position] = Bytes(encoded if kind == 0 else bytes((kind,)) + encoded)
            altered = replace(
                value,
                new_payload_request=replace(
                    request, execution_payload=replace(payload, transactions=tuple(transactions))
                ),
            )
            add(f"signature/type{kind}/wrong-domain", altered)
            assert bytes.fromhex(cases[-1]["output"])[32] == 0
            for label, field, number in (
                ("r-one", -2, 1),
                ("r-last", -2, ORDER - 1),
                ("s-one", -1, 1),
                ("s-low-last", -1, ORDER // 2),
            ):
                boundary_fields = list(fields)
                boundary_fields[field] = number.to_bytes((number.bit_length() + 7) // 8, "big")
                encoded = rlp.encode(boundary_fields)
                transactions = list(payload.transactions)
                transactions[position] = Bytes(encoded if kind == 0 else bytes((kind,)) + encoded)
                altered = replace(
                    value,
                    new_payload_request=replace(
                        request,
                        execution_payload=replace(payload, transactions=tuple(transactions)),
                    ),
                )
                add(f"signature/type{kind}/{label}", altered)
                assert bytes.fromhex(cases[-1]["output"])[32] == 0
            for label, broken in (
                ("truncated", bytes((kind,)) if kind else b"\xc0"),
                ("leading-zero-r", None),
            ):
                if broken is None:
                    malformed_fields = list(fields)
                    malformed_fields[-2] = b"\0" + malformed_fields[-2]
                    encoded = rlp.encode(malformed_fields)
                    broken = encoded if kind == 0 else bytes((kind,)) + encoded
                transactions = list(payload.transactions)
                transactions[position] = Bytes(broken)
                altered = replace(
                    value,
                    new_payload_request=replace(
                        request,
                        execution_payload=replace(payload, transactions=tuple(transactions)),
                    ),
                )
                add(f"signature/type{kind}/{label}", altered)
                assert bytes.fromhex(cases[-1]["output"])[32] == 0
        if seen == set(range(5)):
            break
    assert seen == set(range(5)), seen
    from devtools.tests.witness_mutations import storage_cases

    storage_cases(add, cases)

    # Recommit the parent and payload around deliberately malformed secured
    # state roots. No transaction can hide these supplied malformed branches
    # by avoiding their keys.
    from ethereum.merkle_patricia_trie import EMPTY_TRIE_ROOT
    from ethereum.state import EMPTY_CODE_HASH

    control = deserialize_stateless_input(Bytes.fromhex(cases[0]["input"]))
    account = rlp.encode([U64(0), U256(0), EMPTY_TRIE_ROOT, EMPTY_CODE_HASH])
    leaf63 = Bytes(rlp.encode([b"\x30" + b"\0" * 31, account]))
    leaf62 = Bytes(rlp.encode([b"\x20" + b"\0" * 31, account]))
    extension = Bytes(rlp.encode([b"\x10", keccak256(leaf63)]))
    branch = [b""] * 17
    branch[0] = keccak256(leaf63)
    invalid_width = list(branch)
    invalid_width[1] = b"\x01"
    for name, root, descendants in (
        ("short-leaf", leaf62, ()),
        ("extension-to-leaf", extension, (leaf63,)),
        ("single-child-branch", Bytes(rlp.encode(branch)), (leaf63,)),
        ("invalid-child-width", Bytes(rlp.encode(invalid_width)), (leaf63,)),
    ):
        headers = list(control.witness.headers)
        parent = rlp.decode(headers[-1])
        parent[3] = keccak256(root)
        headers[-1] = Bytes(rlp.encode(parent))
        payload = replace(
            control.new_payload_request.execution_payload,
            parent_hash=Hash32(keccak256(headers[-1])),
        )
        altered = replace(
            control,
            witness=replace(
                control.witness,
                headers=tuple(headers),
                state=(root, *descendants, *control.witness.state),
            ),
            new_payload_request=replace(control.new_payload_request, execution_payload=payload),
        )
        add(f"witness/{name}", altered)
        assert bytes.fromhex(cases[-1]["output"])[32] == 0

    # Keep the committed block header consistent with request mutations so
    # rejection reaches execution's receipt-log authentication.
    value = next(
        v
        for v in embedded("prague/eip6110_deposits/deposits/deposit.json")
        if len(v.new_payload_request.execution_requests.deposits) >= 2
    )
    request = value.new_payload_request
    requests = request.execution_requests
    original = requests.deposits
    add("deposits/control", value)
    for name, changed in (
        ("missing", original[:-1]),
        ("extra", original + original[:1]),
        ("reordered", tuple(reversed(original))),
        ("none", ()),
    ):
        add(
            f"deposits/{name}",
            replace(
                value,
                new_payload_request=replace(
                    request, execution_requests=replace(requests, deposits=changed)
                ),
            ),
        )
        assert bytes.fromhex(cases[-1]["output"])[32] == 0
    changed = replace(original[0], amount=type(original[0].amount)(int(original[0].amount) + 1))
    add(
        "deposits/mutated",
        replace(
            value,
            new_payload_request=replace(
                request, execution_requests=replace(requests, deposits=(changed, *original[1:]))
            ),
        ),
    )
    assert bytes.fromhex(cases[-1]["output"])[32] == 0

    # Add one byte to a fixed-width deposit list and repair all enclosing
    # offsets. This is a malformed entry width, not a broken offset table.
    raw = bytearray(serialize_stateless_input(value))
    npr = 2 + int.from_bytes(raw[2:6], "little")
    request_start = npr + int.from_bytes(raw[npr + 40 : npr + 44], "little")
    deposit_end = request_start + int.from_bytes(
        raw[request_start + 4 : request_start + 8], "little"
    )
    for offset in (4, 8, 12, 16):
        position = request_start + offset
        raw[position : position + 4] = (
            int.from_bytes(raw[position : position + 4], "little") + 1
        ).to_bytes(4, "little")
    raw[6:10] = (int.from_bytes(raw[6:10], "little") + 1).to_bytes(4, "little")
    raw[deposit_end:deposit_end] = b"\0"
    malformed_output = run_stateless_guest(Bytes(raw))
    assert malformed_output[32] == 0
    cases.append(
        {
            "name": "deposits/malformed-entry-width",
            "input": raw.hex(),
            "output": malformed_output.hex(),
            "reason": None,
        }
    )
    for path in sorted((CORPUS / "prague/eip6110_deposits/modified_contract").glob("*.json")):
        for name, entry in json.loads(path.read_text()).items():
            for index, block in enumerate(entry.get("blocks", ())):
                if "statelessInputBytes" in block:
                    cases.append(
                        {
                            "name": f"deposit-logs/{path.stem}/{name}/{index}",
                            "input": block["statelessInputBytes"].removeprefix("0x"),
                            "output": block["statelessOutputBytes"].removeprefix("0x"),
                            "reason": None,
                        }
                    )

    # Progressive request boundaries cross the 1/4/16 subtree capacities.
    for count in (0, 1, 2, 4, 5, 6, 20, 21, 22):
        add(
            f"htr/deposits/{count}",
            replace(
                value,
                new_payload_request=replace(
                    request,
                    execution_requests=replace(
                        requests, deposits=tuple(original[0] for _ in range(count))
                    ),
                ),
            ),
        )
    for size in (0, 1, 31, 32):
        add(
            f"htr/extra-data/{size}",
            replace(
                value,
                new_payload_request=replace(
                    request,
                    execution_payload=replace(
                        request.execution_payload, extra_data=Bytes(b"x" * size)
                    ),
                ),
            ),
        )
    raw = bytearray.fromhex(cases[-1]["input"])
    npr = 2 + int.from_bytes(raw[2:6], "little")
    payload_start = npr + int.from_bytes(raw[npr : npr + 4], "little")
    extra_end = payload_start + int.from_bytes(
        raw[payload_start + 504 : payload_start + 508], "little"
    )
    for position in (
        6,
        npr + 4,
        npr + 40,
        payload_start + 504,
        payload_start + 508,
        payload_start + 528,
    ):
        raw[position : position + 4] = (
            int.from_bytes(raw[position : position + 4], "little") + 1
        ).to_bytes(4, "little")
    raw[extra_end:extra_end] = b"x"
    malformed_output = run_stateless_guest(Bytes(raw))
    assert malformed_output == b"\0" * 43
    cases.append(
        {
            "name": "htr/extra-data/33-rejected",
            "input": raw.hex(),
            "output": malformed_output.hex(),
            "reason": "InvalidConfig",
        }
    )
    assert blob_value is not None
    request = blob_value.new_payload_request
    hashes = request.versioned_hashes
    assert hashes
    for name, changed_hashes in (
        ("missing", hashes[:-1]),
        ("suffix", (*hashes, hashes[0])),
        ("mutated", (type(hashes[0])(b"\x01" + b"\x42" * 31), *hashes[1:])),
    ):
        add(
            f"versioned-hashes/{name}",
            replace(
                blob_value, new_payload_request=replace(request, versioned_hashes=changed_hashes)
            ),
        )
        assert bytes.fromhex(cases[-1]["output"])[32] == 0

    ordered = next(
        v
        for v in embedded(
            "cancun/eip4844_blobs/blobhash_opcode/blobhash_multiple_txs_in_block.json"
        )
        if len(set(v.new_payload_request.versioned_hashes)) >= 2
    )
    request = ordered.new_payload_request
    add("versioned-hashes/ordered-control", ordered)
    add(
        "versioned-hashes/reordered",
        replace(
            ordered,
            new_payload_request=replace(
                request, versioned_hashes=tuple(reversed(request.versioned_hashes))
            ),
        ),
    )
    assert bytes.fromhex(cases[-1]["output"])[32] == 0

    # Unknown fork and transport revision are rejected before execution.
    control = bytes.fromhex(cases[0]["input"])
    for schema in (b"\x09\x01", b"\x16\x01", b"\x15\x00", b"\x15\x02"):
        cases.append(
            {
                "name": f"schema/{schema.hex()}",
                "input": (schema + control[2:]).hex(),
                "output": (b"\0" * 43).hex(),
                "reason": "InvalidConfig",
            }
        )
    return cases


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--produce", type=Path)
    parser.add_argument("--cases", type=Path)
    parser.add_argument("--build", choices=("standard", "optimized"))
    args = parser.parse_args()
    if args.produce:
        args.produce.write_text(json.dumps(produce()) + "\n")
        return 0
    if args.cases:
        from devtools.harness.guest import load_guest, run_once_guest, snapshot

        cast(Any, load_guest)(build_mode=args.build)
        cases = json.loads(args.cases.read_text())
        for case in cases:
            actual = cast(Any, run_once_guest)(bytes.fromhex(case["input"]))
            state = cast(Any, snapshot)()
            assert actual == bytes.fromhex(case["output"]), (case["name"], state)
            if case["reason"]:
                assert state["validation_failure"]["reason"] == case["reason"], (
                    case["name"],
                    state,
                )
            if case.get("allowed_reasons"):
                assert state["validation_failure"]["reason"] in case["allowed_reasons"], (
                    case["name"],
                    state,
                )
            print(f"PASS {args.build} {case['name']}", flush=True)
        return 0
    environment = harness_environment()
    module = "devtools.tests.test_stateless_boundaries"
    with tempfile.TemporaryDirectory(prefix="stateless-boundaries.") as directory:
        path = Path(directory) / "cases.json"
        subprocess.run(
            [environment["EXECSPECS_PY"], "-m", module, "--produce", str(path)],
            cwd=ROOT,
            env=environment,
            check=True,
        )
        for backend in ("standard", "optimized"):
            subprocess.run(
                [sys.executable, "-m", module, "--cases", str(path), "--build", backend],
                cwd=ROOT,
                env=environment,
                check=True,
            )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
