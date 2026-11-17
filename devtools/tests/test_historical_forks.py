"""Check fork-selected execution against the pinned EELS blockchain filler."""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
import tempfile
from copy import deepcopy
from dataclasses import replace
from pathlib import Path
from typing import Any, cast

from devtools.harness.ssz_builder import SUPPORTED_FORKS, SUPPORTED_TRANSITIONS
from devtools.tests.test_memory_frames import (
    LOWER,
    ROOT,
    SENDER,
    call,
    fixture_case,
    harness_environment,
    push,
)


def deployment_probe(size: int) -> bytes:
    initcode = push(size) + push(0) + b"\xf3"
    return (
        push(int.from_bytes(initcode, "big"), len(initcode))
        + push(0)
        + b"\x52"
        + push(len(initcode))
        + push(32 - len(initcode))
        + push(0)
        + b"\xf0\x15\x15"
        + push(0)
        + b"\x55\x00"
    )


def reference_pairs() -> dict[str, Any]:
    # This function runs only under the pinned execution-specs interpreter.
    from ethereum.forks.amsterdam.stateless import StatelessInput, StatelessValidationResult
    from ethereum.forks.amsterdam.stateless_guest import deserialize_stateless_input
    from ethereum_rlp import rlp
    from ethereum_types.bytes import Bytes
    from ethereum_types.numeric import U16

    from devtools.harness.ssz_builder import _signed_transaction, build_guest

    source = (
        ROOT
        / "zkvm/.fixtures/current-v2106-full/blockchain_tests/for_amsterdam/amsterdam/eip2780_reduce_intrinsic_tx_gas/value_moving_transactions/intrinsic_decomposition_across_tx_types.json"
    )
    authorizations = None
    for entry in json.loads(source.read_text()).values():
        for block in entry.get("blocks", ()):
            if "statelessInputBytes" not in block:
                continue
            value = deserialize_stateless_input(Bytes.fromhex(block["statelessInputBytes"][2:]))
            for raw in value.new_payload_request.execution_payload.transactions:
                if raw and raw[0] == 4:
                    authorizations = cast(Any, _signed_transaction)("0x" + raw.hex())[
                        "authorizationList"
                    ]
                    break
            if authorizations:
                break
        if authorizations:
            break
    assert authorizations

    result = {}
    for index, fork in enumerate(SUPPORTED_FORKS):
        probes = {
            "sstore": (bytes.fromhex("600160005500"), True),
            "warm-account": (bytes.fromhex("602a3150602a3150600160005500"), True),
            "warm-slot": (bytes.fromhex("6001545060015450600160005500"), True),
            "precompile-balance": (bytes.fromhex("6001315060013150600160005500"), True),
            "push0": (bytes.fromhex("60015f5500"), index >= 5),
            "transient": (bytes.fromhex("600160005d60005c60005500"), index >= 6),
            # BLOBBASEFEE is nonzero in the empty reference block.
            "blobbasefee": (bytes.fromhex("4a151560005500"), index >= 6),
            "basefee": (bytes.fromhex("48151560005500"), index >= 1),
            "sender": (bytes.fromhex("326000553360015500"), True),
            "unprotected-sender": (bytes.fromhex("326000553360015500"), True),
            "code-bearing-sender": (bytes.fromhex("600160005500"), False),
            "code-size-24576": (deployment_probe(24576), True),
            "code-size-24577": (deployment_probe(24577), index == 11),
            "access-list": (bytes.fromhex("600160005500"), True),
            "dynamic-fee": (bytes.fromhex("600160005500"), True),
            "blob": (bytes.fromhex("600160005500"), True),
            "authorization": (bytes.fromhex("600160005500"), True),
            "transaction-gas-cap": (bytes.fromhex("600160005500"), True),
            "initcode-size-49152": (
                push(49152) + push(0) + push(0) + b"\xf0\x15\x15" + push(0) + b"\x55\x00",
                True,
            ),
            "initcode-size-49153": (
                push(49153) + push(0) + push(0) + b"\xf0\x15\x15" + push(0) + b"\x55\x00",
                index < 5 or index == 11,
            ),
        }
        child = 0x9000000000000000000000000000000000000000
        target = child + 1
        if fork in ("Berlin", "Prague"):
            probes["reverted-warm-access"] = (
                call(child)
                + b"\x50"
                + call(child)
                + b"\x50"
                + push(target, 20)
                + b"\x31\x50\x60\x01\x60\x00\x55\x00",
                True,
            )
        if fork == "Amsterdam":
            probes["code-size-65536"] = (deployment_probe(65536), True)
            probes["code-size-65537"] = (deployment_probe(65537), False)
            for size in (131072, 131073):
                probes[f"initcode-size-{size}"] = (
                    push(size) + push(0) + push(0) + b"\xf0\x15\x15" + push(0) + b"\x55\x00",
                    size == 131072,
                )
            probes["total-gas-at-limit"] = (bytes.fromhex("600160005500"), True)
            probes["total-gas-over-limit"] = (bytes.fromhex("600160005500"), True)
        for name, (code, available) in probes.items():
            codes = {LOWER: code}
            if name == "reverted-warm-access":
                codes[child] = push(target, 20) + bytes.fromhex("31506000545060006000fd")
            fixture = fixture_case(LOWER, codes)
            if name == "sstore":
                cast(dict[str, Any], fixture["pre"])[
                    "0x9000000000000000000000000000000000000000"
                ] = {"nonce": "0x00", "balance": "0x01", "code": "0x", "storage": {"0x01": "0x01"}}
            env = cast(dict[str, Any], fixture["env"]).copy()
            if index == 0:
                env.pop("currentBaseFee")
            if index < 4:
                env.pop("currentRandom")
                env["currentDifficulty"] = "0x020000"
            case = {
                "fork": fork,
                "env": env,
                "pre": fixture["pre"],
                "tx": fixture["transaction"],
                "idx": {"gas": 0, "data": 0, "value": 0},
                "config": fixture["config"],
                "expected_post": {
                    f"0x{LOWER:040x}": {"storage": {"0x00": "0x01" if available else "0x00"}}
                },
            }
            if name in ("sender", "unprotected-sender"):
                case["expected_post"] = {
                    f"0x{LOWER:040x}": {"storage": {"0x00": SENDER, "0x01": SENDER}}
                }
            if name == "unprotected-sender":
                cast(dict[str, Any], case["tx"])["protected"] = False
            if name.startswith("code-size"):
                cast(dict[str, Any], case["tx"])["gasLimit"] = [
                    "0x10000000" if fork == "Amsterdam" else "0x1000000"
                ]
            valid = True
            transaction = cast(dict[str, Any], case["tx"])
            if name == "access-list":
                transaction["accessLists"] = [
                    [{"address": f"0x{LOWER:040x}", "storageKeys": ["0x" + "00" * 32]}]
                ]
            if name in ("dynamic-fee", "blob", "authorization"):
                transaction.pop("gasPrice")
                transaction["maxFeePerGas"] = "0x0a"
                transaction["maxPriorityFeePerGas"] = "0x02"
            if name == "blob":
                transaction["maxFeePerBlobGas"] = "0x0a"
                transaction["blobVersionedHashes"] = ["0x01" + "00" * 31]
            if name == "authorization":
                transaction["authorizationList"] = authorizations
            for probe, activation, kind in (
                ("dynamic-fee", 1, 2),
                ("blob", 6, 3),
                ("authorization", 7, 4),
            ):
                if name == probe and index < activation:
                    case["expected_exception"] = f"TransactionException.TYPE_{kind}_TX_PRE_FORK"
                    case["expected_post"] = {}
                    valid = False
            if name == "transaction-gas-cap":
                transaction["gasLimit"] = ["0x1000001"]
                if 8 <= index <= 10:
                    case["expected_exception"] = "TransactionException.GAS_LIMIT_EXCEEDS_MAXIMUM"
                    case["expected_post"] = {}
                    valid = False
            if name.startswith("total-gas-"):
                transaction["gasLimit"] = [hex(2**32 - (1 if name == "total-gas-at-limit" else 0))]
                if name == "total-gas-over-limit":
                    case["expected_exception"] = "TransactionException.GAS_LIMIT_EXCEEDS_MAXIMUM"
                    case["expected_post"] = {}
                    valid = False
            if name == "code-bearing-sender":
                cast(dict[str, Any], case["pre"])[SENDER]["code"] = "0x00"
                # The pinned EELS reference applies its EOA sender policy
                # to Berlin too; preserve that explicit compatibility rule.
                case["expected_exception"] = "TransactionException.SENDER_NOT_EOA"
                case["expected_post"] = {}
                valid = False
            print(f"reference {fork}/{name}", flush=True)
            inp, out = cast(Any, build_guest)(case)
            assert inp[:2] == bytes((index + 10, 1))
            assert bool(out[32]) == valid, (fork, name)
            result[f"{fork}/{name}"] = {
                "blocks": [
                    {
                        "statelessInputBytes": "0x" + inp.hex(),
                        "statelessOutputBytes": "0x" + out.hex(),
                    }
                ]
            }
    for transition in SUPPORTED_TRANSITIONS:
        for boundary in (False, True):
            fixture = fixture_case(LOWER, {LOWER: bytes.fromhex("600160005500")})
            env = cast(dict[str, Any], fixture["env"]).copy()
            tx = cast(dict[str, Any], fixture["transaction"])
            if transition == "BerlinToLondonAt5":
                env.pop("currentRandom")
                env["currentDifficulty"] = "0x020000"
                env["currentNumber"] = "0x05" if boundary else "0x04"
                if boundary:
                    env["currentBaseFee"] = "0x3b9aca00"
                    tx["gasPrice"] = "0x77359400"
                else:
                    env.pop("currentBaseFee")
            else:
                env["currentTimestamp"] = hex(15000 if boundary else 14999)
            case = {
                "fork": transition,
                "env": env,
                "pre": fixture["pre"],
                "tx": tx,
                "idx": {"gas": 0, "data": 0, "value": 0},
                "config": fixture["config"],
                "expected_post": {f"0x{LOWER:040x}": {"storage": {"0x00": "0x01"}}},
            }
            name = f"{transition}/{'at' if boundary else 'before'}"
            print(f"reference {name}", flush=True)
            inp, out = cast(Any, build_guest)(case)
            assert out[32] == 1, name
            result[name] = {
                "blocks": [
                    {
                        "statelessInputBytes": "0x" + inp.hex(),
                        "statelessOutputBytes": "0x" + out.hex(),
                    }
                ]
            }
    raw = Bytes.fromhex(result["Berlin/sstore"]["blocks"][0]["statelessInputBytes"][2:])
    value = StatelessInput.decode_bytes(raw[2:])
    request = value.new_payload_request
    entries = rlp.decode(request.execution_payload.block_access_list)
    lower = LOWER.to_bytes(20, "big")
    spare = bytes.fromhex("90" + "00" * 19)
    changes = []
    changes.append(
        ("missing-account", [entry for entry in entries if entry[0] != lower], value.witness.state)
    )
    changed = deepcopy(entries)
    next(entry for entry in changed if entry[0] == lower)[1] = []
    changes.append(("missing-slot", changed, value.witness.state))
    changes.append(("duplicate-account", [*entries, entries[-1]], value.witness.state))
    changes.append(("reordered-accounts", list(reversed(entries)), value.witness.state))
    changed = deepcopy(entries)
    next(entry for entry in changed if entry[0] == lower)[1][0][1][0] = [b"\x01", b""]
    changes.append(("malformed-marker", changed, value.witness.state))
    storage_leaf = None
    account_leaf = None
    for node in value.witness.state:
        fields = rlp.decode(node)
        if len(fields) != 2 or not fields[0] or not (fields[0][0] & 0x20):
            continue
        if fields[1] == b"\x01":
            storage_leaf = node
        decoded = rlp.decode(fields[1])
        if isinstance(decoded, list) and len(decoded) == 4 and decoded[1] == b"\x01":
            account_leaf = node
    assert storage_leaf is not None and account_leaf is not None
    changes.append(
        (
            "unused-account-proof-omitted",
            entries,
            tuple(node for node in value.witness.state if node != account_leaf),
        )
    )
    changed = deepcopy(entries)
    next(entry for entry in changed if entry[0] == spare)[1] = [[b"\x01", [[b"", b""]]]]
    changes.append(
        (
            "unused-slot-proof-omitted",
            changed,
            tuple(node for node in value.witness.state if node != storage_leaf),
        )
    )
    for name, access_schema, nodes in changes:
        altered_request = replace(
            request,
            execution_payload=replace(
                request.execution_payload, block_access_list=Bytes(rlp.encode(access_schema))
            ),
        )
        altered = replace(
            value, new_payload_request=altered_request, witness=replace(value.witness, state=nodes)
        )
        out = StatelessValidationResult(
            new_payload_request_root=altered_request.hash_tree_root(),
            successful_validation=False,
            chain_id=value.chain_id,
            schema_id=U16(0x0A01),
        )
        result[f"historical-schema/{name}"] = {
            "blocks": [
                {
                    "statelessInputBytes": "0x" + (raw[:2] + altered.encode_bytes()).hex(),
                    "statelessOutputBytes": "0x" + out.encode_bytes().hex(),
                }
            ]
        }
    raw = Bytes.fromhex(result["Berlin/precompile-balance"]["blocks"][0]["statelessInputBytes"][2:])
    value = StatelessInput.decode_bytes(raw[2:])
    request = value.new_payload_request
    entries = rlp.decode(request.execution_payload.block_access_list)
    request = replace(
        request,
        execution_payload=replace(
            request.execution_payload,
            block_access_list=Bytes(
                rlp.encode([entry for entry in entries if entry[0] != (1).to_bytes(20, "big")])
            ),
        ),
    )
    value = replace(value, new_payload_request=request)
    out = StatelessValidationResult(
        new_payload_request_root=request.hash_tree_root(),
        successful_validation=False,
        chain_id=value.chain_id,
        schema_id=U16(0x0A01),
    )
    result["historical-schema/missing-precompile-account"] = {
        "blocks": [
            {
                "statelessInputBytes": "0x" + (raw[:2] + value.encode_bytes()).hex(),
                "statelessOutputBytes": "0x" + out.encode_bytes().hex(),
            }
        ]
    }
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference-output", type=Path)
    parser.add_argument("--build", action="append", choices=("standard", "optimized"))
    parser.add_argument("--rebuild", action="store_true")
    args = parser.parse_args()
    if args.reference_output:
        args.reference_output.write_text(json.dumps(reference_pairs()) + "\n")
        return 0
    environment = harness_environment()
    with tempfile.TemporaryDirectory(prefix="fork-matrix.") as directory:
        path = Path(directory) / "pairs.json"
        subprocess.run(
            [
                environment["EXECSPECS_PY"],
                "-m",
                "devtools.tests.test_historical_forks",
                "--reference-output",
                str(path),
            ],
            cwd=ROOT,
            env=environment,
            check=True,
        )
        for backend in args.build or ("standard", "optimized"):
            command = [
                sys.executable,
                "-m",
                "devtools.harness.cli",
                "--build",
                backend,
                "--quiet",
                "--debug",
                str(path),
            ]
            if args.rebuild:
                command.append("--rebuild")
            subprocess.run(command, cwd=ROOT, env=environment, check=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
