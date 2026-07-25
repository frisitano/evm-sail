import Evm.Flow
import Evm.Prelude
import Evm.Primitives.CycleScopes
import Evm.Host.CycleScopesDisabled
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Primitives.System
import Evm.Primitives.Block
import Evm.Host.Kernel.Scratch
import Evm.Host.Kernel.Environment
import Evm.Host.Kernel.Accounts
import Evm.Host.Kernel.Lifecycle
import Evm.Evm.Machine
import Evm.Evm.Gas
import Evm.Evm.Transaction
import Evm.Lib.Ssz.StatelessInput
import Evm.Executor.Receipts
import Evm.Executor.SystemCalls

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open Sail.ConcurrencyInterfaceV1

namespace Evm

open ConcurrencyInterfaceV1

open Defs
namespace Functions

open option
open exception
open ast
open TxType
open TrieUpdateSource
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open Register
open NodeRef
open MerkleSlot
open HaltKind
open FrameStatus
open FrameContinuation
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open ByteRegionResult
open BlockError

/-! # The block driver

Block-start system calls, the transaction loop with block-gas
accounting, withdrawals, and block-end request collection. -/

def PRE_MERGE_BLOCK_REWARD := (BitVec.toNatInt 0x1BC16D674EC80000#64)

/-- Returns a block's remaining gas, rejecting an accumulated value above the
header limit with the block-validation error required by EIP-7778. -/
/- Type quantifiers: k_ex418077_ : Nat, k_ex418076_ : Nat, 0 ≤ k_ex418076_ ∧
  k_ex418076_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex418077_ -/
def remaining_block_gas (limit : Nat) (used : Nat) : SailM Nat := do
  if ((used ≤b limit) : Bool)
  then (pure (limit - used))
  else sailThrow ((InvalidBlock GasUsedExceedsLimit))

/-- The block-start writes: beacon root (Cancun+, EIP-4788) and parent
hash history (Prague+, EIP-2935). -/
def run_block_start_system_calls (_ : Unit) : SailM Unit := do
  if ((fork_gteq (← readReg k_fork) Cancun) : Bool)
  then (system_call BEACON_ROOTS_ADDR (← readReg k_header).parent_beacon_block_root)
  else (pure ())
  if ((fork_gteq (← readReg k_fork) Prague) : Bool)
  then (system_call HISTORY_STORAGE_ADDR (← readReg k_header).parent_hash)
  else (pure ())

/-- Executes the block's transactions in order, enforcing per-tx
applicability and block gas/blob-gas availability (EIP-7778 block-gas
accounting), accumulating receipts. -/
/- Type quantifiers: k_ex418085_ : Nat, public_keys_dependentWitness1 : Nat, public_keys_dependentWitness0
  : Nat, 0 ≤ public_keys_dependentWitness0 ∧ 0 ≤ public_keys_dependentWitness1, 0 ≤
  k_ex418085_ ∧ k_ex418085_ ≤ (2 ^ 64 - 1) -/
def execute_block_transactions (transactions : (BoundedSszListRef (2 ^ 20))) (public_keys : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (header_gas_limit : Nat) : SailM BlockExecutionResult := do
  let public_keys_dependentWitness0 := (public_keys).1
  let public_keys_dependentWitness1 := ((public_keys).2).1
  let public_keys := ((public_keys).2).2
  let public_keys_length := public_keys.len
  let public_key_length := PUBLIC_KEY_LENGTH
  let public_key_count_value := (public_keys_length / public_key_length)
  if (((public_key_count_value != transactions.count) || (public_keys_length != (public_key_count_value *i public_key_length))) : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else
    (do
      let gas_limit := header_gas_limit
      let execution_gas_acc : Nat := GAS_ZERO
      let state_gas_acc : Nat := GAS_ZERO
      let blob_gas_acc : Nat := 0
      let tx0_to : (Vector (BitVec 8) 20) := ZERO_ADDRESS
      let receipts := (receipt_accumulator_empty ())
      let deposits_start ← do (scratch_begin ())
      let cursor ← do (ssz_list_cursor transactions)
      let keys : (Sigma fun (public_keys_dependentWitness0 : Nat) =>
        (Sigma fun (public_keys_dependentWitness1 : Nat) =>
        (EvmByteSliceFields public_keys_dependentWitness0 public_keys_dependentWitness1))) :=
        ((⟨_, ⟨_, public_keys⟩⟩ : (Sigma fun (public_keys_dependentWitness0 : Nat) =>
        (Sigma fun (public_keys_dependentWitness1 : Nat) =>
        (EvmByteSliceFields public_keys_dependentWitness0 public_keys_dependentWitness1)))) : (Sigma
        fun (public_keys_dependentWitness0 : Nat) =>
        (Sigma fun (public_keys_dependentWitness1 : Nat) =>
        (EvmByteSliceFields public_keys_dependentWitness0 public_keys_dependentWitness1))))
      let (blob_gas_acc, cursor, execution_gas_acc, keys, receipts, state_gas_acc, tx0_to) ← (( do
        let loop_vars ← whileFuelM (fuel :=(cursor.items.count -i cursor.index)) (fun (blob_gas_acc, cursor, execution_gas_acc, keys, receipts, state_gas_acc, tx0_to) => (pure (! (ssz_list_cursor_empty
              cursor)))) (blob_gas_acc, cursor, execution_gas_acc, keys, receipts, state_gas_acc, tx0_to)
          fun (blob_gas_acc, cursor, execution_gas_acc, keys, receipts, state_gas_acc, tx0_to) => do
            assert true "loop dummy assert"
            let i := cursor.index
            let (transaction, next) ← do (ssz_list_pop cursor)
            let cursor : (BoundedSszListCursor (2 ^ 20)) := next
            let ⟨_, ⟨_, keys_fields⟩⟩ := keys
            let keys_length := keys_fields.len
            if _sailIf0 : ((keys_length <b public_key_length) : Bool) = true
            then
              (do
                sailThrow ((InvalidBlock WitnessDeficient)))
            else
              (do
                let public_key := (sub_slice keys_fields 0 PUBLIC_KEY_LENGTH)
                let keys : (Sigma fun (public_keys_dependentWitness0 : Nat) =>
                  (Sigma fun (public_keys_dependentWitness1 : Nat) =>
                  (EvmByteSliceFields public_keys_dependentWitness0 public_keys_dependentWitness1))) :=
                  ((⟨_, ⟨_, (slice_suffix keys_fields public_key_length)⟩⟩ : (Sigma fun
                  (public_keys_dependentWitness0 : Nat) =>
                  (Sigma fun (public_keys_dependentWitness1 : Nat) =>
                  (EvmByteSliceFields public_keys_dependentWitness0 public_keys_dependentWitness1)))) : (Sigma
                  fun (public_keys_dependentWitness0 : Nat) =>
                  (Sigma fun (public_keys_dependentWitness1 : Nat) =>
                  (EvmByteSliceFields public_keys_dependentWitness0 public_keys_dependentWitness1))))
                let _ : Unit := (cycle_scope_start SCOPE_TX_DECODE)
                let tx ← do (decode_transaction ((transaction).2).2 public_key)
                let _ : Unit := (cycle_scope_end SCOPE_TX_DECODE)
                writeReg k_block_access_index (i + 1)
                let tx0_to : (Vector (BitVec 8) 20) :=
                  if ((i == 0) : Bool)
                  then tx.recipient
                  else tx0_to
                let available_execution_gas ← do (remaining_block_gas gas_limit execution_gas_acc)
                let available_state_gas ← do (remaining_block_gas gas_limit state_gas_acc)
                let transaction_execution_limit : Nat :=
                  if ((AMSTERDAM_TX_MAX_GAS <b tx.gas_limit) : Bool)
                  then AMSTERDAM_TX_MAX_GAS
                  else tx.gas_limit
                let transaction_fits ← (( do
                  if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
                  then
                    (pure ((transaction_execution_limit ≤b available_execution_gas) && ((tx.gas_limit ≤b available_state_gas) : Bool)))
                  else (pure (tx.gas_limit ≤b available_execution_gas)) ) : SailM Bool )
                let (blob_gas_acc, execution_gas_acc, receipts, state_gas_acc) ← (( do
                  if ((! transaction_fits) : Bool)
                  then sailThrow ((InvalidBlock GasUsedExceedsLimit))
                  else
                    (do
                      let tx_blob_gas ← do (transaction_blob_gas_for_count tx.blob_hashes.count)
                      let next_blob_gas ← (( do
                        if ((fork_lt (← readReg k_fork) Cancun) : Bool)
                        then (pure blob_gas_acc)
                        else (checked_block_blob_gas_add blob_gas_acc tx_blob_gas) ) : SailM Nat )
                      let receipt ← do (process_transaction tx)
                      let execution_gas_acc : Nat :=
                        (conserved_gas_add execution_gas_acc receipt.execution_gas)
                      let state_gas_acc : Nat := (conserved_gas_add state_gas_acc receipt.state_gas)
                      if (((gas_limit <b execution_gas_acc) || ((gas_limit <b state_gas_acc) : Bool)) : Bool)
                      then sailThrow ((InvalidBlock GasUsedExceedsLimit))
                      else
                        (do
                          let receipts ← (receipt_accumulator_push receipts receipt next.index)
                          (append_deposit_logs receipt.logs)
                          let blob_gas_acc : Nat := next_blob_gas
                          (pure (blob_gas_acc, execution_gas_acc, receipts, state_gas_acc)))) ) :
                  SailM (Nat × Nat × ReceiptAccumulator × Nat) )
                (pure ((blob_gas_acc, cursor, execution_gas_acc, keys, receipts, state_gas_acc, tx0_to) : (Nat × (BoundedSszListCursor (2 ^ 20)) × Nat × (Sigma
                  fun (public_keys_dependentWitness0 : Nat) =>
                  (Sigma fun (public_keys_dependentWitness1 : Nat) =>
                  (EvmByteSliceFields public_keys_dependentWitness0 public_keys_dependentWitness1))) × ReceiptAccumulator × Nat × (Vector (BitVec 8) 20)))))
        (pure loop_vars) ) : SailM
        (Nat × (BoundedSszListCursor (2 ^ 20)) × Nat × (Sigma fun
        (public_keys_dependentWitness0 : Nat) =>
        (Sigma fun (public_keys_dependentWitness1 : Nat) =>
        (EvmByteSliceFields public_keys_dependentWitness0 public_keys_dependentWitness1))) × ReceiptAccumulator × Nat × (Vector (BitVec 8) 20))
        )
      let header_gas_used ← do
        if (((fork_gteq (← readReg k_fork) Amsterdam) && ((execution_gas_acc <b state_gas_acc) : Bool)) : Bool)
        then (pure state_gas_acc)
        else (pure execution_gas_acc)
      let _ : Unit := (cycle_scope_start SCOPE_RECEIPTS_ROOT)
      let receipts_root ← do (receipt_accumulator_root receipts)
      let _ : Unit := (cycle_scope_end SCOPE_RECEIPTS_ROOT)
      (pure { header_gas_used := header_gas_used,
              execution_gas_used := execution_gas_acc,
              state_gas_used := state_gas_acc,
              blob_gas_used := blob_gas_acc,
              first_tx_recipient := tx0_to,
              receipts_root := receipts_root,
              logs_bloom := receipts.bloom,
              deposits := ← do
                  let publicField ← (scratch_finish deposits_start)
                  pure (publicField),
              requests := EMPTY_EXECUTION_REQUESTS }))

/-- Credits every withdrawal's recipient with its amount in gwei
(EIP-4895); withdrawals cannot fail and charge no gas. -/
def apply_withdrawals (withdrawals : (BoundedSszListRef (2 ^ 4))) : SailM Unit := do
  let rest := withdrawals
  let rest ← (( do
    let loop_vars ← whileFuelM (fuel :=rest.count) (fun rest => (pure (rest.count != 0))) rest
      fun rest => do
        assert true "loop dummy assert"
        let (withdrawal_ref, tail) ← do (ssz_fixed_list_pop rest WD_SIZE)
        let rest : (BoundedSszListRef (2 ^ 4)) := tail
        let withdrawal ← do (decode_withdrawal withdrawal_ref)
        (k_add_balance withdrawal.address
          (alu_mul (word_of_withdrawal_amount withdrawal.amount) 1000000000))
        (pure rest)
    (pure loop_vars) ) : SailM (BoundedSszListRef (2 ^ 4)) )
  (pure ())

/-- The block-end state effects: withdrawals (Shanghai+, EIP-4895), the
pre-merge static block reward before Paris, and the final merge into
the block layer. -/
def apply_block_end_state (body : BlockBody) : SailM Unit := do
  if ((fork_gteq (← readReg k_fork) Shanghai) : Bool)
  then (apply_withdrawals body.withdrawals)
  else (pure ())
  if ((fork_lt (← readReg k_fork) Paris) : Bool)
  then (k_add_balance (← (k_coinbase ())) PRE_MERGE_BLOCK_REWARD)
  else (pure ())
  (k_tx_merge ())

/-- Executes a block body end to end: block-start system calls, the
transaction loop, block-end state effects, and request collection;
invalid execution throws immediately, while successful execution returns
the accumulated [BlockExecutionResult][type-BlockExecutionResult]. -/
/- Type quantifiers: k_ex418093_ : Nat, public_keys_dependentWitness1 : Nat, public_keys_dependentWitness0
  : Nat, 0 ≤ public_keys_dependentWitness0 ∧ 0 ≤ public_keys_dependentWitness1, 0 ≤
  k_ex418093_ ∧ k_ex418093_ ≤ (2 ^ 64 - 1) -/
def execute_block_body (body : BlockBody) (public_keys : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (header_gas_limit : Nat) : SailM BlockExecutionResult := do
  let public_keys_dependentWitness0 := (public_keys).1
  let public_keys_dependentWitness1 := ((public_keys).2).1
  let public_keys := ((public_keys).2).2
  (bal_reset ())
  writeReg k_block_access_index 0
  let _ : Unit := (cycle_scope_start SCOPE_BLOCK_START)
  (run_block_start_system_calls ())
  let _ : Unit := (cycle_scope_end SCOPE_BLOCK_START)
  let _ : Unit := (cycle_scope_start SCOPE_BLOCK_TRANSACTIONS)
  let result ← do
    (execute_block_transactions body.transactions ⟨_, ⟨_, public_keys⟩⟩ header_gas_limit)
  let _ : Unit := (cycle_scope_end SCOPE_BLOCK_TRANSACTIONS)
  let post_tx_index := (body.transactions.count + 1)
  writeReg k_block_access_index post_tx_index
  let _ : Unit := (cycle_scope_start SCOPE_BLOCK_END_STATE)
  (apply_block_end_state body)
  let _ : Unit := (cycle_scope_end SCOPE_BLOCK_END_STATE)
  let _ : Unit := (cycle_scope_start SCOPE_BLOCK_END_REQUESTS)
  let requests ← do
    if ((fork_gteq (← readReg k_fork) Prague) : Bool)
    then (collect_execution_requests result.deposits)
    else (pure EMPTY_EXECUTION_REQUESTS)
  let _ : Unit := (cycle_scope_end SCOPE_BLOCK_END_REQUESTS)
  (pure { result with requests := requests })

