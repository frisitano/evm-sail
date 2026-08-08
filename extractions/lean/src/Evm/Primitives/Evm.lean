import Evm.Prelude
import Evm.Primitives.Gas

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
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
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

/-! # EVM execution types

The per-transaction environment, the transaction validity and frame-result
records, and the per-frame call [Message][type-Message]. Pure data — no
registers, no externs. -/

/- Type quantifiers: gas_price : Nat, k_blob_limit : Nat, (transaction_blob_limit_value k_blob_limit), 0
  ≤ gas_price ∧ gas_price ≤ (2 ^ 256 - 1) -/
def tx_env (origin : (Vector (BitVec 8) 20)) (gas_price : Nat) (blob_hashes : (BlobHashesFields k_blob_limit)) : (TxEnvFields k_blob_limit) :=
  { origin := origin,
    gas_price := gas_price,
    blob_hashes := blob_hashes }

/- Type quantifiers: total : Nat, regular : Nat, intrinsic_execution : Nat, intrinsic_state : Nat, calldata_floor
  : Nat, execution : Nat, state : Nat, (transaction_initial_gas_relation total regular intrinsic_execution intrinsic_state calldata_floor execution state) -/
def transaction_initial_gas_fields (total : Nat) (regular : Nat) (intrinsic_execution : Nat) (intrinsic_state : Nat) (calldata_floor : Nat) (execution : Nat) (state : Nat) : (TransactionInitialGasFields total regular intrinsic_execution intrinsic_state calldata_floor execution state) :=
  { admitted_limit := total,
    regular_limit := regular,
    intrinsic_execution := intrinsic_execution,
    intrinsic_state := intrinsic_state,
    calldata_floor := calldata_floor,
    execution_remaining := execution,
    state_remaining := state }

/- Type quantifiers: priority_fee : Nat, gas_price : Nat, blob_fee : Nat, nonce_before : Nat, k_limit
  : Nat, k_regular : Nat, k_intrinsic_execution : Nat, k_intrinsic_state : Nat, k_calldata_floor :
  Nat, k_execution : Nat, k_state : Nat, (transaction_initial_gas_relation k_limit k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state), 0
  ≤ nonce_before ∧ nonce_before ≤ (2 ^ 64 - 1), 0 ≤ blob_fee ∧ blob_fee ≤ (2 ^ 256 - 1), 0
  ≤ gas_price ∧ gas_price ≤ (2 ^ 256 - 1), 0 ≤ priority_fee ∧
  priority_fee ≤ (2 ^ 256 - 1) -/
def tx_validity_fields (sender : (Vector (BitVec 8) 20)) (nonce_before : Nat) (gas : (TransactionInitialGasFields k_limit k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state)) (blob_fee : Nat) (gas_price : Nat) (priority_fee : Nat) : (TxValidityFields k_limit k_regular k_intrinsic_execution k_intrinsic_state k_calldata_floor k_execution k_state) :=
  { sender := sender,
    nonce_before := nonce_before,
    gas := gas,
    blob_fee := blob_fee,
    gas_price := gas_price,
    priority_fee := priority_fee }

/- Type quantifiers: limit : Nat, regular : Nat, calldata_floor : Nat, remaining : Nat, state_used :
  Nat, (tx_frame_gas_snapshot_relation limit regular calldata_floor remaining state_used) -/
def tx_frame_gas_snapshot_fields (limit : Nat) (regular : Nat) (calldata_floor : Nat) (remaining : Nat) (state_used : Nat) : (TxFrameGasSnapshotFields limit regular calldata_floor remaining state_used) :=
  { admitted_limit := limit,
    regular_limit := regular,
    calldata_floor := calldata_floor,
    remaining := remaining,
    state_used := state_used }

def undefined_CallKind (_ : Unit) : SailM CallKind := do
  (internal_pick [Call, CallCode, DelegateCall, StaticCall])

/- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 3 -/
def CallKind_of_num (arg_ : Nat) : CallKind :=
  match arg_ with
  | 0 => Call
  | 1 => CallCode
  | 2 => DelegateCall
  | _ => StaticCall

def num_of_CallKind (arg_ : CallKind) : Nat :=
  match arg_ with
  | .Call => 0
  | .CallCode => 1
  | .DelegateCall => 2
  | .StaticCall => 3

def undefined_CreateKind (_ : Unit) : SailM CreateKind := do
  (internal_pick [CreateByNonce, CreateBySalt])

/- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 1 -/
def CreateKind_of_num (arg_ : Nat) : CreateKind :=
  match arg_ with
  | 0 => CreateByNonce
  | _ => CreateBySalt

def num_of_CreateKind (arg_ : CreateKind) : Nat :=
  match arg_ with
  | .CreateByNonce => 0
  | .CreateBySalt => 1

def undefined_Message (_ : Unit) : SailM Message := do
  (pure { caller := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          code_address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          value := ← (undefined_range 0 ((2 ^i 256) - 1)),
          state_gas_reservoir := ← (undefined_nat ()),
          is_static := ← (undefined_bool ()),
          depth := ← (undefined_range 0 1024) })

/-- The zero message; frame registers reset to it between transactions. -/
def DEFAULT_MESSAGE : Message :=
  { caller := ZERO_ADDRESS,
    address := ZERO_ADDRESS,
    code_address := ZERO_ADDRESS,
    value := ZERO_WORD,
    state_gas_reservoir := GAS_ZERO,
    is_static := false,
    depth := 0 }

