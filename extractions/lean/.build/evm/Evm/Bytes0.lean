import Evm.Flow

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

def word_to_bytes32 (w : (BitVec 256)) : (List (BitVec 8)) := Id.run do
  let out : (List byte) := []
  let loop_k_lower := 0
  let loop_k_upper := 31
  let mut loop_vars := out
  for k in [loop_k_lower:loop_k_upper:1]i do
    let out := loop_vars
    loop_vars := ((Sail.BitVec.extractLsb (w >>> (8 *i k)) 7 0) :: out)
  (pure loop_vars)

def address_to_bytes (a : (BitVec 160)) : (List (BitVec 8)) := Id.run do
  let out : (List byte) := []
  let loop_k_lower := 0
  let loop_k_upper := 19
  let mut loop_vars := out
  for k in [loop_k_lower:loop_k_upper:1]i do
    let out := loop_vars
    loop_vars := ((Sail.BitVec.extractLsb (a >>> (8 *i k)) 7 0) :: out)
  (pure loop_vars)

def bytes_be256 (bytes : (List (BitVec 8))) : (BitVec 256) := Id.run do
  let acc : (BitVec 256) := (BitVec.zero 256)
  let rest : (List byte) := bytes
  let (acc, rest) ← (( do
    let loop_byte_index_lower := 0
    let loop_byte_index_upper := 31
    let mut loop_vars := (acc, rest)
    for byte_index in [loop_byte_index_lower:loop_byte_index_upper:1]i do
      let (acc, rest) := loop_vars
      loop_vars :=
        let (acc, rest) : ((BitVec 256) × (List (BitVec 8))) :=
          match rest with
          | (byte :: tail) =>
            (let acc : (BitVec 256) := ((acc <<< 8) ||| (Sail.BitVec.zeroExtend byte 256))
            let rest : (List byte) := tail
            (acc, rest))
          | [] => (acc, rest)
        (acc, rest)
    (pure loop_vars) ) : Id ((BitVec 256) × (List (BitVec 8))) )
  (pure acc)

