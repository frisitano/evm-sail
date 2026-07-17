import Evm.Flow
import Evm.Arith
import Evm.Bytes

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open ConcurrencyInterfaceV1

namespace Evm.Functions

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

def output_buffer_slice (len : byte_quantity) : ByteSlice :=
  if ((byte_quantity_equal len BYTE_ZERO) : Bool)
  then EMPTY_SLICE
  else (byte_slice OutputSource BYTE_ZERO len)

def freeze_output (data : ByteSlice) : SailM ByteSlice := do
  let len := data.len
  if ((byte_quantity_equal len BYTE_ZERO) : Bool)
  then (pure EMPTY_SLICE)
  else
    (do
      if ((← (output_buffer_store data)) : Bool)
      then (pure (output_buffer_slice len))
      else (pure EMPTY_SLICE))

def output_buffer_word (value : (BitVec 256)) : SailM ByteSlice := do
  if ((← (output_buffer_store_word value)) : Bool)
  then (pure (output_buffer_slice WORD_BYTE_LENGTH))
  else (pure EMPTY_SLICE)

def output_buffer_words (first : (BitVec 256)) (second : (BitVec 256)) : SailM ByteSlice := do
  if ((← (output_buffer_store_words first second)) : Bool)
  then (pure (output_buffer_slice DOUBLE_WORD_BYTE_LENGTH))
  else (pure EMPTY_SLICE)

