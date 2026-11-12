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
open ast
open TxType
open TxSignatureScheme
open TrieUpdateSource
open TrieUpdateRelation
open TrieLeafValue
open TrieItemValue
open TrieChange
open StorageTxPopResult
open StorageTxLookup
open StorageBlockIterResult
open StateJournalEntry
open StackValidation
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
open OpcodeOutcome
open NodeRef
open LogTopics
open LogData
open InputTrieNode
open IndexedTrieSource
open HtrRequestKind
open HaltKind
open FrameStatus
open FrameContinuation
open FatalError
open ExceptionKind
open EnvField
open DeepStackOperation
open CreateKind
open CalldataSlice
open CallKind
open BalIterEntry
open AcctTxPopResult
open AcctBlockIterResult

/-! # Typed region access

Every host-backed byte span carries only `{off, len}`. Its nominal Sail type
selects the host region and therefore the C operation; no source tag or
runtime resolver crosses the model boundary.

!!! note "Non-normative"
    These declarations are internal executable-specification contracts. -/

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, 0 ≤ off ∧
  off ≤ stateless_input_region_bound, 0 ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def stateless_input_slice_byte (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_syn_off k_len)))) (off : Nat) : SailM (BitVec 8) := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (stateless_input_byte_at ⟨_, ⟨_, s⟩⟩ off)
  else (pure 0x00#8)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, 0 ≤ off ∧
  off ≤ memory_region_bound, 0 ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def memory_slice_byte (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_syn_off k_len)))) (off : Nat) : SailM (BitVec 8) := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (memory_slice_byte_at ⟨_, ⟨_, s⟩⟩ off)
  else (pure 0x00#8)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, 0 ≤ off ∧
  off ≤ code_region_bound, 0 ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def code_slice_byte (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeRegionSliceFields k_syn_off k_len)))) (off : Nat) : SailM (BitVec 8) := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (code_region_byte_at ⟨_, ⟨_, s⟩⟩ off)
  else (pure 0x00#8)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, 0 ≤ off ∧
  off ≤ scratch_region_bound, 0 ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def scratch_byte (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_syn_off k_len)))) (off : Nat) : SailM (BitVec 8) := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (scratch_slice_byte_at ⟨_, ⟨_, s⟩⟩ off)
  else (pure 0x00#8)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, 0 ≤ off ∧
  off ≤ log_data_region_bound, 0 ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def log_data_byte (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (LogDataSliceFields k_syn_off k_len)))) (off : Nat) : SailM (BitVec 8) := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (log_data_slice_byte_at ⟨_, ⟨_, s⟩⟩ off)
  else (pure 0x00#8)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, 0 ≤ off ∧
  off ≤ output_region_bound, 0 ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def output_byte (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_syn_off k_len)))) (off : Nat) : SailM (BitVec 8) := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (output_slice_byte_at ⟨_, ⟨_, s⟩⟩ off)
  else (pure 0x00#8)

/- Type quantifiers: off : Nat, 0 ≤ off ∧ off ≤ default_host_region_bound -/
def calldata_slice_byte (s : CalldataSlice) (off : Nat) : SailM (BitVec 8) := do
  match s with
  | .InputCalldata ⟨_, ⟨_, bytes⟩⟩ =>
    (stateless_input_slice_byte ⟨_, ⟨_, bytes⟩⟩ off)
  | .MemoryCalldata ⟨_, ⟨_, bytes⟩⟩ => (memory_slice_byte ⟨_, ⟨_, bytes⟩⟩ off)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, 0 ≤ s_dependentWitness0
  ∧ 0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def slice_count_nonzero (s : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  (stateless_input_count_nonzero ⟨_, ⟨_, s⟩⟩)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, start : Nat, stride : Nat, width
  : Nat, count : Nat, (source_valid_length start) ∧
  (source_valid_length stride) ∧ (source_valid_length width) ∧ (source_valid_length count), 0
  ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def stateless_input_slice_strided_zero (s : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (start : Nat) (stride : Nat) (width : Nat) (count : Nat) : SailM Bool := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  (stateless_input_strided_zero ⟨_, ⟨_, s⟩⟩ start stride width count)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, start : Nat, stride : Nat, width
  : Nat, count : Nat, (source_valid_length start) ∧
  (source_valid_length stride) ∧ (source_valid_length width) ∧ (source_valid_length count), 0
  ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def memory_slice_strided_zero_value (s : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (start : Nat) (stride : Nat) (width : Nat) (count : Nat) : SailM Bool := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  (memory_slice_strided_zero ⟨_, ⟨_, s⟩⟩ start stride width count)

/- Type quantifiers: start : Nat, stride : Nat, width : Nat, count : Nat, (source_valid_length start)
  ∧ (source_valid_length stride) ∧ (source_valid_length width) ∧ (source_valid_length count) -/
def slice_strided_zero (s : CalldataSlice) (start : Nat) (stride : Nat) (width : Nat) (count : Nat) : SailM Bool := do
  match s with
  | .InputCalldata ⟨_, ⟨_, bytes⟩⟩ =>
    (stateless_input_slice_strided_zero ⟨_, ⟨_, bytes⟩⟩ start stride width count)
  | .MemoryCalldata ⟨_, ⟨_, bytes⟩⟩ =>
    (memory_slice_strided_zero_value ⟨_, ⟨_, bytes⟩⟩ start stride width count)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, (source_valid_length off), 0
  ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def stateless_input_slice_load (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_syn_off k_len)))) (off : Nat) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (stateless_input_load_word ⟨_, ⟨_, s⟩⟩ off)
  else (pure ZERO_WORD)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, (source_valid_length off), 0
  ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def memory_slice_load (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_syn_off k_len)))) (off : Nat) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (memory_slice_load_word ⟨_, ⟨_, s⟩⟩ off)
  else (pure ZERO_WORD)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, (source_valid_length off), 0
  ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def code_slice_load (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeRegionSliceFields k_syn_off k_len)))) (off : Nat) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (code_region_load_word ⟨_, ⟨_, s⟩⟩ off)
  else (pure ZERO_WORD)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, (source_valid_length off), 0
  ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def scratch_slice_load (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_syn_off k_len)))) (off : Nat) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (scratch_slice_load_word ⟨_, ⟨_, s⟩⟩ off)
  else (pure ZERO_WORD)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, (source_valid_length off), 0
  ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def log_data_slice_load (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (LogDataSliceFields k_syn_off k_len)))) (off : Nat) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (log_data_slice_load_word ⟨_, ⟨_, s⟩⟩ off)
  else (pure ZERO_WORD)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, (source_valid_length off), 0
  ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def output_slice_load (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_syn_off k_len)))) (off : Nat) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (output_slice_load_word ⟨_, ⟨_, s⟩⟩ off)
  else (pure ZERO_WORD)

/- Type quantifiers: off : Nat, (source_valid_length off) -/
def calldata_slice_load (s : CalldataSlice) (off : Nat) : SailM Nat := do
  match s with
  | .InputCalldata ⟨_, ⟨_, bytes⟩⟩ =>
    (stateless_input_slice_load ⟨_, ⟨_, bytes⟩⟩ off)
  | .MemoryCalldata ⟨_, ⟨_, bytes⟩⟩ => (memory_slice_load ⟨_, ⟨_, bytes⟩⟩ off)

/-- Loads the word at a 256-bit offset of a stateless-input span; offsets at
or past the slice end yield the zero word. -/
/- Type quantifiers: k_ex422316_ : Nat, s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, 0 ≤
  s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex422316_ ∧ k_ex422316_ ≤ (2 ^ 256 - 1) -/
def stateless_input_slice_load_word_offset (s : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (off : Nat) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (stateless_input_slice_load ⟨_, ⟨_, s⟩⟩ off)
  else (pure ZERO_WORD)

/-- Loads the word at a 256-bit offset of an EVM memory span; offsets at or
past the slice end yield the zero word. -/
/- Type quantifiers: k_ex422323_ : Nat, s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, 0 ≤
  s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex422323_ ∧ k_ex422323_ ≤ (2 ^ 256 - 1) -/
def memory_slice_load_word_offset (s : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (off : Nat) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (memory_slice_load ⟨_, ⟨_, s⟩⟩ off)
  else (pure ZERO_WORD)

/-- Loads the word at a 256-bit offset of a code-region span; offsets at or
past the slice end yield the zero word. -/
/- Type quantifiers: k_ex422330_ : Nat, s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, 0 ≤
  s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex422330_ ∧ k_ex422330_ ≤ (2 ^ 256 - 1) -/
def code_slice_load_word_offset (s : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeRegionSliceFields k_off k_len)))) (off : Nat) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (code_slice_load ⟨_, ⟨_, s⟩⟩ off)
  else (pure ZERO_WORD)

/-- Loads the word at a 256-bit offset of calldata from either provenance;
offsets at or past the slice end yield the zero word. -/
/- Type quantifiers: k_ex422331_ : Nat, 0 ≤ k_ex422331_ ∧ k_ex422331_ ≤ (2 ^ 256 - 1) -/
def calldata_slice_load_word_offset (s : CalldataSlice) (off : Nat) : SailM Nat := do
  match s with
  | .InputCalldata ⟨_, ⟨_, bytes⟩⟩ =>
    (stateless_input_slice_load_word_offset ⟨_, ⟨_, bytes⟩⟩ off)
  | .MemoryCalldata ⟨_, ⟨_, bytes⟩⟩ =>
    (memory_slice_load_word_offset ⟨_, ⟨_, bytes⟩⟩ off)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, n : Nat, 0 ≤
  off ∧ off ≤ stateless_input_region_bound ∧ 0 ≤ n ∧ n ≤ 32, 0 ≤ s_dependentWitness0
  ∧ 0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def stateless_input_slice_load_n (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_syn_off k_len)))) (off : Nat) (n : Nat) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (stateless_input_load_n_word ⟨_, ⟨_, s⟩⟩ off n)
  else (pure ZERO_WORD)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, n : Nat, 0 ≤
  off ∧ off ≤ code_region_bound ∧ 0 ≤ n ∧ n ≤ 32, 0 ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def code_slice_load_n (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeRegionSliceFields k_syn_off k_len)))) (off : Nat) (n : Nat) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (code_region_load_n_word ⟨_, ⟨_, s⟩⟩ off n)
  else (pure ZERO_WORD)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, off : Nat, n : Nat, 0 ≤
  off ∧ off ≤ scratch_region_bound ∧ 0 ≤ n ∧ n ≤ 32, 0 ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def scratch_slice_load_n (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_syn_off k_len)))) (off : Nat) (n : Nat) : SailM Nat := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (scratch_slice_load_n_word ⟨_, ⟨_, s⟩⟩ off n)
  else (pure ZERO_WORD)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, dst : Nat, off : Nat, len
  : Nat, 0 ≤ dst ∧
  dst ≤ memory_region_bound ∧
  0 ≤ off ∧ off ≤ stateless_input_region_bound ∧ 0 ≤ len ∧ len ≤ memory_region_bound, 0
  ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def stateless_input_slice_copy (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_syn_len : Nat) => (StatelessInputSliceFields k_syn_off k_syn_len)))) (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  (stateless_input_copy_to_memory ⟨_, ⟨_, s⟩⟩ dst off len)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, dst : Nat, off : Nat, len
  : Nat, 0 ≤ dst ∧
  dst ≤ memory_region_bound ∧
  0 ≤ off ∧ off ≤ memory_region_bound ∧ 0 ≤ len ∧ len ≤ memory_region_bound, 0 ≤
  s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def memory_slice_copy (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_syn_len : Nat) => (EvmMemorySliceFields k_syn_off k_syn_len)))) (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  (memory_slice_copy_to_memory ⟨_, ⟨_, s⟩⟩ dst off len)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, dst : Nat, off : Nat, len
  : Nat, 0 ≤ dst ∧
  dst ≤ memory_region_bound ∧
  0 ≤ off ∧ off ≤ code_region_bound ∧ 0 ≤ len ∧ len ≤ memory_region_bound, 0 ≤
  s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def code_slice_copy (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_syn_len : Nat) => (CodeRegionSliceFields k_syn_off k_syn_len)))) (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  (code_region_copy_to_memory ⟨_, ⟨_, s⟩⟩ dst off len)

/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, dst : Nat, off : Nat, len
  : Nat, 0 ≤ dst ∧
  dst ≤ memory_region_bound ∧
  0 ≤ off ∧ off ≤ output_region_bound ∧ 0 ≤ len ∧ len ≤ memory_region_bound, 0 ≤
  s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def output_slice_copy (s : (Sigma fun (k_syn_off : Nat) =>
  (Sigma fun (k_syn_len : Nat) => (OutputSliceFields k_syn_off k_syn_len)))) (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  (output_slice_copy_to_memory ⟨_, ⟨_, s⟩⟩ dst off len)

/-- Copies `len` bytes at a 256-bit offset of a stateless-input span into EVM
memory at `dst`; an offset at or past the slice end zero-fills the
destination. -/
/- Type quantifiers: k_ex422382_ : Nat, k_ex422381_ : Nat, k_ex422380_ : Nat, s_dependentWitness1 :
  Nat, s_dependentWitness0 : Nat, 0 ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex422380_ ∧ k_ex422380_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex422381_ ∧ k_ex422381_ ≤ (2 ^ 256 - 1), 0
  ≤ k_ex422382_ ∧ k_ex422382_ ≤ (2 ^ 32 - 1) -/
def stateless_input_slice_copy_word_offset (s : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (stateless_input_slice_copy ⟨_, ⟨_, s⟩⟩ dst off len)
  else (stateless_input_copy_to_memory ⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩ dst 0 len)

/-- Copies `len` bytes at a 256-bit offset of an EVM memory span into EVM
memory at `dst`; an offset at or past the slice end zero-fills the
destination. -/
/- Type quantifiers: k_ex422391_ : Nat, k_ex422390_ : Nat, k_ex422389_ : Nat, s_dependentWitness1 :
  Nat, s_dependentWitness0 : Nat, 0 ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex422389_ ∧ k_ex422389_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex422390_ ∧ k_ex422390_ ≤ (2 ^ 256 - 1), 0
  ≤ k_ex422391_ ∧ k_ex422391_ ≤ (2 ^ 32 - 1) -/
def memory_slice_copy_word_offset (s : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (memory_slice_copy ⟨_, ⟨_, s⟩⟩ dst off len)
  else (stateless_input_copy_to_memory ⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩ dst 0 len)

/-- Copies `len` bytes at a 256-bit offset of a code-region span into EVM
memory at `dst`; an offset at or past the slice end zero-fills the
destination. -/
/- Type quantifiers: k_ex422400_ : Nat, k_ex422399_ : Nat, k_ex422398_ : Nat, s_dependentWitness1 :
  Nat, s_dependentWitness0 : Nat, 0 ≤ s_dependentWitness0 ∧
  0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex422398_ ∧ k_ex422398_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex422399_ ∧ k_ex422399_ ≤ (2 ^ 256 - 1), 0
  ≤ k_ex422400_ ∧ k_ex422400_ ≤ (2 ^ 32 - 1) -/
def code_slice_copy_word_offset (s : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeRegionSliceFields k_off k_len)))) (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  if ((off <b s.len) : Bool)
  then (code_slice_copy ⟨_, ⟨_, s⟩⟩ dst off len)
  else (stateless_input_copy_to_memory ⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩ dst 0 len)

/-- Copies `len` bytes at a 256-bit offset of calldata from either provenance
into EVM memory at `dst`; an offset at or past the slice end zero-fills
the destination. -/
/- Type quantifiers: k_ex422403_ : Nat, k_ex422402_ : Nat, k_ex422401_ : Nat, 0 ≤ k_ex422401_ ∧
  k_ex422401_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex422402_ ∧ k_ex422402_ ≤ (2 ^ 256 - 1), 0 ≤
  k_ex422403_ ∧ k_ex422403_ ≤ (2 ^ 32 - 1) -/
def calldata_slice_copy_word_offset (s : CalldataSlice) (dst : Nat) (off : Nat) (len : Nat) : SailM Unit := do
  match s with
  | .InputCalldata ⟨_, ⟨_, bytes⟩⟩ =>
    (stateless_input_slice_copy_word_offset ⟨_, ⟨_, bytes⟩⟩ dst off len)
  | .MemoryCalldata ⟨_, ⟨_, bytes⟩⟩ =>
    (memory_slice_copy_word_offset ⟨_, ⟨_, bytes⟩⟩ dst off len)
