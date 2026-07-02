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
| Accelerator and crypto | `acc_begin`, `acc_push`, `acc_push8`, `acc_exec`, `acc_exec_to_returndata`, `acc_ok`, `acc_out`, `acc_word`, `acc_in_byte`, `acc_begin_mem`, `acc_begin_txd`; wrappers `keccak256`, `sha256` | Stateful accelerator calls refine reference Keccak/SHA256/precompile functions. Hash outputs are 256-bit words; precompile byte outputs are byte well-formed. |
| Memory, calldata, returndata, code, stack | `host_mem_*`, `hm_move`, `hc_to_mem`, `hr_*`, `cd_*`, `txin_*`, `txd_*`, `hj_valid`, `hc_byte`, `hc_word`, `hc_len`, `fc_*`, `hs_*` | Host memory has read-after-write laws; frame push/pop is LIFO; memory moves, code copy, calldata, returndata, transaction input views, code views, and operand stack operations refine abstract byte/list models. |
| World state and code DB | `transient_storage_*`, `storage_map_*`, `acctmap_*`, `cs_*`, `cs_deleg` | Account, storage, transient storage, and code lookup/update operations refine an abstract world-state map with explicit cache/update and commit/revert laws. |
| Witness and MPT DB | `nodedb_*`, `acctdb_*`, `slotdb_*`, `storage_mark_incomplete`, `storage_harvest_complete_raw` | Witness node/account/storage tables refine authenticated MPT witness spans over SSZ input bytes. Insert/select and iteration operations preserve the same abstract rows. |

## Proof Order

1. Instantiate only the private input oracle contract and prove small facts
   around `input_well_formed`. This avoids pulling in the whole EVM host model.
2. Add output-trace facts for the public-output behavior of `main`.
3. Add accelerator contracts for `keccak256`, `sha256`, and precompile calls.
4. Add memory/stack and world-state contracts for the executable EVM path.
5. Add witness/MPT contracts for state-root and stateless-witness obligations.

The equivalence proof should use the same contracts on both sides: generated
`evm-sail` Coq proves source behavior under this interface, while the
RISC-V/Islaris side proves the compiled guest and runtime implement the same
interface.
