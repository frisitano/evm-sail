import Evm.Flow
import Evm.Arith
import Evm.Primitives.Quantities

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

/-! # Regions and byte slices

Unmaterialized byte sequences. The specification's bulk data lives in
named **regions** of the host interface — the stateless input, frame
memory, code, log data, output, and scratch stores enumerated by
[ByteSource][type-ByteSource]. A [EvmByteSlice][type-EvmByteSlice] denotes a
byte range inside one region without copying it into Sail; a slice's
meaning is the byte sequence it references.

## Constants

The fixed lengths identify addresses, words, limbs, and double words;
`EMPTY_SLICE` is the canonical zero-length range. -/

def undefined_ByteSource (_ : Unit) : SailM ByteSource := do
  (internal_pick
    [StatelessInputSource, EvmMemorySource, CodeSource, LogDataSource, OutputSource, ScratchSource])

/- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 5 -/
def ByteSource_of_num (arg_ : Nat) : ByteSource :=
  match arg_ with
  | 0 => StatelessInputSource
  | 1 => EvmMemorySource
  | 2 => CodeSource
  | 3 => LogDataSource
  | 4 => OutputSource
  | _ => ScratchSource

def num_of_ByteSource (arg_ : ByteSource) : Int :=
  match arg_ with
  | .StatelessInputSource => 0
  | .EvmMemorySource => 1
  | .CodeSource => 2
  | .LogDataSource => 3
  | .OutputSource => 4
  | .ScratchSource => 5

def ADDRESS_BYTE_LENGTH : byte_length := (ByteQuantity 20)

def WORD_BYTE_LENGTH : byte_length := (ByteQuantity 32)

def EIGHT_BYTE_LENGTH : byte_length := (ByteQuantity 8)

def DOUBLE_WORD_BYTE_LENGTH : byte_length := (ByteQuantity 64)

/-- Constructs a [EvmByteSlice][type-EvmByteSlice] from integer offset and
length. -/
def byte_slice (src : ByteSource) (off : source_pointer) (len : byte_length) : EvmByteSlice :=
  { source := src,
    off := off,
    len := len }

/-- The zero-length placeholder slice, for unused value roles (subtree
references, deletes). -/
def EMPTY_SLICE : EvmByteSlice :=
  { source := StatelessInputSource,
    off := BYTE_ZERO,
    len := BYTE_ZERO }

/-- A slice into the stateless input (witness) store. -/
def stateless_input_byte_slice (off : source_pointer) (len : byte_length) : EvmByteSlice :=
  (byte_slice StatelessInputSource off len)

/-- A relative sub-range of a slice, within the same source. -/
def sub_slice (s : EvmByteSlice) (off : source_pointer) (len : byte_length) : SailM EvmByteSlice := do
  assert (byte_quantity_le off s.len) "sail/primitives/bytes.sail:54.23-54.24"
  assert (byte_quantity_le len (← (byte_quantity_sub s.len off))) "sail/primitives/bytes.sail:55.29-55.30"
  assert (byte_quantity_le s.off MAX_BYTE_QUANTITY) "sail/primitives/bytes.sail:56.37-56.38"
  assert (byte_quantity_le off (← (byte_quantity_sub MAX_BYTE_QUANTITY s.off))) "sail/primitives/bytes.sail:57.43-57.44"
  let absolute_off ← do (byte_quantity_add s.off off)
  (pure (byte_slice s.source absolute_off len))

/-- An absolute span in the append-only content-addressed code arena. -/
def code_byte_slice (off : source_pointer) (len : byte_length) : EvmByteSlice :=
  (byte_slice CodeSource off len)

def materialized_bytes (data : (List byte)) (len : byte_length) : MaterializedBytes :=
  { data := data,
    len := len }

def bytes_list (data : (List byte)) (len : byte_length) : Bytes :=
  (BytesList (materialized_bytes data len))

