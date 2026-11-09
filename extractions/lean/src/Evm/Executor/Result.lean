import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Kernel.Scratch
import Evm.Lib.Ssz.StatelessInput
import Evm.Lib.Htr

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
open StackValidation
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
open OpcodeOutcome
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

/-! # The public output

The guest's public output, the 43-byte `StatelessValidationResult`: the
`hash_tree_root` of the new-payload request, the validation verdict, the
chain id, and the schema id of the decoded input. An undecodable input
publishes the all-zero sentinel. -/

/-- The width of the `StatelessValidationResult`: the request root, the
verdict byte, the `uint64` chain id, and the `uint16` schema id. -/
abbrev RESULT_LENGTH : Nat := 43

/-- Serializes and commits the public validation result exactly once. -/
/- Type quantifiers: k_ex428010_ : Nat, k_ex428009_ : Bool, 0 ≤ k_ex428010_ ∧
  k_ex428010_ ≤ (2 ^ 64 - 1) -/
def commit_validation_result (root : (Vector (BitVec 8) 32)) (success : Bool) (chain_id : Nat) (schema_id : (BitVec 16)) : SailM Unit := do
  let start ← do (scratch_reserve RESULT_LENGTH)
  (scratch_push_b256 root WORD_BYTE_LENGTH)
  let success_byte : (BitVec 8) :=
    if (success : Bool)
    then 0x01#8
    else 0x00#8
  (scratch_push_byte success_byte)
  let identity := (htr_uint chain_id)
  let identity : (Vector (BitVec 8) 32) :=
    (vectorUpdate identity 8 (Sail.BitVec.extractLsb schema_id 7 0))
  let identity : (Vector (BitVec 8) 32) :=
    (vectorUpdate identity 9 (Sail.BitVec.extractLsb schema_id 15 8))
  (scratch_push_b256 identity 10)
  let ⟨_, ⟨_, output⟩⟩ ← do (scratch_finish start)
  let written ← do (public_output_write ⟨_, ⟨_, output⟩⟩)
  assert written "public output write"

/-- Emits the full public output for a decoded input: the request root
computed from the input itself, the verdict, and the input's chain id and
schema id. -/
/- Type quantifiers: k_ex428011_ : Bool -/
def write_validation_result (input_ref : StatelessInputRef) (success : Bool) : SailM Unit := do
  let root ← do (htr_new_payload_request input_ref)
  (commit_validation_result root success input_ref.chain_id STATELESS_SCHEMA_ID)

/-- Emits the failure output for an undecodable input: a zero root, a
false verdict, and a zero chain id and schema id. -/
def write_invalid_result (_ : Unit) : SailM Unit := do
  (commit_validation_result ZERO_HASH false 0 0x0000#16)
