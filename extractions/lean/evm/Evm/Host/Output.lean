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

/- Type quantifiers: len : Nat, (source_valid_length len) -/
def output_buffer_slice (len : Nat) : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_syn_len : Nat) => (EvmByteSliceFields k_off k_syn_len))) :=
  if _sailIf0 : ((len == 0) : Bool) = true
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
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧ 0 ≤ data_dependentWitness1 -/
def freeze_output (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM (Sigma fun
  (data_dependentWitness0 : Nat) =>
  (Sigma fun (data_dependentWitness1 : Nat) =>
  (EvmByteSliceFields data_dependentWitness0 data_dependentWitness1))) := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  let len := data.len
  if _sailIf0 : ((len == 0) : Bool) = true
  then
    (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (data_dependentWitness0 : Nat) =>
      (Sigma fun (data_dependentWitness1 : Nat) =>
      (EvmByteSliceFields data_dependentWitness0 data_dependentWitness1)))) : (Sigma fun
      (data_dependentWitness0 : Nat) =>
      (Sigma fun (data_dependentWitness1 : Nat) =>
      (EvmByteSliceFields data_dependentWitness0 data_dependentWitness1)))))
  else
    (do
      if _sailIf1 : ((← (output_buffer_store ⟨_, ⟨_, data⟩⟩)) : Bool) = true
      then
        (pure ((output_buffer_slice len) : (Sigma fun (data_dependentWitness0 : Nat) =>
          (Sigma fun (data_dependentWitness1 : Nat) =>
          (EvmByteSliceFields data_dependentWitness0 data_dependentWitness1)))))
      else
        (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (data_dependentWitness0 : Nat) =>
          (Sigma fun (data_dependentWitness1 : Nat) =>
          (EvmByteSliceFields data_dependentWitness0 data_dependentWitness1)))) : (Sigma fun
          (data_dependentWitness0 : Nat) =>
          (Sigma fun (data_dependentWitness1 : Nat) =>
          (EvmByteSliceFields data_dependentWitness0 data_dependentWitness1))))))

/-- Stores one word as the output (32-byte precompile results). -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def output_buffer_word (value : Nat) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  if _sailIf0 : ((← (output_buffer_store_word value)) : Bool) = true
  then
    (pure ((output_buffer_slice WORD_BYTE_LENGTH) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  else
    (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))

/-- Stores two words as the output (64-byte precompile results, e.g.
`ecrecover`-style pairs). -/
/- Type quantifiers: k_ex416524_ : Nat, k_ex416523_ : Nat, 0 ≤ k_ex416523_ ∧
  k_ex416523_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex416524_ ∧ k_ex416524_ ≤ (2 ^ 256 - 1) -/
def output_buffer_words (first : Nat) (second : Nat) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  if _sailIf0 : ((← (output_buffer_store_words first second)) : Bool) = true
  then
    (pure ((output_buffer_slice DOUBLE_WORD_BYTE_LENGTH) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  else
    (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))

