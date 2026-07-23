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

/-! # Kernel scratch allocation

A bump cursor makes construction order and lifetime visible in the Sail
semantics while the bytes themselves remain in host-backed memory.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules. -/

/-- Marks the start of a scratch construction. -/
def scratch_begin (_ : Unit) : SailM source_pointer := do
  let ⟨_, ⟨_, arena⟩⟩ ← do pure ((← readReg scratch_arena))
  (pure arena.len)

/- Type quantifiers: len : Nat, source_valid_length(len) -/
def scratch_push_bytes (data : (List byte)) (len : Nat) : SailM Unit := do
  if ((len != 0) : Bool)
  then
    (do
      let ⟨_, ⟨_, arena⟩⟩ ← do pure ((← readReg scratch_arena))
      match (← (host_scratch_store_bytes arena.len data len)) with
      | .ByteRegionReady ⟨_, ⟨_, expanded⟩⟩ =>
        writeReg scratch_arena ⟨_, ⟨_, expanded⟩⟩
      | .ByteRegionFailed () => assert false "scratch byte append")
  else (pure ())

/-- Appends a source-backed slice at the cursor. -/
/- Type quantifiers: k_ex407226_ : Nat, k_ex407225_ : Nat, 0 ≤ k_ex407225_ ∧ 0 ≤ k_ex407226_ -/
def scratch_push_slice (data : EvmByteSlice) : SailM Unit := do
  let data := ((data).2).2
  if ((data.len != 0) : Bool)
  then
    (do
      let ⟨_, ⟨_, arena⟩⟩ ← do pure ((← readReg scratch_arena))
      match (← (host_scratch_store_slice arena.len data)) with
      | .ByteRegionReady ⟨_, ⟨_, expanded⟩⟩ =>
        writeReg scratch_arena ⟨_, ⟨_, expanded⟩⟩
      | .ByteRegionFailed () => assert false "scratch slice append")
  else (pure ())

/-- Appends a evm_prefix of a fixed 32-byte value at the cursor. -/
/- Type quantifiers: k_ex407227_ : Nat, 0 ≤ k_ex407227_ ∧ k_ex407227_ ≤ 32 -/
def scratch_push_b256 (data : b256) (len : Nat) : SailM Unit := do
  if ((len != 0) : Bool)
  then
    (do
      let ⟨_, ⟨_, arena⟩⟩ ← do pure ((← readReg scratch_arena))
      match (← (host_scratch_store_b256 arena.len data len)) with
      | .ByteRegionReady ⟨_, ⟨_, expanded⟩⟩ =>
        writeReg scratch_arena ⟨_, ⟨_, expanded⟩⟩
      | .ByteRegionFailed () => assert false "scratch fixed-byte append")
  else (pure ())

/-- The slice covering everything pushed since `start`. -/
/- Type quantifiers: start : Nat, 0 ≤ start -/
def scratch_finish (start : source_pointer) : SailM EvmByteSlice := do
  let start_offset := start
  let ⟨_, ⟨_, arena⟩⟩ ← do pure ((← readReg scratch_arena))
  let stop_offset := arena.len
  if ((start_offset ≤b stop_offset) : Bool)
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
def scratch_rewind (mark : source_pointer) : SailM Unit := do
  let mark_offset := mark
  let ⟨_, ⟨_, arena⟩⟩ ← do pure ((← readReg scratch_arena))
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

