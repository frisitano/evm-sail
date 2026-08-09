import Evm.Primitives.Bytes
import Evm.Primitives.Fork

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

/-! # Transactions, logs, and receipts

The decoded transaction and its EIP-7702 authorization tuple, the log
record, and the receipt. Pure data — no registers, no externs. -/

def undefined_TxType (_ : Unit) : SailM TxType := do
  (internal_pick [LegacyTx, AccessListTx, FeeMarketTx, BlobTx, SetCodeTx])

/- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 4 -/
def TxType_of_num (arg_ : Nat) : TxType :=
  match arg_ with
  | 0 => LegacyTx
  | 1 => AccessListTx
  | 2 => FeeMarketTx
  | 3 => BlobTx
  | _ => SetCodeTx

def num_of_TxType (arg_ : TxType) : Nat :=
  match arg_ with
  | .LegacyTx => 0
  | .AccessListTx => 1
  | .FeeMarketTx => 2
  | .BlobTx => 3
  | .SetCodeTx => 4

/-- The canonical EIP-2718 wire discriminant. Encoding is total over the
closed transaction-type algebra; decoding handles the remaining byte
values explicitly at its validation boundary. -/
def tx_envelope_type (t : TxType) : (BitVec 8) :=
  match t with
  | .LegacyTx => 0x00#8
  | .AccessListTx => 0x01#8
  | .FeeMarketTx => 0x02#8
  | .BlobTx => 0x03#8
  | .SetCodeTx => 0x04#8

def undefined_TxSignatureScheme (_ : Unit) : SailM TxSignatureScheme := do
  (internal_pick [LegacySignature, TypedSignature])

/- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 1 -/
def TxSignatureScheme_of_num (arg_ : Nat) : TxSignatureScheme :=
  match arg_ with
  | 0 => LegacySignature
  | _ => TypedSignature

def num_of_TxSignatureScheme (arg_ : TxSignatureScheme) : Nat :=
  match arg_ with
  | .LegacySignature => 0
  | .TypedSignature => 1

def undefined_TxTypeSemantics (_ : Unit) : SailM TxTypeSemantics := do
  (pure { minimum_fork := ← (undefined_range 0 16),
          signature := ← (undefined_TxSignatureScheme ()),
          blob := ← (undefined_bool ()),
          set_code := ← (undefined_bool ()) })

/-- Derives the protocol requirements of one transaction envelope. -/
def tx_type_semantics (t : TxType) : TxTypeSemantics :=
  match t with
  | .LegacyTx =>
    { minimum_fork := Frontier,
      signature := LegacySignature,
      blob := false,
      set_code := false }
  | .AccessListTx =>
    { minimum_fork := Berlin,
      signature := TypedSignature,
      blob := false,
      set_code := false }
  | .FeeMarketTx =>
    { minimum_fork := London,
      signature := TypedSignature,
      blob := false,
      set_code := false }
  | .BlobTx =>
    { minimum_fork := Cancun,
      signature := TypedSignature,
      blob := true,
      set_code := false }
  | .SetCodeTx =>
    { minimum_fork := Prague,
      signature := TypedSignature,
      blob := false,
      set_code := true }

def undefined_Authorization (_ : Unit) : SailM Authorization := do
  (pure { valid_sig := ← (undefined_bool ()),
          authority := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          nonce := ← (undefined_range 0 ((2 ^i 64) - 1)),
          chain_id := ← (undefined_range 0 ((2 ^i 256) - 1)) })

def EMPTY_BLOB_HASHES : (BlobHashesFields blob_schedule_inactive_count) :=
  ({ bytes := ⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩,
     count := 0 } : (BlobHashesFields 0))

def EMPTY_ACCESS_LIST_REF : AccessListRef :=
  { encoded := ⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩,
    address_count := 0,
    slot_count := 0 }

/- Type quantifiers: encoded_dependentWitness1 : Nat, encoded_dependentWitness0 : Nat, count : Nat, 0
  ≤ count ∧ count ≤ transaction_length_bound, 0 ≤ encoded_dependentWitness0 ∧
  0 ≤ encoded_dependentWitness1 ∧
  (encoded_dependentWitness0 + encoded_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def authorization_list_ref (encoded : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (count : Nat) : (AuthorizationListRefFields count) :=
  let encoded_dependentWitness0 := (encoded).1
  let encoded_dependentWitness1 := ((encoded).2).1
  let encoded := ((encoded).2).2
  { encoded := ⟨_, ⟨_, encoded⟩⟩ }

def EMPTY_AUTHORIZATION_LIST_REF : AuthorizationListRef :=
  ⟨_, (authorization_list_ref ⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩ 0)⟩

/-- Packs a transaction whose concrete blob limit is still in scope into the
existential transaction surface used by envelope dispatch. -/
/- Type quantifiers: k_blob_limit : Nat, (transaction_blob_limit_value k_blob_limit) -/
def pack_transaction (tx : (TransactionFields k_blob_limit)) : (Sigma fun (k_syn_blob_limit : Nat)
  => (TransactionFields k_syn_blob_limit)) :=
  ((⟨_, tx⟩ : (Sigma fun (k_syn_blob_limit : Nat) => (TransactionFields k_syn_blob_limit))) : (Sigma
  fun (k_syn_blob_limit : Nat) => (TransactionFields k_syn_blob_limit)))

/-- Advances a valid log cursor without fixed-width wrapping. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def log_store_index_increment (value : Nat) : SailM Nat := do
  if ((value <b ((2 ^i 64) - 1)) : Bool)
  then (pure (value + 1))
  else
    (do
      assert false "log store index overflow"
      throw Error.Exit)

/-- Adds a relative log offset to its series start without wrapping. -/
/- Type quantifiers: k_ex549733_ : Nat, k_ex549732_ : Nat, 0 ≤ k_ex549732_ ∧
  k_ex549732_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex549733_ ∧ k_ex549733_ ≤ (2 ^ 64 - 1) -/
def log_store_index_add (left : Nat) (right : Nat) : SailM Nat := do
  if ((right ≤b (((2 ^i 64) - 1) - left)) : Bool)
  then (pure (left + right))
  else
    (do
      assert false "log store index overflow"
      throw Error.Exit)

def undefined_LogSeriesRef (_ : Unit) : SailM LogSeriesRef := do
  (pure { start := ← (undefined_range 0 ((2 ^i 64) - 1)),
          count := ← (undefined_range 0 ((2 ^i 64) - 1)) })

/- Type quantifiers: k_ex549789_ : Bool, _limit : Nat, _regular_limit : Nat, gas_used : Nat, execution_gas
  : Nat, state_gas : Nat, (receipt_gas_relation _limit _regular_limit gas_used execution_gas state_gas) -/
def receipt_fields (_limit : Nat) (_regular_limit : Nat) (tx_type : TxType) (success : Bool) (gas_used : Nat) (execution_gas : Nat) (state_gas : Nat) (logs : LogSeriesRef) : (ReceiptFields _limit _regular_limit gas_used execution_gas state_gas) :=
  { tx_type := tx_type,
    success := success,
    logs := logs }

/- Type quantifiers: k_ex549835_ : Bool, limit : Nat, regular_limit : Nat, gas_used : Nat, execution_gas
  : Nat, state_gas : Nat, (receipt_gas_relation limit regular_limit gas_used execution_gas state_gas) -/
def receipt_within (limit : Nat) (regular_limit : Nat) (tx_type : TxType) (success : Bool) (gas_used : Nat) (execution_gas : Nat) (state_gas : Nat) (logs : LogSeriesRef) : (Sigma
  fun (k_syn_state_gas : Nat) =>
  (Sigma fun (k_syn_execution_gas : Nat) =>
  (Sigma fun (k_syn_gas_used : Nat) =>
  (ReceiptFields limit regular_limit k_syn_gas_used k_syn_execution_gas k_syn_state_gas)))) :=
  ((⟨_, ⟨_, ⟨_, (receipt_fields limit regular_limit tx_type success gas_used execution_gas
    state_gas logs)⟩⟩⟩ : (Sigma fun (k_syn_state_gas : Nat) =>
  (Sigma fun (k_syn_execution_gas : Nat) =>
  (Sigma fun (k_syn_gas_used : Nat) =>
  (ReceiptFields limit regular_limit k_syn_gas_used k_syn_execution_gas k_syn_state_gas))))) : (Sigma
  fun (k_syn_state_gas : Nat) =>
  (Sigma fun (k_syn_execution_gas : Nat) =>
  (Sigma fun (k_syn_gas_used : Nat) =>
  (ReceiptFields limit regular_limit k_syn_gas_used k_syn_execution_gas k_syn_state_gas)))))

