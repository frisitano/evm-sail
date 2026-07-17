import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Bytes

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

def slice_byte (s : EvmByteSlice) (off : byte_quantity) : SailM (BitVec 8) := do
  if ((byte_quantity_lt off s.len) : Bool)
  then (host_slice_byte s off)
  else (pure 0x00#8)

def slice_count_nonzero (s : EvmByteSlice) : SailM byte_quantity := do
  (host_slice_count_nonzero s)

def slice_strided_zero (s : EvmByteSlice) (start : byte_quantity) (stride : byte_quantity) (width : byte_quantity) (count : byte_quantity) : SailM Bool := do
  (host_slice_strided_zero s start stride width count)

def slice_load (s : EvmByteSlice) (off : byte_quantity) : SailM (BitVec 256) := do
  if ((byte_quantity_lt off s.len) : Bool)
  then (host_slice_load_word s off)
  else (pure ZERO_WORD)

def slice_load_word_offset (s : EvmByteSlice) (off : (BitVec 256)) : SailM (BitVec 256) := do
  let offset := (BitVec.toNatInt off)
  let .ByteQuantity slice_len := s.len
  if ((offset <b slice_len) : Bool)
  then (slice_load s (ByteQuantity offset))
  else (pure ZERO_WORD)

def slice_load_n (s : EvmByteSlice) (off : byte_quantity) (n : byte_quantity) : SailM (BitVec 256) := do
  if ((byte_quantity_lt off s.len) : Bool)
  then (host_slice_load_n_word s off n)
  else (pure ZERO_WORD)

def slice_copy (s : EvmByteSlice) (dst : byte_quantity) (off : byte_quantity) (len : byte_quantity) : SailM Unit := do
  if ((byte_quantity_not_equal len BYTE_ZERO) : Bool)
  then
    (do
      if ((byte_quantity_lt off s.len) : Bool)
      then (host_slice_copy_to_memory s dst off len)
      else (host_slice_copy_to_memory EMPTY_SLICE dst BYTE_ZERO len))
  else (pure ())

def slice_copy_word_offset (s : EvmByteSlice) (dst : byte_quantity) (off : (BitVec 256)) (len : byte_quantity) : SailM Unit := do
  let offset := (BitVec.toNatInt off)
  let .ByteQuantity slice_len := s.len
  if ((offset <b slice_len) : Bool)
  then (slice_copy s dst (ByteQuantity offset) len)
  else (slice_copy EMPTY_SLICE dst BYTE_ZERO len)

