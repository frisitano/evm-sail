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
open BalIterEntry

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

def num_of_ByteSource (arg_ : ByteSource) : Nat :=
  match arg_ with
  | .StatelessInputSource => 0
  | .EvmMemorySource => 1
  | .CodeSource => 2
  | .LogDataSource => 3
  | .OutputSource => 4
  | .ScratchSource => 5

/-- Returns the length carried by an existential byte slice. -/
/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, 0 ≤ s_dependentWitness0
  ∧ 0 ≤ s_dependentWitness1 -/
def byte_slice_length (s : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : Nat :=
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  s.len

def ADDRESS_BYTE_LENGTH : Nat := 20

def WORD_BYTE_LENGTH : Nat := 32

def EIGHT_BYTE_LENGTH : Nat := 8

def DOUBLE_WORD_BYTE_LENGTH : Nat := 64

/- Type quantifiers: off : Nat, len : Nat, (source_valid_range off len) -/
def byte_slice (src : ByteSource) (off : Nat) (len : Nat) : (EvmByteSliceFields off len) :=
  { source := src,
    off := off,
    len := len }

/-- The zero-length placeholder slice, for unused value roles (subtree
references, deletes). -/
def EMPTY_SLICE : (EvmByteSliceFields 0 0) := (byte_slice StatelessInputSource 0 0)

/- Type quantifiers: k_base : Nat, k_source_len : Nat, off : Nat, len : Nat, (source_valid_range k_base k_source_len)
  ∧ 0 ≤ off ∧ 0 ≤ len ∧ (off + len) ≤ k_source_len -/
def sub_slice (s : (EvmByteSliceFields k_base k_source_len)) (off : Nat) (len : Nat) : (EvmByteSliceFields (k_base + off) len) :=
  (byte_slice s.source (k_base + off) len)

/- Type quantifiers: k_base : Nat, k_source_len : Nat, off : Nat, (source_valid_range k_base k_source_len)
  ∧ 0 ≤ off ∧ off ≤ k_source_len -/
def slice_suffix (s : (EvmByteSliceFields k_base k_source_len)) (off : Nat) : (EvmByteSliceFields (k_base + off) (k_source_len - off)) :=
  (byte_slice s.source (k_base + off) (k_source_len - off))

/- Type quantifiers: len : Nat, (source_valid_length len) -/
def materialized_bytes (data : (List (BitVec 8))) (len : Nat) : MaterializedBytes :=
  { data := data,
    len := len }

def undefined_FixedBytes32 (_ : Unit) : SailM FixedBytes32 := do
  (pure { data := ← (undefined_vector 32 (← (undefined_bitvector 8))),
          len := ← (undefined_range 0 32) })

/- Type quantifiers: len : Nat, (source_valid_length len) -/
def bytes_list (data : (List (BitVec 8))) (len : Nat) : Bytes :=
  (BytesList (materialized_bytes data len))

/- Type quantifiers: k_ex413434_ : Nat, 0 ≤ k_ex413434_ ∧ k_ex413434_ ≤ 32 -/
def bytes_fixed32 (data : (Vector (BitVec 8) 32)) (len : Nat) : Bytes :=
  (BytesFixed32
    { data := data,
      len := len })

