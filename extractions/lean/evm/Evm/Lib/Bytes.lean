import Evm.Flow
import Evm.Prelude

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

/-! # Byte conversions

Big-endian conversions between words, addresses, and materialized byte
lists. -/

/-- The 32-byte big-endian encoding of a word. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_to_bytes32 (value : Nat) : (List (BitVec 8)) := Id.run do
  let remaining : Nat := value
  let out : (List (BitVec 8)) := []
  let (out, remaining) ← (( do
    let loop_byte_index_lower := 0
    let loop_byte_index_upper := 31
    let mut loop_vars := (out, remaining)
    for byte_index in [loop_byte_index_lower:loop_byte_index_upper:1]i do
      let (out, remaining) := loop_vars
      loop_vars :=
        let out : (List (BitVec 8)) := ((word_low_byte remaining) :: out)
        let remaining : Nat := (word_shift_right remaining 8)
        (out, remaining)
    (pure loop_vars) ) : Id ((List (BitVec 8)) × Nat) )
  (pure out)

/-- The 32-byte big-endian encoding of a hash. -/
def hash_to_bytes32 (bytes : (Vector (BitVec 8) 32)) : (List (BitVec 8)) := Id.run do
  let out : (List (BitVec 8)) := []
  let loop_k_lower := 0
  let loop_k_upper := 31
  let mut loop_vars := out
  for k in [loop_k_lower:loop_k_upper:1]i do
    let out := loop_vars
    loop_vars := ((GetElem?.getElem! bytes k) :: out)
  (pure loop_vars)

/-- The 20-byte big-endian encoding of an address. -/
def address_to_bytes (bytes : (Vector (BitVec 8) 20)) : (List (BitVec 8)) := Id.run do
  let out : (List (BitVec 8)) := []
  let loop_k_lower := 0
  let loop_k_upper := 19
  let mut loop_vars := out
  for k in [loop_k_lower:loop_k_upper:1]i do
    let out := loop_vars
    loop_vars := ((GetElem?.getElem! bytes k) :: out)
  (pure loop_vars)

