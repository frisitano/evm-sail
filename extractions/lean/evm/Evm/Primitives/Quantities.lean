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

/-! # Protocol quantities

Semantic numeric domains used outside the EVM's 256-bit word algebra.
Protocol fields retain their wire bounds, structural counters carry the
limits imposed by their data structures, and byte positions remain exact
non-negative quantities after operand validation.

## Types

The aliases below name the semantic role of protocol and structural
quantities while preserving their mathematical values. Where the protocol or
data structure supplies a bound, the alias records it explicitly. -/

/-- Converts a word to an account nonce when it fits the protocol field. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 256 - 1) -/
def word_to_account_nonce (value : Nat) : (Option Nat) :=
  if ((value ≤b ((2 ^i 64) - 1)) : Bool)
  then (some value)
  else none

/-- Embeds an EIP-2681 account nonce in the EVM word domain. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def word_of_account_nonce (value : Nat) : Nat :=
  value

/-- Embeds an EIP-4895 withdrawal amount in the EVM word domain. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def word_of_withdrawal_amount (value : Nat) : Nat :=
  value

/-- Embeds an EIP-7843 slot number in the EVM word domain. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def word_of_slot_number (value : Nat) : Nat :=
  value

/- Type quantifiers: value : Nat, 0 ≤ value -/
def word_of_block_number (value : Nat) : SailM Nat := do
  if ((value <b (2 ^i 256)) : Bool)
  then (pure value)
  else
    (do
      assert false "sail/primitives/quantities.sail:193.20-193.21"
      throw Error.Exit)

/- Type quantifiers: value : Nat, 0 ≤ value -/
def word_of_block_timestamp (value : Nat) : SailM Nat := do
  if ((value <b (2 ^i 256)) : Bool)
  then (pure value)
  else
    (do
      assert false "sail/primitives/quantities.sail:210.20-210.21"
      throw Error.Exit)

/-- Converts a chain identifier to the value exposed by CHAINID. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def word_of_chain_identifier (value : Nat) : Nat :=
  value

/-- Encodes a precompiled-contract identifier directly as its 20-byte
address.  Identifiers currently occupy only the two least-significant
bytes, including Osaka's `0x0100` P256VERIFY address. -/
/- Type quantifiers: value : Nat, 1 ≤ value ∧ value ≤ 256 -/
def precompile_id_to_address (value : Nat) : (Vector (BitVec 8) 20) :=
  let result : (Vector (BitVec 8) 20) := ZERO_ADDRESS
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 0 (get_slice_int 8 value 0))
  (vectorUpdate result 1 (get_slice_int 8 value 8))

/-- Increments an account nonce after establishing that it is not maximal. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def account_nonce_increment (value : Nat) : SailM Nat := do
  assert (value <b ((2 ^i 64) - 1)) "sail/primitives/quantities.sail:229.46-229.47"
  (pure (value + 1))

/-- Advances the call depth and rejects an attempt to exceed its bound. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 1024 -/
def frame_depth_increment (value : Nat) : SailM Nat := do
  if ((value <b 1024) : Bool)
  then (pure (value + 1))
  else
    (do
      assert false "sail/primitives/quantities.sail:238.20-238.21"
      throw Error.Exit)

/-- Consumes one validated EIP-4844 transaction blob hash. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 9 -/
def transaction_blob_count_decrement (value : Nat) : Nat :=
  if ((value != 0) : Bool)
  then (value - 1)
  else 0

/-- Descends by one level in a bounded Merkle tree. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def merkle_depth_increment (value : Nat) : SailM Nat := do
  assert (value <b 64) "sail/primitives/quantities.sail:252.21-252.22"
  (pure (value + 1))

/-- Ascends by one level in a bounded Merkle tree. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def merkle_depth_decrement (value : Nat) : SailM Nat := do
  assert (0 <b value) "sail/primitives/quantities.sail:258.20-258.21"
  (pure (value - 1))

/- Type quantifiers: off : Nat, len : Nat, (memory_valid_range off len) -/
def memory_range (off : Nat) (len : Nat) : (MemoryRangeFields off len) :=
  { off := off,
    len := len }

/-- The canonical inactive range used for a zero-sized operand or halt. -/
def EMPTY_MEMORY_RANGE : (MemoryRangeFields 0 0) := (memory_range 0 0)

/- Type quantifiers: value : Nat, 0 ≤ value -/
def word_of_nat_byte_count (value : Nat) : SailM Nat := do
  if ((value <b (2 ^i 256)) : Bool)
  then (pure (U256 value))
  else
    (do
      assert false "sail/primitives/quantities.sail:351.20-351.21"
      throw Error.Exit)

/- Type quantifiers: value : Nat, (source_valid_length value) -/
def word_of_source_byte_count (value : Nat) : SailM Nat := do
  (word_of_nat_byte_count value)

