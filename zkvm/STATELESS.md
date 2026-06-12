# Canonical stateless guest — design + staged roadmap

Goal: make the EL-IR Sail zkVM guest read its block from **private input** in the
**canonical Amsterdam stateless format**, validate it statelessly against an
execution **witness**, and emit the canonical result — matching
`ethereum/execution-specs` (branch `projects/zkevm`, the `forks/amsterdam`
`stateless_*` modules). This replaces the build-time fixture block in
`zkvm_block.sail` with a real, arbitrary, witness-backed input.

Reference (local): `/Users/f/dev/ethereum/execution-specs/src/ethereum/forks/amsterdam/`
- `stateless_guest.py` — `run_stateless_guest(input_bytes) -> output_bytes`
- `stateless.py` — `StatelessInput`/`StatelessValidationResult`, `verify_stateless_new_payload`
- `stateless_ssz.py` — SSZ containers + schema id
- `witness_state.py` — witness-backed reads (`build_node_db`, `_trie_lookup`, ...)
- `stateless_host.py` — host-side encoder (test-vector generation)

## The contract (what the guest must do)

```
run_stateless_guest(input_bytes):
  # input_bytes = 2-byte BE schema id (0x0001) ++ SSZ(SszStatelessInput)
  stateless_input = decode(input_bytes)                 # else -> failed sentinel result
  result          = verify_stateless_new_payload(input)
  return SSZ(SszStatelessValidationResult)              # the public output
```

`verify_stateless_new_payload`:
1. `new_payload_request_root = hash_tree_root(SszNewPayloadRequest)` — the **public
   commitment** to *which* block was validated (SSZ merkleization → **SHA-256**).
2. `validate_chain_config` — active fork == Amsterdam, blob schedule matches.
3. `validate_headers(witness.headers)` — RLP-decode, assert `parent_hash`
   contiguity, `block_hash[i] = keccak(header_bytes[i])`; `parent = headers[-1]`.
4. `pre_state = WitnessState(node_db=build_node_db(witness.state),
   state_root=parent.state_root, code_db=build_code_db(witness.codes))`.
5. `execute_new_payload_request(npr, pre_state, chain_context, public_keys)` —
   run the block; recompute post-state root via the witness incremental-MPT and
   assert it equals `payload.state_root`; check block hash, receipts/gas, etc.
6. `successful_validation = (no exception)`.

Output: `{ new_payload_request_root: Bytes32, successful_validation: bool,
chain_config: SszChainConfig }`.

## SSZ schema (the wire format to decode/encode)

Schema id: `STATELESS_INPUT_SCHEMA_ID = 0x0001`, 2 bytes, big-endian, prepended.

```
SszStatelessInput        { new_payload_request, witness, chain_config, public_keys[ByteVector[65]] }
SszNewPayloadRequest     { execution_payload, versioned_hashes[Bytes32], parent_beacon_block_root, execution_requests }
SszExecutionPayload      { parent_hash, fee_recipient[20], state_root, receipts_root, logs_bloom[256],
                           prev_randao, block_number:u64, gas_limit:u64, gas_used:u64, timestamp:u64,
                           extra_data:ByteList, base_fee_per_gas:u256, block_hash,
                           transactions:List[ByteList], withdrawals:List[SszWithdrawal],
                           blob_gas_used:u64, excess_blob_gas:u64, block_access_list:ByteList, slot_number:u64 }
SszWithdrawal            { index:u64, validator_index:u64, address[20], amount:u64 }
SszExecutionWitness      { state:List[ByteList], codes:List[ByteList], headers:List[ByteList] }
SszChainConfig           { chain_id:u64, active_fork:SszForkConfig }
SszForkConfig            { fork:u64, activation:{block_number:Opt[u64], timestamp:Opt[u64]}, blob_schedule:Opt[SszBlobSchedule] }
SszStatelessValidationResult { new_payload_request_root:Bytes32, successful_validation:boolean, chain_config:SszChainConfig }
```

SSZ rules we must implement: fixed-size fields inline; each variable-size field
(ByteList, List[...]) is a 4-byte little-endian **offset** in the fixed section,
data packed in the variable section in field order; `List` length is recovered
from offsets / remaining bytes; `Optional` is modeled as `List[T, 1]` (0 or 1
elements). `uint256` is 32-byte little-endian; `uint64` 8-byte little-endian;
`Bytes32`/`ByteVector[N]` are fixed N-byte. **`hash_tree_root` uses SHA-256**
(distinct from execution keccak).

## Mapping onto the EL-IR Sail model (leverage we already have)

- **keccak** — FFI accelerator (`ffi/keccak_ffi.c`); used by node_db, header
  hashes, secure-trie key hashing, post-state root.
- **RLP** — `sail/rlp.sail` (encode). We need RLP **decode** for headers,
  account leaves, storage leaves, transactions.
- **MPT** — `sail/mpt.sail` builds a trie from pairs → root. We need MPT
  **decode + walk** (`decode_witness_to_mpt` + `_trie_lookup`) over a
  `keccak(node) → node` db.
- **state root** — `sail/state_root.sail` recomputes a full root; witness mode
  recomputes from a partial incremental-MPT (post-change).
- **block hash / header RLP** — `sail/block_hash.sail`.
- **execution** — `process_block` already runs txs/withdrawals; today it reads a
  flat seeded world. Stateless mode must serve `k_get_balance/nonce/code` + SLOAD
  from the witness MPT (lazy), keyed by `keccak(addr)`/`keccak(slot)`.

New, not yet present: an **SSZ codec** (decode + encode), an **SSZ
hash_tree_root** (needs **SHA-256**, a second C accelerator alongside keccak),
**RLP decode**, and **witness-MPT lazy reads**.

## Staged roadmap — DONE (S0–S5, each validated on spike)

- **S0 ✅ Test-vector generator (host).** `gen_vector.py` (remerkleable + py-trie)
  emits a schema-prefixed `SszStatelessInput` for the fixture block with a real
  secure-trie witness, plus a sidecar of expected fields/roots/result bytes.
  `--bad` emits a tampered (corrupted `state_root`) vector for the fail path.
- **S1 ✅ SSZ decode envelope** (`sail/ssz.sail`). Decodes `SszStatelessInput`
  from `read_input` via `ssz_src_byte`; offset navigation + the 540-byte payload
  fixed-region map.
- **S2 ✅ Witness-backed reads** (`sail/rlp_decode.sail`, `sail/mpt_witness.sail`).
  RLP decode, `build_node_db`, secure-trie walk (branch/extension/leaf, hex-prefix),
  account/storage leaf decode; the decoded block executes against witness state.
- **S3 ✅ Post-state verification.** `compute_state_root()` over the executed
  world is checked against the decoded `payload.state_root`.
- **S4 ✅ Public-output commitment** (`sail/sha256.sail`, `sail/ssz_htr.sail`).
  SHA-256 accelerator + full SSZ `hash_tree_root` (zero-hash-pruned merkleization)
  → `new_payload_request_root`; emits SSZ `SszStatelessValidationResult` via
  `write_output` (105 bytes, byte-exact vs remerkleable).
- **S5 ✅ Result/closure.** `successful_validation` from chain-config (Amsterdam)
  + header-contiguity + execution + post-state checks; a failed validation is a
  normal result (exit 0), only a guest malfunction is abnormal (exit 134).

## Remaining refinements (documented cuts, not blockers)

- **block_hash check**: recompute `keccak(rlp(header))` — needs the full
  Amsterdam header field set + tx/receipt/withdrawal tries (state root is done).
- **sender from `public_keys`**: DONE — the guest derives the tx sender as
  `keccak(pubkey_xy)[12:]` from `public_keys[0]` (no ecrecover; the host attests
  the pubkey). The whole block — env, tx, withdrawal, and sender — now comes from
  the private input.
- **non-empty `execution_requests`** (deposits/withdrawals/consolidations) and
  blob/SetCode tx types in the htr/decode paths.
- **lazy arbitrary-access witness reads**: pre-state is seeded for the touched
  accounts; serving SLOAD/balance for runtime-discovered addresses lazily (the
  general stateless model) is the generalization of S2.

## Target architecture: BAL-driven execution (the scaling end-state)

The current kernel world state is list-based, which has two measured/confirmed
scaling problems for large blocks:
- **O(N²) lookups** — `st_lookup`/`acc_lookup` scan a growing list per access
  (`zkvm_bench.sail`: 97M instructions for 400 SSTOREs, clean O(N²)).
- **O(C·S) snapshot deep-copy** — *fixed* by journal-based revert (commit
  `5bfe0ab`): per-frame undo log instead of deep-copying the whole world per CALL.

The lookup O(N²) is NOT cleanly fixable with a Sail data-structure swap: Sail's
value model **deep-copies on assignment** (so a persistent map/tree still has
O(n) updates), and the only in-place primitive — a register `vector` — has no
large-N initializer and would *regress* small blocks (per-snapshot bucket
overhead). So the right fix is architectural, not a structure swap:

**Use the EIP-7928 `block_access_list` (BAL) to make transactions independent.**
The BAL records, per tx, the access set (addresses + storage keys) and the
post-values. With it:
1. Each tx loads a **small working set** = its BAL slice (pre-values from the
   witness, overlaid with the running diff for those keys). It executes over
   *only* that set — every access is O(its access set), bounded.
2. **Cross-tx reads don't scan**: the BAL provides the value a tx sees, so txs
   are independent — the block-wide state list **never materializes**.
3. The post-state root is built **once** at the end by folding all BAL diffs
   over the witness base (the incremental-MPT recompute).

For a realistic block (many txs, each touching a handful of slots) this is
**linear** in total accesses — the O(N²) is gone and working sets stay tiny
(snapshot/revert is cheap too). It also subsumes the lazy-witness-reads gap (the
per-tx loads) and the incremental-root gap (the one-shot BAL fold). A single tx
touching N distinct slots still scans its N-slot working set (the pathological
`zkvm_bench` shape), but real txs touch few slots.

Implementation stages (we currently decode `block_access_list` and ignore it):
1. `gen_vector.py` populates the BAL (per-tx accesses + post-values); guest
   decodes it; add an "executed accesses ⊆ BAL" verification (sound, testable,
   no perf change yet — the soundness hook that authenticates the access set).
2. Restructure `process_block`/`el_kernel.sail` onto **per-tx working sets**
   loaded from `witness ∪ BAL-overlay`, restricted to the access set.
3. Accumulate diffs; build the post-state root once from
   `witness-base ⊕ all-BAL-diffs` (incremental MPT, replacing the full
   `compute_state_root` rebuild).

This is the largest single change in the roadmap — it rewires the execution
model, the kernel state, and the root computation around the BAL. Best done as a
focused effort with the BAL populated. The current tip (`5bfe0ab`) is a
validated, 8.15×-faster, journal-reverting stateless guest to build it on.
