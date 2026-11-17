"""Pinned-reference storage witness mutations for the stateless boundary suite."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import replace
from typing import Any, cast


def storage_cases(add: Callable[..., None], cases: list[dict[str, Any]]) -> None:
    from ethereum.crypto.hash import Hash32, keccak256
    from ethereum.forks.amsterdam.stateless_guest import deserialize_stateless_input
    from ethereum_rlp import rlp
    from ethereum_types.bytes import Bytes

    from devtools.harness.ssz_builder import build_guest
    from devtools.tests.test_memory_frames import LOWER, fixture_case, push

    spare = 0x9000000000000000000000000000000000000000
    code = push(spare, 20) + b"\x31\x50" + push(0) + b"\x54" + push(1) + b"\x55\x00"
    fixture = fixture_case(LOWER, {LOWER: code})
    pre = cast(dict[str, Any], fixture["pre"])
    pre[f"0x{LOWER:040x}"]["storage"] = {"0x00": "0x01"}
    pre[f"0x{spare:040x}"] = {
        "nonce": "0x00",
        "balance": "0x01",
        "code": "0x",
        "storage": {"0x01": "0x01"},
    }
    inp, _ = cast(Any, build_guest)(
        {
            "fork": "Amsterdam",
            "env": fixture["env"],
            "pre": pre,
            "tx": fixture["transaction"],
            "idx": {"gas": 0, "data": 0, "value": 0},
            "config": fixture["config"],
            "expected_post": {f"0x{LOWER:040x}": {"storage": {"0x00": "0x01", "0x01": "0x01"}}},
        }
    )
    control = deserialize_stateless_input(Bytes(inp))
    add("storage-witness/control", control)

    def nibbles(data: bytes) -> list[int]:
        return [n for byte in data for n in (byte >> 4, byte & 15)]

    def hp(path: list[int], leaf: bool = True) -> bytes:
        flag = (2 if leaf else 0) + len(path) % 2
        values = [flag, *path] if len(path) % 2 else [flag, 0, *path]
        return bytes((values[i] << 4) | values[i + 1] for i in range(0, len(values), 2))

    path = nibbles(keccak256(b"\0" * 32))
    correct_leaf = Bytes(rlp.encode([hp(path), b"\x01"]))
    child = Bytes(rlp.encode([hp(path[1:]), b"\x01"]))
    short_sibling = Bytes(rlp.encode([hp(path[2:]), b"\x02"]))
    branch = [b""] * 17
    branch[path[0]] = keccak256(child)
    with_sibling = list(branch)
    with_sibling[(path[0] + 1) % 16] = keccak256(short_sibling)
    extension = Bytes(rlp.encode([hp(path[:1], False), keccak256(child)]))

    def mutated(root: bytes, descendants: tuple[bytes, ...], unused: bool = False) -> Any:
        old_root = keccak256(correct_leaf)
        if unused:
            old_root = keccak256(
                rlp.encode([hp(nibbles(keccak256((1).to_bytes(32, "big")))), b"\x01"])
            )
        database = {keccak256(node): node for node in control.witness.state}
        rewritten = []
        found = False

        def reference(ref: Any) -> Any:
            if isinstance(ref, list):
                return rlp.decode(rewrite(Bytes(rlp.encode(ref))))
            if len(ref) == 32 and ref in database:
                return keccak256(rewrite(database[ref]))
            return ref

        def rewrite(encoded: bytes) -> Bytes:
            nonlocal found
            fields = rlp.decode(Bytes(encoded))
            if len(fields) == 17:
                fields[:16] = [reference(ref) for ref in fields[:16]]
            elif fields[0][0] & 0x20:
                account = rlp.decode(fields[1])
                if isinstance(account, list) and len(account) == 4 and account[2] == old_root:
                    account[2] = keccak256(root)
                    fields[1] = rlp.encode(account)
                    found = True
            else:
                fields[1] = reference(fields[1])
            result = Bytes(rlp.encode(fields))
            rewritten.append(result)
            return result

        headers = list(control.witness.headers)
        parent = rlp.decode(headers[-1])
        parent[3] = keccak256(rewrite(database[parent[3]]))
        assert found, "touched account proof missing from reference fixture"
        headers[-1] = Bytes(rlp.encode(parent))
        request = control.new_payload_request
        return replace(
            control,
            witness=replace(
                control.witness,
                headers=tuple(headers),
                state=tuple(
                    dict.fromkeys(
                        [*control.witness.state, *rewritten, Bytes(root), *map(Bytes, descendants)]
                    )
                ),
            ),
            new_payload_request=replace(
                request,
                execution_payload=replace(
                    request.execution_payload, parent_hash=Hash32(keccak256(headers[-1]))
                ),
            ),
        )

    mutations = (
        ("short-leaf", rlp.encode([hp(path[1:]), b"\x01"]), ()),
        ("overlong-leaf", rlp.encode([hp([0, *path]), b"\x01"]), ()),
        ("list-valued-leaf", rlp.encode([hp(path), [b"\x01"]]), ()),
        ("extension-to-leaf", extension, (child,)),
        ("single-child-branch", rlp.encode(branch), (child,)),
        ("malformed-supplied-sibling", rlp.encode(with_sibling), (child, short_sibling)),
    )
    for name, root, descendants in mutations:
        add(f"storage-witness/{name}", mutated(root, descendants))
        assert bytes.fromhex(cases[-1]["output"])[32] == 0
        cases[-1]["allowed_reasons"] = ["WitnessDeficient", "RlpDecode"]
    # The same malformed storage material under an account whose balance alone
    # is read stays lazy. The changed parent hash also changes the EIP-2935
    # system write, so the original BAL fails authentication before the old
    # claimed post-state root is checked. A storage decoder error here would
    # reveal eager validation of an unused storage subtree.
    add("storage-witness/unused-subtree-stays-lazy", mutated(mutations[0][1], (), unused=True))
    assert bytes.fromhex(cases[-1]["output"])[32] == 0
    cases[-1]["reason"] = "InvalidBlockAccessList"
