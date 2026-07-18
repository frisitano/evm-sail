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

def undefined_ByteSource (_ : Unit) : SailM ByteSource := do
  (internal_pick
    [StatelessInputSource, EvmMemorySource, CodeSource, LogDataSource, OutputSource, ScratchSource])

/-- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 5 -/
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

def byte_slice (src : ByteSource) (off : source_pointer) (len : byte_length) : EvmByteSlice :=
  { source := src,
    off := off,
    len := len }

def EMPTY_SLICE : EvmByteSlice :=
  { source := StatelessInputSource,
    off := BYTE_ZERO,
    len := BYTE_ZERO }

def stateless_input_byte_slice (off : source_pointer) (len : byte_length) : EvmByteSlice :=
  (byte_slice StatelessInputSource off len)

def sub_slice (s : EvmByteSlice) (off : source_pointer) (len : byte_length) : SailM EvmByteSlice := do
  assert (byte_quantity_le off s.len) "sail/primitives/bytes.sail:54.23-54.24"
  assert (byte_quantity_le len (← (byte_quantity_sub s.len off))) "sail/primitives/bytes.sail:55.29-55.30"
  assert (byte_quantity_le s.off MAX_BYTE_QUANTITY) "sail/primitives/bytes.sail:56.37-56.38"
  assert (byte_quantity_le off (← (byte_quantity_sub MAX_BYTE_QUANTITY s.off))) "sail/primitives/bytes.sail:57.43-57.44"
  let absolute_off ← do (byte_quantity_add s.off off)
  (pure (byte_slice s.source absolute_off len))

def code_byte_slice (off : source_pointer) (len : byte_length) : EvmByteSlice :=
  (byte_slice CodeSource off len)

def materialized_bytes (data : (List byte)) (len : byte_length) : MaterializedBytes :=
  { data := data,
    len := len }

def bytes_list (data : (List byte)) (len : byte_length) : Bytes :=
  (BytesList (materialized_bytes data len))

