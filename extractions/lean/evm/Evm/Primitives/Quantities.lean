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
def word_to_account_nonce (value : word) : (Option account_nonce) :=
  let value := (value).value
  (Option.map (fun semanticValue => ⟨semanticValue⟩) (if ((value ≤b ((2 ^i 64) - 1)) : Bool)
  then (some value)
  else none))

/-- Embeds an EIP-2681 account nonce in the EVM word domain. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def word_of_account_nonce (value : account_nonce) : account_nonce :=
  let value := (value).value
  ⟨value⟩

/-- Embeds an EIP-4895 withdrawal amount in the EVM word domain. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def word_of_withdrawal_amount (value : withdrawal_amount) : withdrawal_amount :=
  let value := (value).value
  ⟨value⟩

/-- Embeds an EIP-7843 slot number in the EVM word domain. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def word_of_slot_number (value : slot_number) : slot_number :=
  let value := (value).value
  ⟨value⟩

/- Type quantifiers: value : Nat, 0 ≤ value -/
def word_of_block_number (value : Nat) : SailM Nat := do
  if ((value <b (2 ^i 256)) : Bool)
  then (pure value)
  else
    (do
      assert false "sail/primitives/quantities.sail:191.20-191.21"
      throw Error.Exit)

/- Type quantifiers: value : Nat, 0 ≤ value -/
def word_of_block_timestamp (value : Nat) : SailM Nat := do
  if ((value <b (2 ^i 256)) : Bool)
  then (pure value)
  else
    (do
      assert false "sail/primitives/quantities.sail:208.20-208.21"
      throw Error.Exit)

/- Type quantifiers: value : Nat, 0 ≤ value -/
def word_of_chain_identifier (value : Nat) : SailM Nat := do
  if ((value <b (2 ^i 256)) : Bool)
  then (pure value)
  else
    (do
      assert false "sail/primitives/quantities.sail:225.20-225.21"
      throw Error.Exit)

/-- Encodes a precompiled-contract identifier directly as its 20-byte
address.  Identifiers currently occupy only the two least-significant
bytes, including Osaka's `0x0100` P256VERIFY address. -/
/- Type quantifiers: value : Nat, 1 ≤ value ∧ value ≤ 256 -/
def precompile_id_to_address (value : precompile_id) : address :=
  let value := (value).value
  let result : (Vector (BitVec 8) 20) := ZERO_ADDRESS
  let result : (Vector (BitVec 8) 20) := (vectorUpdate result 0 (get_slice_int 8 value 0))
  (vectorUpdate result 1 (get_slice_int 8 value 8))

/-- Increments an account nonce after establishing that it is not maximal. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def account_nonce_increment (value : account_nonce) : SailM account_nonce := do
  let value := (value).value
  let publicResult ← do
    assert (value <b ((2 ^i 64) - 1)) "sail/primitives/quantities.sail:241.46-241.47"
    (pure (value + 1))
  pure (⟨publicResult⟩)

/-- Advances the call depth and rejects an attempt to exceed its bound. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 1024 -/
def frame_depth_increment (value : frame_depth) : SailM frame_depth := do
  let value := (value).value
  let publicResult ← do
    if ((value <b 1024) : Bool)
    then (pure (value + 1))
    else
      (do
        assert false "sail/primitives/quantities.sail:250.20-250.21"
        throw Error.Exit)
  pure (⟨publicResult⟩)

/-- Consumes one validated EIP-4844 transaction blob hash. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 9 -/
def transaction_blob_count_decrement (value : transaction_blob_count) : transaction_blob_count :=
  let value := (value).value
  ⟨if ((value != 0) : Bool)
  then (value - 1)
  else 0⟩

/-- Descends by one level in a bounded Merkle tree. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def merkle_depth_increment (value : merkle_depth) : SailM merkle_depth := do
  let value := (value).value
  let publicResult ← do
    assert (value <b 64) "sail/primitives/quantities.sail:264.21-264.22"
    (pure (value + 1))
  pure (⟨publicResult⟩)

/-- Ascends by one level in a bounded Merkle tree. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ 64 -/
def merkle_depth_decrement (value : merkle_depth) : SailM merkle_depth := do
  let value := (value).value
  let publicResult ← do
    assert (0 <b value) "sail/primitives/quantities.sail:270.20-270.21"
    (pure (value - 1))
  pure (⟨publicResult⟩)

/- Type quantifiers: off : Nat, len : Nat, memory_valid_range(off, len) -/
def memory_range (off : Nat) (len : Nat) : (MemoryRangeFields off len) :=
  { off := off,
    len := len }

/-- The canonical inactive range used for a zero-sized operand or halt. -/
def EMPTY_MEMORY_RANGE : (MemoryRangeFields 0 0) := (memory_range 0 0)

/- Type quantifiers: value : Nat, 0 ≤ value -/
def word_of_nat_byte_count (value : Nat) : SailM word := do
  let publicResult ← do
    if ((value <b (2 ^i 256)) : Bool)
    then (pure ((U256 value)).value)
    else
      (do
        assert false "sail/primitives/quantities.sail:363.20-363.21"
        throw Error.Exit)
  pure (⟨publicResult⟩)

/- Type quantifiers: value : Nat, source_valid_length(value) -/
def word_of_source_byte_count (value : Nat) : SailM word := do
  let publicResult ← do
    (do
        let publicResult ← (word_of_nat_byte_count value)
        pure ((publicResult).value))
  pure (⟨publicResult⟩)

