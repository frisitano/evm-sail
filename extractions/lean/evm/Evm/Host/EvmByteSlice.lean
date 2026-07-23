import Evm.Flow
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

/-! # Byte-slice access

[EvmByteSlice][type-EvmByteSlice] is the common read-only view used by calldata
and executable frame code alike. Transaction input is already a
source-backed slice; no parallel input buffer or per-frame source
descriptor exists. Proof targets see the `val`s as bodyless axioms over
abstract byte lists.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules. -/

/- Type quantifiers: k_ex409306_ : Nat, k_ex409305_ : Nat, off : Nat, source_valid_length(off), 0
  ≤ k_ex409305_ ∧ 0 ≤ k_ex409306_ -/
def slice_byte (s : EvmByteSlice) (off : Nat) : SailM byte := do
  let s := ((s).2).2
  let offset := off
  let length := s.len
  if ((offset <b length) : Bool)
  then (host_slice_byte ⟨_, ⟨_, s⟩⟩ off)
  else (pure 0x00#8)

/-- The number of nonzero bytes in the slice (EIP-2028 / EIP-7623
calldata gas). -/
/- Type quantifiers: k_ex409312_ : Nat, k_ex409311_ : Nat, 0 ≤ k_ex409311_ ∧ 0 ≤ k_ex409312_ -/
def slice_count_nonzero (s : EvmByteSlice) : SailM source_length := do
  let s := ((s).2).2
  (host_slice_count_nonzero ⟨_, ⟨_, s⟩⟩)

/- Type quantifiers: k_ex409328_ : Nat, k_ex409327_ : Nat, start : Nat, stride : Nat, width : Nat, count
  : Nat, source_valid_length(start) ∧
  source_valid_length(stride) ∧ source_valid_length(width) ∧ source_valid_length(count), 0 ≤
  k_ex409327_ ∧ 0 ≤ k_ex409328_ -/
def slice_strided_zero (s : EvmByteSlice) (start : Nat) (stride : Nat) (width : Nat) (count : Nat) : SailM Bool := do
  let s := ((s).2).2
  (host_slice_strided_zero ⟨_, ⟨_, s⟩⟩ start stride width count)

/- Type quantifiers: k_ex409343_ : Nat, k_ex409342_ : Nat, off : Nat, source_valid_length(off), 0
  ≤ k_ex409342_ ∧ 0 ≤ k_ex409343_ -/
def slice_load (s : EvmByteSlice) (off : Nat) : SailM word := do
  let s := ((s).2).2
  let publicResult ← do
    let offset := off
    let length := s.len
    if ((offset <b length) : Bool)
    then
      (do
          let publicResult ← (host_slice_load_word ⟨_, ⟨_, s⟩⟩ off)
          pure ((publicResult).value))
    else (pure (ZERO_WORD).value)
  pure (⟨publicResult⟩)

/-- The word at a transaction-controlled 256-bit source offset, returning zero
when the offset cannot designate a byte in the slice. -/
/- Type quantifiers: k_ex409350_ : Nat, k_ex409349_ : Nat, k_ex409348_ : Nat, 0 ≤ k_ex409348_ ∧
  0 ≤ k_ex409349_, 0 ≤ k_ex409350_ ∧ k_ex409350_ ≤ (2 ^ 256 - 1) -/
def slice_load_word_offset (s : EvmByteSlice) (off : word) : SailM word := do
  let s := ((s).2).2
  let off := (off).value
  let publicResult ← do
    let slice_len := s.len
    if ((off <b slice_len) : Bool)
    then
      (do
          let publicResult ← (slice_load ⟨_, ⟨_, s⟩⟩ off)
          pure ((publicResult).value))
    else (pure (ZERO_WORD).value)
  pure (⟨publicResult⟩)

/- Type quantifiers: k_ex409360_ : Nat, k_ex409359_ : Nat, off : Nat, n : Nat, source_valid_length(off)
  ∧ source_valid_length(n), 0 ≤ k_ex409359_ ∧ 0 ≤ k_ex409360_ -/
def slice_load_n (s : EvmByteSlice) (off : Nat) (n : Nat) : SailM word := do
  let s := ((s).2).2
  let publicResult ← do
    let offset := off
    let length := s.len
    if ((offset <b length) : Bool)
    then
      (do
          let publicResult ← (host_slice_load_n_word ⟨_, ⟨_, s⟩⟩ off n)
          pure ((publicResult).value))
    else (pure (ZERO_WORD).value)
  pure (⟨publicResult⟩)

/- Type quantifiers: k_ex409377_ : Nat, k_ex409376_ : Nat, dst : Nat, off : Nat, len : Nat, host_valid_access(dst)
  ∧ source_valid_length(off) ∧ host_valid_access(len), 0 ≤ k_ex409376_ ∧ 0 ≤ k_ex409377_ -/
def slice_copy (s : EvmByteSlice) (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  let s := ((s).2).2
  if ((len != 0) : Bool)
  then
    (do
      let offset := off
      let source_length := s.len
      if ((offset <b source_length) : Bool)
      then (host_slice_copy_to_memory ⟨_, ⟨_, s⟩⟩ dst off len)
      else (host_slice_copy_to_memory ⟨_, ⟨_, EMPTY_SLICE⟩⟩ dst 0 len))
  else (pure ())

/-- Copies from a transaction-controlled 256-bit source offset into EVM
memory, applying the source operation's empty-read and zero-padding rules. -/
/- Type quantifiers: k_ex409390_ : Nat, k_ex409389_ : Nat, k_ex409388_ : Nat, k_ex409387_ : Nat, k_ex409386_
  : Nat, 0 ≤ k_ex409386_ ∧ 0 ≤ k_ex409387_, 0 ≤ k_ex409388_, 0 ≤ k_ex409389_ ∧
  k_ex409389_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex409390_ -/
def slice_copy_word_offset (s : EvmByteSlice) (dst : memory_pointer) (off : word) (len : memory_length) : SailM Unit := do
  let s := ((s).2).2
  let off := (off).value
  let slice_len := s.len
  if ((off <b slice_len) : Bool)
  then (slice_copy ⟨_, ⟨_, s⟩⟩ dst off len)
  else (slice_copy ⟨_, ⟨_, EMPTY_SLICE⟩⟩ dst 0 len)

