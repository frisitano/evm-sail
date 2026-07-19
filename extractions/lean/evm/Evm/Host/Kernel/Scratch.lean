import Evm.Flow
import Evm.Arith
import Evm.Primitives.Quantities
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

open word
open option
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open b256
open ast
open address
open TxType
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open StateCheckpoint
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
open BlockError

/-! # Kernel scratch allocation

A bump cursor makes construction order and lifetime visible in the Sail
semantics while the bytes themselves remain in host-backed memory.

!!! note "Non-normative"
    This page documents the model's host interface — internal contracts
    of the executable specification, not protocol rules. -/

/-- Marks the start of a scratch construction. -/
def scratch_begin (_ : Unit) : SailM source_pointer := do
  readReg scratch_cursor

/-- Advances the scratch bump cursor by `width` bytes within the byte domain. -/
def scratch_advance (width : byte_length) : SailM Unit := do
  let cursor ← (( do readReg scratch_cursor ) : SailM byte_quantity )
  if ((byte_quantity_le cursor MAX_BYTE_QUANTITY) : Bool)
  then
    (do
      if ((byte_quantity_le width (← (byte_quantity_sub MAX_BYTE_QUANTITY cursor))) : Bool)
      then writeReg scratch_cursor (← (byte_quantity_add cursor width))
      else assert false "scratch cursor overflow")
  else assert false "scratch cursor overflow"

/-- Appends materialized bytes at the cursor. -/
def scratch_push_bytes (data : (List byte)) (len : byte_length) : SailM Unit := do
  if ((bne len BYTE_ZERO) : Bool)
  then
    (do
      assert (← (host_scratch_store_bytes (← readReg scratch_cursor) data len)) "scratch byte append"
      (scratch_advance len))
  else (pure ())

/-- Appends a source-backed slice at the cursor. -/
def scratch_push_slice (data : EvmByteSlice) : SailM Unit := do
  if ((bne data.len BYTE_ZERO) : Bool)
  then
    (do
      assert (← (host_scratch_store_slice (← readReg scratch_cursor) data)) "scratch slice append"
      (scratch_advance data.len))
  else (pure ())

/-- The slice covering everything pushed since `start`. -/
def scratch_finish (start : source_pointer) : SailM EvmByteSlice := do
  let stop ← (( do readReg scratch_cursor ) : SailM byte_quantity )
  if ((byte_quantity_le start stop) : Bool)
  then
    (do
      let len ← (( do (byte_quantity_sub stop start) ) : SailM byte_quantity )
      (pure (byte_slice ScratchSource start len)))
  else
    (do
      assert false "scratch finish mark"
      throw Error.Exit)

/-- Discards everything pushed since `mark`. -/
def scratch_rewind (mark : source_pointer) : SailM Unit := do
  let cursor ← (( do readReg scratch_cursor ) : SailM byte_quantity )
  if ((byte_quantity_le mark cursor) : Bool)
  then
    (do
      writeReg scratch_cursor mark
      (host_scratch_truncate (← readReg scratch_cursor)))
  else assert false "scratch rewind mark"

/-- Empties the arena (per-block lifetime). -/
def scratch_reset (_ : Unit) : SailM Unit := do
  writeReg scratch_cursor BYTE_ZERO
  (host_scratch_truncate (← readReg scratch_cursor))

