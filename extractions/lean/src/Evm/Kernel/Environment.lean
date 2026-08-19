import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Host.RegionAccess
import Evm.Lib.Rlp.Codecs.Address
import Evm.Lib.Address

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

/-! # The execution environment

The block- and transaction-level environment registers, and the
environment reads the `evm` layer serves opcodes from. -/

def undefined_EnvField (_ : Unit) : SailM EnvField := do
  (internal_pick
    [F_Number, F_Timestamp, F_Coinbase, F_BaseFee, F_ChainId, F_GasLimit, F_PrevRandao, F_Origin, F_GasPrice, F_SlotNumber])

/- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 9 -/
def EnvField_of_num (arg_ : Nat) : EnvField :=
  match arg_ with
  | 0 => F_Number
  | 1 => F_Timestamp
  | 2 => F_Coinbase
  | 3 => F_BaseFee
  | 4 => F_ChainId
  | 5 => F_GasLimit
  | 6 => F_PrevRandao
  | 7 => F_Origin
  | 8 => F_GasPrice
  | _ => F_SlotNumber

def num_of_EnvField (arg_ : EnvField) : Nat :=
  match arg_ with
  | .F_Number => 0
  | .F_Timestamp => 1
  | .F_Coinbase => 2
  | .F_BaseFee => 3
  | .F_ChainId => 4
  | .F_GasLimit => 5
  | .F_PrevRandao => 6
  | .F_Origin => 7
  | .F_GasPrice => 8
  | .F_SlotNumber => 9

/-- An environment field as the word its opcode pushes. -/
def k_env (f : EnvField) : SailM Nat := do
  let ⟨_, active_tx⟩ ← do readReg k_tx
  match f with
  | .F_Number =>
    (do
      let number ← do (pure (word_of_block_number (← readReg k_header).number))
      (pure (u256 number)))
  | .F_Timestamp =>
    (do
      let timestamp ← do (pure (word_of_block_timestamp (← readReg k_header).timestamp))
      (pure (u256 timestamp)))
  | .F_Coinbase => (pure (address_to_word (← readReg k_header).fee_recipient))
  | .F_BaseFee => (pure (← readReg k_header).base_fee)
  | .F_ChainId =>
    (do
      let chain_id ← do (pure (word_of_chain_identifier (← readReg k_chain_id)))
      (pure (u256 chain_id)))
  | .F_GasLimit => (pure (u256 (← readReg k_header).gas_limit))
  | .F_PrevRandao => (pure (← readReg k_header).prev_randao)
  | .F_Origin => (pure (address_to_word active_tx.origin))
  | .F_GasPrice => (pure active_tx.gas_price)
  | .F_SlotNumber =>
    (do
      let slot_number ← do (pure (word_of_slot_number (← readReg k_header).slot_number))
      (pure (u256 slot_number)))

/-- The block's fee recipient (`COINBASE`). -/
def k_coinbase (_ : Unit) : SailM (Vector (BitVec 8) 20) := do
  (pure (← readReg k_header).fee_recipient)

/- Type quantifiers: number : Nat, current : Nat, 0 ≤ number ∧
  number < current ∧ current < (2 ^ 256) -/
def blockhash_word_distance (current : Nat) (number : Nat) : Nat :=
  (current - number)

/-- `BLOCKHASH`: the hash of ancestor `number`, zero outside the 256-block
window; an in-window ancestor missing from the witness is a deficient
witness. -/
/- Type quantifiers: number_word : Nat, 0 ≤ number_word ∧ number_word ≤ (2 ^ 256 - 1) -/
def k_blockhash (number_word : Nat) : SailM (Vector (BitVec 8) 32) := do
  let current ← do (pure (← readReg k_header).number)
  let current_number := (word_of_block_number current)
  let current_word : Nat := (u256 current_number)
  if ((number_word <b current_word) : Bool)
  then
    (do
      let distance_word := (blockhash_word_distance current_word number_word)
      if ((distance_word ≤b 256) : Bool)
      then
        (do
          let distance : Nat := distance_word
          if (((← readReg k_n_headers) <b distance) : Bool)
          then (fatal_error WitnessDeficient)
          else
            (do
              let index : Nat := (distance - 1)
              (ancestor_hash_read index)))
      else (pure ZERO_HASH))
  else (pure ZERO_HASH)

/-- `BLOBHASH` (EIP-4844): the `i`-th versioned hash, zero out of
range. -/
/- Type quantifiers: index_word : Nat, 0 ≤ index_word ∧ index_word ≤ (2 ^ 256 - 1) -/
def k_blobhash (index_word : Nat) : SailM Nat := do
  let ⟨_, active_tx⟩ ← do readReg k_tx
  let count := active_tx.blob_hashes.count
  if ((index_word <b count) : Bool)
  then
    (do
      let index := index_word
      let offset : Nat := ((33 *i index) + 1)
      (stateless_input_slice_load_n active_tx.blob_hashes.bytes offset WORD_BYTE_LENGTH))
  else (pure ZERO_WORD)

/-- The `CREATE` address rule, in kernel form. -/
/- Type quantifiers: k_ex549868_ : Nat, 0 ≤ k_ex549868_ ∧ k_ex549868_ ≤ (2 ^ 64 - 1) -/
def k_create_addr (a : (Vector (BitVec 8) 20)) (nonce : Nat) : SailM (Vector (BitVec 8) 20) := do
  (create_address a nonce)

/-- The `CREATE2` address rule, in kernel form. -/
/- Type quantifiers: k_ex549869_ : Nat, 0 ≤ k_ex549869_ ∧ k_ex549869_ ≤ (2 ^ 256 - 1) -/
def k_create2_addr (a : (Vector (BitVec 8) 20)) (salt : Nat) (inithash : (Vector (BitVec 8) 32)) : SailM (Vector (BitVec 8) 20) := do
  (create2_address a salt inithash)

