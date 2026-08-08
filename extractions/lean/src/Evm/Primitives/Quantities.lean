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
open ast
open TxType
open TxSignatureScheme
open TrieUpdateSource
open TrieUpdateRelation
open TrieLeafValue
open TrieItemValue
open TrieChange
open StorageTxPopResult
open StorageTxLookup
open StorageBlockIterResult
open StateJournalEntry
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
open NodeRef
open LogTopics
open LogData
open InputTrieNode
open IndexedTrieSource
open HtrRequestKind
open HaltKind
open FrameStatus
open FrameContinuation
open FatalError
open ExceptionKind
open EnvField
open DeepStackOperation
open CreateKind
open CalldataSlice
open CallKind
open BalIterEntry
open AcctTxPopResult
open AcctBlockIterResult

/-! # Protocol quantities

Semantic numeric domains used outside the EVM's 256-bit word algebra.
Protocol fields retain their wire bounds, structural counters carry the
limits imposed by their data structures, and byte positions remain exact
non-negative quantities after operand validation.

## Types

The aliases below name the semantic role of protocol and structural
quantities while preserving their mathematical values. Where the protocol or
data structure supplies a bound, the alias records it explicitly. -/

def undefined_PrecompileId (_ : Unit) : SailM PrecompileId := do
  (internal_pick
    [NotPrecompile, Ecrecover, Sha256, Ripemd160, Identity, Modexp, Bn254Add, Bn254Mul, Bn254Pairing, Blake2f, KzgPointEvaluation, BlsG1Add, BlsG1Msm, BlsG2Add, BlsG2Msm, BlsPairing, BlsMapFpToG1, BlsMapFp2ToG2, P256Verify])

/- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 18 -/
def PrecompileId_of_num (arg_ : Nat) : PrecompileId :=
  match arg_ with
  | 0 => NotPrecompile
  | 1 => Ecrecover
  | 2 => Sha256
  | 3 => Ripemd160
  | 4 => Identity
  | 5 => Modexp
  | 6 => Bn254Add
  | 7 => Bn254Mul
  | 8 => Bn254Pairing
  | 9 => Blake2f
  | 10 => KzgPointEvaluation
  | 11 => BlsG1Add
  | 12 => BlsG1Msm
  | 13 => BlsG2Add
  | 14 => BlsG2Msm
  | 15 => BlsPairing
  | 16 => BlsMapFpToG1
  | 17 => BlsMapFp2ToG2
  | _ => P256Verify

def num_of_PrecompileId (arg_ : PrecompileId) : Nat :=
  match arg_ with
  | .NotPrecompile => 0
  | .Ecrecover => 1
  | .Sha256 => 2
  | .Ripemd160 => 3
  | .Identity => 4
  | .Modexp => 5
  | .Bn254Add => 6
  | .Bn254Mul => 7
  | .Bn254Pairing => 8
  | .Blake2f => 9
  | .KzgPointEvaluation => 10
  | .BlsG1Add => 11
  | .BlsG1Msm => 12
  | .BlsG2Add => 13
  | .BlsG2Msm => 14
  | .BlsPairing => 15
  | .BlsMapFpToG1 => 16
  | .BlsMapFp2ToG2 => 17
  | .P256Verify => 18

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

/-- Embeds the SSZ-bounded block number in the EVM word domain. The uint64
schema proof makes a runtime 256-bit range check unnecessary. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def word_of_block_number (value : Nat) : Nat :=
  value

/-- Embeds the SSZ-bounded block timestamp in the EVM word domain. The uint64
schema proof makes a runtime 256-bit range check unnecessary. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def word_of_block_timestamp (value : Nat) : Nat :=
  value

/-- Converts a chain identifier to the value exposed by CHAINID. -/
/- Type quantifiers: value : Nat, 0 ≤ value ∧ value ≤ (2 ^ 64 - 1) -/
def word_of_chain_identifier (value : Nat) : Nat :=
  value

/- Type quantifiers: off : Nat, len : Nat, (memory_valid_range off len) -/
def memory_range (off : Nat) (len : Nat) : (MemoryRangeFields off len) :=
  {  }

/-- The canonical inactive range used for a zero-sized operand or halt. -/
def EMPTY_MEMORY_RANGE : (MemoryRangeFields 0 0) := (memory_range 0 0)

/-- The canonical inactive memory operand. -/
def EMPTY_MEMORY_ACCESS : (MemoryAccessFields 0 0 0) := { range := EMPTY_MEMORY_RANGE }

/- Type quantifiers: value : Nat, 0 ≤ value -/
def word_of_nat_byte_count (value : Nat) : SailM Nat := do
  if ((value <b (2 ^i 256)) : Bool)
  then (pure (u256 value))
  else
    (do
      assert false "sail/primitives/quantities.sail:607.20-607.21"
      throw Error.Exit)

/- Type quantifiers: value : Nat, (source_valid_length value) -/
def word_of_source_byte_count (value : Nat) : SailM Nat := do
  (word_of_nat_byte_count value)

