#!/usr/bin/env python3
"""Synthesize v21 stateless pairs with the pinned EELS blockchain filler.

Run under the execution-specs test environment. EELS owns predeploys, parent
headers, signing, and execution. Amsterdam returns upstream stateless bytes.
Historical transport uses the same SSZ containers with an explicit fork and
an uncommitted access schema; its execution verdict comes from that fork's
blockchain filler, with no manufactured post-state or validity outcome.
"""

import json
import os
import subprocess
import sys
from contextlib import redirect_stdout
from pathlib import Path
from typing import Any

from devtools.paths import REPO_ROOT

EELS_REVISION = "59c1473ddfba341babaeca1c97a50e996f639a0b"
EXECSPECS_ROOT = Path(os.environ.get("EXECSPECS_ROOT", REPO_ROOT.parent / "execution-specs"))
_verified = False
SUPPORTED_FORKS = (
    "Berlin",
    "London",
    "ArrowGlacier",
    "GrayGlacier",
    "Paris",
    "Shanghai",
    "Cancun",
    "Prague",
    "Osaka",
    "BPO1",
    "BPO2",
    "Amsterdam",
)
SUPPORTED_TRANSITIONS = (
    "BerlinToLondonAt5",
    "ParisToShanghaiAtTime15k",
    "ShanghaiToCancunAtTime15k",
    "CancunToPragueAtTime15k",
    "PragueToOsakaAtTime15k",
    "OsakaToBPO1AtTime15k",
    "BPO1ToBPO2AtTime15k",
    "BPO2ToAmsterdamAtTime15k",
)


def _verify_source():
    global _verified
    if _verified:
        return
    revision = subprocess.check_output(
        ["git", "-C", str(EXECSPECS_ROOT), "rev-parse", "HEAD"], text=True
    ).strip()
    if revision != EELS_REVISION:
        raise RuntimeError(f"execution-specs must be {EELS_REVISION}, got {revision}")
    import ethereum
    import execution_testing

    for module in (ethereum, execution_testing):
        if not Path(module.__file__).resolve().is_relative_to(EXECSPECS_ROOT.resolve()):
            raise RuntimeError(f"{module.__name__} is not loaded from pinned execution-specs")
    _verified = True


def _chain_id(case: dict[str, Any]) -> int:
    value = case.get("config", {}).get("chainid", case["env"].get("currentChainId", 1))
    return int(value, 16) if isinstance(value, str) else int(value)


def _signed_transaction(encoded: str) -> dict[str, Any]:
    from ethereum_rlp import rlp
    from ethereum_types.bytes import Bytes
    from execution_testing import Transaction

    raw = bytes.fromhex(encoded.removeprefix("0x"))
    if not raw:
        raise ValueError("empty transaction envelope")
    kind = 0 if raw[0] >= 0xC0 else raw[0]
    fields = rlp.decode(Bytes(raw if kind == 0 else raw[1:]))
    arities = {0: 9, 1: 11, 2: 12, 3: 14, 4: 13}
    if kind not in arities or not isinstance(fields, list) or len(fields) != arities[kind]:
        raise ValueError("unsupported or malformed signed transaction shape")

    def quantity(i):
        return int.from_bytes(fields[i], "big")

    values = {
        "type": kind,
        "v": quantity(len(fields) - 3),
        "r": quantity(len(fields) - 2),
        "s": quantity(len(fields) - 1),
    }
    if kind == 0:
        values.update(
            nonce=quantity(0),
            gasPrice=quantity(1),
            gasLimit=quantity(2),
            to=fields[3] or None,
            value=quantity(4),
            data=fields[5],
        )
        values["protected"] = values["v"] >= 35
        if values["protected"]:
            values["chainId"] = (values["v"] - 35) // 2
    else:
        values.update(chainId=quantity(0), nonce=quantity(1))
        if kind == 1:
            values["gasPrice"] = quantity(2)
            gas_index = 3
        else:
            values.update(maxPriorityFeePerGas=quantity(2), maxFeePerGas=quantity(3))
            gas_index = 4
        values.update(
            gasLimit=quantity(gas_index),
            to=fields[gas_index + 1] or None,
            value=quantity(gas_index + 2),
            data=fields[gas_index + 3],
        )
        values["accessList"] = [
            {"address": item[0], "storageKeys": item[1]} for item in fields[gas_index + 4]
        ]
        if kind == 3:
            values.update(maxFeePerBlobGas=quantity(9), blobVersionedHashes=fields[10])
        if kind == 4:
            values["authorizationList"] = [
                {
                    "chainId": int.from_bytes(a[0], "big"),
                    "address": a[1],
                    "nonce": int.from_bytes(a[2], "big"),
                    "v": int.from_bytes(a[3], "big"),
                    "r": int.from_bytes(a[4], "big"),
                    "s": int.from_bytes(a[5], "big"),
                }
                for a in fields[9]
            ]
    transaction = Transaction.model_validate(values)
    if bytes(transaction.rlp()) != raw:
        raise ValueError("signed transaction cannot be represented without changing its envelope")
    return values


def _transaction(case: dict[str, Any], chain_id: int) -> dict[str, Any]:
    if case.get("txbytes"):
        transaction = _signed_transaction(case["txbytes"])
        if case["tx"].get("sender"):
            # Reference-filler metadata only. The raw envelope is unchanged,
            # and the Sail guest always recovers its sender independently.
            # Supplying this avoids EELS's signed-transaction copy dropping
            # the excluded expected-error field during sender preparation.
            transaction["sender"] = case["tx"]["sender"]
        return transaction
    tx, idx = case["tx"], case["idx"]
    values = {
        "nonce": tx["nonce"],
        "gasLimit": tx["gasLimit"][idx["gas"]],
        "data": tx["data"][idx["data"]],
        "to": tx.get("to") or None,
        "value": tx["value"][idx["value"]],
        "chainId": chain_id,
    }
    for key in (
        "protected",
        "gasPrice",
        "maxFeePerGas",
        "maxPriorityFeePerGas",
        "maxFeePerBlobGas",
        "blobVersionedHashes",
        "authorizationList",
    ):
        if key in tx:
            values[key] = tx[key]
    if "accessLists" in tx:
        values["accessList"] = tx["accessLists"][idx["data"]]
    values["secretKey"] = tx["secretKey"]
    return values


def _historical_pair(fixture, reference, chain_id, fork_name, accesses):
    """Use v21 containers with an explicit historical execution-fork ID.

    The fork-correct EELS blockchain filler authenticates execution and header
    commitments. Its pre-state supplies complete witnesses using the pinned
    EELS MPT implementation; Amsterdam's guest is never used as a historical
    execution oracle.
    """
    from ethereum.crypto.hash import Hash32, keccak256
    from ethereum.forks.amsterdam.blocks import Withdrawal
    from ethereum.forks.amsterdam.execution_engine.requests import decode_execution_requests
    from ethereum.forks.amsterdam.execution_engine.types import ExecutionPayload, NewPayloadRequest
    from ethereum.forks.amsterdam.fork_types import Bloom
    from ethereum.forks.amsterdam.incremental_mpt import build_mpt, mpt_get, mpt_root
    from ethereum.forks.amsterdam.stateless import (
        ExecutionWitness,
        ProtocolFork,
        StatelessInput,
        StatelessValidationResult,
    )
    from ethereum.merkle_patricia_trie import EMPTY_TRIE_ROOT
    from ethereum.state import Address, Root
    from ethereum_rlp import rlp
    from ethereum_types.bytes import Bytes, Bytes32
    from ethereum_types.numeric import U16, U64, U256, Uint

    state = fixture.pre._materialize_state()
    storages = {
        address: build_mpt(trie._data, secured=True, default=U256(0))
        for address, trie in state._storage_tries.items()
    }
    for address, trie in storages.items():
        mpt_root(trie)
        for key in state._storage_tries[address]._data:
            mpt_get(trie, key)
    accounts = build_mpt(
        state._main_trie._data,
        secured=True,
        default=None,
        get_storage_root=lambda address: (
            mpt_root(storages[address]) if address in storages else EMPTY_TRIE_ROOT
        ),
    )
    if bytes(mpt_root(accounts)) != bytes(rlp.decode(Bytes(bytes(fixture.genesis_rlp)))[0][3]):
        raise RuntimeError("historical witness root disagrees with EELS genesis")
    for address in state._main_trie._data:
        mpt_get(accounts, address)
    nodes = dict(accounts.witness.accessed_nodes)
    for trie in storages.values():
        nodes.update(trie.witness.accessed_nodes)
    block = fixture.blocks[0]
    encoded_block = rlp.decode(Bytes(bytes(block.rlp)))
    header = encoded_block[0]

    def number(field):
        return int.from_bytes(header[field], "big")

    fork_id = int(ProtocolFork[fork_name])
    if fork_id < 0x0E and (
        header[13] != b"\0" * 32
        or header[14] != b"\0" * 8
        or keccak256(rlp.encode(())) != header[1]
    ):
        raise ValueError("historical PoW transport requires zero mix-hash/nonce and no ommers")
    transactions = tuple(
        Bytes(tx if isinstance(tx, bytes) else rlp.encode(tx)) for tx in encoded_block[1]
    )
    withdrawals = tuple(
        Withdrawal(
            U64(int.from_bytes(w[0], "big")),
            U64(int.from_bytes(w[1], "big")),
            Address(w[2]),
            U64(int.from_bytes(w[3], "big")),
        )
        for w in (encoded_block[3] if len(encoded_block) > 3 else ())
    )
    payload = ExecutionPayload(
        parent_hash=Hash32(header[0]),
        fee_recipient=Address(header[2]),
        state_root=Root(header[3]),
        receipts_root=Root(header[5]),
        logs_bloom=Bloom(header[6]),
        prev_randao=Bytes32(header[13] if fork_id >= 0x0E else number(7).to_bytes(32, "big")),
        block_number=Uint(number(8)),
        gas_limit=Uint(number(9)),
        gas_used=Uint(number(10)),
        timestamp=U256(number(11)),
        extra_data=Bytes(header[12]),
        base_fee_per_gas=Uint(number(15) if fork_id >= 0x0B else 0),
        block_hash=Hash32(keccak256(rlp.encode(header))),
        transactions=transactions,
        withdrawals=withdrawals,
        blob_gas_used=U64(number(17) if fork_id >= 0x10 else 0),
        excess_blob_gas=U64(number(18) if fork_id >= 0x10 else 0),
        # Historical selectors use this otherwise inactive transport field
        # as an uncommitted access schema. Zero markers reserve writable
        # slots; they are not historical EIP-7928 change claims.
        block_access_list=Bytes(
            rlp.encode(
                [
                    [
                        address,
                        [
                            [U256(int.from_bytes(slot, "big")), [[U64(0), U256(0)]]]
                            for slot in sorted(slots)
                        ],
                        [],
                        [],
                        [],
                        [],
                    ]
                    for address, slots in sorted(accesses.items())
                ]
            )
        ),
        slot_number=U64(0),
    )
    versioned_hashes: list[bytes] = []
    for raw in transactions:
        # The reference may intentionally reject an otherwise canonical typed
        # envelope (for example, a blob contract-creation transaction). Do not
        # require Amsterdam's executable transaction type merely to retain its
        # ordered versioned-hash commitment in historical transport.
        if fork_id >= 0x10 and raw[:1] == b"\x03":
            fields = rlp.decode(Bytes(raw[1:]))
            versioned_hashes.extend(Hash32(value) for value in fields[10])
    requests = decode_execution_requests(tuple(Bytes(bytes(r)) for r in (reference.requests or ())))
    request = NewPayloadRequest(
        execution_payload=payload,
        versioned_hashes=tuple(versioned_hashes),
        parent_beacon_block_root=Root(header[19] if fork_id >= 0x10 else b"\0" * 32),
        execution_requests=requests,
    )
    witness = ExecutionWitness(
        state=tuple(Bytes(node) for node in sorted(nodes.values())),
        codes=tuple(
            sorted(
                {
                    Bytes(bytes(a.code))
                    for a in fixture.pre.root.values()
                    if a is not None and a.code
                }
            )
        ),
        headers=(Bytes(rlp.encode(rlp.decode(Bytes(bytes(fixture.genesis_rlp)))[0])),),
    )
    schema = (fork_id << 8) | 1
    inp = StatelessInput(new_payload_request=request, witness=witness, chain_id=U64(chain_id))
    successful = not reference.rejected_transactions and reference.block_exception is None
    out = StatelessValidationResult(
        new_payload_request_root=request.hash_tree_root(),
        successful_validation=successful,
        chain_id=U64(chain_id),
        schema_id=U16(schema),
    )
    return schema.to_bytes(2, "big") + bytes(inp.encode_bytes()), bytes(out.encode_bytes())


def build_guest(case):
    """Return upstream Amsterdam bytes or the declared historical transport."""
    _verify_source()
    from execution_testing import Account, Address, Alloc, Environment, Transaction, forks
    from execution_testing.client_clis.clis.execution_specs import ExecutionSpecsTransitionTool
    from execution_testing.fixtures.blockchain import BlockchainFixture
    from execution_testing.fixtures.common import FixtureTransactionReceipt
    from execution_testing.fixtures.state import StateFixture
    from execution_testing.specs.state import StateTest

    if case["fork"] not in (*SUPPORTED_FORKS, *SUPPORTED_TRANSITIONS):
        raise ValueError(f"unsupported execution fork: {case['fork']}")
    fork = getattr(forks, case["fork"])
    environment = Environment.model_validate(case["env"])
    active_fork = fork.fork_at(
        block_number=int(environment.number), timestamp=int(environment.timestamp)
    ).__name__
    chain_id = _chain_id(case)
    transaction = _transaction(case, chain_id)
    if case.get("expected_exception"):
        from execution_testing.exceptions import ExceptionBase

        expected = case["expected_exception"]
        names = expected if isinstance(expected, list) else expected.split("|")
        parsed = [ExceptionBase.from_str(name) for name in names]
        transaction["error"] = parsed[0] if len(parsed) == 1 else parsed
    test = StateTest(
        fork=fork,
        env=environment,
        pre=Alloc.model_validate(case["pre"]),
        post=Alloc.model_validate(case.get("expected_post", {})),
        tx=Transaction.model_validate(transaction),
        chain_id=chain_id,
        is_exception_test=bool(case.get("expected_exception")),
    )

    class RecordingTool(ExecutionSpecsTransitionTool):
        def _evaluate(self, **kwargs):
            from functools import wraps

            from execution_testing import Address

            self.accesses: dict[bytes, set[bytes]] = {
                bytes(Address(address)): set() for address in case["pre"]
            }
            self.accesses.setdefault(bytes(Address(case["env"]["currentCoinbase"])), set())
            self.writes: set[bytes] = set()
            for entry in transaction.get("accessList", ()):
                slots = self.accesses.setdefault(bytes(Address(entry["address"])), set())
                slots.update(
                    (
                        int(s, 16) if isinstance(s, str) else int.from_bytes(bytes(s), "big")
                    ).to_bytes(32, "big")
                    for s in entry.get("storageKeys", ())
                )
            original_get = self.fork_cache.get
            replacements = []

            def instrument(function, storage, write):
                @wraps(function)
                def record(*args, **kw):
                    address = args[1] if len(args) > 1 else kw.get("address")
                    if address is not None:
                        slots = self.accesses.setdefault(bytes(address), set())
                        if write:
                            self.writes.add(bytes(address))
                        if storage:
                            key = args[2] if len(args) > 2 else kw["key"]
                            slots.add(bytes(key))
                    return function(*args, **kw)

                return record

            def tracked_get(*args, **kw):
                hardfork = original_get(*args, **kw)
                hardfork.module("fork")
                tracker = hardfork.module("state_tracker")
                functions = {}
                for name in (
                    "get_account_optional",
                    "get_account",
                    "set_account",
                    "get_storage",
                    "get_storage_original",
                    "set_storage",
                    "set_account_balance",
                    "set_code",
                    "destroy_account",
                    "increment_nonce",
                    "create_ether",
                ):
                    function = getattr(tracker, name, None)
                    if function is not None:
                        functions[function] = instrument(
                            function,
                            name in ("get_storage", "get_storage_original", "set_storage"),
                            not name.startswith("get_"),
                        )
                interpreter = hardfork.module("vm.interpreter")
                process_message = getattr(interpreter, "process_message", None)

                if process_message is not None:

                    @wraps(process_message)
                    def record_message(*args, **kw):
                        evm = process_message(*args, **kw)
                        # A call target can become warm before a balance/depth
                        # failure without its account being read. Retain these
                        # accesses too; reservations are not state writes.
                        for address in getattr(evm, "accessed_addresses", ()):
                            self.accesses.setdefault(bytes(address), set())
                        for address, key in getattr(evm, "accessed_storage_keys", ()):
                            slot = key.to_bytes(32, "big") if isinstance(key, int) else bytes(key)
                            self.accesses.setdefault(bytes(address), set()).add(slot)
                        return evm

                    functions[process_message] = record_message
                for module_name, module in tuple(sys.modules.items()):
                    if module is not None and module_name.startswith(hardfork.mod.__name__ + "."):
                        for name, value in tuple(vars(module).items()):
                            if callable(value) and value in functions:
                                replacements.append((module, name, value))
                                setattr(module, name, functions[value])
                for address in hardfork.module(
                    "vm.precompiled_contracts.mapping"
                ).PRE_COMPILED_CONTRACTS:
                    self.accesses.setdefault(bytes(address), set())
                return hardfork

            self.fork_cache.get = tracked_get
            try:
                output = super()._evaluate(**kwargs)
                self.reference = output.result
                return output
            finally:
                self.fork_cache.get = original_get
                for module, name, value in reversed(replacements):
                    setattr(module, name, value)

    tool = RecordingTool()
    try:
        block_test = test
        state_commitments = None
        if (
            case.get("expected_state_root")
            or case.get("expected_logs_hash")
            or case.get("expected_receipt")
        ):
            state_fixture = test.generate(tool, StateFixture).fixture
            posts = [entry for entries in state_fixture.post.values() for entry in entries]
            if len(posts) != 1:
                raise RuntimeError("state-test synthesis must produce one indexed state result")
            for key, actual in (
                ("expected_state_root", posts[0].state_root),
                ("expected_logs_hash", posts[0].logs_hash),
            ):
                expected = case.get(key)
                if expected and bytes(actual).hex() != expected.removeprefix("0x").lower():
                    raise ValueError(
                        f"{key} differs from the pinned reference: expected {expected}, got {actual}"
                    )
            if case.get("expected_receipt") is not None:
                expected_receipt = FixtureTransactionReceipt.model_validate(
                    case["expected_receipt"]
                )
                if posts[0].receipt != expected_receipt:
                    raise ValueError("state execution differs from the original fixture receipt")
            state_commitments = (
                bytes(tool.reference.logs_hash),
                bytes(tool.reference.receipts_root),
            )
            # Preserve transaction postconditions across transport conversion.
            # Untouched protocol predeploys may acquire block-system effects;
            # all transaction writes/deletions remain asserted, and a mining
            # reward is added explicitly to the original coinbase balance.
            # Observable context changes therefore fail synthesis.
            post = posts[0].state.model_copy(deep=True)
            for address in getattr(forks, active_fork).pre_allocation():
                if bytes(Address(address)) not in tool.writes:
                    post.root.pop(Address(address), None)
            for address in tool.writes:
                post.root.setdefault(Address(address), None)
            reward = getattr(forks, active_fork).get_reward()
            if reward and not case.get("expected_exception"):
                coinbase = environment.fee_recipient
                account = post.root.get(coinbase)
                if account is None:
                    post.root[coinbase] = Account(balance=reward, nonce=0, code="0x", storage={})
                else:
                    post.root[coinbase] = account.model_copy(
                        update={"balance": int(account.balance or 0) + reward}
                    )
            block_test = test.model_copy(update={"post": post})
        if case["fork"] in SUPPORTED_TRANSITIONS:
            blockchain = block_test.generate_blockchain_test()
            genesis = blockchain.genesis_environment
            parent_fork = fork.fork_at(block_number=int(environment.number) - 1, timestamp=0)
            if parent_fork.__name__ == "Berlin":
                updates: dict[str, Any] = {"base_fee_per_gas": None}
                if active_fork == "London":
                    if int(environment.base_fee_per_gas) != 1_000_000_000:
                        raise ValueError("London activation requires the EIP-1559 initial base fee")
                    updates["gas_limit"] = int(environment.gas_limit) // 2
                    blockchain.blocks[0].base_fee_per_gas = environment.base_fee_per_gas
                blockchain.genesis_environment = genesis.model_copy(update=updates)
            fixture = blockchain.generate(tool, BlockchainFixture).fixture
        else:
            fixture = block_test.generate(tool, BlockchainFixture).fixture
        if state_commitments is not None:
            if bytes(tool.reference.logs_hash) != state_commitments[0]:
                raise ValueError("converted block changes the original transaction logs")
            if bytes(tool.reference.receipts_root) != state_commitments[1]:
                raise ValueError("converted block changes the original transaction receipt")
        if len(fixture.blocks) != 1:
            raise RuntimeError("state-test synthesis must produce exactly one block")
        block = fixture.blocks[0]
        if active_fork != "Amsterdam":
            return _historical_pair(fixture, tool.reference, chain_id, active_fork, tool.accesses)
        if block.stateless_input_bytes is None or block.stateless_output_bytes is None:
            raise RuntimeError("EELS did not produce a stateless reference pair")
        return bytes(block.stateless_input_bytes), bytes(block.stateless_output_bytes)
    finally:
        tool.shutdown()


def _serve():
    for line in sys.stdin:
        if not line.strip():
            continue
        try:
            with redirect_stdout(sys.stderr):
                inp, exp = build_guest(json.loads(line))
            response = {"input": inp.hex(), "expected": exp.hex()}
        except Exception as error:
            response = {"err": f"{type(error).__name__}: {error}"}
        print(json.dumps(response), flush=True)


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--serve":
        _serve()
    else:
        inp, exp = build_guest(json.loads(Path(sys.argv[1]).read_text()))
        print(json.dumps({"input": inp.hex(), "expected": exp.hex()}))
