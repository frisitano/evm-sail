import Sail
import Evm.Defs
import Evm.Specialization
import Evm.FakeReal
import Evm.HostAxioms

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
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
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

/-! # Typed byte regions

Unmaterialized byte sequences. The specification's bulk data lives in named
host regions. A slice's nominal type identifies its region; the semantic value
contains only a source coordinate and a length. There is deliberately no
generic slice and no runtime source discriminant.

The standard C ABI lowers the coordinate to an offset in the named region. The
fixed-capacity optimized C ABI lowers the same coordinate to a validated
absolute pointer, after allocating every mutable region before Sail execution.
This representation choice is not part of the protocol semantics: subslicing
is coordinate addition in both builds, and the nominal type retains provenance.

## Constants

The fixed lengths identify addresses, words, limbs, and double words. -/

/-- The byte length of a calldata slice, independent of its provenance. -/
def calldata_slice_length (s : CalldataSlice) : Nat :=
  match s with
  | .InputCalldata ⟨_, ⟨_, bytes⟩⟩ => bytes.len
  | .MemoryCalldata ⟨_, ⟨_, bytes⟩⟩ => bytes.len

/-- The byte length of a stateless-input slice. -/
/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, 0 ≤ s_dependentWitness0
  ∧ 0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def stateless_input_slice_length (s : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : Nat :=
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  s.len

def ADDRESS_BYTE_LENGTH : Nat := 20

def WORD_BYTE_LENGTH : Nat := 32

def EIGHT_BYTE_LENGTH : Nat := 8

def DOUBLE_WORD_BYTE_LENGTH : Nat := 64

/- Type quantifiers: off : Nat, len : Nat, (stateless_input_valid_range off len) -/
def stateless_input_slice (off : Nat) (len : Nat) : (StatelessInputSliceFields off len) :=
  { bytes := off,
    len := len }

/- Type quantifiers: off : Nat, len : Nat, (scratch_valid_range off len) -/
def scratch_slice (off : Nat) (len : Nat) : (ScratchSliceFields off len) :=
  { bytes := off,
    len := len }

/- Type quantifiers: off : Nat, len : Nat, (memory_region_valid_range off len) -/
def evm_memory_slice (off : Nat) (len : Nat) : (EvmMemorySliceFields off len) :=
  { bytes := off,
    len := len }

/- Type quantifiers: off : Nat, len : Nat, (code_region_valid_range off len) -/
def code_region_slice (off : Nat) (len : Nat) : (CodeRegionSliceFields off len) :=
  { bytes := off,
    len := len }

/- Type quantifiers: off : Nat, len : Nat, (log_data_valid_range off len) -/
def log_data_slice (off : Nat) (len : Nat) : (LogDataSliceFields off len) :=
  { bytes := off,
    len := len }

/- Type quantifiers: off : Nat, len : Nat, (output_region_valid_range off len) -/
def output_slice (off : Nat) (len : Nat) : (OutputSliceFields off len) :=
  { bytes := off,
    len := len }

def EMPTY_STATELESS_INPUT_SLICE : (StatelessInputSliceFields 0 0) := (stateless_input_slice 0 0)

def EMPTY_SCRATCH_SLICE : (ScratchSliceFields 0 0) := (scratch_slice 0 0)

def EMPTY_EVM_MEMORY_SLICE : (EvmMemorySliceFields 0 0) := (evm_memory_slice 0 0)

def EMPTY_CODE_REGION_SLICE : (CodeRegionSliceFields 0 0) := (code_region_slice 0 0)

def EMPTY_LOG_DATA_SLICE : (LogDataSliceFields 0 0) := (log_data_slice 0 0)

def EMPTY_OUTPUT_SLICE : (OutputSliceFields 0 0) := (output_slice 0 0)

def EMPTY_CALLDATA : CalldataSlice := (InputCalldata ⟨_, ⟨_, EMPTY_STATELESS_INPUT_SLICE⟩⟩)

/- Type quantifiers: k_base : Nat, k_source_len : Nat, off : Nat, len : Nat, (stateless_input_valid_range k_base k_source_len)
  ∧ 0 ≤ off ∧ 0 ≤ len ∧ (off + len) ≤ k_source_len -/
def stateless_input_sub_slice (s : (StatelessInputSliceFields k_base k_source_len)) (off : Nat) (len : Nat) : (StatelessInputSliceFields (k_base + off) len) :=
  { bytes := (k_base + off),
    len := len }

/- Type quantifiers: k_base : Nat, k_source_len : Nat, off : Nat, len : Nat, (scratch_valid_range k_base k_source_len)
  ∧ 0 ≤ off ∧ 0 ≤ len ∧ (off + len) ≤ k_source_len -/
def scratch_sub_slice (s : (ScratchSliceFields k_base k_source_len)) (off : Nat) (len : Nat) : (ScratchSliceFields (k_base + off) len) :=
  { bytes := (k_base + off),
    len := len }

/- Type quantifiers: k_base : Nat, k_source_len : Nat, off : Nat, len : Nat, (memory_region_valid_range k_base k_source_len)
  ∧ 0 ≤ off ∧ 0 ≤ len ∧ (off + len) ≤ k_source_len -/
def memory_sub_slice (s : (EvmMemorySliceFields k_base k_source_len)) (off : Nat) (len : Nat) : (EvmMemorySliceFields (k_base + off) len) :=
  { bytes := (k_base + off),
    len := len }

/- Type quantifiers: k_base : Nat, k_source_len : Nat, off : Nat, len : Nat, (log_data_valid_range k_base k_source_len)
  ∧ 0 ≤ off ∧ 0 ≤ len ∧ (off + len) ≤ k_source_len -/
def log_data_sub_slice (s : (LogDataSliceFields k_base k_source_len)) (off : Nat) (len : Nat) : (LogDataSliceFields (k_base + off) len) :=
  { bytes := (k_base + off),
    len := len }

/- Type quantifiers: off : Nat, len : Nat, (source_valid_length off) ∧ (source_valid_length len) -/
def calldata_sub_slice (s : CalldataSlice) (off : Nat) (len : Nat) : SailM CalldataSlice := do
  match s with
  | .InputCalldata ⟨_, ⟨_, bytes⟩⟩ =>
    (do
      if (((off + len) ≤b bytes.len) : Bool)
      then
        (let subslice := (stateless_input_sub_slice bytes off len)
        (pure (InputCalldata ⟨_, ⟨_, subslice⟩⟩)))
      else
        (do
          assert false "calldata sub-slice bounds"
          throw Error.Exit))
  | .MemoryCalldata ⟨_, ⟨_, bytes⟩⟩ =>
    (do
      if (((off + len) ≤b bytes.len) : Bool)
      then
        (let subslice := (memory_sub_slice bytes off len)
        (pure (MemoryCalldata ⟨_, ⟨_, subslice⟩⟩)))
      else
        (do
          assert false "calldata sub-slice bounds"
          throw Error.Exit))

/- Type quantifiers: k_base : Nat, k_source_len : Nat, off : Nat, (stateless_input_valid_range k_base k_source_len)
  ∧ 0 ≤ off ∧ off ≤ k_source_len -/
def stateless_input_slice_suffix (s : (StatelessInputSliceFields k_base k_source_len)) (off : Nat) : (StatelessInputSliceFields (k_base + off) (k_source_len - off)) :=
  { bytes := (k_base + off),
    len := (k_source_len - off) }

/- Type quantifiers: k_base : Nat, k_source_len : Nat, off : Nat, (scratch_valid_range k_base k_source_len)
  ∧ 0 ≤ off ∧ off ≤ k_source_len -/
def scratch_slice_suffix (s : (ScratchSliceFields k_base k_source_len)) (off : Nat) : (ScratchSliceFields (k_base + off) (k_source_len - off)) :=
  { bytes := (k_base + off),
    len := (k_source_len - off) }

