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

/-! # Byte-slice access

[EvmByteSlice][type-EvmByteSlice] is the common read-only view used by calldata
and executable frame code alike. Transaction input is already a
source-backed slice; no parallel input buffer or per-frame source
descriptor exists. Proof targets see the `val`s as bodyless axioms over
abstract byte lists.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules. -/

/-- The byte at slice offset `off`; zero past the end. -/
def slice_byte (s : EvmByteSlice) (off : source_pointer) : SailM byte := do
  if ((byte_quantity_lt off s.len) : Bool)
  then (host_slice_byte s off)
  else (pure 0x00#8)

/-- The number of nonzero bytes in the slice (EIP-2028 / EIP-7623
calldata gas). -/
def slice_count_nonzero (s : EvmByteSlice) : SailM byte_length := do
  (host_slice_count_nonzero s)

/-- Whether `count` fixed-width regions in a regular source layout contain
only zero bytes — a bulk slice predicate, not a format-specific
rule. -/
def slice_strided_zero (s : EvmByteSlice) (start : source_pointer) (stride : byte_length) (width : byte_length) (count : byte_length) : SailM Bool := do
  (host_slice_strided_zero s start stride width count)

/-- The 32-byte word at slice offset `off`, zero-padded past the end. -/
def slice_load (s : EvmByteSlice) (off : source_pointer) : SailM word := do
  if ((byte_quantity_lt off s.len) : Bool)
  then (host_slice_load_word s off)
  else (pure ZERO_WORD)

/-- The word at a transaction-controlled 256-bit source offset, returning zero
when the offset cannot designate a byte in the slice. -/
def slice_load_word_offset (s : EvmByteSlice) (off : word) : SailM word := do
  match (word_to_limb off) with
  | .some offset_bits =>
    (do
      let offset := (BitVec.toNatInt offset_bits)
      let .ByteQuantity slice_len := s.len
      if ((offset <b slice_len) : Bool)
      then (slice_load s (ByteQuantity offset))
      else (pure ZERO_WORD))
  | none => (pure ZERO_WORD)

/-- An `n`-byte big-endian word at slice offset `off`, right-aligned and
zero-padded past the end; used by `PUSH0`–`PUSH32`. -/
def slice_load_n (s : EvmByteSlice) (off : source_pointer) (n : byte_length) : SailM word := do
  if ((byte_quantity_lt off s.len) : Bool)
  then (host_slice_load_n_word s off n)
  else (pure ZERO_WORD)

/-- Copies slice range `[off, off+len)` into frame memory at `dst`,
zero-filling past the end. -/
def slice_copy (s : EvmByteSlice) (dst : memory_pointer) (off : source_pointer) (len : memory_length) : SailM Unit := do
  if ((bne len BYTE_ZERO) : Bool)
  then
    (do
      if ((byte_quantity_lt off s.len) : Bool)
      then (host_slice_copy_to_memory s dst off len)
      else (host_slice_copy_to_memory EMPTY_SLICE dst BYTE_ZERO len))
  else (pure ())

/-- Copies from a transaction-controlled 256-bit source offset into EVM
memory, applying the source operation's empty-read and zero-padding rules. -/
def slice_copy_word_offset (s : EvmByteSlice) (dst : memory_pointer) (off : word) (len : memory_length) : SailM Unit := do
  match (word_to_limb off) with
  | .some offset_bits =>
    (do
      let offset := (BitVec.toNatInt offset_bits)
      let .ByteQuantity slice_len := s.len
      if ((offset <b slice_len) : Bool)
      then (slice_copy s dst (ByteQuantity offset) len)
      else (slice_copy EMPTY_SLICE dst BYTE_ZERO len))
  | none => (slice_copy EMPTY_SLICE dst BYTE_ZERO len)

