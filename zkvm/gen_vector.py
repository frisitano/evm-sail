#!/usr/bin/env python3
"""
S0 — Canonical stateless test-vector generator (host side).

Emits a schema-prefixed SSZ ``SszStatelessInput`` for the EL-IR fixture block
(the same 1-tx block zkvm_block.sail runs: contract `PUSH1 0x2a; PUSH1 0x00;
SSTORE; STOP`, plus a 500-wei withdrawal) together with a JSON sidecar of the
decoded scalar fields, so the Sail SSZ decoder (S1) can diff against it.

The SSZ containers mirror execution-specs (projects/zkevm) forks/amsterdam
stateless_ssz.py exactly (field order + types), so the bytes are wire-correct.
The pre-state is a real Ethereum secure-trie (py-trie): keys keccak(address),
values RLP([nonce,balance,storage_root,code_hash]); the trie's backing node db
is the witness `state`, and the trie root is the parent header's state_root.

Run:  ./.vecgen/bin/python gen_vector.py   (from el-ir/zkvm/)
Out:  vectors/fixture_block.ssz  +  vectors/fixture_block.json

NOTE: payload.state_root (POST-state) is a placeholder here (zeroed) — S1 only
decodes/echoes; the faithful post-state root + block_hash land in S3.
"""
import hashlib
import json
import os
import sys

import rlp
from eth_hash.auto import keccak
from remerkleable.basic import boolean, uint64, uint256
from remerkleable.byte_arrays import ByteList, ByteVector, Bytes32
from remerkleable.complex import Container
from remerkleable.complex import List as SszList
from trie import HexaryTrie

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "vectors")

EMPTY_TRIE_ROOT = bytes.fromhex(
    "56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421"
)
EMPTY_CODE_HASH = keccak(b"")
SCHEMA_ID = (0x0001).to_bytes(2, "big")

# --- SSZ list caps (mirror stateless_ssz.py) ---
MAX_EXTRA_DATA_BYTES = 32
MAX_BYTES_PER_TRANSACTION = 2**30
MAX_TRANSACTIONS_PER_PAYLOAD = 2**20
MAX_WITHDRAWALS_PER_PAYLOAD = 2**4
MAX_BLOB_COMMITMENTS_PER_BLOCK = 4096
MAX_DEPOSIT_REQUESTS_PER_PAYLOAD = 2**13
MAX_WITHDRAWAL_REQUESTS_PER_PAYLOAD = 2**4
MAX_CONSOLIDATION_REQUESTS_PER_PAYLOAD = 2**1
MAX_WITNESS_NODES = 2**22
MAX_WITNESS_CODES = 2**18
MAX_WITNESS_HEADERS = 256
MAX_BYTES_PER_CODE = 2**16
MAX_BYTES_PER_HEADER = 2**10
MAX_BYTES_PER_WITNESS_NODE = 2**10
MAX_OPTIONAL_FORK_ACTIVATION_VALUES = 1
MAX_BLOB_SCHEDULES_PER_FORK = 1
MAX_PUBLIC_KEYS = 2**15
PUBLIC_KEY_BYTES = 65


# --- SSZ containers (mirror forks/amsterdam/stateless_ssz.py) ---
class SszWithdrawal(Container):
    index: uint64
    validator_index: uint64
    address: ByteVector[20]
    amount: uint64


class SszExecutionPayload(Container):
    parent_hash: Bytes32
    fee_recipient: ByteVector[20]
    state_root: Bytes32
    receipts_root: Bytes32
    logs_bloom: ByteVector[256]
    prev_randao: Bytes32
    block_number: uint64
    gas_limit: uint64
    gas_used: uint64
    timestamp: uint64
    extra_data: ByteList[MAX_EXTRA_DATA_BYTES]
    base_fee_per_gas: uint256
    block_hash: Bytes32
    transactions: SszList[
        ByteList[MAX_BYTES_PER_TRANSACTION], MAX_TRANSACTIONS_PER_PAYLOAD
    ]
    withdrawals: SszList[SszWithdrawal, MAX_WITHDRAWALS_PER_PAYLOAD]
    blob_gas_used: uint64
    excess_blob_gas: uint64
    block_access_list: ByteList[MAX_BYTES_PER_TRANSACTION]
    slot_number: uint64


class SszDepositRequest(Container):
    pubkey: ByteVector[48]
    withdrawal_credentials: Bytes32
    amount: uint64
    signature: ByteVector[96]
    index: uint64


class SszWithdrawalRequest(Container):
    source_address: ByteVector[20]
    validator_pubkey: ByteVector[48]
    amount: uint64


class SszConsolidationRequest(Container):
    source_address: ByteVector[20]
    source_pubkey: ByteVector[48]
    target_pubkey: ByteVector[48]


class SszExecutionRequests(Container):
    deposits: SszList[SszDepositRequest, MAX_DEPOSIT_REQUESTS_PER_PAYLOAD]
    withdrawals: SszList[
        SszWithdrawalRequest, MAX_WITHDRAWAL_REQUESTS_PER_PAYLOAD
    ]
    consolidations: SszList[
        SszConsolidationRequest, MAX_CONSOLIDATION_REQUESTS_PER_PAYLOAD
    ]


class SszNewPayloadRequest(Container):
    execution_payload: SszExecutionPayload
    versioned_hashes: SszList[Bytes32, MAX_BLOB_COMMITMENTS_PER_BLOCK]
    parent_beacon_block_root: Bytes32
    execution_requests: SszExecutionRequests


class SszExecutionWitness(Container):
    state: SszList[ByteList[MAX_BYTES_PER_WITNESS_NODE], MAX_WITNESS_NODES]
    codes: SszList[ByteList[MAX_BYTES_PER_CODE], MAX_WITNESS_CODES]
    headers: SszList[ByteList[MAX_BYTES_PER_HEADER], MAX_WITNESS_HEADERS]


class SszForkActivation(Container):
    block_number: SszList[uint64, MAX_OPTIONAL_FORK_ACTIVATION_VALUES]
    timestamp: SszList[uint64, MAX_OPTIONAL_FORK_ACTIVATION_VALUES]


class SszBlobSchedule(Container):
    target: uint64
    max: uint64
    base_fee_update_fraction: uint64


class SszForkConfig(Container):
    fork: uint64
    activation: SszForkActivation
    blob_schedule: SszList[SszBlobSchedule, MAX_BLOB_SCHEDULES_PER_FORK]


class SszChainConfig(Container):
    chain_id: uint64
    active_fork: SszForkConfig


class SszStatelessInput(Container):
    new_payload_request: SszNewPayloadRequest
    witness: SszExecutionWitness
    chain_config: SszChainConfig
    public_keys: SszList[ByteVector[PUBLIC_KEY_BYTES], MAX_PUBLIC_KEYS]


class SszStatelessValidationResult(Container):
    new_payload_request_root: Bytes32
    successful_validation: boolean
    chain_config: SszChainConfig


# --- fixture (matches zkvm_block.sail) ---
# Sender is derived from public_keys (no ecrecover in the guest):
# address = keccak(pubkey_xy)[12:]. The pubkey is arbitrary (the guest trusts
# public_keys; it does not verify the signature), so any 0x04||XY works.
PUBKEY = bytes([0x04]) + bytes((i * 7 + 1) & 0xFF for i in range(64))
EOA = keccak(PUBKEY[1:])[-20:]   # tx sender = keccak(X||Y)[12:]
CONTRACT = bytes.fromhex("000000000000000000000000000000000000c0de")
COINBASE = bytes.fromhex("000000000000000000000000000000000000c01b")
WITHDRAWER = bytes.fromhex("0000000000000000000000000000000000004895")
CODE = bytes([0x60, 0x2A, 0x60, 0x00, 0x55, 0x00])
CHAIN_ID = 1
BLOCK_NUMBER = 21000000
TIMESTAMP = 1700000000
GAS_LIMIT = 30000000

# Amsterdam blob schedule (forks/amsterdam vm/gas.py)
BLOB_SCHEDULE_TARGET = 6
BLOB_SCHEDULE_MAX = 9
BLOB_BASE_FEE_UPDATE_FRACTION = 5007716


def account_rlp(nonce, balance, storage_root, code_hash):
    return rlp.encode([nonce, balance, storage_root, code_hash])


# Block access list (simplified encoding for the guest's access-set
# authentication; the full EIP-7928 RLP/SSZ encoding is a follow-on). Layout:
#   u32-LE count, then count * (address[20] ++ slot[32, big-endian]).
# The block touches one storage slot: contract slot 0 (the SSTORE).
def encode_bal(storage_keys):
    out = len(storage_keys).to_bytes(4, "little")
    for addr, slot in storage_keys:
        out += addr + slot.to_bytes(32, "big")
    return out


BAL_KEYS = [(CONTRACT, 0)]
BAL_BYTES = encode_bal(BAL_KEYS)


def build_prestate():
    """Real secure-trie pre-state for the two fixture accounts."""
    trie = HexaryTrie(db={})
    accounts = {
        EOA: (0, 10**18, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
        CONTRACT: (1, 0, EMPTY_TRIE_ROOT, keccak(CODE)),
    }
    for addr, (nonce, bal, sroot, chash) in accounts.items():
        trie[keccak(addr)] = account_rlp(nonce, bal, sroot, chash)
    node_db = dict(trie.db)  # keccak(node) -> node
    return trie.root_hash, list(node_db.values())


# Post-state of the fixture block (computed independently of the Sail model):
#   gas_used=43106, gas_price=2, base_fee=0 -> sender pays fee=86212, all tip to
#   coinbase. eoa nonce+1, contract storage[0]=42, withdrawer +500.
GAS_USED = 43106
FEE = GAS_USED * 2  # 86212


def build_poststate_root():
    """Real secure-trie root of the post-state (the payload.state_root)."""
    strie = HexaryTrie(db={})
    strie[keccak(b"\x00" * 32)] = rlp.encode(42)  # contract storage[0] = 42
    storage_root = strie.root_hash
    trie = HexaryTrie(db={})
    accounts = {
        EOA: (1, 10**18 - FEE, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
        CONTRACT: (1, 0, storage_root, keccak(CODE)),
        COINBASE: (0, FEE, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
        WITHDRAWER: (0, 500, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
    }
    for addr, (nonce, bal, sroot, chash) in accounts.items():
        trie[keccak(addr)] = account_rlp(nonce, bal, sroot, chash)
    return trie.root_hash


def legacy_tx_bytes():
    """RLP legacy tx: nonce0, gasPrice2, gas100000, to=CONTRACT, value0, data[],
    with placeholder signature (S1 doesn't recover keys)."""
    return rlp.encode([0, 2, 100000, CONTRACT, 0, b"", 27, 1, 1])


def parent_header_bytes(state_root):
    """A structurally-real RLP header whose state_root is the pre-state root and
    number = BLOCK_NUMBER-1. Field set kept simple for S0/S1; exact equivalence
    to the Amsterdam Header is refined when header decode lands (S2/S3)."""
    zero32 = b"\x00" * 32
    bloom = b"\x00" * 256
    return rlp.encode([
        zero32,            # parent_hash
        keccak(rlp.encode([])),  # ommers_hash (placeholder)
        COINBASE,          # coinbase
        state_root,        # state_root  <-- the pre-state root
        EMPTY_TRIE_ROOT,   # transactions_root
        EMPTY_TRIE_ROOT,   # receipt_root
        bloom,             # bloom
        0,                 # difficulty
        BLOCK_NUMBER - 1,  # number
        GAS_LIMIT,         # gas_limit
        0,                 # gas_used
        TIMESTAMP - 12,    # timestamp
        b"",               # extra_data
        zero32,            # prev_randao
        b"\x00" * 8,       # nonce
        0,                 # base_fee_per_gas
    ])


def main():
    os.makedirs(OUT, exist_ok=True)
    bad = "--bad" in sys.argv            # tamper the post-state root (fail path)
    name = "fixture_block_bad" if bad else "fixture_block"
    pre_root, witness_nodes = build_prestate()
    post_root = build_poststate_root()
    if bad:                              # corrupt payload.state_root so validation fails
        post_root = post_root[:-1] + bytes([post_root[-1] ^ 0xFF])

    payload = SszExecutionPayload(
        parent_hash=Bytes32(b"\x00" * 32),
        fee_recipient=ByteVector[20](COINBASE),
        state_root=Bytes32(post_root),  # POST-state root (S3)
        receipts_root=Bytes32(EMPTY_TRIE_ROOT),
        logs_bloom=ByteVector[256](b"\x00" * 256),
        prev_randao=Bytes32(b"\x00" * 32),
        block_number=uint64(BLOCK_NUMBER),
        gas_limit=uint64(GAS_LIMIT),
        gas_used=uint64(43106),
        timestamp=uint64(TIMESTAMP),
        extra_data=ByteList[MAX_EXTRA_DATA_BYTES](b""),
        base_fee_per_gas=uint256(0),
        block_hash=Bytes32(b"\x00" * 32),
        transactions=SszList[
            ByteList[MAX_BYTES_PER_TRANSACTION], MAX_TRANSACTIONS_PER_PAYLOAD
        ](ByteList[MAX_BYTES_PER_TRANSACTION](legacy_tx_bytes())),
        withdrawals=SszList[SszWithdrawal, MAX_WITHDRAWALS_PER_PAYLOAD](
            SszWithdrawal(
                index=uint64(0),
                validator_index=uint64(0),
                address=ByteVector[20](WITHDRAWER),
                amount=uint64(500),
            )
        ),
        blob_gas_used=uint64(0),
        excess_blob_gas=uint64(0),
        block_access_list=ByteList[MAX_BYTES_PER_TRANSACTION](BAL_BYTES),
        slot_number=uint64(0),
    )

    npr = SszNewPayloadRequest(
        execution_payload=payload,
        versioned_hashes=SszList[Bytes32, MAX_BLOB_COMMITMENTS_PER_BLOCK](),
        parent_beacon_block_root=Bytes32(b"\x00" * 32),
        execution_requests=SszExecutionRequests(),
    )

    witness = SszExecutionWitness(
        state=SszList[
            ByteList[MAX_BYTES_PER_WITNESS_NODE], MAX_WITNESS_NODES
        ](*[ByteList[MAX_BYTES_PER_WITNESS_NODE](n) for n in witness_nodes]),
        codes=SszList[ByteList[MAX_BYTES_PER_CODE], MAX_WITNESS_CODES](
            ByteList[MAX_BYTES_PER_CODE](CODE)
        ),
        headers=SszList[ByteList[MAX_BYTES_PER_HEADER], MAX_WITNESS_HEADERS](
            ByteList[MAX_BYTES_PER_HEADER](parent_header_bytes(pre_root))
        ),
    )

    chain_config = SszChainConfig(
        chain_id=uint64(CHAIN_ID),
        active_fork=SszForkConfig(
            fork=uint64(20),  # ProtocolFork index: Amsterdam (0-based, 21 forks)
            activation=SszForkActivation(
                block_number=SszList[uint64, 1](),
                timestamp=SszList[uint64, 1](uint64(0)),
            ),
            blob_schedule=SszList[SszBlobSchedule, 1](
                SszBlobSchedule(
                    target=uint64(BLOB_SCHEDULE_TARGET),
                    max=uint64(BLOB_SCHEDULE_MAX),
                    base_fee_update_fraction=uint64(
                        BLOB_BASE_FEE_UPDATE_FRACTION
                    ),
                )
            ),
        ),
    )

    inp = SszStatelessInput(
        new_payload_request=npr,
        witness=witness,
        chain_config=chain_config,
        public_keys=SszList[ByteVector[PUBLIC_KEY_BYTES], MAX_PUBLIC_KEYS](
            ByteVector[PUBLIC_KEY_BYTES](PUBKEY)),
    )

    body = inp.encode_bytes()
    blob = SCHEMA_ID + bytes(body)
    with open(os.path.join(OUT, name + ".ssz"), "wb") as f:
        f.write(blob)

    # Expected canonical output: SszStatelessValidationResult (no schema prefix).
    result = SszStatelessValidationResult(
        new_payload_request_root=Bytes32(npr.hash_tree_root()),
        successful_validation=boolean(not bad),
        chain_config=chain_config,
    )
    result_bytes = bytes(result.encode_bytes())

    sidecar = {
        "schema_id": 1,
        "total_bytes": len(blob),
        "pre_state_root": pre_root.hex(),
        "post_state_root": post_root.hex(),
        "witness": {
            "n_state_nodes": len(witness_nodes),
            "n_codes": 1,
            "n_headers": 1,
            "code0_keccak": keccak(CODE).hex(),
        },
        "payload": {
            "block_number": BLOCK_NUMBER,
            "gas_limit": GAS_LIMIT,
            "gas_used": 43106,
            "timestamp": TIMESTAMP,
            "fee_recipient": COINBASE.hex(),
            "base_fee_per_gas": 0,
            "n_transactions": 1,
            "n_withdrawals": 1,
            "withdrawal0": {"address": WITHDRAWER.hex(), "amount": 500},
        },
        "chain_config": {"chain_id": CHAIN_ID, "fork": 20},
        "htr": {
            "withdrawal0": payload.withdrawals[0].hash_tree_root().hex(),
            "execution_payload": payload.hash_tree_root().hex(),
            "new_payload_request": npr.hash_tree_root().hex(),
        },
        "result": {
            "len": len(result_bytes),
            "ssz": result_bytes.hex(),
            "sha256": hashlib.sha256(result_bytes).hexdigest(),
        },
    }
    with open(os.path.join(OUT, name + ".json"), "w") as f:
        json.dump(sidecar, f, indent=2)

    # round-trip self-check: decode the SSZ body back and assert key fields
    rt = SszStatelessInput.decode_bytes(body)
    assert int(rt.new_payload_request.execution_payload.gas_used) == 43106
    assert int(rt.chain_config.chain_id) == CHAIN_ID
    assert len(rt.witness.state) == len(witness_nodes)
    print(f"wrote vectors/fixture_block.ssz ({len(blob)} bytes), "
          f"pre_state_root={pre_root.hex()[:16]}.., "
          f"{len(witness_nodes)} witness nodes; round-trip OK")


if __name__ == "__main__":
    main()
