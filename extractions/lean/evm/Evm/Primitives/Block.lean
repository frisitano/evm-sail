import Evm.Flow
import Evm.Vector
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

/-! # Block types

Block-level data structures. [BlockHeader][type-BlockHeader] is pure data;
its `k_header` register lives in the kernel environment alongside the
EIP-4895 withdrawal record. -/

def EMPTY_LOGS_BLOOM : LogsBloom := (vectorInit 0x00#8)

/-- Byte-wise bloom equality (the header `logs_bloom` check). -/
def logs_bloom_equal (a : (Vector (BitVec 8) 256)) (b : (Vector (BitVec 8) 256)) : Bool := Id.run do
  let equal : Bool := true
  let loop_i_lower := 0
  let loop_i_upper := 255
  let mut loop_vars := equal
  for i in [loop_i_lower:loop_i_upper:1]i do
    let equal := loop_vars
    loop_vars := (equal && ((GetElem?.getElem! a i) == (GetElem?.getElem! b i)))
  (pure loop_vars)

/-- The bloom as a most-significant-first byte list, for RLP encoding. -/
def logs_bloom_bytes (bloom : (Vector (BitVec 8) 256)) : (List (BitVec 8)) := Id.run do
  let out : (List (BitVec 8)) := []
  let loop_k_lower := 0
  let loop_k_upper := 255
  let mut loop_vars := out
  for k in [loop_k_lower:loop_k_upper:1]i do
    let out := loop_vars
    loop_vars := ((GetElem?.getElem! bloom k) :: out)
  (pure loop_vars)

/-- `keccak256(rlp([]))` — the ommers hash of every post-merge block
(EIP-3675 requires an empty ommers list). -/
def EMPTY_OMMER_HASH : hash :=
  (B256
    (to_bytes_le (n := 32) 0x1DCC4DE8DEC75D7AAB85B567B6CCD41AD312451B948A7413F0A142FD40D49347#256))

def undefined_Withdrawal (_ : Unit) : SailM Withdrawal := do
  (pure { index := ← (undefined_range 0 ((2 ^i 64) - 1)),
          validator_index := ← (undefined_range 0 ((2 ^i 64) - 1)),
          address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          amount := ← (undefined_range 0 ((2 ^i 64) - 1)) })

def EMPTY_EXECUTION_REQUESTS : ExecutionRequests :=
  { deposits := ⟨_, ⟨_, EMPTY_SLICE⟩⟩,
    withdrawals := ⟨_, ⟨_, EMPTY_SLICE⟩⟩,
    consolidations := ⟨_, ⟨_, EMPTY_SLICE⟩⟩,
    builder_deposits := ⟨_, ⟨_, EMPTY_SLICE⟩⟩,
    builder_exits := ⟨_, ⟨_, EMPTY_SLICE⟩⟩ }

