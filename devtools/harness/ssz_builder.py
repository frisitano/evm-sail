#!/usr/bin/env python3
"""Build a valid stateless block input/output pair from an EEST state-test case.

Runs under the execution-specs venv (it reuses that project's stateless SSZ types
and serializer, so the bytes match the model's stateless-input decoder exactly).
The devtools.harness.cli module
(which lacks these deps) drives this over --serve: one case JSON per stdin line,
one JSON response per stdout line ({"input": hex[, "expected": hex]} or
{"err": msg}).

A fully VALID single-tx block is built by executing the case through the
in-process EELS t8n (blockchain mode). t8n runs the reference STF and fills
every header commitment from the results. The narrow v0.6.2 adapter then runs
EELS run_stateless_guest over its native three-request-list input, appends the
two known-empty EIP-8282 request lists, and carries the independent reference
validation verdict into the expanded output with its recomputed request root.
Two things make validity achievable from a bare state test:
  - the canonical system-contract predeploys are injected into the alloc
    (_PREDEPLOYS): Amsterdam's CHECKED block-end system calls reject any block
    whose 7002/7251/8282 predeploys are absent, which would silently collapse the
    gate to agreeing-on-invalid (only the request-root echo + the validity bit
    would be compared, not execution);
  - the synthesized parent header is economically consistent with the block
    (gas_limit equal, gas_used at target, base_fee equal), so the reference's
    EIP-1559/4844 header expectations hold.
The dummy ancestor chain is hash-chained; the parent anchors the pre-state
root; t8n gets the same chain via env blockHeaders/blockHashes.
"""

import argparse
import dataclasses
import io
import json
import logging
import os
import sys

from devtools.harness import prestate_mpt
from devtools.paths import REPO_ROOT

HERE = os.path.dirname(os.path.abspath(__file__))
EVM_SAIL_ROOT = str(REPO_ROOT)
EXECSPECS_ROOT = os.environ.get(
    "EXECSPECS_ROOT",
    os.path.abspath(os.path.join(EVM_SAIL_ROOT, "..", "execution-specs")),
)
sys.path.insert(0, os.path.join(EXECSPECS_ROOT, "src"))

from ethereum.crypto.hash import keccak256
from ethereum_rlp import rlp
from ethereum_spec_tools.evm_tools.loaders.fork_loader import ForkLoad
from ethereum_spec_tools.evm_tools.t8n import T8N, ForkCache
from ethereum_spec_tools.forks import Hardfork
from ethereum_types.bytes import Bytes, Bytes8, Bytes32
from ethereum_types.numeric import U64, U256, Uint

_FORKS = {f.name.split(".")[-1]: f for f in Hardfork.discover()}
_FL = {}


def _fork(name):
    if name not in _FL:
        _FL[name] = ForkLoad(_FORKS[name.lower()])
    return _FL[name]


Z32 = b"\x00" * 32
EMPTY_TRIE = keccak256(rlp.encode(b""))


def _hi(s):
    return int(s, 16) if isinstance(s, str) else int(s or 0)


def _header(fk, **ov):
    """A fork-shaped block header with zeroed defaults."""
    d = {
        "parent_hash": Bytes32(Z32),
        "ommers_hash": keccak256(rlp.encode([])),
        "coinbase": Bytes(b"\x00" * 20),
        "state_root": Bytes32(Z32),
        "transactions_root": Bytes32(Z32),
        "receipt_root": Bytes32(Z32),
        "bloom": Bytes(b"\x00" * 256),
        "difficulty": Uint(0),
        "number": Uint(0),
        "gas_limit": Uint(0),
        "gas_used": Uint(0),
        "timestamp": U256(0),
        "extra_data": Bytes(b""),
        "mix_digest": Bytes32(Z32),
        "prev_randao": Bytes32(Z32),
        "nonce": Bytes8(b"\x00" * 8),
        "base_fee_per_gas": Uint(0),
        "withdrawals_root": Bytes32(EMPTY_TRIE),
        "blob_gas_used": U64(0),
        "excess_blob_gas": U64(0),
        "parent_beacon_block_root": Bytes32(Z32),
        "requests_hash": Bytes32(Z32),
        "block_access_list_hash": Bytes32(Z32),
        "slot_number": U64(0),
    }
    d.update(ov)
    header_fields = {field.name for field in dataclasses.fields(fk.Header)}
    return fk.Header(**{name: value for name, value in d.items() if name in header_fields})


# ---------------------------- EELS t8n ------------------------------------

_T8N_CACHE = None


def _t8n_options(chain_id, fork="Amsterdam", no_stateless=False, state_reward=None):
    """The argparse surface T8N reads, shaped for one in-process blockchain-mode
    run over stdin-style inputs. state_test=False is what makes t8n build the
    full block + stateless input/output pair (t8n_types.Result.update)."""
    return argparse.Namespace(
        input_alloc="stdin",
        input_env="stdin",
        input_txs="stdin",
        blob_parameters=None,
        output_alloc="alloc.json",
        output_result="result.json",
        output_body=None,
        output_basedir=".",
        state_chainid=chain_id,
        state_fork=fork,
        state_reward=state_reward,
        trace=False,
        opcode_count=None,
        state_test=False,
        no_stateless=no_stateless,
    )


_BLOB_TARGET = None


def _blob_target(fk):
    """TARGET_BLOB_GAS_PER_BLOCK, probed from calculate_excess_blob_gas so the
    blob schedule is never hardcoded: calc(excess=Q, used=0) = Q - target."""
    global _BLOB_TARGET
    if _BLOB_TARGET is None:
        q = 10**12
        probe = _header(fk, excess_blob_gas=U64(q), blob_gas_used=U64(0))
        _BLOB_TARGET = q - int(fk.calculate_excess_blob_gas(probe))
    return _BLOB_TARGET


def _ancestor_headers(fk, pre_root, number, parent_excess, parent_blob_used, base_fee, gas_limit):
    """Synthesize the dummy ancestor chain for blocks [n-min(256,n) .. n-1]:
    hash-chained RLP headers, the direct parent carrying the pre-state root
    anchor. The reference validates the BLOCK header against this parent
    (EELS validate_header), so the parent must be economically consistent
    with the block: gas_limit equal to the block's (within the 1/1024 bound),
    gas_used at target (gas_limit/2) so the expected base fee carries over
    unchanged, base_fee equal to the block's, and the blob-gas parameters the
    excess-blob-gas expectation is derived from (EIP-7918 reads the parent
    base fee too). t8n's env gets the SAME parent values, so both sides
    compute identical header expectations.
    Returns ({hex(number): "0x"+rlp}, {hex(number): "0x"+hash}) for t8n env."""
    count = min(256, number)
    headers, hashes = {}, {}
    phash = Z32
    for num in range(number - count, number):
        is_parent = num == number - 1
        h = _header(
            fk,
            number=Uint(num),
            parent_hash=Bytes32(phash),
            state_root=Bytes32(pre_root if is_parent else Z32),
            excess_blob_gas=U64(parent_excess if is_parent else 0),
            blob_gas_used=U64(parent_blob_used if is_parent else 0),
            base_fee_per_gas=Uint(base_fee if is_parent else 0),
            gas_limit=Uint(gas_limit if is_parent else 0),
            gas_used=Uint(gas_limit // 2 if is_parent else 0),
        )
        enc = rlp.encode(h)
        phash = keccak256(enc)
        headers[hex(num)] = "0x" + enc.hex()
        hashes[hex(num)] = "0x" + phash.hex()
    return headers, hashes


def _t8n_tx(tx, idx, chain_id):
    """The fixture's indexed tx as a geth-t8n JSON tx (v=r=s=0 + secretKey:
    t8n signs it with the fork-correct signing hash, including typed txs)."""
    j = {
        "nonce": tx["nonce"],
        "gas": tx["gasLimit"][idx["gas"]],
        "input": tx["data"][idx["data"]],
        "to": tx.get("to") or "",
        "value": tx["value"][idx["value"]],
        "secretKey": tx["secretKey"],
        "v": "0x0",
        "r": "0x0",
        "s": "0x0",
        "chainId": hex(chain_id),
    }
    if "gasPrice" in tx:
        j["gasPrice"] = tx["gasPrice"]
    else:
        j["maxFeePerGas"] = tx["maxFeePerGas"]
        j["maxPriorityFeePerGas"] = tx["maxPriorityFeePerGas"]
    # Presence selects EIP-2930 even when the selected list is empty. Dropping
    # [] would make t8n sign a legacy transaction while the fixture's txbytes
    # still carry a type-1 envelope, yielding a legacy receipt root for a typed
    # payload.
    if "accessLists" in tx and idx["data"] < len(tx["accessLists"]):
        j["accessList"] = tx["accessLists"][idx["data"]]
    for k in ("maxFeePerBlobGas", "blobVersionedHashes", "authorizationList"):
        if k in tx:
            j[k] = tx[k]
    return j


def _t8n_rlp_transactions(tx_hex):
    """Wrap one fixture transaction in t8n's RLP block-body input."""
    raw = bytes.fromhex(tx_hex.removeprefix("0x"))
    transaction = raw if raw[0] < 0x80 else rlp.decode(raw)
    return "0x" + rlp.encode([transaction]).hex()


# Canonical system-contract predeploys (extracted from the aligned Amsterdam
# corpus fill; consensus constants). A state-test alloc lacks them, but the
# Amsterdam block-end system calls are CHECKED: without the 7002/7251/8282 code the
# reference rejects EVERY block (block_exception), collapsing the byte-exact
# gate to agreeing-on-invalid. build_guest injects any that are absent so the
# built block is genuinely VALID end-to-end.
_PREDEPLOYS = {
    # EIP-4788 beacon roots
    "0x000f3df6d732807ef1319fb7b8bb8522d0beac02": {
        "nonce": "0x01",
        "balance": "0x00",
        "storage": {},
        "code": "0x3373fffffffffffffffffffffffffffffffffffffffe14604d57602036146024575f5ffd5b5f35801560495762001fff810690815414603c575f5ffd5b62001fff01545f5260205ff35b5f5ffd5b62001fff42064281555f359062001fff015500",
    },
    # EIP-2935 history storage
    "0x0000f90827f1c53a10cb7a02335b175320002935": {
        "nonce": "0x01",
        "balance": "0x00",
        "storage": {},
        "code": "0x3373fffffffffffffffffffffffffffffffffffffffe14604657602036036042575f35600143038111604257611fff81430311604257611fff9006545f5260205ff35b5f5ffd5b5f35611fff60014303065500",
    },
    # EIP-7002 withdrawal requests
    "0x00000961ef480eb55e80d19ad83579a64c007002": {
        "nonce": "0x01",
        "balance": "0x00",
        "storage": {},
        "code": "0x3373fffffffffffffffffffffffffffffffffffffffe1460cb5760115f54807fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff146101f457600182026001905f5b5f82111560685781019083028483029004916001019190604d565b909390049250505036603814608857366101f457346101f4575f5260205ff35b34106101f457600154600101600155600354806003026004013381556001015f35815560010160203590553360601b5f5260385f601437604c5fa0600101600355005b6003546002548082038060101160df575060105b5f5b8181146101835782810160030260040181604c02815460601b8152601401816001015481526020019060020154807fffffffffffffffffffffffffffffffff00000000000000000000000000000000168252906010019060401c908160381c81600701538160301c81600601538160281c81600501538160201c81600401538160181c81600301538160101c81600201538160081c81600101535360010160e1565b910180921461019557906002556101a0565b90505f6002555f6003555b5f54807fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff14156101cd57505f5b6001546002828201116101e25750505f6101e8565b01600290035b5f555f600155604c025ff35b5f5ffd",
    },
    # EIP-7251 consolidation requests
    "0x0000bbddc7ce488642fb579f8b00f3a590007251": {
        "nonce": "0x01",
        "balance": "0x00",
        "storage": {},
        "code": "0x3373fffffffffffffffffffffffffffffffffffffffe1460d35760115f54807fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1461019a57600182026001905f5b5f82111560685781019083028483029004916001019190604d565b9093900492505050366060146088573661019a573461019a575f5260205ff35b341061019a57600154600101600155600354806004026004013381556001015f358155600101602035815560010160403590553360601b5f5260605f60143760745fa0600101600355005b6003546002548082038060021160e7575060025b5f5b8181146101295782810160040260040181607402815460601b815260140181600101548152602001816002015481526020019060030154905260010160e9565b910180921461013b5790600255610146565b90505f6002555f6003555b5f54807fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff141561017357505f5b6001546001828201116101885750505f61018e565b01600190035b5f555f6001556074025ff35b5f5ffd",
    },
    # EIP-8282 builder deposit requests
    "0x0000bff46984e3725691fa540a8c7589300d8282": {
        "nonce": "0x01",
        "balance": "0x00",
        "storage": {},
        "code": "0x3373fffffffffffffffffffffffffffffffffffffffe1461011c575f54807fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff146102705760015460088111605257506058565b60089003015b601190600182026001905f5b5f821115607f57810190830284830290049160010191906064565b90939004925050503660b814609f57366102705734610270575f5260205ff35b8034106102705760383567ffffffffffffffff1680633b9aca001161027057633b9aca00029034031061027057600154600101600155600354806006026004015f358155600101602035815560010160403581556001016060358155600101608035815560010160a035905560b85f5f3760b85fa0600101600355005b60035460025480820380604011610131575060405b5f5b8181146101d7578281016006026004018160b8028154815260200181600101548152602001816002015480825260401c67ffffffffffffffff16816010018160381c81600701538160301c81600601538160281c81600501538160201c81600401538160181c81600301538160101c81600201538160081c816001015353602001816003015481526020018160040154815260200190600501549052600101610133565b91018092146101e957906002556101f4565b90505f6002555f6003555b36610242575f54600154817fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1461023057600882820111610238575b50505f610264565b0160089003610264565b7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff5b5f555f60015560b8025ff35b5f5ffd",
    },
    # EIP-8282 builder exit requests
    "0x000064d678505ad48f8ccb093bc65613800e8282": {
        "nonce": "0x01",
        "balance": "0x00",
        "storage": {},
        "code": "0x3373fffffffffffffffffffffffffffffffffffffffe1460e1575f54807fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff146101c65760015460028111605157506057565b60029003015b601190600182026001905f5b5f821115607e57810190830284830290049160010191906063565b909390049250505036603014609e57366101c657346101c6575f5260205ff35b34106101c657600154600101600155600354806003026004013381556001015f35815560010160203590553360601b5f5260305f60143760445fa0600101600355005b6003546002548082038060101160f5575060105b5f5b81811461012d5782810160030260040181604402815460601b8152601401816001015481526020019060020154905260010160f7565b910180921461013f579060025561014a565b90505f6002555f6003555b36610198575f54600154817fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff146101865760028282011161018e575b50505f6101ba565b01600290036101ba565b7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff5b5f555f6001556044025ff35b5f5ffd",
    },
}


def _fixture_protocol_fork(fork_name):
    """Resolve the stable v0.6.2 schema byte for a fixture fork."""
    from ethereum.forks.amsterdam.stateless import ProtocolFork

    try:
        return ProtocolFork[fork_name]
    except KeyError as exc:
        raise ValueError(f"unsupported stateless fixture fork: {fork_name}") from exc


def _fixture_chain_config(fork_name, chain_id):
    """Build the shared stateless-input chain config for one active fork."""
    from ethereum.forks.amsterdam.stateless import (
        ChainConfig,
        ForkActivation,
        ForkConfig,
        ProtocolFork,
    )

    protocol_fork = _fixture_protocol_fork(fork_name)
    timestamp_fork = protocol_fork >= ProtocolFork.Shanghai
    return ChainConfig(
        chain_id=U64(chain_id),
        active_fork=ForkConfig(
            activation=ForkActivation(
                block_number=None if timestamp_fork else U64(0),
                timestamp=U64(0) if timestamp_fork else None,
            ),
        ),
    )


def _quantity_bytes32(value):
    return Bytes32(_hi(value).to_bytes(32, "big"))


def _serialize_v062_pair(stateless_input, successful_validation, fork_name):
    from devtools.harness.v062_compat import serialize_v062_pair

    return serialize_v062_pair(
        stateless_input,
        successful_validation,
        _fixture_protocol_fork(fork_name),
    )


def _build_state_test_guest(case):
    """Build a fork-correct shared SSZ input from one state-test case."""
    from ethereum.crypto.hash import Hash32
    from ethereum.forks.amsterdam.execution_engine.requests import (
        decode_execution_requests,
    )
    from ethereum.forks.amsterdam.execution_engine.types import (
        ExecutionPayload,
        NewPayloadRequest,
    )
    from ethereum.forks.amsterdam.fork_types import Bloom
    from ethereum.forks.amsterdam.stateless import (
        ExecutionWitness,
        StatelessInput,
    )
    from ethereum.forks.amsterdam.transactions import (
        BlobTransaction,
        decode_transaction,
        recover_transaction_public_key,
    )
    from ethereum.state import Address, Root

    env, pre, tx, idx = case["env"], case["pre"], case["tx"], case["idx"]
    fork_name = case["fork"]
    fk = _fork(fork_name.lower())
    header_fields = {field.name for field in dataclasses.fields(fk.Header)}
    chain_id = _hi(case.get("config", {}).get("chainid", env.get("currentChainId", "0x1")))
    number = _hi(env.get("currentNumber", "0x1"))
    if number == 0:
        raise ValueError("stateless state-test blocks require a parent header")

    root_hex, nodes = prestate_mpt.build(pre)
    pre_root = bytes.fromhex(root_hex[2:])
    base_fee = _hi(env.get("currentBaseFee", "0x0"))
    gas_limit = _hi(env.get("currentGasLimit", "0x0"))
    has_blob_fields = "excess_blob_gas" in header_fields
    if has_blob_fields and ("parentExcessBlobGas" in env or "parentBlobGasUsed" in env):
        parent_excess = _hi(env.get("parentExcessBlobGas", "0x0"))
        parent_blob_used = _hi(env.get("parentBlobGasUsed", "0x0"))
    elif has_blob_fields and "currentExcessBlobGas" in env:
        parent_excess = _hi(env["currentExcessBlobGas"]) + _blob_target(fk)
        parent_blob_used = 0
    else:
        parent_excess = parent_blob_used = 0

    block_headers, block_hashes = _ancestor_headers(
        fk,
        pre_root,
        number,
        parent_excess,
        parent_blob_used,
        base_fee,
        gas_limit,
    )
    randao = env.get("currentRandom", env.get("currentDifficulty", "0x0"))
    t8n_env = {
        "currentCoinbase": env["currentCoinbase"],
        "currentGasLimit": env.get("currentGasLimit", "0x0"),
        "currentNumber": hex(number),
        "currentTimestamp": env.get("currentTimestamp", "0x0"),
        "blockHeaders": block_headers,
        "blockHashes": block_hashes,
    }
    if "prev_randao" in header_fields:
        t8n_env["currentRandom"] = randao
    else:
        t8n_env["currentDifficulty"] = env.get("currentDifficulty", "0x0")
    if "base_fee_per_gas" in header_fields:
        t8n_env["currentBaseFee"] = hex(base_fee)
        t8n_env["parentBaseFee"] = hex(base_fee)
    if "withdrawals_root" in header_fields:
        t8n_env["withdrawals"] = []
    if has_blob_fields:
        t8n_env["parentExcessBlobGas"] = hex(parent_excess)
        t8n_env["parentBlobGasUsed"] = hex(parent_blob_used)
    if "parent_beacon_block_root" in header_fields:
        t8n_env["parentBeaconBlockRoot"] = "0x" + Z32.hex()

    tx_hex = case.get("txbytes")
    txs_input = _t8n_rlp_transactions(tx_hex) if tx_hex else [_t8n_tx(tx, idx, chain_id)]
    stdin_json = {
        "alloc": pre,
        "env": t8n_env,
        "txs": txs_input,
    }
    global _T8N_CACHE
    if _T8N_CACHE is None:
        _T8N_CACHE = ForkCache()
    t8n = T8N(
        _t8n_options(
            chain_id,
            fork=fork_name,
            no_stateless=True,
        ),
        io.StringIO(),
        io.StringIO(json.dumps(stdin_json)),
        _T8N_CACHE,
    )
    t8n.run_blockchain_test()
    result = t8n.result

    if tx_hex:
        tx_bytes = Bytes(bytes.fromhex(tx_hex.removeprefix("0x")))
    else:
        if len(t8n.txs.all_txs) != 1:
            raise ValueError("t8n did not produce one signed transaction")
        signed_tx = t8n.txs.all_txs[0]
        tx_bytes = Bytes(
            bytes(signed_tx) if isinstance(signed_tx, bytes) else rlp.encode(signed_tx)
        )

    decoded_tx = decode_transaction(tx_bytes)
    public_key = recover_transaction_public_key(U64(chain_id), decoded_tx)
    versioned_hashes = (
        tuple(decoded_tx.blob_versioned_hashes) if isinstance(decoded_tx, BlobTransaction) else ()
    )
    execution_requests = decode_execution_requests(
        tuple(Bytes(request) for request in (result.requests or ()))
    )

    parent_hash = Bytes32(bytes.fromhex(block_hashes[hex(number - 1)].removeprefix("0x")))
    is_pos = "prev_randao" in header_fields
    request_hash = (
        result.requests_hash
        if result.requests_hash is not None
        else __import__("hashlib").sha256(b"").digest()
    )
    header = _header(
        fk,
        parent_hash=parent_hash,
        coinbase=Bytes(bytes.fromhex(env["currentCoinbase"][2:])),
        state_root=Bytes32(bytes(result.state_root)),
        transactions_root=Bytes32(bytes(result.tx_root)),
        receipt_root=Bytes32(bytes(result.receipt_root)),
        bloom=Bytes(bytes(result.bloom)),
        difficulty=Uint(0 if is_pos else int(result.difficulty)),
        number=Uint(number),
        gas_limit=Uint(gas_limit),
        gas_used=Uint(int(result.gas_used)),
        timestamp=U256(_hi(env.get("currentTimestamp", "0x0"))),
        extra_data=Bytes(b""),
        mix_digest=Bytes32(Z32),
        prev_randao=_quantity_bytes32(randao),
        base_fee_per_gas=Uint(int(result.base_fee or 0)),
        withdrawals_root=Bytes32(bytes(result.withdrawals_root or EMPTY_TRIE)),
        blob_gas_used=U64(int(result.blob_gas_used or 0)),
        excess_blob_gas=U64(int(result.excess_blob_gas or 0)),
        parent_beacon_block_root=Bytes32(Z32),
        requests_hash=Bytes32(bytes(request_hash)),
        slot_number=U64(_hi(env.get("slotNumber", "0x0"))),
    )
    block_hash = Hash32(keccak256(rlp.encode(header)))

    witness_codes = sorted(
        {
            bytes.fromhex(account.get("code", "0x").removeprefix("0x"))
            for account in pre.values()
            if account.get("code", "0x") not in ("", "0x")
        }
    )
    witness = ExecutionWitness(
        state=tuple(Bytes(node) for node in nodes.values()),
        codes=tuple(Bytes(code) for code in witness_codes),
        headers=tuple(
            Bytes(bytes.fromhex(encoded.removeprefix("0x"))) for encoded in block_headers.values()
        ),
    )
    payload_prev_randao = (
        _quantity_bytes32(randao) if is_pos else _quantity_bytes32(result.difficulty)
    )
    payload = ExecutionPayload(
        parent_hash=parent_hash,
        fee_recipient=Address(bytes.fromhex(env["currentCoinbase"][2:])),
        state_root=Root(bytes(result.state_root)),
        receipts_root=Root(bytes(result.receipt_root)),
        logs_bloom=Bloom(bytes(result.bloom)),
        prev_randao=payload_prev_randao,
        block_number=Uint(number),
        gas_limit=Uint(gas_limit),
        gas_used=Uint(int(result.gas_used)),
        timestamp=U256(_hi(env.get("currentTimestamp", "0x0"))),
        extra_data=Bytes(b""),
        base_fee_per_gas=Uint(int(result.base_fee or 0)),
        block_hash=block_hash,
        transactions=(tx_bytes,),
        withdrawals=(),
        blob_gas_used=U64(int(result.blob_gas_used or 0)),
        excess_blob_gas=U64(int(result.excess_blob_gas or 0)),
        block_access_list=Bytes(rlp.encode([])),
        slot_number=U64(_hi(env.get("slotNumber", "0x0"))),
    )
    stateless_input = StatelessInput(
        new_payload_request=NewPayloadRequest(
            execution_payload=payload,
            versioned_hashes=versioned_hashes,
            parent_beacon_block_root=Root(Z32),
            execution_requests=execution_requests,
        ),
        witness=witness,
        chain_config=_fixture_chain_config(fork_name, chain_id),
        public_keys=(Bytes(public_key),),
    )
    return _serialize_v062_pair(
        stateless_input,
        result.block_exception is None and not t8n.txs.rejected_txs,
        fork_name,
    )


def build_guest(case):
    """case: {env, pre, tx, fork, idx} -> (input_bytes, expected_output_bytes).
    The model guest is Amsterdam-locked, but historical state tests are lifted
    into an Amsterdam stateless input after their fork-specific EELS transition.
    The expected validation verdict comes from EELS run_stateless_guest; the
    isolated v0.6.2 adapter accounts for the two additional empty request lists
    and their changed SSZ request root."""
    return _build_state_test_guest(case)


def _serve():
    """One case JSON per stdin line -> {input, expected} or {err}."""
    logging.getLogger("T8N").setLevel(logging.ERROR)  # rejected-tx noise
    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue
        try:
            case = json.loads(line)
            inp, exp = build_guest(case)
            resp = {"input": inp.hex(), "expected": exp.hex()}
        except Exception as e:
            resp = {"err": f"{type(e).__name__}: {e}"}
        sys.stdout.write(json.dumps(resp) + "\n")
        sys.stdout.flush()


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--serve":
        _serve()
    else:
        with open(sys.argv[1]) as case_file:
            case = json.load(case_file)
        inp, exp = build_guest(case)
        print(json.dumps({"input": inp.hex(), "expected": exp.hex()}))
