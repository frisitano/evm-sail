import Evm.Primitives.Gas
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

def OSAKA_TRANSACTION_GAS_LIMIT_VALUE : transaction_gas := TRANSACTION_EXECUTION_GAS_LIMIT

def OSAKA_TRANSACTION_GAS_LIMIT : transaction_gas := TRANSACTION_EXECUTION_GAS_LIMIT

def undefined_Authorization (_ : Unit) : SailM Authorization := do
  (pure { valid_sig := ← (undefined_bool ()),
          authority := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          nonce := ← (undefined_range 0 ((2 ^i 64) - 1)),
          chain_id := ← (undefined_range 0 ((2 ^i 256) - 1)) })

def EMPTY_BLOB_HASHES : BlobHashes :=
  { bytes := ⟨_, ⟨_, EMPTY_SLICE⟩⟩,
    count := 0 }

/-- The envelope type byte, as it appears in tx and receipt encodings. -/
def tx_type_byte (t : TxType) : (BitVec 8) :=
  match t with
  | .LegacyTx => 0x00#8
  | .AccessListTx => 0x01#8
  | .FeeMarketTx => 0x02#8
  | .BlobTx => 0x03#8
  | .SetCodeTx => 0x04#8

/-- Whether the type carries an EIP-2930 access list envelope (type 1). -/
def tx_is_access_list (t : TxType) : Bool :=
  match t with
  | .AccessListTx => true
  | _ => false

/-- Whether the type is an EIP-1559-style fee-market envelope (types
2/3/4). -/
def tx_is_dynamic_fee (t : TxType) : Bool :=
  match t with
  | .FeeMarketTx => true
  | .BlobTx => true
  | .SetCodeTx => true
  | _ => false

/-- Whether the type is an EIP-4844 blob transaction (type 3). -/
def tx_is_blob (t : TxType) : Bool :=
  match t with
  | .BlobTx => true
  | _ => false

/-- Whether the type is an EIP-7702 set-code transaction (type 4). -/
def tx_is_set_code (t : TxType) : Bool :=
  match t with
  | .SetCodeTx => true
  | _ => false

