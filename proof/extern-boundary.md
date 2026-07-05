# Generated Extern Boundary

This file inventories the generated Sail-to-Coq boundary for the full
`evm-sail` guest. It is the proof-facing contract layer for operations that the
Sail model marks as `impure`.

The generated Coq currently has no obvious top-level `Axiom`, `Parameter`, or
`Admitted` declarations in `evm_guest.v` / `evm_guest_types.v`. That does not
make the external behavior pure. Sail's Coq backend represents the model in its
monad, and the host-facing operations still need explicit semantics before we
can state useful source-side theorems or compare them to the RISC-V/Islaris
guest.

The first Coq contract skeleton lives in:

```text
proof/coq/ExternBoundary.v
```

## Contract Groups

| Group | Sail externs | Proof contract |
| --- | --- | --- |
| Private input oracle | `ssz_src_len`, `ssz_src_byte`, `ssz_src_le`, `ssz_src_be` | All reads are consistent views over one bounded byte vector. Little-endian and big-endian helpers are derived from the same bytes. |
| Output trace | `emit_out` / `el_emit_out` | Emitting a byte appends exactly that byte to the guest output trace and preserves byte well-formedness. |
| Byte sources | `host_source_keccak`, `host_source_sha256_prefixed`, `code_db_store_source`, `precompile_run_source_to_returndata` | Each C source tag (`Witness`, `Memory`, `TxInput`, `ActiveCode`) denotes an abstract byte sequence. Optimized pointer/length implementations must refine the corresponding materialized byte-list semantics, e.g. `host_source_keccak(source, off, len) = keccak256(read_source_bytes(source, off, len))`, the prefixed SHA-256 digest equals `sha256(type_byte :: read_source_bytes(source, off, len))`, code storage equals `code_db_store_code(keccak256(bytes), bytes)`, and source-backed precompile execution equals `run_precompile(id, bytes)`. |
| Crypto and precompiles | `keccak256`, `sha256`, `run_precompile`, `host_bytes_*`, `host_keccak_word`, `host_keccak_address`, `host_keccak_create2`, `host_sha256_pair`, `precompile_secp256k1_verify_hash_sig_pub`, `precompile_ecrecover_hash_sig` | `keccak256`, `sha256`, and `run_precompile` are abstract byte-list functions in Sail; the build backend implements the first two by streaming the list through `host_bytes_*`, and `keccak_rlp_payload(domain, payload)` must equal `keccak256(domain ++ rlp_list_prefix(len(payload)) ++ payload)`. All shared consensus hashing (`lib/consensus_hash.sail`) is defined once over these primitives. Fixed-width hash and signing helpers refine the corresponding canonical byte serialization without exposing EVM-Sail lists to C. Hash outputs are 256-bit words; typed precompile outputs carry explicit success bits. |
| Memory, calldata, returndata, code, stack | `host_mem_*`, `hm_move`, `returndata_*`, `txin_*`, `txd_*`, `code_db_frame_*`, `code_db_copy_frame_code_to_memory`, `code_db_calldata_load_word`, `hs_*` | Host memory has read-after-write laws; frame push/pop is LIFO; memory moves, code copy, calldata, returndata, transaction input views, code views, and operand stack operations refine abstract byte/list models. |
| World state and code DB | `transient_storage_*`, `storage_map_*`, `acctmap_*`, `code_db_*` | Account, storage, transient storage, and code lookup/update operations refine an abstract world-state map with explicit cache/update and commit/revert laws. |
| Witness and MPT DB | `nodedb_*` | The witness node table refines authenticated MPT witness spans over SSZ input bytes. Insert/select operations preserve the same abstract rows; together with the Sail-side anchored parent root (`k_parent_state_root`) every lazy walk is a hash-chain proof through `keccak(node)`-keyed lookups. |

## Sail Interface Files

The abstract `val` declarations for each contract group live in
`sail/iface/*.sail` (included immediately after `sail/prelude.sail`); the
backend definitions live in `sail/spec/*.sail` and `sail/build/*.sail`.

| Contract group | Interface file |
| --- | --- |
| Private input oracle | `sail/iface/oracle.sail` |
| Output trace | `sail/iface/oracle.sail` |
| Byte sources | `sail/iface/crypto.sail` |
| Crypto and precompiles | `sail/iface/crypto.sail` |
| Memory, calldata, returndata, code, stack | `sail/iface/stack.sail`, `sail/iface/memory.sail`, `sail/iface/code_db.sail` |
| World state and code DB | `sail/iface/state.sail`, `sail/iface/code_db.sail` |
| Witness and MPT DB | `sail/iface/witness_db.sail` |

## Proof Order

1. Instantiate only the private input oracle contract and prove small facts
   around `input_well_formed`. This avoids pulling in the whole EVM host model.
2. Add output-trace facts for the public-output behavior of `main`.
3. Add accelerator contracts for source-backed hashing/storage/precompile calls,
   build-only constructed-byte hash sinks, and typed fixed-width crypto helpers.
4. Add memory/stack and world-state contracts for the executable EVM path.
5. Add witness/MPT contracts for state-root and stateless-witness obligations.

The equivalence proof should use the same contracts on both sides: generated
`evm-sail` Coq proves source behavior under this interface, while the
RISC-V/Islaris side proves the compiled guest and runtime implement the same
interface.
