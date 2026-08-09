import Evm.Flow
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

/-! # Kernel scratch allocation

A bump cursor makes construction order and lifetime visible in the Sail
semantics while the bytes themselves remain in host-backed memory.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules. -/

/- Type quantifiers: left : Nat, right : Nat, (source_valid_length left) ∧
  (source_valid_length right) -/
def scratch_length_add (left : Nat) (right : Nat) : SailM Nat := do
  if ((right ≤b (((2 ^i 32) - 1) - left)) : Bool)
  then (pure (left + right))
  else
    (do
      assert false "scratch length overflow"
      throw Error.Exit)

/-- Marks the start of a scratch construction. -/
def scratch_begin (_ : Unit) : SailM Nat := do
  let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
  (pure arena.len)

/- Type quantifiers: len : Nat, (source_valid_length len) -/
def scratch_reserve (len : Nat) : SailM Nat := do
  let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
  let reserved ← do (host_scratch_reserve arena.len len)
  assert reserved "scratch reserve"
  (pure arena.len)

/-- Appends one byte without constructing a Sail list. -/
def scratch_push_byte (data : (BitVec 8)) : SailM Unit := do
  let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
  writeReg scratch_arena ⟨_, ⟨_, (((← (host_scratch_store_byte arena.len data))).2).2⟩⟩

/-- Appends a source-backed slice at the cursor. -/
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧
  0 ≤ data_dependentWitness1 ∧
  (data_dependentWitness0 + data_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def stateless_input_scratch_push_slice (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM Unit := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  if ((data.len != 0) : Bool)
  then
    (do
      let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
      writeReg scratch_arena ⟨_, ⟨_, (((← (host_scratch_store_stateless_input arena.len data))).2).2⟩⟩)
  else (pure ())

/-- Appends an existing arena-backed slice at the cursor. -/
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧
  0 ≤ data_dependentWitness1 ∧
  (data_dependentWitness0 + data_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def scratch_scratch_push_slice (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : SailM Unit := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  if ((data.len != 0) : Bool)
  then
    (do
      let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
      writeReg scratch_arena ⟨_, ⟨_, (((← (host_scratch_store_scratch arena.len data))).2).2⟩⟩)
  else (pure ())

/-- Appends a retained-log-data slice at the cursor. -/
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧
  0 ≤ data_dependentWitness1 ∧
  (data_dependentWitness0 + data_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def log_data_scratch_push_slice (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (LogDataSliceFields k_off k_len)))) : SailM Unit := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  if ((data.len != 0) : Bool)
  then
    (do
      let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
      writeReg scratch_arena ⟨_, ⟨_, (((← (host_scratch_store_log_data arena.len data))).2).2⟩⟩)
  else (pure ())

/-- Appends an output-buffer-backed slice at the cursor. -/
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧
  0 ≤ data_dependentWitness1 ∧
  (data_dependentWitness0 + data_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def output_scratch_push_slice (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : SailM Unit := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  if ((data.len != 0) : Bool)
  then
    (do
      let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
      writeReg scratch_arena ⟨_, ⟨_, (((← (host_scratch_store_output arena.len data))).2).2⟩⟩)
  else (pure ())

/-- Appends a fixed 20-byte address at the cursor. -/
def scratch_push_address (data : (Vector (BitVec 8) 20)) : SailM Unit := do
  let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
  writeReg scratch_arena ⟨_, ⟨_, (((← (host_scratch_store_address arena.len data))).2).2⟩⟩

/-- Appends a evm_prefix of a fixed 32-byte value at the cursor. -/
/- Type quantifiers: k_ex548775_ : Nat, 0 ≤ k_ex548775_ ∧ k_ex548775_ ≤ 32 -/
def scratch_push_b256 (data : (Vector (BitVec 8) 32)) (len : Nat) : SailM Unit := do
  if ((len != 0) : Bool)
  then
    (do
      let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
      writeReg scratch_arena ⟨_, ⟨_, (((← (host_scratch_store_b256 arena.len data len))).2).2⟩⟩)
  else (pure ())

/-- Appends a decreasing fixed 256-byte vector in canonical wire order. -/
def scratch_push_fixed_bytes_256 (data : (Vector (BitVec 8) 256)) : SailM Unit := do
  let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
  writeReg scratch_arena ⟨_, ⟨_, (((← (host_scratch_store_fixed_bytes_256 arena.len data))).2).2⟩⟩

/-- Appends the low `len` bytes of a word in canonical big-endian order. -/
/- Type quantifiers: k_ex548777_ : Nat, k_ex548776_ : Nat, 0 ≤ k_ex548776_ ∧
  k_ex548776_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex548777_ ∧ k_ex548777_ ≤ 32 -/
def scratch_push_word_be (data : Nat) (len : Nat) : SailM Unit := do
  if ((len != 0) : Bool)
  then
    (do
      let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
      writeReg scratch_arena ⟨_, ⟨_, (((← (host_scratch_store_word arena.len data len))).2).2⟩⟩)
  else (pure ())

/-- The slice covering everything pushed since `start`. -/
/- Type quantifiers: start : Nat, 0 ≤ start ∧ start ≤ (2 ^ 32 - 1) -/
def scratch_finish (start : Nat) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))) := do
  let start_offset := start
  let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
  let stop_offset := arena.len
  if _sailIf0 : ((start_offset ≤b stop_offset) : Bool) = true
  then
    (pure ((⟨_, ⟨_, (scratch_sub_slice arena start (stop_offset - start_offset))⟩⟩ : (Sigma
      fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : (Sigma
      fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))))
  else
    (do
      assert false "scratch finish mark"
      throw Error.Exit)

/-- Discards everything pushed since `mark`. -/
/- Type quantifiers: mark : Nat, 0 ≤ mark ∧ mark ≤ (2 ^ 32 - 1) -/
def scratch_rewind (mark : Nat) : SailM Unit := do
  let mark_offset := mark
  let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
  let cursor_offset := arena.len
  if ((mark_offset ≤b cursor_offset) : Bool)
  then
    (do
      writeReg scratch_arena ⟨_, ⟨_, (scratch_sub_slice arena 0 mark)⟩⟩
      (host_scratch_truncate mark))
  else assert false "scratch rewind mark"

/-- Empties the arena (per-block lifetime). -/
def scratch_reset (_ : Unit) : SailM Unit := do
  writeReg scratch_arena ⟨_, ⟨_, EMPTY_SCRATCH_SLICE⟩⟩
  (host_scratch_truncate 0)

