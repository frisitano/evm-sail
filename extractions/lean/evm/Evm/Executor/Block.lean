import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Primitives.System
import Evm.Primitives.Block
import Evm.Host.Kernel.Scratch
import Evm.Host.Kernel.Environment
import Evm.Host.Kernel.Accounts
import Evm.Host.Kernel.Lifecycle
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

open word
open option
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open b256
open ast
open address
open TxType
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open StateCheckpoint
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
open BlockError

/-! # The block driver

Block-start system calls, the transaction loop with block-gas
accounting, withdrawals, and block-end request collection. -/

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
def execute_block_transactions (transactions : SszListRef) (public_keys : EvmByteSlice) (block_gas_limit : gas) : SailM BlockExecutionResult := do
  let public_key_count ← do (byte_quantity_quotient public_keys.len PUBLIC_KEY_LENGTH)
  let .ByteQuantity public_key_count_value := public_key_count
  if (((public_key_count_value != (transactions.count).value) || (bne public_keys.len
         (← (byte_quantity_mul public_key_count PUBLIC_KEY_LENGTH)))) : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else (pure ())
  let all_ok : Bool := true
  let gas_limit := block_gas_limit
  let gas_acc : gas := GAS_ZERO
  let blob_gas_acc : Nat := 0
  let tx0_to : address := ZERO_ADDR
  let block_gas_overflow : Bool := false
  let receipts := (receipt_accumulator_empty ())
  let deposits_start ← do (scratch_begin ())
  let cursor ← do (ssz_list_cursor transactions)
  let keys := public_keys
  let (all_ok, blob_gas_acc, block_gas_overflow, cursor, gas_acc, keys, receipts, tx0_to) ← (( do
    let loop_vars ← whileFuelM (fuel :=((cursor.items.count).value -i (cursor.index).value)) (fun (all_ok, blob_gas_acc, block_gas_overflow, cursor, gas_acc, keys, receipts, tx0_to) => (pure (! (ssz_list_cursor_empty
          cursor)))) (all_ok, blob_gas_acc, block_gas_overflow, cursor, gas_acc, keys, receipts, tx0_to)
      fun (all_ok, blob_gas_acc, block_gas_overflow, cursor, gas_acc, keys, receipts, tx0_to) => do
        assert true "loop dummy assert"
        let i := (cursor.index).value
        let (transaction, next) ← do (ssz_list_pop cursor)
        let cursor : SszListCursor := next
        let public_key ← do (sub_slice keys BYTE_ZERO PUBLIC_KEY_LENGTH)
        let keys ←
          (sub_slice keys PUBLIC_KEY_LENGTH (← (byte_quantity_sub keys.len PUBLIC_KEY_LENGTH)))
        let tx ← do (decode_transaction transaction public_key)
        (bal_set_index ⟨((← (protocol_quantity_increment ⟨i⟩))).value⟩)
        let (all_ok, blob_gas_acc, block_gas_overflow, gas_acc, receipts, tx0_to) ← (( do
          if ((! block_gas_overflow) : Bool)
          then
            (do
              let tx0_to : address :=
                if ((i == 0) : Bool)
                then tx.recipient
                else tx0_to
              let (all_ok, blob_gas_acc, block_gas_overflow, gas_acc, receipts) ← (( do
                if ((gas_lt gas_limit gas_acc) : Bool)
                then
                  (let block_gas_overflow : Bool := true
                  (pure (all_ok, blob_gas_acc, block_gas_overflow, gas_acc, receipts)))
                else
                  (do
                    let available_gas ← do (gas_sub gas_limit gas_acc)
                    let (all_ok, blob_gas_acc, block_gas_overflow, gas_acc, receipts) ← (( do
                      if ((gas_lt available_gas tx.gas_limit) : Bool)
                      then
                        (let block_gas_overflow : Bool := true
                        (pure (all_ok, blob_gas_acc, block_gas_overflow, gas_acc, receipts)))
                      else
                        (do
                          let tx_blob_gas ← do
                            (do
                                let semanticResult ← (transaction_blob_gas
                                ⟨(tx.blob_hashes.count).value⟩)
                                pure ((semanticResult).value))
                          let next_blob_gas ← (( do
                            if ((fork_lt (← readReg k_fork) Cancun) : Bool)
                            then (pure blob_gas_acc)
                            else
                              (do
                                  let semanticResult ← (checked_block_blob_gas_add
                                  ⟨blob_gas_acc⟩ ⟨tx_blob_gas⟩)
                                  pure ((semanticResult).value)) ) : SailM Nat )
                          let receipt ← do (process_transaction tx)
                          let all_ok : Bool := (all_ok && receipt.valid)
                          let (block_gas_overflow, gas_acc) ← (( do
                            if ((gas_sum_supported gas_acc receipt.block_gas) : Bool)
                            then
                              (do
                                let gas_acc ← (gas_add gas_acc receipt.block_gas)
                                let block_gas_overflow : Bool := (gas_lt gas_limit gas_acc)
                                (pure (block_gas_overflow, gas_acc)))
                            else
                              (let block_gas_overflow : Bool := true
                              (pure (block_gas_overflow, gas_acc))) ) : SailM (Bool × gas) )
                          let (blob_gas_acc, receipts) ← (( do
                            if (receipt.valid : Bool)
                            then
                              (do
                                let receipts ← (receipt_accumulator_push receipts receipt)
                                (append_deposit_logs receipt.logs)
                                let blob_gas_acc : Nat := next_blob_gas
                                (pure (blob_gas_acc, receipts)))
                            else (pure (blob_gas_acc, receipts)) ) : SailM
                            (Nat × ReceiptAccumulator) )
                          (pure (all_ok, blob_gas_acc, block_gas_overflow, gas_acc, receipts))) ) :
                      SailM (Bool × Nat × Bool × gas × ReceiptAccumulator) )
                    (pure (all_ok, blob_gas_acc, block_gas_overflow, gas_acc, receipts))) ) : SailM
                (Bool × Nat × Bool × gas × ReceiptAccumulator) )
              (pure (all_ok, blob_gas_acc, block_gas_overflow, gas_acc, receipts, tx0_to)))
          else (pure (all_ok, blob_gas_acc, block_gas_overflow, gas_acc, receipts, tx0_to)) ) :
          SailM (Bool × Nat × Bool × gas × ReceiptAccumulator × address) )
        (pure (all_ok, blob_gas_acc, block_gas_overflow, cursor, gas_acc, keys, receipts, tx0_to))
    (pure loop_vars) ) : SailM
    (Bool × Nat × Bool × SszListCursor × gas × EvmByteSlice × ReceiptAccumulator × address) )
  (pure { all_ok := (all_ok && (! block_gas_overflow)),
          gas_acc := gas_acc,
          blob_gas_acc := ⟨blob_gas_acc⟩,
          first_tx_recipient := tx0_to,
          block_gas_overflow := block_gas_overflow,
          receipts_root := ← (receipt_accumulator_root receipts),
          logs_bloom := receipts.bloom,
          deposits := ← (scratch_finish deposits_start),
          requests := EMPTY_EXECUTION_REQUESTS })

/-- Credits every withdrawal's recipient with its amount in gwei
(EIP-4895); withdrawals cannot fail and charge no gas. -/
def apply_withdrawals (withdrawals : SszListRef) : SailM Unit := do
  let rest := withdrawals
  let rest ← (( do
    let loop_vars ← whileFuelM (fuel :=(rest.count).value) (fun rest => (pure ((rest.count).value != 0))) rest
      fun rest => do
        assert true "loop dummy assert"
        let (withdrawal_ref, tail) ← do (ssz_fixed_list_pop rest WD_SIZE)
        let rest : SszListRef := tail
        let withdrawal ← do (decode_withdrawal withdrawal_ref)
        (k_add_balance withdrawal.address
          (alu_mul (← (word_of_protocol_quantity ⟨(withdrawal.amount).value⟩))
            (← (word_of_protocol_quantity ⟨1000000000⟩))))
        (pure rest)
    (pure loop_vars) ) : SailM SszListRef )
  (pure ())

/-- Runs the checked block-end request collection (Prague+, EIP-7685);
skipped entirely when the body already failed. -/
/- Type quantifiers: k_ex161658_ : Bool -/
def run_checked_block_end_system_calls (all_ok : Bool) (deposits : EvmByteSlice) : SailM (Bool × ExecutionRequests) := do
  if ((! all_ok) : Bool)
  then (pure (false, EMPTY_EXECUTION_REQUESTS))
  else
    (do
      if ((fork_gteq (← readReg k_fork) Prague) : Bool)
      then (collect_execution_requests deposits)
      else (pure (true, EMPTY_EXECUTION_REQUESTS)))

/-- The block-end state effects: withdrawals (Shanghai+, EIP-4895), the
pre-merge static block reward before Paris, and the final merge into
the block layer. -/
def apply_block_end_state (body : BlockBody) : SailM Unit := do
  if ((fork_gteq (← readReg k_fork) Shanghai) : Bool)
  then (apply_withdrawals body.withdrawals)
  else (pure ())
  if ((fork_lt (← readReg k_fork) Paris) : Bool)
  then
    (k_add_balance (← (k_coinbase ())) (← (word_of_protocol_quantity ⟨2000000000000000000⟩)))
  else (pure ())
  (k_tx_merge ())

/-- Executes a block body end to end: block-start system calls, the
transaction loop, block-end state effects, and request collection;
returns overall success with the accumulated
[BlockExecutionResult][type-BlockExecutionResult]. -/
def execute_block_body (body : BlockBody) (public_keys : EvmByteSlice) (block_gas_limit : gas) : SailM (Bool × BlockExecutionResult) := do
  (bal_reset ())
  (bal_set_index ⟨0⟩)
  (run_block_start_system_calls ())
  let result ← do (execute_block_transactions body.transactions public_keys block_gas_limit)
  let post_tx_index ← (( do
    if (((body.transactions.count).value <b (BYTE_QUANTITY_MAX).value) : Bool)
    then
      (do
          let semanticResult ← (protocol_quantity_increment ⟨(body.transactions.count).value⟩)
          pure ((semanticResult).value))
    else sailThrow ((InvalidBlock WitnessDeficient)) ) : SailM Nat )
  (bal_set_index ⟨post_tx_index⟩)
  (apply_block_end_state body)
  let (exec_ok, requests) ← do (run_checked_block_end_system_calls result.all_ok result.deposits)
  (pure (exec_ok, { result with requests := requests }))

