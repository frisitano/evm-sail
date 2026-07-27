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

/-! # Kernel scratch allocation

A bump cursor makes construction order and lifetime visible in the Sail
semantics while the bytes themselves remain in host-backed memory.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules. -/

/-- Marks the start of a scratch construction. -/
def scratch_begin (_ : Unit) : SailM Nat := do
  let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
  (pure arena.len)

/- Type quantifiers: len : Nat, (source_valid_length len) -/
def scratch_push_bytes (data : (List (BitVec 8))) (len : Nat) : SailM Unit := do
  if ((len != 0) : Bool)
  then
    (do
      let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
      match (← (host_scratch_store_bytes arena.len data len)) with
      | .ByteRegionReady ⟨_, ⟨_, expanded⟩⟩ =>
        writeReg scratch_arena ⟨_, ⟨_, expanded⟩⟩
      | .ByteRegionFailed () => assert false "scratch byte append")
  else (pure ())

/-- Appends a source-backed slice at the cursor. -/
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧ 0 ≤ data_dependentWitness1 -/
def scratch_push_slice (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM Unit := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  if ((data.len != 0) : Bool)
  then
    (do
      let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
      match (← (host_scratch_store_slice arena.len data)) with
      | .ByteRegionReady ⟨_, ⟨_, expanded⟩⟩ =>
        writeReg scratch_arena ⟨_, ⟨_, expanded⟩⟩
      | .ByteRegionFailed () => assert false "scratch slice append")
  else (pure ())

/-- Appends a evm_prefix of a fixed 32-byte value at the cursor. -/
/- Type quantifiers: k_ex414004_ : Nat, 0 ≤ k_ex414004_ ∧ k_ex414004_ ≤ 32 -/
def scratch_push_b256 (data : (Vector (BitVec 8) 32)) (len : Nat) : SailM Unit := do
  if ((len != 0) : Bool)
  then
    (do
      let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
      match (← (host_scratch_store_b256 arena.len data len)) with
      | .ByteRegionReady ⟨_, ⟨_, expanded⟩⟩ =>
        writeReg scratch_arena ⟨_, ⟨_, expanded⟩⟩
      | .ByteRegionFailed () => assert false "scratch fixed-byte append")
  else (pure ())

/-- The slice covering everything pushed since `start`. -/
/- Type quantifiers: start : Nat, 0 ≤ start -/
def scratch_finish (start : Nat) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  let start_offset := start
  let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
  let stop_offset := arena.len
  if _sailIf0 : ((start_offset ≤b stop_offset) : Bool) = true
  then
    (pure ((⟨_, ⟨_, (sub_slice arena start (stop_offset - start_offset))⟩⟩ : (Sigma fun
      (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
      (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  else
    (do
      assert false "scratch finish mark"
      throw Error.Exit)

/-- Discards everything pushed since `mark`. -/
/- Type quantifiers: mark : Nat, 0 ≤ mark -/
def scratch_rewind (mark : Nat) : SailM Unit := do
  let mark_offset := mark
  let ⟨_, ⟨_, arena⟩⟩ ← do readReg scratch_arena
  let cursor_offset := arena.len
  if ((mark_offset ≤b cursor_offset) : Bool)
  then
    (do
      writeReg scratch_arena ⟨_, ⟨_, (sub_slice arena 0 mark)⟩⟩
      (host_scratch_truncate mark))
  else assert false "scratch rewind mark"

/-- Empties the arena (per-block lifetime). -/
def scratch_reset (_ : Unit) : SailM Unit := do
  writeReg scratch_arena ⟨_, ⟨_, (byte_slice ScratchSource 0 0)⟩⟩
  (host_scratch_truncate 0)

