import Evm.Arith
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

/-! # The output buffer

Host-owned storage for a frame or precompile output that must outlive the
mutable EVM memory from which it was produced. EVM returndata state and
frame transitions remain entirely in Sail.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules. -/

/-- A slice over the first `len` bytes of the output buffer. -/
def output_buffer_slice (len : byte_length) : EvmByteSlice :=
  if ((len == BYTE_ZERO) : Bool)
  then EMPTY_SLICE
  else (byte_slice OutputSource BYTE_ZERO len)

/-- Copies frame output into the host buffer so it survives frame
teardown; the canonical returndata source. -/
def freeze_output (data : EvmByteSlice) : SailM EvmByteSlice := do
  let len := data.len
  if ((len == BYTE_ZERO) : Bool)
  then (pure EMPTY_SLICE)
  else
    (do
      if ((← (output_buffer_store data)) : Bool)
      then (pure (output_buffer_slice len))
      else (pure EMPTY_SLICE))

/-- Stores one word as the output (32-byte precompile results). -/
def output_buffer_word (value : word) : SailM EvmByteSlice := do
  if ((← (output_buffer_store_word value)) : Bool)
  then (pure (output_buffer_slice WORD_BYTE_LENGTH))
  else (pure EMPTY_SLICE)

/-- Stores two words as the output (64-byte precompile results, e.g.
`ecrecover`-style pairs). -/
def output_buffer_words (first : word) (second : word) : SailM EvmByteSlice := do
  if ((← (output_buffer_store_words first second)) : Bool)
  then (pure (output_buffer_slice DOUBLE_WORD_BYTE_LENGTH))
  else (pure EMPTY_SLICE)

