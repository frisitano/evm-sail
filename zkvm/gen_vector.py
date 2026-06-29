#!/usr/bin/env python3
"""
S0 — Canonical stateless test-vector generator (host side).

Emits a schema-prefixed SSZ ``SszStatelessInput`` for the evm-sail fixture block
(the same 1-tx block zkvm_block.sail runs: contract `PUSH1 0x2a; PUSH1 0x00;
SSTORE; STOP`, plus a 500-wei withdrawal) together with a JSON sidecar of the
decoded scalar fields, so the Sail SSZ decoder (S1) can diff against it.

The SSZ containers mirror execution-specs (projects/zkevm) forks/amsterdam
stateless_ssz.py exactly (field order + types), so the bytes are wire-correct.
The pre-state is a real Ethereum secure-trie (py-trie): keys keccak(address),
values RLP([nonce,balance,storage_root,code_hash]); the trie's backing node db
is the witness `state`, and the trie root is the parent header's state_root.

Run:  ./.vecgen/bin/python gen_vector.py   (from zkvm/)
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

# --- minimal pure-Python secp256k1 (fixtures only) --------------------------
# Real ECDSA signatures so the guest can AUTHENTICATE public_keys (it verifies,
# not recovers). Deterministic nonce k; low-s per EIP-2. Not for production use.
_SECP_P = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
_SECP_N = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
_SECP_G = (0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798,
           0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8)


def _pt_add(p, q):
    if p is None:
        return q
    if q is None:
        return p
    if p[0] == q[0] and (p[1] + q[1]) % _SECP_P == 0:
        return None
    if p == q:
        m = (3 * p[0] * p[0]) * pow(2 * p[1], -1, _SECP_P) % _SECP_P
    else:
        m = (q[1] - p[1]) * pow((q[0] - p[0]) % _SECP_P, -1, _SECP_P) % _SECP_P
    x = (m * m - p[0] - q[0]) % _SECP_P
    return (x, (m * (p[0] - x) - p[1]) % _SECP_P)


def _pt_mul(k, p):
    r = None
    while k:
        if k & 1:
            r = _pt_add(r, p)
        p = _pt_add(p, p)
        k >>= 1
    return r


def _pubkey(priv):
    return _pt_mul(priv, _SECP_G)


def _sign(priv, z):
    k = int.from_bytes(keccak(priv.to_bytes(32, "big") + z.to_bytes(32, "big")), "big") % _SECP_N or 1
    R = _pt_mul(k, _SECP_G)
    r = R[0] % _SECP_N
    s = (pow(k, -1, _SECP_N) * (z + r * priv)) % _SECP_N
    yp = R[1] & 1
    if s > _SECP_N // 2:           # EIP-2 low-s (flips parity)
        s, yp = _SECP_N - s, yp ^ 1
    return r, s, yp


def sign_tx(ttype, unsigned):
    """Sign `unsigned` (the RLP field list before v,r,s) with PRIVKEY and return
    the full tx bytes. Legacy (ttype 0) uses the EIP-155 signing scheme; typed
    envelopes prefix the type byte and sign type||rlp(unsigned)."""
    if ttype == 0:
        z = int.from_bytes(keccak(rlp.encode(unsigned + [CHAIN_ID, 0, 0])), "big")
        r, s, yp = _sign(PRIVKEY, z)
        return rlp.encode(unsigned + [CHAIN_ID * 2 + 35 + yp, r, s])
    z = int.from_bytes(keccak(bytes([ttype]) + rlp.encode(unsigned)), "big")
    r, s, yp = _sign(PRIVKEY, z)
    return bytes([ttype]) + rlp.encode(unsigned + [yp, r, s])


def sign_auth(priv, chain_id, address, nonce):
    """EIP-7702 authorization tuple signed by `priv`: the authority signs
    keccak(0x05 || rlp([chainId, address, nonce])). -> [chainId, address, nonce,
    yParity, r, s]."""
    z = int.from_bytes(keccak(b"\x05" + rlp.encode([chain_id, address, nonce])), "big")
    r, s, yp = _sign(priv, z)
    return [chain_id, address, nonce, yp, r, s]


def deleg_code_hash(target):
    """codeHash of the EIP-7702 delegation designation 0xef0100 || target."""
    return keccak(bytes([0xEF, 0x01, 0x00]) + target)

# --- SSZ list caps (mirror stateless_ssz.py) ---
MAX_EXTRA_DATA_BYTES = 32
MAX_BYTES_PER_TRANSACTION = 2**30
MAX_TRANSACTIONS_PER_PAYLOAD = 2**20
MAX_BLOCK_ACCESS_LIST_BYTES = 2**30  # EIP-7928 cap = MAX_BYTES_PER_TRANSACTION @ 02c6c2510 (was 2**24 in v0.4.1)
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
    block_access_list: ByteList[MAX_BLOCK_ACCESS_LIST_BYTES]
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


# --- fixture (matches main.sail) ---
# The guest AUTHENTICATES public_keys: it verifies each tx's signature against
# the supplied key, so the key must be the real signer and the tx must carry a
# valid signature. sender = keccak(pubkey_xy)[12:].
PRIVKEY = 0xA11CE5EEDC0FFEE  # throwaway fixture signing key
_PUB = _pubkey(PRIVKEY)
PUBKEY = bytes([0x04]) + _PUB[0].to_bytes(32, "big") + _PUB[1].to_bytes(32, "big")
EOA = keccak(PUBKEY[1:])[-20:]   # tx sender = keccak(X||Y)[12:]
# EIP-7702 authority: a second key whose address is RECOVERED from the auth tuple
AUTH_PRIVKEY = 0xB0BCAFE
_AUTHPUB = _pubkey(AUTH_PRIVKEY)
AUTHORITY = keccak(_AUTHPUB[0].to_bytes(32, "big") + _AUTHPUB[1].to_bytes(32, "big"))[-20:]
CONTRACT = bytes.fromhex("000000000000000000000000000000000000c0de")
COINBASE = bytes.fromhex("000000000000000000000000000000000000c01b")
WITHDRAWER = bytes.fromhex("0000000000000000000000000000000000004895")
# EIP-7002 withdrawal-request + EIP-7251 consolidation-request system predeploys.
# A valid Amsterdam block REQUIRES them: the protocol issues a CHECKED system call
# to each at block end, so a block without them is invalid. On an empty request
# queue the dequeue is a no-op (storage stays empty), so they appear UNCHANGED in
# both pre- and post-state. Real predeploy bytecode (from the EEST Amsterdam set).
WR_ADDR = bytes.fromhex("00000961ef480eb55e80d19ad83579a64c007002")
CR_ADDR = bytes.fromhex("0000bbddc7ce488642fb579f8b00f3a590007251")
WR_CODE = bytes.fromhex("3373fffffffffffffffffffffffffffffffffffffffe1460cb5760115f54807fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff146101f457600182026001905f5b5f82111560685781019083028483029004916001019190604d565b909390049250505036603814608857366101f457346101f4575f5260205ff35b34106101f457600154600101600155600354806003026004013381556001015f35815560010160203590553360601b5f5260385f601437604c5fa0600101600355005b6003546002548082038060101160df575060105b5f5b8181146101835782810160030260040181604c02815460601b8152601401816001015481526020019060020154807fffffffffffffffffffffffffffffffff00000000000000000000000000000000168252906010019060401c908160381c81600701538160301c81600601538160281c81600501538160201c81600401538160181c81600301538160101c81600201538160081c81600101535360010160e1565b910180921461019557906002556101a0565b90505f6002555f6003555b5f54807fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff14156101cd57505f5b6001546002828201116101e25750505f6101e8565b01600290035b5f555f600155604c025ff35b5f5ffd")
CR_CODE = bytes.fromhex("3373fffffffffffffffffffffffffffffffffffffffe1460d35760115f54807fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1461019a57600182026001905f5b5f82111560685781019083028483029004916001019190604d565b9093900492505050366060146088573661019a573461019a575f5260205ff35b341061019a57600154600101600155600354806004026004013381556001015f358155600101602035815560010160403590553360601b5f5260605f60143760745fa0600101600355005b6003546002548082038060021160e7575060025b5f5b8181146101295782810160040260040181607402815460601b815260140181600101548152602001816002015481526020019060030154905260010160e9565b910180921461013b5790600255610146565b90505f6002555f6003555b5f54807fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff141561017357505f5b6001546001828201116101885750505f61018e565b01600190035b5f555f6001556074025ff35b5f5ffd")
EOA2 = bytes.fromhex("00000000000000000000000000000000000022e0")  # typed-tx recipient
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
        WR_ADDR: (1, 0, EMPTY_TRIE_ROOT, keccak(WR_CODE)),   # EIP-7002 predeploy
        CR_ADDR: (1, 0, EMPTY_TRIE_ROOT, keccak(CR_CODE)),   # EIP-7251 predeploy
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
        WITHDRAWER: (0, 500 * 10**9, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),  # EIP-4895: amount 500 is Gwei -> wei
        WR_ADDR: (1, 0, EMPTY_TRIE_ROOT, keccak(WR_CODE)),   # EIP-7002: unchanged
        CR_ADDR: (1, 0, EMPTY_TRIE_ROOT, keccak(CR_CODE)),   # EIP-7251: unchanged
    }
    for addr, (nonce, bal, sroot, chash) in accounts.items():
        trie[keccak(addr)] = account_rlp(nonce, bal, sroot, chash)
    return trie.root_hash


def legacy_tx_bytes():
    """Signed legacy tx: nonce0, gasPrice2, gas100000, to=CONTRACT, value0, data[]."""
    return sign_tx(0, [0, 2, 100000, CONTRACT, 0, b""])


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


# --- Generalized witness builder (reused by the EEST witness-reroot harness) ---
# Same secure-trie construction as build_prestate(), but for an arbitrary EEST
# `pre` alloc: a real py-trie state trie (keys keccak(addr), values
# RLP([nonce,balance,storage_root,code_hash])) over per-account storage tries.
# Returns (state_root, witness_nodes, codes) — witness_nodes is the merged trie
# node-db (keccak(node)->node for nodes >= 32 bytes; embedded nodes live inside
# their parents, exactly what an execution witness carries).

def _hexint(x):
    if isinstance(x, int):
        return x
    x = str(x)
    return int(x, 16) if x.lower().startswith("0x") else int(x)


def _addr20(a):
    h = str(a).lower().replace("0x", "")
    return bytes.fromhex(h).rjust(20, b"\x00")


def build_witness_from_alloc(alloc):
    node_db = {}
    codes = []
    main_trie = HexaryTrie(db={})
    for addr_hex, acc in alloc.items():
        nonce = _hexint(acc.get("nonce", 0))
        bal = _hexint(acc.get("balance", 0))
        code_hex = (acc.get("code") or "").lower().replace("0x", "")
        code = bytes.fromhex(code_hex) if code_hex else b""
        chash = keccak(code) if code else EMPTY_CODE_HASH
        if code:
            codes.append(code)
        # zero storage values are absent from the trie (EEST sometimes lists them)
        storage = {k: v for k, v in (acc.get("storage") or {}).items()
                   if _hexint(v) != 0}
        if storage:
            strie = HexaryTrie(db={})
            for slot_hex, val_hex in storage.items():
                key = keccak(_hexint(slot_hex).to_bytes(32, "big"))
                strie[key] = rlp.encode(_hexint(val_hex))
            sroot = strie.root_hash
            node_db.update(strie.db)
        else:
            sroot = EMPTY_TRIE_ROOT
        main_trie[keccak(_addr20(addr_hex))] = account_rlp(nonce, bal, sroot, chash)
    node_db.update(main_trie.db)
    # The root node is always referenced by hash; guarantee it is in the witness.
    if main_trie.root_hash != EMPTY_TRIE_ROOT and main_trie.root_hash not in node_db:
        node_db[main_trie.root_hash] = main_trie.db[main_trie.root_hash]
    return main_trie.root_hash, list(node_db.values()), codes


def make_stateless_blob(state_root, witness_nodes, codes):
    """Wrap a witness in a structurally-valid SszStatelessInput. Payload values
    are placeholders; only `witness.state`/`witness.headers` matter for a
    witness-reroot probe (parent header carries `state_root`)."""
    payload = SszExecutionPayload(
        parent_hash=Bytes32(b"\x00" * 32),
        fee_recipient=ByteVector[20](b"\x00" * 20),
        state_root=Bytes32(state_root),
        receipts_root=Bytes32(EMPTY_TRIE_ROOT),
        logs_bloom=ByteVector[256](b"\x00" * 256),
        prev_randao=Bytes32(b"\x00" * 32),
        block_number=uint64(BLOCK_NUMBER),
        gas_limit=uint64(GAS_LIMIT),
        gas_used=uint64(0),
        timestamp=uint64(TIMESTAMP),
        extra_data=ByteList[MAX_EXTRA_DATA_BYTES](b""),
        base_fee_per_gas=uint256(0),
        block_hash=Bytes32(b"\x00" * 32),
        transactions=SszList[
            ByteList[MAX_BYTES_PER_TRANSACTION], MAX_TRANSACTIONS_PER_PAYLOAD](),
        withdrawals=SszList[SszWithdrawal, MAX_WITHDRAWALS_PER_PAYLOAD](),
        blob_gas_used=uint64(0),
        excess_blob_gas=uint64(0),
        block_access_list=ByteList[MAX_BLOCK_ACCESS_LIST_BYTES](b""),
        slot_number=uint64(0),
    )
    npr = SszNewPayloadRequest(
        execution_payload=payload,
        versioned_hashes=SszList[Bytes32, MAX_BLOB_COMMITMENTS_PER_BLOCK](),
        parent_beacon_block_root=Bytes32(b"\x00" * 32),
        execution_requests=SszExecutionRequests(),
    )
    witness = SszExecutionWitness(
        state=SszList[ByteList[MAX_BYTES_PER_WITNESS_NODE], MAX_WITNESS_NODES](
            *[ByteList[MAX_BYTES_PER_WITNESS_NODE](n) for n in witness_nodes]),
        codes=SszList[ByteList[MAX_BYTES_PER_CODE], MAX_WITNESS_CODES](
            *[ByteList[MAX_BYTES_PER_CODE](c) for c in codes]),
        headers=SszList[ByteList[MAX_BYTES_PER_HEADER], MAX_WITNESS_HEADERS](
            ByteList[MAX_BYTES_PER_HEADER](parent_header_bytes(state_root))),
    )
    chain_config = SszChainConfig(
        chain_id=uint64(CHAIN_ID),
        active_fork=SszForkConfig(
            fork=uint64(24),
            activation=SszForkActivation(
                block_number=SszList[uint64, 1](),
                timestamp=SszList[uint64, 1](uint64(0)),
            ),
            blob_schedule=SszList[SszBlobSchedule, 1](
                SszBlobSchedule(
                    target=uint64(BLOB_SCHEDULE_TARGET),
                    max=uint64(BLOB_SCHEDULE_MAX),
                    base_fee_update_fraction=uint64(BLOB_BASE_FEE_UPDATE_FRACTION),
                )
            ),
        ),
    )
    inp = SszStatelessInput(
        new_payload_request=npr,
        witness=witness,
        chain_config=chain_config,
        public_keys=SszList[ByteVector[PUBLIC_KEY_BYTES], MAX_PUBLIC_KEYS](),
    )
    return SCHEMA_ID + bytes(inp.encode_bytes())


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
        block_access_list=ByteList[MAX_BLOCK_ACCESS_LIST_BYTES](b""),
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
            ByteList[MAX_BYTES_PER_CODE](CODE),
            ByteList[MAX_BYTES_PER_CODE](WR_CODE),
            ByteList[MAX_BYTES_PER_CODE](CR_CODE),
        ),
        headers=SszList[ByteList[MAX_BYTES_PER_HEADER], MAX_WITNESS_HEADERS](
            ByteList[MAX_BYTES_PER_HEADER](parent_header_bytes(pre_root))
        ),
    )

    chain_config = SszChainConfig(
        chain_id=uint64(CHAIN_ID),
        active_fork=SszForkConfig(
            fork=uint64(20),  # ProtocolFork index: Amsterdam @ execution-specs 02c6c2510
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
            "n_codes": 3,
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


# --- non-zero pre-state storage fixture (validates witness-as-base) -----------
# A contract that READS a non-zero pre-state slot and writes it forward:
#   PUSH1 0x00 SLOAD PUSH1 0x01 SSTORE STOP   (slot1 := slot0)
# Pre-state contract.storage[0] = 99 (in the witness storage trie). With
# gas_price=0/base_fee=0 the fee is 0, so the post-state root is gas-independent
# and hand-computable; the only way the post-state matches is if the guest's
# SLOAD of slot0 resolves the real witness value (99). A broken witness-as-base
# fall-through (returning 0) writes slot1=0 -> post-state root mismatch ->
# successful_validation=0. So this fixture differentiates the fall-through that
# the zero-pre-state golden vector cannot.
CODE_S = bytes([0x60, 0x00, 0x54, 0x60, 0x01, 0x55, 0x00])
STORAGE_V = 99


def _storage_trie(slots):
    strie = HexaryTrie(db={})
    for slot, val in slots.items():
        strie[keccak(slot.to_bytes(32, "big"))] = rlp.encode(val)
    return strie


def storage_tx_bytes():
    """Signed legacy tx, gasPrice 0 (fee-free), to=CONTRACT, value 0, no data."""
    return sign_tx(0, [0, 0, 100000, CONTRACT, 0, b""])


def gen_storage_fixture():
    # pre-state: contract.storage[0]=99; coinbase pre-exists (bal 1) so a 0 fee
    # never creates/prunes it (no EIP-158 ambiguity in the post-state root).
    pre_strie = _storage_trie({0: STORAGE_V})
    sroot_pre = pre_strie.root_hash
    node_db = dict(pre_strie.db)
    pre = HexaryTrie(db={})
    # Only sender + contract are materialized (account lazy-load is a follow-on);
    # with fee=0 the coinbase stays empty -> EIP-158 pruned in pre and post, so it
    # never needs to be in the witness or the state.
    pre_accounts = {
        EOA: (0, 10**18, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
        CONTRACT: (1, 0, sroot_pre, keccak(CODE_S)),
    }
    for addr, (n, b, sr, ch) in pre_accounts.items():
        pre[keccak(addr)] = account_rlp(n, b, sr, ch)
    node_db.update(pre.db)
    if pre.root_hash not in node_db:
        node_db[pre.root_hash] = pre.db[pre.root_hash]
    pre_root = pre.root_hash
    witness_nodes = list(node_db.values())

    # post-state: slot1 := slot0 (=99); sender nonce+1, balances unchanged (fee 0).
    post_strie = _storage_trie({0: STORAGE_V, 1: STORAGE_V})
    post = HexaryTrie(db={})
    post_accounts = {
        EOA: (1, 10**18, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
        CONTRACT: (1, 0, post_strie.root_hash, keccak(CODE_S)),
    }
    for addr, (n, b, sr, ch) in post_accounts.items():
        post[keccak(addr)] = account_rlp(n, b, sr, ch)
    post_root = post.root_hash

    payload = SszExecutionPayload(
        parent_hash=Bytes32(b"\x00" * 32),
        fee_recipient=ByteVector[20](COINBASE),
        state_root=Bytes32(post_root),
        receipts_root=Bytes32(EMPTY_TRIE_ROOT),
        logs_bloom=ByteVector[256](b"\x00" * 256),
        prev_randao=Bytes32(b"\x00" * 32),
        block_number=uint64(BLOCK_NUMBER),
        gas_limit=uint64(GAS_LIMIT),
        gas_used=uint64(0),
        timestamp=uint64(TIMESTAMP),
        extra_data=ByteList[MAX_EXTRA_DATA_BYTES](b""),
        base_fee_per_gas=uint256(0),
        block_hash=Bytes32(b"\x00" * 32),
        transactions=SszList[
            ByteList[MAX_BYTES_PER_TRANSACTION], MAX_TRANSACTIONS_PER_PAYLOAD
        ](ByteList[MAX_BYTES_PER_TRANSACTION](storage_tx_bytes())),
        withdrawals=SszList[SszWithdrawal, MAX_WITHDRAWALS_PER_PAYLOAD](),
        blob_gas_used=uint64(0),
        excess_blob_gas=uint64(0),
        block_access_list=ByteList[MAX_BLOCK_ACCESS_LIST_BYTES](b""),
        slot_number=uint64(0),
    )
    npr = SszNewPayloadRequest(
        execution_payload=payload,
        versioned_hashes=SszList[Bytes32, MAX_BLOB_COMMITMENTS_PER_BLOCK](),
        parent_beacon_block_root=Bytes32(b"\x00" * 32),
        execution_requests=SszExecutionRequests(),
    )
    witness = SszExecutionWitness(
        state=SszList[ByteList[MAX_BYTES_PER_WITNESS_NODE], MAX_WITNESS_NODES](
            *[ByteList[MAX_BYTES_PER_WITNESS_NODE](n) for n in witness_nodes]),
        codes=SszList[ByteList[MAX_BYTES_PER_CODE], MAX_WITNESS_CODES](
            ByteList[MAX_BYTES_PER_CODE](CODE_S)),
        headers=SszList[ByteList[MAX_BYTES_PER_HEADER], MAX_WITNESS_HEADERS](
            ByteList[MAX_BYTES_PER_HEADER](parent_header_bytes(pre_root))),
    )
    chain_config = SszChainConfig(
        chain_id=uint64(CHAIN_ID),
        active_fork=SszForkConfig(
            fork=uint64(24),
            activation=SszForkActivation(
                block_number=SszList[uint64, 1](),
                timestamp=SszList[uint64, 1](uint64(0)),
            ),
            blob_schedule=SszList[SszBlobSchedule, 1](
                SszBlobSchedule(
                    target=uint64(BLOB_SCHEDULE_TARGET),
                    max=uint64(BLOB_SCHEDULE_MAX),
                    base_fee_update_fraction=uint64(BLOB_BASE_FEE_UPDATE_FRACTION),
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
    with open(os.path.join(OUT, "fixture_storage.ssz"), "wb") as f:
        f.write(blob)
    result = SszStatelessValidationResult(
        new_payload_request_root=Bytes32(npr.hash_tree_root()),
        successful_validation=boolean(True),
        chain_config=chain_config,
    )
    result_bytes = bytes(result.encode_bytes())
    sidecar = {
        "schema_id": 1,
        "total_bytes": len(blob),
        "pre_state_root": pre_root.hex(),
        "post_state_root": post_root.hex(),
        "contract_storage_pre": {"0": STORAGE_V},
        "contract_storage_post": {"0": STORAGE_V, "1": STORAGE_V},
        "result": {"len": len(result_bytes), "ssz": result_bytes.hex()},
    }
    with open(os.path.join(OUT, "fixture_storage.json"), "w") as f:
        json.dump(sidecar, f, indent=2)
    print(f"wrote vectors/fixture_storage.ssz ({len(blob)} bytes), "
          f"{len(witness_nodes)} witness nodes; result={result_bytes.hex()}")


# --- Partial (proof-style) witness ------------------------------------------
# A REAL stateless witness is not the whole trie: it carries only the nodes on
# the accessed paths, with untouched sibling subtrees BLINDED (referenced by hash
# only). We build that by replaying the accessed-key lookups through a recording
# node-db: every node the lookup touches is, by definition, the proof; everything
# else stays blinded. This is the fixture that distinguishes a flat post-state
# enumerator (which can only see harvested/present leaves -> wrong root when
# subtrees are blinded) from an incremental root that reuses blinded subtrees.
class _RecDB:
    def __init__(self, base):
        self.base = base
        self.seen = {}

    def __getitem__(self, k):
        v = self.base[k]
        self.seen[k] = v
        return v

    def __contains__(self, k):
        return k in self.base

    def get(self, k, default=None):
        return self.__getitem__(k) if k in self.base else default


def _proof(full_db, root, keys):
    """Nodes on the paths to `keys` (present OR absent) under the trie rooted at
    `root`; siblings off every path stay blinded."""
    rec = _RecDB(full_db)
    t = HexaryTrie(db=rec, root_hash=root)
    for k in keys:
        try:
            _ = t[k]            # present key: records its leaf + ancestors
        except KeyError:
            pass                # absent key: records the proof-of-absence path
    return rec.seen


def gen_partial_fixture():
    # CONTRACT storage with UNTOUCHED filler slots (2, 3) alongside the read slot
    # (0). The block (CODE_S: SLOAD 0 -> SSTORE 1) touches only slots 0 and 1, so
    # the witness blinds slots 2 and 3 -> the post-state storage root can only be
    # computed by REUSING those blinded subtrees.
    pre_slots = {0: STORAGE_V, 2: 0x22, 3: 0x33}
    pre_strie = _storage_trie(pre_slots)
    sroot_pre = pre_strie.root_hash
    storage_db = dict(pre_strie.db)

    pre = HexaryTrie(db={})
    pre_accounts = {
        EOA: (0, 10**18, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
        CONTRACT: (1, 0, sroot_pre, keccak(CODE_S)),
    }
    for addr, (n, b, sr, ch) in pre_accounts.items():
        pre[keccak(addr)] = account_rlp(n, b, sr, ch)
    state_db = dict(pre.db)
    if pre.root_hash not in state_db:
        state_db[pre.root_hash] = pre.db[pre.root_hash]
    pre_root = pre.root_hash

    # PARTIAL witness: proofs for the accessed keys only.
    state_keys = [keccak(EOA), keccak(CONTRACT)]
    storage_keys = [keccak((0).to_bytes(32, "big")),   # SLOAD slot 0 (present)
                    keccak((1).to_bytes(32, "big"))]   # SSTORE slot 1 (absent -> insert)
    state_proof = _proof(state_db, pre_root, state_keys)
    storage_proof = _proof(storage_db, sroot_pre, storage_keys)
    assert len(storage_proof) < len(storage_db), \
        "storage witness must be PARTIAL (some filler subtree must stay blinded)"
    witness_nodes = list(state_proof.values()) + list(storage_proof.values())

    # post-state: slot1 := slot0 (=99); fillers 2,3 unchanged (blinded subtrees).
    post_strie = _storage_trie({0: STORAGE_V, 1: STORAGE_V, 2: 0x22, 3: 0x33})
    post = HexaryTrie(db={})
    post_accounts = {
        EOA: (1, 10**18, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
        CONTRACT: (1, 0, post_strie.root_hash, keccak(CODE_S)),
    }
    for addr, (n, b, sr, ch) in post_accounts.items():
        post[keccak(addr)] = account_rlp(n, b, sr, ch)
    post_root = post.root_hash

    payload = SszExecutionPayload(
        parent_hash=Bytes32(b"\x00" * 32),
        fee_recipient=ByteVector[20](COINBASE),
        state_root=Bytes32(post_root),
        receipts_root=Bytes32(EMPTY_TRIE_ROOT),
        logs_bloom=ByteVector[256](b"\x00" * 256),
        prev_randao=Bytes32(b"\x00" * 32),
        block_number=uint64(BLOCK_NUMBER),
        gas_limit=uint64(GAS_LIMIT),
        gas_used=uint64(0),
        timestamp=uint64(TIMESTAMP),
        extra_data=ByteList[MAX_EXTRA_DATA_BYTES](b""),
        base_fee_per_gas=uint256(0),
        block_hash=Bytes32(b"\x00" * 32),
        transactions=SszList[
            ByteList[MAX_BYTES_PER_TRANSACTION], MAX_TRANSACTIONS_PER_PAYLOAD
        ](ByteList[MAX_BYTES_PER_TRANSACTION](storage_tx_bytes())),
        withdrawals=SszList[SszWithdrawal, MAX_WITHDRAWALS_PER_PAYLOAD](),
        blob_gas_used=uint64(0),
        excess_blob_gas=uint64(0),
        block_access_list=ByteList[MAX_BLOCK_ACCESS_LIST_BYTES](b""),
        slot_number=uint64(0),
    )
    npr = SszNewPayloadRequest(
        execution_payload=payload,
        versioned_hashes=SszList[Bytes32, MAX_BLOB_COMMITMENTS_PER_BLOCK](),
        parent_beacon_block_root=Bytes32(b"\x00" * 32),
        execution_requests=SszExecutionRequests(),
    )
    witness = SszExecutionWitness(
        state=SszList[ByteList[MAX_BYTES_PER_WITNESS_NODE], MAX_WITNESS_NODES](
            *[ByteList[MAX_BYTES_PER_WITNESS_NODE](n) for n in witness_nodes]),
        codes=SszList[ByteList[MAX_BYTES_PER_CODE], MAX_WITNESS_CODES](
            ByteList[MAX_BYTES_PER_CODE](CODE_S)),
        headers=SszList[ByteList[MAX_BYTES_PER_HEADER], MAX_WITNESS_HEADERS](
            ByteList[MAX_BYTES_PER_HEADER](parent_header_bytes(pre_root))),
    )
    chain_config = SszChainConfig(
        chain_id=uint64(CHAIN_ID),
        active_fork=SszForkConfig(
            fork=uint64(24),
            activation=SszForkActivation(
                block_number=SszList[uint64, 1](),
                timestamp=SszList[uint64, 1](uint64(0)),
            ),
            blob_schedule=SszList[SszBlobSchedule, 1](
                SszBlobSchedule(
                    target=uint64(BLOB_SCHEDULE_TARGET),
                    max=uint64(BLOB_SCHEDULE_MAX),
                    base_fee_update_fraction=uint64(BLOB_BASE_FEE_UPDATE_FRACTION),
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
    with open(os.path.join(OUT, "fixture_partial.ssz"), "wb") as f:
        f.write(blob)
    result = SszStatelessValidationResult(
        new_payload_request_root=Bytes32(npr.hash_tree_root()),
        successful_validation=boolean(True),
        chain_config=chain_config,
    )
    result_bytes = bytes(result.encode_bytes())
    sidecar = {
        "schema_id": 1,
        "total_bytes": len(blob),
        "pre_state_root": pre_root.hex(),
        "post_state_root": post_root.hex(),
        "storage_full_nodes": len(storage_db),
        "storage_proof_nodes": len(storage_proof),
        "blinded_storage_subtrees": len(storage_db) - len(storage_proof),
        "result": {"len": len(result_bytes), "ssz": result_bytes.hex()},
    }
    with open(os.path.join(OUT, "fixture_partial.json"), "w") as f:
        json.dump(sidecar, f, indent=2)
    print(f"wrote vectors/fixture_partial.ssz ({len(blob)} bytes), "
          f"{len(witness_nodes)} witness nodes "
          f"({len(storage_db) - len(storage_proof)} storage subtrees blinded); "
          f"result={result_bytes.hex()}")


# --- typed (EIP-1559) envelope fixture --------------------------------------
# Exercises decode_tx_ssz's typed path end-to-end: the 0x02 type byte, the
# chain-id-prefixed field layout with (max_priority, max_fee) split, the nested
# access-list decode AND its intrinsic-gas cost, and the effective-fee
# computation under a NON-ZERO base fee. A plain value transfer (no code) so the
# post-state is hand-computable: gas_used is exactly intrinsic, and the only way
# the post-state root matches is if every typed field, the access-list cost, and
# eff_gas_price = min(max_fee, base_fee + max_priority) all decode correctly. A
# legacy-only or mis-indexed decode produces a different fee/gas -> root mismatch
# -> successful_validation = 0.
TY_BASE_FEE = 5
TY_MAX_PRIO = 2
TY_MAX_FEE = 10
TY_VALUE = 1000
# 1 access-list address (2400) + 1 storage key (1900) on top of the 21000 base
TY_GAS_USED = 21000 + 2400 + 1900           # = 25300 (plain transfer, no execution)
TY_EFF_PRICE = min(TY_MAX_FEE, TY_BASE_FEE + TY_MAX_PRIO)   # = 7
TY_PRIORITY = TY_EFF_PRICE - TY_BASE_FEE                    # = 2


def typed_1559_tx_bytes():
    """Signed EIP-1559 (type-2) tx: chain_id, nonce0, maxPrio, maxFee, gas,
    to=EOA2, value, no data, accessList=[[EOA2, [slot 0]]]."""
    access_list = [[EOA2, [(0).to_bytes(32, "big")]]]
    return sign_tx(2, [
        CHAIN_ID, 0, TY_MAX_PRIO, TY_MAX_FEE, 100000, EOA2, TY_VALUE, b"",
        access_list,
    ])


def gen_typed_fixture():
    fee = TY_GAS_USED * TY_EFF_PRICE         # sender pays gas_used * effective price
    tip = TY_GAS_USED * TY_PRIORITY          # coinbase receives gas_used * priority
    pre_accounts = {
        EOA: (0, 10**18, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
        EOA2: (0, 500, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
        COINBASE: (0, 1, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),   # pre-exists (no EIP-158)
    }
    pre = HexaryTrie(db={})
    for addr, (n, b, sr, ch) in pre_accounts.items():
        pre[keccak(addr)] = account_rlp(n, b, sr, ch)
    pre_root = pre.root_hash
    witness_nodes = list(pre.db.values())

    post_accounts = {
        EOA: (1, 10**18 - TY_VALUE - fee, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
        EOA2: (0, 500 + TY_VALUE, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
        COINBASE: (0, 1 + tip, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
    }
    post = HexaryTrie(db={})
    for addr, (n, b, sr, ch) in post_accounts.items():
        post[keccak(addr)] = account_rlp(n, b, sr, ch)
    post_root = post.root_hash

    payload = SszExecutionPayload(
        parent_hash=Bytes32(b"\x00" * 32),
        fee_recipient=ByteVector[20](COINBASE),
        state_root=Bytes32(post_root),
        receipts_root=Bytes32(EMPTY_TRIE_ROOT),
        logs_bloom=ByteVector[256](b"\x00" * 256),
        prev_randao=Bytes32(b"\x00" * 32),
        block_number=uint64(BLOCK_NUMBER),
        gas_limit=uint64(GAS_LIMIT),
        gas_used=uint64(TY_GAS_USED),
        timestamp=uint64(TIMESTAMP),
        extra_data=ByteList[MAX_EXTRA_DATA_BYTES](b""),
        base_fee_per_gas=uint256(TY_BASE_FEE),
        block_hash=Bytes32(b"\x00" * 32),
        transactions=SszList[
            ByteList[MAX_BYTES_PER_TRANSACTION], MAX_TRANSACTIONS_PER_PAYLOAD
        ](ByteList[MAX_BYTES_PER_TRANSACTION](typed_1559_tx_bytes())),
        withdrawals=SszList[SszWithdrawal, MAX_WITHDRAWALS_PER_PAYLOAD](),
        blob_gas_used=uint64(0),
        excess_blob_gas=uint64(0),
        block_access_list=ByteList[MAX_BLOCK_ACCESS_LIST_BYTES](b""),
        slot_number=uint64(0),
    )
    npr = SszNewPayloadRequest(
        execution_payload=payload,
        versioned_hashes=SszList[Bytes32, MAX_BLOB_COMMITMENTS_PER_BLOCK](),
        parent_beacon_block_root=Bytes32(b"\x00" * 32),
        execution_requests=SszExecutionRequests(),
    )
    witness = SszExecutionWitness(
        state=SszList[ByteList[MAX_BYTES_PER_WITNESS_NODE], MAX_WITNESS_NODES](
            *[ByteList[MAX_BYTES_PER_WITNESS_NODE](n) for n in witness_nodes]),
        codes=SszList[ByteList[MAX_BYTES_PER_CODE], MAX_WITNESS_CODES](),
        headers=SszList[ByteList[MAX_BYTES_PER_HEADER], MAX_WITNESS_HEADERS](
            ByteList[MAX_BYTES_PER_HEADER](parent_header_bytes(pre_root))),
    )
    chain_config = SszChainConfig(
        chain_id=uint64(CHAIN_ID),
        active_fork=SszForkConfig(
            fork=uint64(24),
            activation=SszForkActivation(
                block_number=SszList[uint64, 1](),
                timestamp=SszList[uint64, 1](uint64(0)),
            ),
            blob_schedule=SszList[SszBlobSchedule, 1](
                SszBlobSchedule(
                    target=uint64(BLOB_SCHEDULE_TARGET),
                    max=uint64(BLOB_SCHEDULE_MAX),
                    base_fee_update_fraction=uint64(BLOB_BASE_FEE_UPDATE_FRACTION),
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
    with open(os.path.join(OUT, "fixture_typed.ssz"), "wb") as f:
        f.write(blob)
    result = SszStatelessValidationResult(
        new_payload_request_root=Bytes32(npr.hash_tree_root()),
        successful_validation=boolean(True),
        chain_config=chain_config,
    )
    result_bytes = bytes(result.encode_bytes())
    sidecar = {
        "schema_id": 1,
        "total_bytes": len(blob),
        "tx_type": 2,
        "base_fee": TY_BASE_FEE,
        "max_priority_fee": TY_MAX_PRIO,
        "max_fee": TY_MAX_FEE,
        "effective_gas_price": TY_EFF_PRICE,
        "gas_used": TY_GAS_USED,
        "pre_state_root": pre_root.hex(),
        "post_state_root": post_root.hex(),
        "result": {"len": len(result_bytes), "ssz": result_bytes.hex()},
    }
    with open(os.path.join(OUT, "fixture_typed.json"), "w") as f:
        json.dump(sidecar, f, indent=2)
    print(f"wrote vectors/fixture_typed.ssz ({len(blob)} bytes), "
          f"{len(witness_nodes)} witness nodes; type-2 tx, "
          f"eff_price={TY_EFF_PRICE} gas_used={TY_GAS_USED}; "
          f"result={result_bytes.hex()}")


# --- EIP-7702 (type-4) set-code fixture -------------------------------------
# A type-4 tx whose authorization tuple delegates AUTHORITY -> CONTRACT. The guest
# RECOVERS the authority from the tuple signature (no pubkey witness for it) and
# applies the delegation: AUTHORITY's code becomes 0xef0100||CONTRACT and its nonce
# bumps to 1. Fee-free (base_fee/maxFee/maxPrio = 0) so the post-state is exact and
# gas-independent. The only way the post-state root matches is if the guest decodes
# the tuple, recovers the right authority (ecrecover), and writes the delegation.
def type4_tx_bytes():
    """Signed EIP-7702 tx: delegate AUTHORITY -> CONTRACT; to=EOA (no-op self call)."""
    auth = sign_auth(AUTH_PRIVKEY, CHAIN_ID, CONTRACT, 0)  # authority's current nonce = 0
    return sign_tx(4, [CHAIN_ID, 0, 0, 0, 200000, EOA, 0, b"", [], [auth]])


def gen_auth_fixture():
    pre_accounts = {
        EOA: (0, 10**18, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
        AUTHORITY: (0, 1, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),  # pre-exists, empty code
    }
    pre = HexaryTrie(db={})
    for addr, (n, b, sr, ch) in pre_accounts.items():
        pre[keccak(addr)] = account_rlp(n, b, sr, ch)
    pre_root = pre.root_hash
    witness_nodes = list(pre.db.values())

    # post: sender nonce+1 (fee-free); authority delegated to CONTRACT, nonce -> 1
    post_accounts = {
        EOA: (1, 10**18, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
        AUTHORITY: (1, 1, EMPTY_TRIE_ROOT, deleg_code_hash(CONTRACT)),
    }
    post = HexaryTrie(db={})
    for addr, (n, b, sr, ch) in post_accounts.items():
        post[keccak(addr)] = account_rlp(n, b, sr, ch)
    post_root = post.root_hash

    payload = SszExecutionPayload(
        parent_hash=Bytes32(b"\x00" * 32),
        fee_recipient=ByteVector[20](COINBASE),
        state_root=Bytes32(post_root),
        receipts_root=Bytes32(EMPTY_TRIE_ROOT),
        logs_bloom=ByteVector[256](b"\x00" * 256),
        prev_randao=Bytes32(b"\x00" * 32),
        block_number=uint64(BLOCK_NUMBER),
        gas_limit=uint64(GAS_LIMIT),
        gas_used=uint64(46000),
        timestamp=uint64(TIMESTAMP),
        extra_data=ByteList[MAX_EXTRA_DATA_BYTES](b""),
        base_fee_per_gas=uint256(0),
        block_hash=Bytes32(b"\x00" * 32),
        transactions=SszList[
            ByteList[MAX_BYTES_PER_TRANSACTION], MAX_TRANSACTIONS_PER_PAYLOAD
        ](ByteList[MAX_BYTES_PER_TRANSACTION](type4_tx_bytes())),
        withdrawals=SszList[SszWithdrawal, MAX_WITHDRAWALS_PER_PAYLOAD](),
        blob_gas_used=uint64(0),
        excess_blob_gas=uint64(0),
        block_access_list=ByteList[MAX_BLOCK_ACCESS_LIST_BYTES](b""),
        slot_number=uint64(0),
    )
    npr = SszNewPayloadRequest(
        execution_payload=payload,
        versioned_hashes=SszList[Bytes32, MAX_BLOB_COMMITMENTS_PER_BLOCK](),
        parent_beacon_block_root=Bytes32(b"\x00" * 32),
        execution_requests=SszExecutionRequests(),
    )
    witness = SszExecutionWitness(
        state=SszList[ByteList[MAX_BYTES_PER_WITNESS_NODE], MAX_WITNESS_NODES](
            *[ByteList[MAX_BYTES_PER_WITNESS_NODE](n) for n in witness_nodes]),
        codes=SszList[ByteList[MAX_BYTES_PER_CODE], MAX_WITNESS_CODES](),
        headers=SszList[ByteList[MAX_BYTES_PER_HEADER], MAX_WITNESS_HEADERS](
            ByteList[MAX_BYTES_PER_HEADER](parent_header_bytes(pre_root))),
    )
    chain_config = SszChainConfig(
        chain_id=uint64(CHAIN_ID),
        active_fork=SszForkConfig(
            fork=uint64(24),
            activation=SszForkActivation(
                block_number=SszList[uint64, 1](),
                timestamp=SszList[uint64, 1](uint64(0)),
            ),
            blob_schedule=SszList[SszBlobSchedule, 1](
                SszBlobSchedule(
                    target=uint64(BLOB_SCHEDULE_TARGET),
                    max=uint64(BLOB_SCHEDULE_MAX),
                    base_fee_update_fraction=uint64(BLOB_BASE_FEE_UPDATE_FRACTION),
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
    with open(os.path.join(OUT, "fixture_auth.ssz"), "wb") as f:
        f.write(blob)
    result = SszStatelessValidationResult(
        new_payload_request_root=Bytes32(npr.hash_tree_root()),
        successful_validation=boolean(True),
        chain_config=chain_config,
    )
    result_bytes = bytes(result.encode_bytes())
    sidecar = {
        "schema_id": 1,
        "total_bytes": len(blob),
        "tx_type": 4,
        "authority": "0x" + AUTHORITY.hex(),
        "delegate_to": "0x" + CONTRACT.hex(),
        "pre_state_root": pre_root.hex(),
        "post_state_root": post_root.hex(),
        "result": {"len": len(result_bytes), "ssz": result_bytes.hex()},
    }
    with open(os.path.join(OUT, "fixture_auth.json"), "w") as f:
        json.dump(sidecar, f, indent=2)
    print(f"wrote vectors/fixture_auth.ssz ({len(blob)} bytes), "
          f"{len(witness_nodes)} witness nodes; type-4 tx, "
          f"authority={'0x'+AUTHORITY.hex()} -> {'0x'+CONTRACT.hex()}; "
          f"result={result_bytes.hex()}")


# --- EIP-4844 (type-3) blob fixture -----------------------------------------
# A type-3 tx carrying one blob. The guest decodes max_fee_per_blob_gas (field 9)
# and the versioned hashes (field 10), validates the 0x01 version tag, and burns
# the blob fee: blob_count * GAS_PER_BLOB * blob_base_fee. With excess_blob_gas=0
# the blob base fee is 1, so the burn is exactly 131072 wei from the sender (and
# the regular gas fee is 0 at base_fee=0). The post-state matches only if the
# guest decoded the blob fields and applied that burn.
BLOB_VHASH = bytes([0x01]) + keccak(b"evm-sail blob")[1:]   # 0x01 version-tagged
BLOB_FEE = 1 * 131072 * 1   # blob_count * GAS_PER_BLOB * blob_base_fee(excess=0)=1


def type3_tx_bytes():
    """Signed EIP-4844 (type-3) tx: 1 blob, to=EOA (no-op self call), value 0."""
    return sign_tx(3, [
        CHAIN_ID, 0, 0, 0, 100000, EOA, 0, b"", [], 1, [BLOB_VHASH],
    ])  # ... maxFeePerBlobGas=1, blobVersionedHashes=[h]


def gen_blob_fixture():
    pre_accounts = {EOA: (0, 10**18, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH)}
    pre = HexaryTrie(db={})
    for addr, (n, b, sr, ch) in pre_accounts.items():
        pre[keccak(addr)] = account_rlp(n, b, sr, ch)
    pre_root = pre.root_hash
    witness_nodes = list(pre.db.values())

    # post: sender nonce+1, balance -= blob-fee burn (regular gas fee is 0)
    post_accounts = {EOA: (1, 10**18 - BLOB_FEE, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH)}
    post = HexaryTrie(db={})
    for addr, (n, b, sr, ch) in post_accounts.items():
        post[keccak(addr)] = account_rlp(n, b, sr, ch)
    post_root = post.root_hash

    payload = SszExecutionPayload(
        parent_hash=Bytes32(b"\x00" * 32),
        fee_recipient=ByteVector[20](COINBASE),
        state_root=Bytes32(post_root),
        receipts_root=Bytes32(EMPTY_TRIE_ROOT),
        logs_bloom=ByteVector[256](b"\x00" * 256),
        prev_randao=Bytes32(b"\x00" * 32),
        block_number=uint64(BLOCK_NUMBER),
        gas_limit=uint64(GAS_LIMIT),
        gas_used=uint64(21000),
        timestamp=uint64(TIMESTAMP),
        extra_data=ByteList[MAX_EXTRA_DATA_BYTES](b""),
        base_fee_per_gas=uint256(0),
        block_hash=Bytes32(b"\x00" * 32),
        transactions=SszList[
            ByteList[MAX_BYTES_PER_TRANSACTION], MAX_TRANSACTIONS_PER_PAYLOAD
        ](ByteList[MAX_BYTES_PER_TRANSACTION](type3_tx_bytes())),
        withdrawals=SszList[SszWithdrawal, MAX_WITHDRAWALS_PER_PAYLOAD](),
        blob_gas_used=uint64(131072),
        excess_blob_gas=uint64(0),
        block_access_list=ByteList[MAX_BLOCK_ACCESS_LIST_BYTES](b""),
        slot_number=uint64(0),
    )
    npr = SszNewPayloadRequest(
        execution_payload=payload,
        versioned_hashes=SszList[Bytes32, MAX_BLOB_COMMITMENTS_PER_BLOCK](Bytes32(BLOB_VHASH)),
        parent_beacon_block_root=Bytes32(b"\x00" * 32),
        execution_requests=SszExecutionRequests(),
    )
    witness = SszExecutionWitness(
        state=SszList[ByteList[MAX_BYTES_PER_WITNESS_NODE], MAX_WITNESS_NODES](
            *[ByteList[MAX_BYTES_PER_WITNESS_NODE](n) for n in witness_nodes]),
        codes=SszList[ByteList[MAX_BYTES_PER_CODE], MAX_WITNESS_CODES](),
        headers=SszList[ByteList[MAX_BYTES_PER_HEADER], MAX_WITNESS_HEADERS](
            ByteList[MAX_BYTES_PER_HEADER](parent_header_bytes(pre_root))),
    )
    chain_config = SszChainConfig(
        chain_id=uint64(CHAIN_ID),
        active_fork=SszForkConfig(
            fork=uint64(24),
            activation=SszForkActivation(
                block_number=SszList[uint64, 1](),
                timestamp=SszList[uint64, 1](uint64(0)),
            ),
            blob_schedule=SszList[SszBlobSchedule, 1](
                SszBlobSchedule(
                    target=uint64(BLOB_SCHEDULE_TARGET),
                    max=uint64(BLOB_SCHEDULE_MAX),
                    base_fee_update_fraction=uint64(BLOB_BASE_FEE_UPDATE_FRACTION),
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
    with open(os.path.join(OUT, "fixture_blob.ssz"), "wb") as f:
        f.write(blob)
    result = SszStatelessValidationResult(
        new_payload_request_root=Bytes32(npr.hash_tree_root()),
        successful_validation=boolean(True),
        chain_config=chain_config,
    )
    result_bytes = bytes(result.encode_bytes())
    sidecar = {
        "schema_id": 1,
        "total_bytes": len(blob),
        "tx_type": 3,
        "blob_count": 1,
        "blob_fee_burned": BLOB_FEE,
        "versioned_hash": "0x" + BLOB_VHASH.hex(),
        "pre_state_root": pre_root.hex(),
        "post_state_root": post_root.hex(),
        "result": {"len": len(result_bytes), "ssz": result_bytes.hex()},
    }
    with open(os.path.join(OUT, "fixture_blob.json"), "w") as f:
        json.dump(sidecar, f, indent=2)
    print(f"wrote vectors/fixture_blob.ssz ({len(blob)} bytes), "
          f"{len(witness_nodes)} witness nodes; type-3 tx, 1 blob, "
          f"burn={BLOB_FEE}; result={result_bytes.hex()}")


# --- partial STATE witness: an untouched but present account ----------------
# The block touches only the sender (fee-free self-tx); a second account UNTOUCHED
# is present in the state trie. A flat post-state enumeration over the acctmap
# (touched accounts only) DROPS the untouched account -> wrong state root. The
# incremental state root must REUSE its unchanged witness leaf. This is the
# state-trie analogue of fixture_partial (which blinds storage subtrees).
UNTOUCHED = bytes.fromhex("0000000000000000000000000000000000b0b0b0")


def gen_state_fixture():
    untouched = (5, 999, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH)  # present, never touched
    pre_accounts = {
        EOA: (0, 10**18, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
        UNTOUCHED: untouched,
    }
    pre = HexaryTrie(db={})
    for addr, (n, b, sr, ch) in pre_accounts.items():
        pre[keccak(addr)] = account_rlp(n, b, sr, ch)
    pre_root = pre.root_hash
    witness_nodes = list(pre.db.values())

    # post: sender nonce+1 (fee-free self call); UNTOUCHED account unchanged
    post_accounts = {
        EOA: (1, 10**18, EMPTY_TRIE_ROOT, EMPTY_CODE_HASH),
        UNTOUCHED: untouched,
    }
    post = HexaryTrie(db={})
    for addr, (n, b, sr, ch) in post_accounts.items():
        post[keccak(addr)] = account_rlp(n, b, sr, ch)
    post_root = post.root_hash

    payload = SszExecutionPayload(
        parent_hash=Bytes32(b"\x00" * 32),
        fee_recipient=ByteVector[20](COINBASE),
        state_root=Bytes32(post_root),
        receipts_root=Bytes32(EMPTY_TRIE_ROOT),
        logs_bloom=ByteVector[256](b"\x00" * 256),
        prev_randao=Bytes32(b"\x00" * 32),
        block_number=uint64(BLOCK_NUMBER),
        gas_limit=uint64(GAS_LIMIT),
        gas_used=uint64(21000),
        timestamp=uint64(TIMESTAMP),
        extra_data=ByteList[MAX_EXTRA_DATA_BYTES](b""),
        base_fee_per_gas=uint256(0),
        block_hash=Bytes32(b"\x00" * 32),
        transactions=SszList[
            ByteList[MAX_BYTES_PER_TRANSACTION], MAX_TRANSACTIONS_PER_PAYLOAD
        ](ByteList[MAX_BYTES_PER_TRANSACTION](sign_tx(0, [0, 0, 100000, EOA, 0, b""]))),
        withdrawals=SszList[SszWithdrawal, MAX_WITHDRAWALS_PER_PAYLOAD](),
        blob_gas_used=uint64(0),
        excess_blob_gas=uint64(0),
        block_access_list=ByteList[MAX_BLOCK_ACCESS_LIST_BYTES](b""),
        slot_number=uint64(0),
    )
    npr = SszNewPayloadRequest(
        execution_payload=payload,
        versioned_hashes=SszList[Bytes32, MAX_BLOB_COMMITMENTS_PER_BLOCK](),
        parent_beacon_block_root=Bytes32(b"\x00" * 32),
        execution_requests=SszExecutionRequests(),
    )
    witness = SszExecutionWitness(
        state=SszList[ByteList[MAX_BYTES_PER_WITNESS_NODE], MAX_WITNESS_NODES](
            *[ByteList[MAX_BYTES_PER_WITNESS_NODE](n) for n in witness_nodes]),
        codes=SszList[ByteList[MAX_BYTES_PER_CODE], MAX_WITNESS_CODES](),
        headers=SszList[ByteList[MAX_BYTES_PER_HEADER], MAX_WITNESS_HEADERS](
            ByteList[MAX_BYTES_PER_HEADER](parent_header_bytes(pre_root))),
    )
    chain_config = SszChainConfig(
        chain_id=uint64(CHAIN_ID),
        active_fork=SszForkConfig(
            fork=uint64(24),
            activation=SszForkActivation(
                block_number=SszList[uint64, 1](),
                timestamp=SszList[uint64, 1](uint64(0)),
            ),
            blob_schedule=SszList[SszBlobSchedule, 1](
                SszBlobSchedule(
                    target=uint64(BLOB_SCHEDULE_TARGET),
                    max=uint64(BLOB_SCHEDULE_MAX),
                    base_fee_update_fraction=uint64(BLOB_BASE_FEE_UPDATE_FRACTION),
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
    with open(os.path.join(OUT, "fixture_state.ssz"), "wb") as f:
        f.write(blob)
    result = SszStatelessValidationResult(
        new_payload_request_root=Bytes32(npr.hash_tree_root()),
        successful_validation=boolean(True),
        chain_config=chain_config,
    )
    result_bytes = bytes(result.encode_bytes())
    sidecar = {
        "schema_id": 1,
        "total_bytes": len(blob),
        "untouched_account": "0x" + UNTOUCHED.hex(),
        "pre_state_root": pre_root.hex(),
        "post_state_root": post_root.hex(),
        "result": {"len": len(result_bytes), "ssz": result_bytes.hex()},
    }
    with open(os.path.join(OUT, "fixture_state.json"), "w") as f:
        json.dump(sidecar, f, indent=2)
    print(f"wrote vectors/fixture_state.ssz ({len(blob)} bytes), "
          f"{len(witness_nodes)} witness nodes; untouched account "
          f"{'0x'+UNTOUCHED.hex()} reused; result={result_bytes.hex()}")


if __name__ == "__main__":
    if "--state" in sys.argv:
        gen_state_fixture()
    elif "--storage" in sys.argv:
        gen_storage_fixture()
    elif "--partial" in sys.argv:
        gen_partial_fixture()
    elif "--typed" in sys.argv:
        gen_typed_fixture()
    elif "--auth" in sys.argv:
        gen_auth_fixture()
    elif "--blob" in sys.argv:
        gen_blob_fixture()
    else:
        main()
