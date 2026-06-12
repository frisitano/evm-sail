# EL-IR Sail ↔ Rust IR mapping notes

The EL-IR Sail model is an executable EVM split into **user space (EVM)** and a
**host kernel (EL)**. This file maps that structure to the `evm-asm` Rust crates (the downstream AIR-compiler workspace this specification was extracted from).
The key correspondence: execution is recorded into **`ir_trace`**, an interleaved
stream of `TEvm` opcode steps (user space) and `TSyscall` kernel effects — the
full EL-IR row stream = exec_skeleton opcode rows + interface channels. The
**`TSyscall` subset is the AIR interface channel stream**: each `k_*` syscall the
EVM issues is the Sail analogue of a spine→chip channel send. Paths are relative
to the repository root.

## The central mapping: EL syscalls ↔ AIR channels

In the AIR, `exec_skeleton` emits one opcode-step row stream (the EVM user-space
computation) plus interface-claim channels that are balanced against companion
chips (the host). The Sail `el_trace` is exactly that channel stream:

| EL syscall (`ELOp`) | AIR interface channel / chip |
| --- | --- |
| `EL_AccessSlot` / `EL_AccessAccount` | EIP-2929 access-list warming (cost authenticated in-spine) |
| `EL_SLoad` / `EL_SStore` | `storage_spine_read` / `storage_spine_write` → `storage_bal` (BAL) |
| `EL_TLoad` / `EL_TStore` | `transient_state` |
| `EL_GetBalance` / `EL_GetNonce` / `EL_GetCode` / `EL_GetCodeHash` | `account_state` (BAL account timeline) |
| `EL_Transfer` / `EL_BumpNonce` / `EL_SetCode` | account-write channels; `frame_codes` binding |
| `EL_Env` | `env_context` (CHAINID/NUMBER/TIMESTAMP/…) |
| `EL_BlockHash` / `EL_BlobHash` | `block_hashes` / `blob_hashes` |
| `EL_Keccak` | `keccak_sites` → keccak_memory_adapter |
| `EL_Precompile` | precompile boundary (per-family claim) |
| `EL_Log` | `log_sites` / `log_topic*` |
| `EL_Snapshot` / `EL_Commit` / `EL_Revert` | the journal: REVERT rollback (`STORAGE_RESTORE` / frame-entry snapshot) |
| `EL_Refund` | EIP-3529 `refund_credit` (TX_END) |

A **no-op `SSTORE`** issues `EL_AccessSlot` + `EL_SLoad` but **no `EL_SStore`** —
matching the BAL "per-access channel, only for actual writes": the spine charges
gas but sends no storage-write channel. This is the user/kernel split's payoff.

## Module → Rust correspondence

| Sail module | Models | Rust counterpart |
| --- | --- | --- |
| `prelude_evm.sail` | 256-bit ALU, KECCAK primitive | arithmetic/bitwise/keccak chips; `[u64;4]` word model (`ir-core` `parse_u256`) |
| `el_kernel.sail` | **host kernel**: world state + EL syscall ISA + `el_trace` | `storage_bal`/account/BAL chips, env/log channels, REVERT journal, the interface-claim layer |
| `memory.sail` | per-frame byte memory | memory chip + `MemAccess` |
| `machine.sail` | EVM frame regs + **gas counter** + stack | exec_skeleton gas chain + `StackAccess` |
| `gas.sail` | gas schedule (user-space policy) | `exec_fixture.rs` gas helpers (`mem_expansion_gas`, `sstore_dynamic_gas`, `account_cold_gas`, `capped_refund`) |
| `instructions.sail` / `execute.sail` | opcode AST + semantics | `EXEC_OPCODES`; `check_exec_coverage` |
| `interpreter.sail` | fetch/decode, calls, creates, precompiles | call-boundary chip, `frame_codes`, precompile families |
| `transaction.sail` | intrinsic gas, state transition, receipt | `TxBegin`/`TxEnd`, `intrinsic_gas`, `tx_dyn_lump`, `ReceiptEmit` |
| `block.sail` | block execution + withdrawals | block-boundary work (`crates/rlp-boundary`) |

## Value model (`prelude_evm.sail`)

| Sail | Rust |
| --- | --- |
| `word = bits(256)` | `[u64; 4]` LE limbs |
| `address = bits(160)` | low 160 bits of a word |
| `gas = int` | `u64` running gas (`gas_before`/`gas_after`) |
| `alu_*` | arithmetic / bitwise / signextend chips |
| `keccak256` (declared `val`) | `keccak_memory_adapter` |

## Pseudo rows realized structurally

EL-IR's exec_skeleton pseudo rows (`TX_BEGIN`/`TX_END`/`CALL_RETURN`/
`STORAGE_RESTORE`/`EXCEPTIONAL_HALT`, bytes `0xD0..0xD5`) are realized as control
structure in the Sail model:

| pseudo row | Sail realization |
| --- | --- |
| `TX_BEGIN` / `TX_END` | `process_transaction` setup / finalize + `k_tx_reset` |
| `CALL_RETURN` | the caller-frame restore tail of `run_call` |
| `STORAGE_RESTORE` | `k_revert` (kernel snapshot rollback) |
| `EXCEPTIONAL_HALT` | `frame_status = Exceptional(...)` + `gas_remaining = 0` |

## Gas (user-space policy, `gas.sail` + `transaction.sail`)

| Sail | Rust (`ir-core/exec_fixture.rs`) |
| --- | --- |
| `mem_cost` / `expansion_gas` | `mem_expansion_gas` |
| `account_cost` / `sload_cost` (from kernel warm bit) | `account_cold_gas` |
| `sstore_gas` (EIP-2200/3529) | `sstore_dynamic_gas` + `capped_refund` |
| `keccak_gas` / `copy_gas` / `log_gas` / `precompile_gas` | per-row `dyn_gas` |
| `call_gas_cap` (EIP-150 63/64) | call-boundary forwarding |
| `intrinsic_gas` | `ExecFixture.intrinsic_gas` (`tx_intrinsic`) |

## Rust tests / fixtures the model tracks

| Sail | Rust fixture / test |
| --- | --- |
| `run_block.sail` (call tx: SSTORE + gas + receipt) | `exec_arith_fixture`; `exec_coverage_e2e` |
| `syscall_demo.sail` (no-op SSTORE emits no write) | BAL per-access write channel |
| `k_revert` on a reverting frame | `exec_revert_rollback_fixture`, `exec_subcall_revert_fixture` |
| `run_call` / `run_create` | `exec_dynamic_frame_fixture`, `exec_calldata_linkage_fixture` |

## Declared primitives (the crypto boundary)

Pinned by signature in `el_kernel.sail` / `prelude_evm.sail`; bodies for
execution in `examples/evm_primitives.sail`:

- `keccak256`, `create_address`, `create2_address`
- precompiles `ecrecover_p` / `sha256_p` / `ripemd160_p` / `modexp_p` /
  `ecadd_p` / `ecmul_p` / `ecpairing_p` / `blake2f_p` / `point_eval_p`

## Remaining fidelity notes

- Precompile gas/output structural with placeholder crypto; `BLOCKHASH` → 0.
- EIP-7702 delegation, full blob-gas accounting, and EIP-6780 same-tx
  SELFDESTRUCT nuance are next-pass refinements.
- A `Transaction` is assumed already sender-recovered and fee-resolved.
