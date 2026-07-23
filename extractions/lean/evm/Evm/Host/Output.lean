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

/-! # The output buffer

Host-owned storage for a frame or precompile output that must outlive the
mutable EVM memory from which it was produced. EVM returndata state and
frame transitions remain entirely in Sail.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules. -/

/- Type quantifiers: len : Nat, source_valid_length(len) -/
def output_buffer_slice (len : Nat) : EvmByteSlice :=
  if ((len == 0) : Bool)
  then
    ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_off k_syn_len)))) : (Sigma fun (k_off : Nat)
    => (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_off k_syn_len))))
  else
    ((⟨_, ⟨_, (byte_slice OutputSource 0 len)⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_off k_syn_len)))) : (Sigma fun (k_off : Nat)
    => (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_off k_syn_len))))

/-- Copies frame output into the host buffer so it survives frame
teardown; the canonical returndata source. -/
/- Type quantifiers: k_ex410427_ : Nat, k_ex410426_ : Nat, 0 ≤ k_ex410426_ ∧ 0 ≤ k_ex410427_ -/
def freeze_output (data : EvmByteSlice) : SailM EvmByteSlice := do
  let data := ((data).2).2
  let len := data.len
  if ((len == 0) : Bool)
  then
    (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  else
    (do
      if ((← (output_buffer_store ⟨_, ⟨_, data⟩⟩)) : Bool)
      then
        (pure ((⟨_, ⟨_, (((output_buffer_slice len)).2).2⟩⟩ : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
      else
        (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))))

/-- Stores one word as the output (32-byte precompile results). -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def output_buffer_word (value : word) : SailM EvmByteSlice := do
  let value := (value).value
  if ((← (output_buffer_store_word ⟨value⟩)) : Bool)
  then
    (pure ((⟨_, ⟨_, (((output_buffer_slice WORD_BYTE_LENGTH)).2).2⟩⟩ : (Sigma fun
      (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
      (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  else
    (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))

/-- Stores two words as the output (64-byte precompile results, e.g.
`ecrecover`-style pairs). -/
/- Type quantifiers: k_ex410430_ : Nat, k_ex410429_ : Nat, 0 ≤ k_ex410429_ ∧
  k_ex410429_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex410430_ ∧ k_ex410430_ ≤ (2 ^ 256 - 1) -/
def output_buffer_words (first : word) (second : word) : SailM EvmByteSlice := do
  let first := (first).value
  let second := (second).value
  if ((← (output_buffer_store_words ⟨first⟩ ⟨second⟩)) : Bool)
  then
    (pure ((⟨_, ⟨_, (((output_buffer_slice DOUBLE_WORD_BYTE_LENGTH)).2).2⟩⟩ : (Sigma fun
      (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
      (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  else
    (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))

