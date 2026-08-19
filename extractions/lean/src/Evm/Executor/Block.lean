import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Primitives.Fork
import Evm.Primitives.System
import Evm.Primitives.Tx
import Evm.Kernel.Environment
import Evm.Kernel.Accounts
import Evm.Kernel.Lifecycle
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
open ast
open TxType
open TxSignatureScheme
open TrieUpdateSource
open TrieUpdateRelation
open TrieLeafValue
open TrieItemValue
open TrieChange
open StorageTxPopResult
open StorageTxLookup
open StorageBlockIterResult
open StateJournalEntry
open StackValidation
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
open OpcodeOutcome
open NodeRef
open LogTopics
open LogData
open InputTrieNode
open IndexedTrieSource
open HtrRequestKind
open HaltKind
open FrameStatus
open FrameContinuation
open FatalError
open ExceptionKind
open EnvField
open DeepStackOperation
open CreateKind
open CalldataSlice
open CallKind
open BalIterEntry
open AcctTxPopResult
open AcctBlockIterResult

/-! # The block driver

Block-start system calls, the transaction loop with block-gas
accounting, withdrawals, and block-end request collection. -/

/- Type quantifiers: _limit : Nat, 0 ≤ _limit ∧ _limit ≤ block_gas_limit_bound -/
def block_gas_usage_empty (_limit : Nat) : (BlockGasUsageFields _limit 0 0 0) :=
  {  }

/- Type quantifiers: k_limit : Nat, k_execution : Nat, k_state : Nat, k_receipts : Nat, add_execution
  : Nat, add_state : Nat, add_receipt : Nat, (block_gas_usage_relation k_limit k_execution k_state k_receipts)
  ∧
  0 ≤ add_execution ∧
  add_execution ≤ (k_limit - k_execution) ∧
  0 ≤ add_state ∧
  add_state ≤ (k_limit - k_state) ∧
  0 ≤ add_receipt ∧ add_receipt ≤ (add_execution + add_state) -/
def block_gas_usage_add (usage : (BlockGasUsageFields k_limit k_execution k_state k_receipts)) (add_execution : Nat) (add_state : Nat) (add_receipt : Nat) : (BlockGasUsageFields k_limit (k_execution + add_execution) (k_state + add_state) (k_receipts + add_receipt)) :=
  {  }

def PRE_MERGE_BLOCK_REWARD := (BitVec.toNatInt 0x1BC16D674EC80000#64)

/-- The block-start writes: beacon root (Cancun+, EIP-4788) and parent
hash history (Prague+, EIP-2935). -/
def run_block_start_system_calls (_ : Unit) : SailM Unit := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if ((profile.fork ≥b Cancun) : Bool)
  then (system_call BEACON_ROOTS_ADDR (← readReg k_header).parent_beacon_block_root)
  else (pure ())
  if ((profile.fork ≥b Prague) : Bool)
  then (system_call HISTORY_STORAGE_ADDR (← readReg k_header).parent_hash)
  else (pure ())

/-- Executes the block's transactions in order, enforcing per-tx
applicability and block gas/blob-gas availability (EIP-7778 block-gas
accounting), accumulating receipts. -/
/- Type quantifiers: expected_deposits_dependentWitness1 : Nat, expected_deposits_dependentWitness0
  : Nat, public_keys_dependentWitness1 : Nat, public_keys_dependentWitness0 : Nat, 0 ≤
  public_keys_dependentWitness0 ∧
  0 ≤ public_keys_dependentWitness1 ∧
  (public_keys_dependentWitness0 + public_keys_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤
  expected_deposits_dependentWitness0 ∧
  0 ≤ expected_deposits_dependentWitness1 ∧
  (expected_deposits_dependentWitness0 + expected_deposits_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def execute_block_transactions (transactions : (BoundedSszListRef (2 ^ 20))) (public_keys : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (expected_deposits : (Sigma
  fun (public_keys_dependentWitness0 : Nat) =>
  (Sigma fun (public_keys_dependentWitness1 : Nat) =>
  (StatelessInputSliceFields public_keys_dependentWitness0 public_keys_dependentWitness1)))) : SailM BlockExecutionResult := do
  let public_keys_dependentWitness0 := (public_keys).1
  let public_keys_dependentWitness1 := ((public_keys).2).1
  let public_keys := ((public_keys).2).2
  let expected_deposits_dependentWitness0 := (expected_deposits).1
  let expected_deposits_dependentWitness1 := ((expected_deposits).2).1
  let expected_deposits := ((expected_deposits).2).2
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let gas_limits := execution_profile.gas
  let public_keys_length := public_keys.len
  let public_key_length := PUBLIC_KEY_LENGTH
  let public_key_count_value := (public_keys_length / public_key_length)
  if (((public_key_count_value != transactions.count) || (public_keys_length != (public_key_count_value *i public_key_length))) : Bool)
  then (fatal_error WitnessDeficient)
  else (pure ())
  let gas_limit := gas_limits.block_limit
  let gas_usage : (Sigma fun (k_execution : Nat) =>
    (Sigma fun (k_state : Nat) =>
    (Sigma fun (k_receipts : Nat) => (BlockGasUsageFields gas_limit k_execution k_state k_receipts)))) :=
    ((⟨_, ⟨_, ⟨_, (block_gas_usage_empty gas_limit)⟩⟩⟩ : (Sigma fun (k_execution : Nat)
    =>
    (Sigma fun (k_state : Nat) =>
    (Sigma fun (k_receipts : Nat) => (BlockGasUsageFields gas_limit k_execution k_state k_receipts))))) : (Sigma
    fun (k_execution : Nat) =>
    (Sigma fun (k_state : Nat) =>
    (Sigma fun (k_receipts : Nat) => (BlockGasUsageFields gas_limit k_execution k_state k_receipts)))))
  let blob_gas_acc : Nat := 0
  let tx0_to : (Vector (BitVec 8) 20) := ZERO_ADDRESS
  let records_start ← do (receipt_store_begin ())
  let transaction_logs_start ← do (logs_tx_start ())
  let transaction_logs_count ← do (logs_tx_count ())
  let logs_start ← do (log_store_index_add transaction_logs_start transaction_logs_count)
  let remaining_deposits : (Sigma fun (expected_deposits_dependentWitness0 : Nat) =>
    (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
    (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1))) :=
    ((⟨_, ⟨_, expected_deposits⟩⟩ : (Sigma fun (expected_deposits_dependentWitness0 : Nat)
    =>
    (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
    (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))) : (Sigma
    fun (expected_deposits_dependentWitness0 : Nat) =>
    (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
    (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1))))
  let cursor ← do (ssz_list_cursor transactions)
  let keys : (Sigma fun (expected_deposits_dependentWitness0 : Nat) =>
    (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
    (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1))) :=
    ((⟨_, ⟨_, public_keys⟩⟩ : (Sigma fun (expected_deposits_dependentWitness0 : Nat) =>
    (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
    (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))) : (Sigma
    fun (expected_deposits_dependentWitness0 : Nat) =>
    (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
    (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1))))
  let initial_cursor_empty := (ssz_list_cursor_empty cursor)
  let cursor_has_item : Bool := (! initial_cursor_empty)
  let (blob_gas_acc, cursor, cursor_has_item, gas_usage, keys, remaining_deposits, tx0_to) ← (( do
    let loop_vars ← whileFuelM (fuel :=(cursor.items.count -i cursor.index)) (fun (blob_gas_acc, cursor, cursor_has_item, gas_usage, keys, remaining_deposits, tx0_to) => (pure cursor_has_item)) (blob_gas_acc, cursor, cursor_has_item, gas_usage, keys, remaining_deposits, tx0_to)
      fun (blob_gas_acc, cursor, cursor_has_item, gas_usage, keys, remaining_deposits, tx0_to) => do
        assert true "loop dummy assert"
        let i := cursor.index
        let (transaction, next) ← do (ssz_list_pop cursor)
        let cursor : (BoundedSszListCursor (2 ^ 20)) := next
        let ⟨_, ⟨_, keys_fields⟩⟩ := keys
        let ⟨_, ⟨_, keys_fields⟩⟩ ← (( do
          if _sailIf0 : ((public_key_length ≤b keys_fields.len) : Bool) = true
          then
            (pure ((⟨_, ⟨_, keys_fields⟩⟩ : (Sigma fun
              (expected_deposits_dependentWitness0 : Nat) =>
              (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
              (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))) : (Sigma
              fun (expected_deposits_dependentWitness0 : Nat) =>
              (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
              (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))))
          else
            (do
              (fatal_error WitnessDeficient)) ) : SailM
          (Sigma fun (expected_deposits_dependentWitness0 : Nat) =>
          (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
          (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))
          )
        let public_key := (stateless_input_sub_slice keys_fields 0 PUBLIC_KEY_LENGTH)
        let keys : (Sigma fun (expected_deposits_dependentWitness0 : Nat) =>
          (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
          (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1))) :=
          ((⟨_, ⟨_, (stateless_input_slice_suffix keys_fields public_key_length)⟩⟩ : (Sigma
          fun (expected_deposits_dependentWitness0 : Nat) =>
          (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
          (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))) : (Sigma
          fun (expected_deposits_dependentWitness0 : Nat) =>
          (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
          (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1))))
        let ⟨_, tx⟩ ← do (decode_transaction ((transaction).2).2 public_key)
        writeReg k_current_transaction_epoch (i + 1)
        let tx0_to : (Vector (BitVec 8) 20) :=
          if ((i == 0) : Bool)
          then tx.recipient
          else tx0_to
        let ⟨_, ⟨_, ⟨_, usage⟩⟩⟩ := gas_usage
        let available_execution_gas := (gas_limit - usage.execution)
        let available_state_gas := (gas_limit - usage.state)
        let ⟨_, ⟨_, allowance⟩⟩ ← do
          (transaction_gas_allowance tx.gas_limit gas_limits.transaction_total_limit
            gas_limits.transaction_regular_limit)
        let (blob_gas_acc, gas_usage, remaining_deposits) ← (( do
          if _sailIf0 : ((profile.fork ≥b Amsterdam) : Bool) = true
          then
            (do
              let (blob_gas_acc, gas_usage, remaining_deposits) ← (( do
                if _sailIf1 : (((available_execution_gas <b allowance.regular) || (available_state_gas <b allowance.total)) : Bool) = true
                then
                  (do
                    (fatal_error GasUsedExceedsLimit)
                    (pure ((blob_gas_acc : Nat), (gas_usage : (Sigma fun (k_execution : Nat) =>
                      (Sigma fun (k_state : Nat) =>
                      (Sigma fun (k_receipts : Nat) =>
                      (BlockGasUsageFields gas_limit k_execution k_state k_receipts))))), (remaining_deposits : (Sigma
                      fun (expected_deposits_dependentWitness0 : Nat) =>
                      (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
                      (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))))))
                else
                  (do
                    let tx_blob_gas : Nat := ((2 ^i 17) *i tx.blob_hashes.count)
                    let next_blob_gas ← (( do
                      (block_blob_gas_add profile.blob_schedule.max blob_gas_acc tx_blob_gas) ) :
                      SailM Nat )
                    let ⟨_, ⟨_, ⟨_, receipt⟩⟩⟩ ← do
                      (process_transaction ⟨_, tx⟩ allowance)
                    let next_usage :=
                      (block_gas_usage_add usage receipt.execution_gas receipt.state_gas
                        receipt.gas_used)
                    let gas_usage : (Sigma fun (k_execution : Nat) =>
                      (Sigma fun (k_state : Nat) =>
                      (Sigma fun (k_receipts : Nat) =>
                      (BlockGasUsageFields gas_limit k_execution k_state k_receipts)))) :=
                      ((⟨_, ⟨_, ⟨_, next_usage⟩⟩⟩ : (Sigma fun (k_execution : Nat) =>
                      (Sigma fun (k_state : Nat) =>
                      (Sigma fun (k_receipts : Nat) =>
                      (BlockGasUsageFields gas_limit k_execution k_state k_receipts))))) : (Sigma
                      fun (k_execution : Nat) =>
                      (Sigma fun (k_state : Nat) =>
                      (Sigma fun (k_receipts : Nat) =>
                      (BlockGasUsageFields gas_limit k_execution k_state k_receipts)))))
                    (receipt_store_append ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, receipt⟩⟩⟩⟩⟩
                      next_usage.receipts i)
                    let ⟨_, ⟨_, remaining_deposits⟩⟩ ←
                      (authenticate_deposit_logs receipt.logs remaining_deposits)
                    let blob_gas_acc : Nat := next_blob_gas
                    (pure ((blob_gas_acc : Nat), (gas_usage : (Sigma fun (k_execution : Nat) =>
                      (Sigma fun (k_state : Nat) =>
                      (Sigma fun (k_receipts : Nat) =>
                      (BlockGasUsageFields gas_limit k_execution k_state k_receipts))))), ((⟨_, ⟨_, remaining_deposits⟩⟩ : (Sigma
                      fun (expected_deposits_dependentWitness0 : Nat) =>
                      (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
                      (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))) : (Sigma
                      fun (expected_deposits_dependentWitness0 : Nat) =>
                      (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
                      (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))))))
                ) : SailM
                (Nat × (Sigma fun (k_execution : Nat) =>
                (Sigma fun (k_state : Nat) =>
                (Sigma fun (k_receipts : Nat) =>
                (BlockGasUsageFields gas_limit k_execution k_state k_receipts)))) × (Sigma fun
                (expected_deposits_dependentWitness0 : Nat) =>
                (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
                (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1))))
                )
              (pure ((blob_gas_acc : Nat), (gas_usage : (Sigma fun (k_execution : Nat) =>
                (Sigma fun (k_state : Nat) =>
                (Sigma fun (k_receipts : Nat) =>
                (BlockGasUsageFields gas_limit k_execution k_state k_receipts))))), (remaining_deposits : (Sigma
                fun (expected_deposits_dependentWitness0 : Nat) =>
                (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
                (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))))))
          else
            (do
              let (blob_gas_acc, gas_usage, remaining_deposits) ← (( do
                if _sailIf1 : ((available_execution_gas <b allowance.total) : Bool) = true
                then
                  (do
                    (fatal_error GasUsedExceedsLimit)
                    (pure ((blob_gas_acc : Nat), (gas_usage : (Sigma fun (k_execution : Nat) =>
                      (Sigma fun (k_state : Nat) =>
                      (Sigma fun (k_receipts : Nat) =>
                      (BlockGasUsageFields gas_limit k_execution k_state k_receipts))))), (remaining_deposits : (Sigma
                      fun (expected_deposits_dependentWitness0 : Nat) =>
                      (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
                      (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))))))
                else
                  (do
                    let tx_blob_gas : Nat := ((2 ^i 17) *i tx.blob_hashes.count)
                    let next_blob_gas ← (( do
                      if ((profile.fork <b Cancun) : Bool)
                      then (pure blob_gas_acc)
                      else (block_blob_gas_add profile.blob_schedule.max blob_gas_acc tx_blob_gas) )
                      : SailM Nat )
                    let ⟨_, ⟨_, ⟨_, receipt⟩⟩⟩ ← do
                      (process_transaction ⟨_, tx⟩ allowance)
                    let next_usage :=
                      (block_gas_usage_add usage receipt.gas_used 0 receipt.gas_used)
                    let gas_usage : (Sigma fun (k_execution : Nat) =>
                      (Sigma fun (k_state : Nat) =>
                      (Sigma fun (k_receipts : Nat) =>
                      (BlockGasUsageFields gas_limit k_execution k_state k_receipts)))) :=
                      ((⟨_, ⟨_, ⟨_, next_usage⟩⟩⟩ : (Sigma fun (k_execution : Nat) =>
                      (Sigma fun (k_state : Nat) =>
                      (Sigma fun (k_receipts : Nat) =>
                      (BlockGasUsageFields gas_limit k_execution k_state k_receipts))))) : (Sigma
                      fun (k_execution : Nat) =>
                      (Sigma fun (k_state : Nat) =>
                      (Sigma fun (k_receipts : Nat) =>
                      (BlockGasUsageFields gas_limit k_execution k_state k_receipts)))))
                    (receipt_store_append ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, receipt⟩⟩⟩⟩⟩
                      next_usage.receipts i)
                    let ⟨_, ⟨_, remaining_deposits⟩⟩ ← (( do
                      if _sailIf2 : ((profile.fork ≥b Prague) : Bool) = true
                      then
                        (do
                          (authenticate_deposit_logs receipt.logs remaining_deposits))
                      else
                        (pure (remaining_deposits : (Sigma fun
                          (expected_deposits_dependentWitness0 : Nat) =>
                          (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
                          (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))))
                      ) : SailM
                      (Sigma fun (expected_deposits_dependentWitness0 : Nat) =>
                      (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
                      (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))
                      )
                    let blob_gas_acc : Nat := next_blob_gas
                    (pure ((blob_gas_acc : Nat), (gas_usage : (Sigma fun (k_execution : Nat) =>
                      (Sigma fun (k_state : Nat) =>
                      (Sigma fun (k_receipts : Nat) =>
                      (BlockGasUsageFields gas_limit k_execution k_state k_receipts))))), ((⟨_, ⟨_, remaining_deposits⟩⟩ : (Sigma
                      fun (expected_deposits_dependentWitness0 : Nat) =>
                      (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
                      (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))) : (Sigma
                      fun (expected_deposits_dependentWitness0 : Nat) =>
                      (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
                      (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))))))
                ) : SailM
                (Nat × (Sigma fun (k_execution : Nat) =>
                (Sigma fun (k_state : Nat) =>
                (Sigma fun (k_receipts : Nat) =>
                (BlockGasUsageFields gas_limit k_execution k_state k_receipts)))) × (Sigma fun
                (expected_deposits_dependentWitness0 : Nat) =>
                (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
                (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1))))
                )
              (pure ((blob_gas_acc : Nat), (gas_usage : (Sigma fun (k_execution : Nat) =>
                (Sigma fun (k_state : Nat) =>
                (Sigma fun (k_receipts : Nat) =>
                (BlockGasUsageFields gas_limit k_execution k_state k_receipts))))), (remaining_deposits : (Sigma
                fun (expected_deposits_dependentWitness0 : Nat) =>
                (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
                (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))))))
          ) : SailM
          (Nat × (Sigma fun (k_execution : Nat) =>
          (Sigma fun (k_state : Nat) =>
          (Sigma fun (k_receipts : Nat) =>
          (BlockGasUsageFields gas_limit k_execution k_state k_receipts)))) × (Sigma fun
          (expected_deposits_dependentWitness0 : Nat) =>
          (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
          (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1))))
          )
        let cursor_empty := (ssz_list_cursor_empty cursor)
        let cursor_has_item : Bool := (! cursor_empty)
        (pure ((blob_gas_acc : Nat), (cursor : (BoundedSszListCursor (2 ^ 20))), (cursor_has_item : Bool), (gas_usage : (Sigma
          fun (k_execution : Nat) =>
          (Sigma fun (k_state : Nat) =>
          (Sigma fun (k_receipts : Nat) =>
          (BlockGasUsageFields gas_limit k_execution k_state k_receipts))))), (keys : (Sigma fun
          (expected_deposits_dependentWitness0 : Nat) =>
          (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
          (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))), (remaining_deposits : (Sigma
          fun (expected_deposits_dependentWitness0 : Nat) =>
          (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
          (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1)))), (tx0_to : (Vector (BitVec 8) 20))))
    (pure loop_vars) ) : SailM
    (Nat × (BoundedSszListCursor (2 ^ 20)) × Bool × (Sigma fun (k_execution : Nat) =>
    (Sigma fun (k_state : Nat) =>
    (Sigma fun (k_receipts : Nat) => (BlockGasUsageFields gas_limit k_execution k_state k_receipts)))) × (Sigma
    fun (expected_deposits_dependentWitness0 : Nat) =>
    (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
    (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1))) × (Sigma
    fun (expected_deposits_dependentWitness0 : Nat) =>
    (Sigma fun (expected_deposits_dependentWitness1 : Nat) =>
    (StatelessInputSliceFields expected_deposits_dependentWitness0 expected_deposits_dependentWitness1))) × (Vector (BitVec 8) 20))
    )
  let remaining_deposits_length := (stateless_input_slice_length remaining_deposits)
  if (((profile.fork ≥b Prague) && (remaining_deposits_length != 0)) : Bool)
  then (fatal_error InvalidExecutionRequests)
  else (pure ())
  let ⟨_, ⟨_, ⟨_, final_usage⟩⟩⟩ := gas_usage
  let header_gas_used :=
    if (((profile.fork ≥b Amsterdam) && (final_usage.execution <b final_usage.state)) : Bool)
    then final_usage.state
    else final_usage.execution
  let receipts_root ← do (receipt_store_root records_start transactions.count)
  let retained_logs_start ← do (logs_tx_start ())
  let retained_logs_count ← do (logs_tx_count ())
  let retained ← do (log_store_index_add retained_logs_start retained_logs_count)
  let logs_count : Nat :=
    if ((logs_start ≤b retained) : Bool)
    then (retained - logs_start)
    else 0
  (pure { header_gas_used := header_gas_used,
          execution_gas_used := final_usage.execution,
          state_gas_used := final_usage.state,
          blob_gas_used := blob_gas_acc,
          first_tx_recipient := tx0_to,
          receipts_root := receipts_root,
          logs := { start := logs_start,
                    count := logs_count } })

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
        let amount_in_wei := (withdrawal.amount *i 1000000000)
        (k_add_balance withdrawal.address amount_in_wei)
        (pure rest)
    (pure loop_vars) ) : SailM (BoundedSszListRef (2 ^ 4)) )
  (pure ())

/-- The block-end state effects: withdrawals (Shanghai+, EIP-4895), the
pre-merge static block reward before Paris, and the final merge into
the block layer. -/
def apply_block_end_state (body : BlockBody) : SailM Unit := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if ((profile.fork ≥b Shanghai) : Bool)
  then (apply_withdrawals body.withdrawals)
  else (pure ())
  if ((profile.fork <b Paris) : Bool)
  then
    (do
      let coinbase ← do (k_coinbase ())
      (k_add_balance coinbase PRE_MERGE_BLOCK_REWARD))
  else (pure ())
  (k_tx_merge ())

/-- Executes a block body end to end: block-start system calls, the
transaction loop, block-end state effects, and request validation;
invalid execution throws immediately, while successful execution returns
the accumulated [BlockExecutionResult][type-BlockExecutionResult]. -/
def execute_block_body (body : BlockBody) (input_ref : StatelessInputRef) : SailM BlockExecutionResult := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  (bal_reset ())
  writeReg k_current_transaction_epoch 0
  (warm_reset (← readReg k_current_transaction_epoch))
  (run_block_start_system_calls ())
  let result ← do
    (execute_block_transactions body.transactions input_ref.public_keys input_ref.deposits)
  let post_tx_index := (body.transactions.count + 1)
  writeReg k_current_transaction_epoch post_tx_index
  (warm_reset (← readReg k_current_transaction_epoch))
  (apply_block_end_state body)
  if ((profile.fork ≥b Prague) : Bool)
  then (validate_execution_requests input_ref)
  else (pure ())
  (pure result)
