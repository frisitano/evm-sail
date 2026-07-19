import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
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

def num_of_EnvField (arg_ : EnvField) : Int :=
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
  match f with
  | .F_Number => (word_of_protocol_quantity ⟨((← readReg k_header).number).value⟩)
  | .F_Timestamp => (word_of_protocol_quantity ⟨((← readReg k_header).timestamp).value⟩)
  | .F_Coinbase => (pure (address_to_word (← readReg k_header).fee_recipient))
  | .F_BaseFee => (pure (← readReg k_header).base_fee)
  | .F_ChainId => (word_of_protocol_quantity ⟨(← readReg k_chain_id)⟩)
  | .F_GasLimit => (word_of_gas (← readReg k_header).gas_limit)
  | .F_PrevRandao => (pure (← readReg k_header).prev_randao)
  | .F_Origin => (pure (address_to_word (← readReg k_tx).origin))
  | .F_GasPrice => (pure (← readReg k_tx).gas_price)
  | .F_SlotNumber => (word_of_protocol_quantity ⟨((← readReg k_header).slot_number).value⟩)

/-- The block's fee recipient (`COINBASE`). -/
def k_coinbase (_ : Unit) : SailM address := do
  (pure (← readReg k_header).fee_recipient)

/-- `BLOCKHASH`: the hash of ancestor `number`, zero outside the 256-block
window; an in-window ancestor missing from the witness is a deficient
witness. -/
def k_blockhash (number_word : word) : SailM hash := do
  match (word_to_limb number_word) with
  | .some number_bits =>
    (do
      let current ← do (pure ((← readReg k_header).number).value)
      let number := (BitVec.toNatInt number_bits)
      if ((number <b current) : Bool)
      then
        (do
          let distance := (current -i number)
          if ((distance ≤b 256) : Bool)
          then
            (do
              if (((← readReg k_n_headers) <b distance) : Bool)
              then sailThrow ((InvalidBlock WitnessDeficient))
              else
                (do
                  let index : Nat := (distance -i 1)
                  (ancestor_hash_read ⟨index⟩)))
          else (pure ZERO_HASH))
      else (pure ZERO_HASH))
  | none => (pure ZERO_HASH)

/-- `BLOBHASH` (EIP-4844): the `i`-th versioned hash, zero out of
range. -/
def k_blobhash (index_word : word) : SailM word := do
  match (Option.map (fun semanticValue => (semanticValue).value) ((word_to_protocol_quantity
    index_word))) with
  | .some index =>
    (do
      let count ← do (pure ((← readReg k_tx).blob_hashes.count).value)
      if ((index <b count) : Bool)
      then
        (do
          if ((index ≤b 558992244657865200) : Bool)
          then
            (do
              let offset : Nat := ((33 *i index) + 1)
              (slice_load_n (← readReg k_tx).blob_hashes.bytes (ByteQuantity offset)
                WORD_BYTE_LENGTH))
          else (pure ZERO_WORD))
      else (pure ZERO_WORD))
  | none => (pure ZERO_WORD)

/-- The `CREATE` address rule, in kernel form. -/
/- Type quantifiers: k_ex161278_ : Nat, 0 ≤ k_ex161278_ ∧ k_ex161278_ ≤ (2 ^ 64 - 1) -/
def k_create_addr (a : address) (nonce : account_nonce) : SailM address := do
  let nonce := (nonce).value
  (create_address a ⟨nonce⟩)

/-- The `CREATE2` address rule, in kernel form. -/
def k_create2_addr (a : address) (salt : word) (inithash : hash) : SailM address := do
  (create2_address a salt inithash)

