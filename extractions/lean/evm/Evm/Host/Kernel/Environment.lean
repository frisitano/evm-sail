import Evm.Arith
import Evm.Prelude
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

noncomputable section
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
open Register
open NodeRef
open MerkleSlot
open JEntry
open HaltKind
open FrameStatus
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

def undefined_EnvField (_ : Unit) : SailM EnvField := do
  (internal_pick
    [F_Number, F_Timestamp, F_Coinbase, F_BaseFee, F_ChainId, F_GasLimit, F_PrevRandao, F_Origin, F_GasPrice, F_SlotNumber])

/-- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 9 -/
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

def k_env (f : EnvField) : SailM (BitVec 256) := do
  match f with
  | .F_Number => (word_of_nat (← readReg k_header).number)
  | .F_Timestamp => (word_of_nat (← readReg k_header).timestamp)
  | .F_Coinbase => (pure (address_to_word (← readReg k_header).fee_recipient))
  | .F_BaseFee => (pure (← readReg k_header).base_fee)
  | .F_ChainId => (word_of_nat (← readReg k_chain_id))
  | .F_GasLimit => (word_of_gas (← readReg k_header).gas_limit)
  | .F_PrevRandao => (pure (← readReg k_header).prev_randao)
  | .F_Origin => (pure (address_to_word (← readReg k_tx).origin))
  | .F_GasPrice => (pure (← readReg k_tx).gas_price)
  | .F_SlotNumber => (word_of_nat (← readReg k_header).slot_number)

def k_coinbase (_ : Unit) : SailM (BitVec 160) := do
  (pure (← readReg k_header).fee_recipient)

def k_blockhash (number_word : (BitVec 256)) : SailM (BitVec 256) := do
  let current ← do (pure (← readReg k_header).number)
  let number := (BitVec.toNatInt number_word)
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
              let index : ancestor_index := (distance -i 1)
              (ancestor_hash_read index)))
      else (pure ZERO_WORD))
  else (pure ZERO_WORD)

def k_blobhash (index_word : (BitVec 256)) : SailM (BitVec 256) := do
  let count ← do (pure (← readReg k_tx).blob_hashes.count)
  let index := (BitVec.toNatInt index_word)
  if ((index <b count) : Bool)
  then
    (do
      let offset := ((33 *i index) +i 1)
      if ((offset ≤b BYTE_QUANTITY_MAX) : Bool)
      then
        (slice_load_n (← readReg k_tx).blob_hashes.bytes (ByteQuantity offset) WORD_BYTE_LENGTH)
      else (pure ZERO_WORD))
  else (pure ZERO_WORD)

/-- Type quantifiers: nonce : Nat, 0 ≤ nonce ∧ nonce ≤ (2 ^ 64 - 1) -/
def k_create_addr (a : (BitVec 160)) (nonce : Nat) : SailM (BitVec 160) := do
  (create_address a nonce)

def k_create2_addr (a : (BitVec 160)) (salt : (BitVec 256)) (inithash : (BitVec 256)) : SailM (BitVec 160) := do
  (create2_address a salt inithash)

