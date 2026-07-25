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

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, (source_valid_length off), 0
  ≤ s_dependentWitness0 ∧ 0 ≤ s_dependentWitness1 -/
def slice_byte (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_syn_off k_len)))) (off : Nat) : SailM (BitVec 8) := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  let offset := off
  let length := s.len
  if ((offset <b length) : Bool)
  then (host_slice_byte ⟨_, ⟨_, s⟩⟩ off)
  else (pure 0x00#8)

/-- The number of nonzero bytes in the slice (EIP-2028 / EIP-7623
calldata gas). -/
/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, 0 ≤ s_dependentWitness0
  ∧ 0 ≤ s_dependentWitness1 -/
def slice_count_nonzero (s : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  (host_slice_count_nonzero ⟨_, ⟨_, s⟩⟩)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, start : Nat, stride : Nat, width
  : Nat, count : Nat, (source_valid_length start) ∧
  (source_valid_length stride) ∧ (source_valid_length width) ∧ (source_valid_length count), 0
  ≤ s_dependentWitness0 ∧ 0 ≤ s_dependentWitness1 -/
def slice_strided_zero (s : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (start : Nat) (stride : Nat) (width : Nat) (count : Nat) : SailM Bool := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  (host_slice_strided_zero ⟨_, ⟨_, s⟩⟩ start stride width count)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, (source_valid_length off), 0
  ≤ s_dependentWitness0 ∧ 0 ≤ s_dependentWitness1 -/
def slice_load (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_syn_off k_len)))) (off : Nat) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  let offset := off
  let length := s.len
  if ((offset <b length) : Bool)
  then (host_slice_load_word ⟨_, ⟨_, s⟩⟩ off)
  else (pure ZERO_WORD)

/-- The word at a transaction-controlled 256-bit source offset, returning zero
when the offset cannot designate a byte in the slice. -/
/- Type quantifiers: k_ex415376_ : Nat, s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, 0 ≤
  s_dependentWitness0 ∧ 0 ≤ s_dependentWitness1, 0 ≤ k_ex415376_ ∧
  k_ex415376_ ≤ (2 ^ 256 - 1) -/
def slice_load_word_offset (s : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (off : Nat) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  let slice_len := s.len
  if ((off <b slice_len) : Bool)
  then (slice_load ⟨_, ⟨_, s⟩⟩ off)
  else (pure ZERO_WORD)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, n : Nat, (source_valid_length off)
  ∧ (source_valid_length n), 0 ≤ s_dependentWitness0 ∧ 0 ≤ s_dependentWitness1 -/
def slice_load_n (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_syn_off k_len)))) (off : Nat) (n : Nat) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  let offset := off
  let length := s.len
  if ((offset <b length) : Bool)
  then (host_slice_load_n_word ⟨_, ⟨_, s⟩⟩ off n)
  else (pure ZERO_WORD)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, dst : Nat, off : Nat, len
  : Nat, (host_valid_access dst) ∧ (source_valid_length off) ∧ (host_valid_access len), 0 ≤
  s_dependentWitness0 ∧ 0 ≤ s_dependentWitness1 -/
def slice_copy (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_syn_off k_syn_len)))) (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
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
/- Type quantifiers: k_ex415422_ : Nat, k_ex415421_ : Nat, k_ex415420_ : Nat, s_dependentWitness1 :
  Nat, s_dependentWitness0 : Nat, 0 ≤ s_dependentWitness0 ∧ 0 ≤ s_dependentWitness1, 0 ≤
  k_ex415420_, 0 ≤ k_ex415421_ ∧ k_ex415421_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex415422_ -/
def slice_copy_word_offset (s : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  let slice_len := s.len
  if ((off <b slice_len) : Bool)
  then (slice_copy ⟨_, ⟨_, s⟩⟩ dst off len)
  else (slice_copy ⟨_, ⟨_, EMPTY_SLICE⟩⟩ dst 0 len)

