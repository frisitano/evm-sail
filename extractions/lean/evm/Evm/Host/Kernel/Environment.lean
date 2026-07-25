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
def k_env (f : EnvField) : SailM Nat := do
  match f with
  | .F_Number => (pure (U256 (← (word_of_block_number (← readReg k_header).number))))
  | .F_Timestamp => (pure (U256 (← (word_of_block_timestamp (← readReg k_header).timestamp))))
  | .F_Coinbase => (pure (address_to_word (← readReg k_header).fee_recipient))
  | .F_BaseFee => (pure (← readReg k_header).base_fee)
  | .F_ChainId => (pure (U256 (word_of_chain_identifier (← readReg k_chain_id))))
  | .F_GasLimit => (pure (U256 (← readReg k_header).gas_limit))
  | .F_PrevRandao => (pure (← readReg k_header).prev_randao)
  | .F_Origin => (pure (address_to_word (← readReg k_tx).origin))
  | .F_GasPrice => (pure (← readReg k_tx).gas_price)
  | .F_SlotNumber => (pure (U256 (word_of_slot_number (← readReg k_header).slot_number)))

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
  let current_word ← (( do (pure (U256 (← (word_of_block_number current)))) ) : SailM Nat )
  if ((number_word <b current_word) : Bool)
  then
    (do
      let distance_word := (blockhash_word_distance current_word number_word)
      if ((distance_word ≤b 256) : Bool)
      then
        (do
          let distance : Nat := distance_word
          if (((← readReg k_n_headers) <b distance) : Bool)
          then sailThrow ((InvalidBlock WitnessDeficient))
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
  let count ← do (pure (← readReg k_tx).blob_hashes.count)
  if ((index_word <b count) : Bool)
  then
    (do
      let index : Nat := index_word
      let offset : Nat := ((33 *i index) + 1)
      (do
          let dependentArg0 := (← readReg k_tx).blob_hashes.bytes
          (slice_load_n dependentArg0 offset WORD_BYTE_LENGTH)))
  else (pure ZERO_WORD)

/-- The `CREATE` address rule, in kernel form. -/
/- Type quantifiers: k_ex416527_ : Nat, 0 ≤ k_ex416527_ ∧ k_ex416527_ ≤ (2 ^ 64 - 1) -/
def k_create_addr (a : (Vector (BitVec 8) 20)) (nonce : Nat) : SailM (Vector (BitVec 8) 20) := do
  (create_address a nonce)

/-- The `CREATE2` address rule, in kernel form. -/
/- Type quantifiers: k_ex416528_ : Nat, 0 ≤ k_ex416528_ ∧ k_ex416528_ ≤ (2 ^ 256 - 1) -/
def k_create2_addr (a : (Vector (BitVec 8) 20)) (salt : Nat) (inithash : (Vector (BitVec 8) 32)) : SailM (Vector (BitVec 8) 20) := do
  (create2_address a salt inithash)

