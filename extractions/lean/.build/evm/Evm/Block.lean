import Evm.Flow
import Evm.Prelude
import Evm.Bytes

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open ConcurrencyInterfaceV1

namespace Evm.Functions

open option
open gas_refund
open gas_cost
open gas_constant
open gas
open exception
open byte_quantity
open ast
open TxType
open TrieNode
open TrieItemValue
open TrieChange
open StatelessValidationResult
open Register
open NodeRef
open MerkleSlot
open JEntry
open HaltKind
open FrameStatus
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

def EMPTY_LOGS_BLOOM : LogsBloom := (vectorInit LIMB_ZERO)

def logs_bloom_equal (a : (Vector (BitVec 64) 32)) (b : (Vector (BitVec 64) 32)) : Bool := Id.run do
  let equal : Bool := true
  let loop_i_lower := 0
  let loop_i_upper := 31
  let mut loop_vars := equal
  for i in [loop_i_lower:loop_i_upper:1]i do
    let equal := loop_vars
    loop_vars := (equal && ((GetElem?.getElem! a i) == (GetElem?.getElem! b i)))
  (pure loop_vars)

def logs_bloom_bytes (bloom : (Vector (BitVec 64) 32)) : (List (BitVec 8)) := Id.run do
  let out : (List byte) := []
  let loop_k_lower := 0
  let loop_k_upper := 31
  let mut loop_vars := out
  for k in [loop_k_lower:loop_k_upper:1]i do
    let out := loop_vars
    loop_vars :=
      let limb := (GetElem?.getElem! bloom (31 -i k))
      let out : (List byte) := ((Sail.BitVec.extractLsb limb 7 0) :: out)
      let out : (List byte) := ((Sail.BitVec.extractLsb limb 15 8) :: out)
      let out : (List byte) := ((Sail.BitVec.extractLsb limb 23 16) :: out)
      let out : (List byte) := ((Sail.BitVec.extractLsb limb 31 24) :: out)
      let out : (List byte) := ((Sail.BitVec.extractLsb limb 39 32) :: out)
      let out : (List byte) := ((Sail.BitVec.extractLsb limb 47 40) :: out)
      let out : (List byte) := ((Sail.BitVec.extractLsb limb 55 48) :: out)
      ((Sail.BitVec.extractLsb limb 63 56) :: out)
  (pure loop_vars)

def EMPTY_OMMER_HASH : hash :=
  0x1DCC4DE8DEC75D7AAB85B567B6CCD41AD312451B948A7413F0A142FD40D49347#256

def undefined_Withdrawal (_ : Unit) : SailM Withdrawal := do
  (pure { index := ← (undefined_range 0 ((2 ^i 64) -i 1))
          validator_index := ← (undefined_range 0 ((2 ^i 64) -i 1))
          address := ← (undefined_bitvector 160)
          amount := ← (undefined_range 0 ((2 ^i 64) -i 1)) })

def EMPTY_EXECUTION_REQUESTS : ExecutionRequests :=
  { deposits := EMPTY_SLICE
    withdrawals := EMPTY_SLICE
    consolidations := EMPTY_SLICE }

