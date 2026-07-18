import Evm.Primitives.Bytes

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
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open ast
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
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

def undefined_TxType (_ : Unit) : SailM TxType := do
  (internal_pick [LegacyTx, AccessListTx, FeeMarketTx, BlobTx, SetCodeTx])

/-- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 4 -/
def TxType_of_num (arg_ : Nat) : TxType :=
  match arg_ with
  | 0 => LegacyTx
  | 1 => AccessListTx
  | 2 => FeeMarketTx
  | 3 => BlobTx
  | _ => SetCodeTx

def num_of_TxType (arg_ : TxType) : Int :=
  match arg_ with
  | .LegacyTx => 0
  | .AccessListTx => 1
  | .FeeMarketTx => 2
  | .BlobTx => 3
  | .SetCodeTx => 4

def OSAKA_TRANSACTION_GAS_LIMIT_VALUE : Nat := (2 ^i 24)

def OSAKA_TRANSACTION_GAS_LIMIT : gas := (Gas (2 ^i 24))

def undefined_Authorization (_ : Unit) : SailM Authorization := do
  (pure { valid_sig := ← (undefined_bool ()),
          authority := ← (undefined_bitvector 160),
          address := ← (undefined_bitvector 160),
          nonce := ← do
              let semanticField ← (undefined_range 0 ((2 ^i 64) -i 1))
              pure (⟨semanticField⟩),
          chain_id := ← (undefined_bitvector 256) })

def EMPTY_BLOB_HASHES : BlobHashes :=
  { bytes := EMPTY_SLICE,
    count := ⟨0⟩ }

def tx_type_byte (t : TxType) : byte :=
  match t with
  | .LegacyTx => 0x00#8
  | .AccessListTx => 0x01#8
  | .FeeMarketTx => 0x02#8
  | .BlobTx => 0x03#8
  | .SetCodeTx => 0x04#8

def tx_is_access_list (t : TxType) : Bool :=
  match t with
  | .AccessListTx => true
  | _ => false

def tx_is_dynamic_fee (t : TxType) : Bool :=
  match t with
  | .FeeMarketTx => true
  | .BlobTx => true
  | .SetCodeTx => true
  | _ => false

def tx_is_blob (t : TxType) : Bool :=
  match t with
  | .BlobTx => true
  | _ => false

def tx_is_set_code (t : TxType) : Bool :=
  match t with
  | .SetCodeTx => true
  | _ => false

