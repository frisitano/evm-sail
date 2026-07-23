import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Bytes
import Evm.Primitives.Fork
import Evm.Primitives.ChainConfig
import Evm.Host.EvmByteSlice
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
def k_env (f : EnvField) : SailM word := do
  let publicResult ← do
    match f with
    | .F_Number => (pure ((U256 (← (word_of_block_number (← readReg k_header).number)))).value)
    | .F_Timestamp =>
      (pure ((U256 (← (word_of_block_timestamp (← readReg k_header).timestamp)))).value)
    | .F_Coinbase => (pure ((address_to_word (← readReg k_header).fee_recipient)).value)
    | .F_BaseFee => (pure ((← readReg k_header).base_fee).value)
    | .F_ChainId => (pure ((U256 (← (word_of_chain_identifier (← readReg k_chain_id))))).value)
    | .F_GasLimit => (pure ((U256 ((← readReg k_header).gas_limit).value)).value)
    | .F_PrevRandao => (pure ((← readReg k_header).prev_randao).value)
    | .F_Origin => (pure ((address_to_word (← readReg k_tx).origin)).value)
    | .F_GasPrice => (pure ((← readReg k_tx).gas_price).value)
    | .F_SlotNumber =>
      (pure ((U256 ((word_of_slot_number ⟨((← readReg k_header).slot_number).value⟩)).value)).value)
  pure (⟨publicResult⟩)

/-- The block's fee recipient (`COINBASE`). -/
def k_coinbase (_ : Unit) : SailM address := do
  (pure (← readReg k_header).fee_recipient)

/- Type quantifiers: number : Nat, current : Nat, 0 ≤ number ∧
  number < current ∧ current < (2 ^ 256) -/
def blockhash_word_distance (current : Nat) (number : Nat) : Nat :=
  (current - number)

/-- `BLOCKHASH`: the hash of ancestor `number`, zero outside the 256-block
window; an in-window ancestor missing from the witness is a deficient
witness. -/
/- Type quantifiers: number_word : Nat, 0 ≤ number_word ∧ number_word ≤ (2 ^ 256 - 1) -/
def k_blockhash (number_word : word) : SailM hash := do
  let number_word := (number_word).value
  let current ← do (pure (← readReg k_header).number)
  let current_word ← (( do (pure ((U256 (← (word_of_block_number current)))).value) ) : SailM
    Nat )
  if ((number_word <b current_word) : Bool)
  then
    (do
      let distance_word := (blockhash_word_distance current_word number_word)
      if ((distance_word ≤b 256) : Bool)
      then
        (do
          let distance : Nat := distance_word
          if ((((← readReg k_n_headers)).value <b distance) : Bool)
          then sailThrow ((InvalidBlock WitnessDeficient))
          else
            (do
              let index : Nat := (distance - 1)
              (ancestor_hash_read ⟨index⟩)))
      else (pure ZERO_HASH))
  else (pure ZERO_HASH)

/-- `BLOBHASH` (EIP-4844): the `i`-th versioned hash, zero out of
range. -/
/- Type quantifiers: index_word : Nat, 0 ≤ index_word ∧ index_word ≤ (2 ^ 256 - 1) -/
def k_blobhash (index_word : word) : SailM word := do
  let index_word := (index_word).value
  let publicResult ← do
    let count ← do (pure ((← readReg k_tx).blob_hashes.count).value)
    if ((index_word <b count) : Bool)
    then
      (do
        let index : Nat := index_word
        let offset : Nat := ((33 *i index) + 1)
        (do
            let publicResult ← (slice_load_n (← readReg k_tx).blob_hashes.bytes offset
            WORD_BYTE_LENGTH)
            pure ((publicResult).value)))
    else (pure (ZERO_WORD).value)
  pure (⟨publicResult⟩)

/-- The `CREATE` address rule, in kernel form. -/
/- Type quantifiers: k_ex410433_ : Nat, 0 ≤ k_ex410433_ ∧ k_ex410433_ ≤ (2 ^ 64 - 1) -/
def k_create_addr (a : address) (nonce : account_nonce) : SailM address := do
  let nonce := (nonce).value
  (create_address a ⟨nonce⟩)

/-- The `CREATE2` address rule, in kernel form. -/
/- Type quantifiers: k_ex410434_ : Nat, 0 ≤ k_ex410434_ ∧ k_ex410434_ ≤ (2 ^ 256 - 1) -/
def k_create2_addr (a : address) (salt : word) (inithash : hash) : SailM address := do
  let salt := (salt).value
  (create2_address a ⟨salt⟩ inithash)

