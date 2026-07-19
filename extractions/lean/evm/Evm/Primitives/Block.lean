import Evm.Flow
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

/-! # Block types

Block-level data structures. [BlockHeader][type-BlockHeader] is pure data;
its `k_header` register lives in the kernel environment alongside the
EIP-4895 withdrawal record. -/

def EMPTY_LOGS_BLOOM : LogsBloom := (vectorInit LIMB_ZERO)

/-- Byte-wise bloom equality (the header `logs_bloom` check). -/
def logs_bloom_equal (a : LogsBloom) (b : LogsBloom) : Bool := Id.run do
  let equal : Bool := true
  let loop_i_lower := 0
  let loop_i_upper := 31
  let mut loop_vars := equal
  for i in [loop_i_lower:loop_i_upper:1]i do
    let equal := loop_vars
    loop_vars := (equal && ((GetElem?.getElem! a i) == (GetElem?.getElem! b i)))
  (pure loop_vars)

/-- The bloom as a most-significant-first byte list, for RLP encoding. -/
def logs_bloom_bytes (bloom : LogsBloom) : (List byte) := Id.run do
  let out : (List (BitVec 8)) := []
  let loop_k_lower := 0
  let loop_k_upper := 31
  let mut loop_vars := out
  for k in [loop_k_lower:loop_k_upper:1]i do
    let out := loop_vars
    loop_vars :=
      let limb := (GetElem?.getElem! bloom (31 -i k))
      let out : (List (BitVec 8)) := ((Sail.BitVec.extractLsb limb 7 0) :: out)
      let out : (List (BitVec 8)) := ((Sail.BitVec.extractLsb limb 15 8) :: out)
      let out : (List (BitVec 8)) := ((Sail.BitVec.extractLsb limb 23 16) :: out)
      let out : (List (BitVec 8)) := ((Sail.BitVec.extractLsb limb 31 24) :: out)
      let out : (List (BitVec 8)) := ((Sail.BitVec.extractLsb limb 39 32) :: out)
      let out : (List (BitVec 8)) := ((Sail.BitVec.extractLsb limb 47 40) :: out)
      let out : (List (BitVec 8)) := ((Sail.BitVec.extractLsb limb 55 48) :: out)
      ((Sail.BitVec.extractLsb limb 63 56) :: out)
  (pure loop_vars)

/-- `keccak256(rlp([]))` — the ommers hash of every post-merge block
(EIP-3675 requires an empty ommers list). -/
def EMPTY_OMMER_HASH : hash :=
  (word_to_hash (U256 0x1DCC4DE8DEC75D7AAB85B567B6CCD41AD312451B948A7413F0A142FD40D49347#256))

def EMPTY_EXECUTION_REQUESTS : ExecutionRequests :=
  { deposits := EMPTY_SLICE,
    withdrawals := EMPTY_SLICE,
    consolidations := EMPTY_SLICE }

