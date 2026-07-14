#!/usr/bin/env python3
"""Build a valid stateless block input/output pair from an EEST state-test case.

Runs under the execution-specs venv (it reuses that project's stateless SSZ types
and serializer, so the bytes match the model's stateless-input decoder exactly). run.py
(which lacks these deps) drives this over --serve: one case JSON per stdin line,
one JSON response per stdout line ({"input": hex[, "expected": hex]} or
{"err": msg}).

A fully VALID single-tx block is built by
executing the case through the in-process EELS t8n (blockchain mode): t8n runs
the reference STF, fills every header commitment from the results, builds the
execution witness, serializes the SszStatelessInput, AND runs the reference
stateless guest (run_stateless_guest) over those exact bytes -- returning the
byte-exact expected SszStatelessValidationResult alongside the input. t8n itself
asserts the result validates (successful_validation) unless the tx was rejected,
so a builder bug cannot silently produce an invalid-but-agreeing pair. Two
things make validity achievable from a bare state test:
  - the canonical system-contract predeploys are injected into the alloc
    (_PREDEPLOYS): Amsterdam's CHECKED block-end system calls reject any block
    whose 7002/7251 predeploys are absent, which would silently collapse the
    gate to agreeing-on-invalid (only the request-root echo + the validity bit
    would be compared, not execution);
  - the synthesized parent header is economically consistent with the block
    (gas_limit equal, gas_used at target, base_fee equal), so the reference's
    EIP-1559/4844 header expectations hold.
The dummy ancestor chain is hash-chained; the parent anchors the pre-state
root; t8n gets the same chain via env blockHeaders/blockHashes.
"""
import argparse, io, logging, sys, json
sys.path.insert(0, "/Users/f/dev/ethereum/execution-specs/src")
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from ethereum_rlp import rlp
from ethereum.crypto.hash import keccak256
from ethereum_types.numeric import U256, Uint, U64
from ethereum_types.bytes import Bytes, Bytes8, Bytes32
from ethereum_spec_tools.forks import Hardfork
from ethereum_spec_tools.evm_tools.loaders.fork_loader import ForkLoad
from ethereum_spec_tools.evm_tools.t8n import T8N, ForkCache
import prestate_mpt  # reuse build() for the pre-state MPT

_FORKS = {f.name.split(".")[-1]: f for f in Hardfork.discover()}
_FL = {}
def _fork(name):
    if name not in _FL:
        _FL[name] = ForkLoad(_FORKS[name.lower()])
    return _FL[name]

Z32 = b"\x00" * 32
EMPTY_TRIE = keccak256(rlp.encode(b""))

def _hi(s): return int(s, 16) if isinstance(s, str) else int(s or 0)

def _header(fk, **ov):
    """An amsterdam block header with zeroed defaults; override the live fields."""
    d = dict(parent_hash=Bytes32(Z32), ommers_hash=keccak256(rlp.encode([])),
             coinbase=Bytes(b"\x00" * 20), state_root=Bytes32(Z32), transactions_root=Bytes32(Z32),
             receipt_root=Bytes32(Z32), bloom=Bytes(b"\x00" * 256), difficulty=Uint(0), number=Uint(0),
             gas_limit=Uint(0), gas_used=Uint(0), timestamp=U256(0), extra_data=Bytes(b""),
             prev_randao=Bytes32(Z32), nonce=Bytes8(b"\x00" * 8), base_fee_per_gas=Uint(0),
             withdrawals_root=Bytes32(EMPTY_TRIE), blob_gas_used=U64(0), excess_blob_gas=U64(0),
             parent_beacon_block_root=Bytes32(Z32), requests_hash=Bytes32(Z32),
             block_access_list_hash=Bytes32(Z32), slot_number=U64(0))
    d.update(ov)
    return fk.Header(**d)

# ---------------------------- EELS t8n ------------------------------------

_T8N_CACHE = None


def _t8n_options(chain_id):
    """The argparse surface T8N reads, shaped for one in-process blockchain-mode
    run over stdin-style inputs. state_test=False is what makes t8n build the
    full block + stateless input/output pair (t8n_types.Result.update)."""
    return argparse.Namespace(
        input_alloc="stdin", input_env="stdin", input_txs="stdin",
        blob_parameters=None,
        output_alloc="alloc.json", output_result="result.json",
        output_body=None, output_basedir=".",
        state_chainid=chain_id, state_fork="Amsterdam", state_reward=None,
        trace=False, opcode_count=None,
        state_test=False, no_stateless=False,
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


def _ancestor_headers(fk, pre_root, number, parent_excess, parent_blob_used,
                      base_fee, gas_limit):
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
        h = _header(fk, number=Uint(num), parent_hash=Bytes32(phash),
                    state_root=Bytes32(pre_root if is_parent else Z32),
                    excess_blob_gas=U64(parent_excess if is_parent else 0),
                    blob_gas_used=U64(parent_blob_used if is_parent else 0),
                    base_fee_per_gas=Uint(base_fee if is_parent else 0),
                    gas_limit=Uint(gas_limit if is_parent else 0),
                    gas_used=Uint(gas_limit // 2 if is_parent else 0))
        enc = rlp.encode(h)
        phash = keccak256(enc)
        headers[hex(num)] = "0x" + enc.hex()
        hashes[hex(num)] = "0x" + phash.hex()
    return headers, hashes


def _t8n_tx(tx, idx, chain_id):
    """The fixture's indexed tx as a geth-t8n JSON tx (v=r=s=0 + secretKey:
    t8n signs it with the fork-correct signing hash, including typed txs)."""
    j = {"nonce": tx["nonce"], "gas": tx["gasLimit"][idx["gas"]],
         "input": tx["data"][idx["data"]], "to": tx.get("to") or "",
         "value": tx["value"][idx["value"]], "secretKey": tx["secretKey"],
         "v": "0x0", "r": "0x0", "s": "0x0", "chainId": hex(chain_id)}
    if "gasPrice" in tx:
        j["gasPrice"] = tx["gasPrice"]
    else:
        j["maxFeePerGas"] = tx["maxFeePerGas"]
        j["maxPriorityFeePerGas"] = tx["maxPriorityFeePerGas"]
    if "accessLists" in tx and idx["data"] < len(tx["accessLists"]) \
            and tx["accessLists"][idx["data"]]:
        j["accessList"] = tx["accessLists"][idx["data"]]
    for k in ("maxFeePerBlobGas", "blobVersionedHashes", "authorizationList"):
        if k in tx:
            j[k] = tx[k]
    return j


# Canonical system-contract predeploys (extracted from the aligned Amsterdam
# corpus fill; consensus constants). A state-test alloc lacks them, but the
# Amsterdam block-end system calls are CHECKED: without the 7002/7251 code the
# reference rejects EVERY block (block_exception), collapsing the byte-exact
# gate to agreeing-on-invalid. build_guest injects any that are absent so the
# built block is genuinely VALID end-to-end.
_PREDEPLOYS = {
    # EIP-4788 beacon roots
    "0x000f3df6d732807ef1319fb7b8bb8522d0beac02": {
        "nonce": "0x01", "balance": "0x00", "storage": {},
        "code": "0x3373fffffffffffffffffffffffffffffffffffffffe14604d57602036146024575f5ffd5b5f35801560495762001fff810690815414603c575f5ffd5b62001fff01545f5260205ff35b5f5ffd5b62001fff42064281555f359062001fff015500",
    },
    # EIP-2935 history storage
    "0x0000f90827f1c53a10cb7a02335b175320002935": {
        "nonce": "0x01", "balance": "0x00", "storage": {},
        "code": "0x3373fffffffffffffffffffffffffffffffffffffffe14604657602036036042575f35600143038111604257611fff81430311604257611fff9006545f5260205ff35b5f5ffd5b5f35611fff60014303065500",
    },
    # EIP-7002 withdrawal requests
    "0x00000961ef480eb55e80d19ad83579a64c007002": {
        "nonce": "0x01", "balance": "0x00", "storage": {},
        "code": "0x3373fffffffffffffffffffffffffffffffffffffffe1460cb5760115f54807fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff146101f457600182026001905f5b5f82111560685781019083028483029004916001019190604d565b909390049250505036603814608857366101f457346101f4575f5260205ff35b34106101f457600154600101600155600354806003026004013381556001015f35815560010160203590553360601b5f5260385f601437604c5fa0600101600355005b6003546002548082038060101160df575060105b5f5b8181146101835782810160030260040181604c02815460601b8152601401816001015481526020019060020154807fffffffffffffffffffffffffffffffff00000000000000000000000000000000168252906010019060401c908160381c81600701538160301c81600601538160281c81600501538160201c81600401538160181c81600301538160101c81600201538160081c81600101535360010160e1565b910180921461019557906002556101a0565b90505f6002555f6003555b5f54807fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff14156101cd57505f5b6001546002828201116101e25750505f6101e8565b01600290035b5f555f600155604c025ff35b5f5ffd",
    },
    # EIP-7251 consolidation requests
    "0x0000bbddc7ce488642fb579f8b00f3a590007251": {
        "nonce": "0x01", "balance": "0x00", "storage": {},
        "code": "0x3373fffffffffffffffffffffffffffffffffffffffe1460d35760115f54807fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1461019a57600182026001905f5b5f82111560685781019083028483029004916001019190604d565b9093900492505050366060146088573661019a573461019a575f5260205ff35b341061019a57600154600101600155600354806004026004013381556001015f358155600101602035815560010160403590553360601b5f5260605f60143760745fa0600101600355005b6003546002548082038060021160e7575060025b5f5b8181146101295782810160040260040181607402815460601b815260140181600101548152602001816002015481526020019060030154905260010160e9565b910180921461013b5790600255610146565b90505f6002555f6003555b5f54807fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff141561017357505f5b6001546001828201116101885750505f61018e565b01600190035b5f555f6001556074025ff35b5f5ffd",
    },
}


def build_guest(case):
    """case: {env, pre, tx, fork, idx} -> (input_bytes, expected_output_bytes).
    Amsterdam only (the stateless guest is Amsterdam-locked); the expected bytes
    are the EELS reference guest's SszStatelessValidationResult over the SAME
    input, so the model guest is gated byte-exact against the reference."""
    env, pre, tx, idx = case["env"], case["pre"], case["tx"], case["idx"]
    chain_id = _hi(env.get("currentChainId", "0x1"))
    fk = _fork("amsterdam")
    number = _hi(env.get("currentNumber", "0x1"))

    # Inject any missing system-contract predeploys (see _PREDEPLOYS): the block
    # must be genuinely valid, and Amsterdam's checked block-end system calls
    # reject a block whose predeploys are absent. A test that carries its own
    # copy keeps it.
    have = {a.lower() for a in pre}
    pre = dict(pre)
    for addr, acct in _PREDEPLOYS.items():
        if addr not in have:
            pre[addr] = acct

    root_hex, _ = prestate_mpt.build(pre)
    pre_root = bytes.fromhex(root_hex[2:])

    # Parent blob-gas parameters: prefer the fixture's parent values; else invert
    # currentExcessBlobGas through the schedule (excess = E + target, used = 0
    # => calc(parent) = E); else zeros. The SAME values go to t8n's env and into
    # the synthesized parent header, so t8n's header excess and the reference
    # verifier's expectation agree by construction.
    if "parentExcessBlobGas" in env or "parentBlobGasUsed" in env:
        p_excess = _hi(env.get("parentExcessBlobGas", "0x0"))
        p_used = _hi(env.get("parentBlobGasUsed", "0x0"))
    elif "currentExcessBlobGas" in env:
        p_excess = _hi(env["currentExcessBlobGas"]) + _blob_target(fk)
        p_used = 0
    else:
        p_excess = p_used = 0
    # The parent's base fee IS the block's (parent gas_used at target keeps the
    # EIP-1559 expectation unchanged); a fixture parentBaseFee is deliberately
    # ignored -- the reference derives the block's base fee from OUR parent.
    base_fee = _hi(env.get("currentBaseFee", "0x0"))
    gas_limit = _hi(env.get("currentGasLimit", "0x0"))
    block_headers, block_hashes = _ancestor_headers(
        fk, pre_root, number, p_excess, p_used, base_fee, gas_limit)

    randao = env.get("currentRandom", env.get("currentDifficulty", "0x0"))
    t8n_env = {
        "currentCoinbase": env["currentCoinbase"],
        "currentGasLimit": env.get("currentGasLimit", "0x0"),
        "currentNumber": hex(number),
        "currentTimestamp": env.get("currentTimestamp", "0x0"),
        "currentRandom": randao,
        "currentBaseFee": hex(base_fee),
        "parentBaseFee": hex(base_fee),
        "parentExcessBlobGas": hex(p_excess),
        "parentBlobGasUsed": hex(p_used),
        "parentBeaconBlockRoot": "0x" + Z32.hex(),
        "slotNumber": hex(number),
        "withdrawals": [],
        "blockHeaders": block_headers,
        "blockHashes": block_hashes,
    }
    stdin_json = {"alloc": pre, "env": t8n_env,
                  "txs": [_t8n_tx(tx, idx, chain_id)]}

    global _T8N_CACHE
    if _T8N_CACHE is None:
        _T8N_CACHE = ForkCache()
    t8n = T8N(_t8n_options(chain_id), io.StringIO(),
              io.StringIO(json.dumps(stdin_json)), _T8N_CACHE)
    t8n.run_blockchain_test()
    r = t8n.result
    inp = getattr(r, "stateless_input_bytes", None)
    outp = getattr(r, "stateless_output_bytes", None)
    if inp is None or outp is None:
        raise RuntimeError(
            f"t8n produced no stateless pair (block_exception={r.block_exception!r},"
            f" rejected={t8n.txs.rejected_txs!r})")
    return bytes(inp), bytes(outp)


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
        case = json.load(open(sys.argv[1]))
        inp, exp = build_guest(case)
        print(json.dumps({"input": inp.hex(), "expected": exp.hex()}))
