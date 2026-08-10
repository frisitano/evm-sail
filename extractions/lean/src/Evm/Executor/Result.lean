import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Kernel.Scratch
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

/-! # The public output

The guest's public output: the `hash_tree_root` of the new-payload
request, the validation verdict, and the echoed chain configuration. -/

abbrev RESULT_METADATA_LENGTH : Nat := 5

/-- Writes the request root and validation metadata evm_prefix. -/
/- Type quantifiers: k_ex555165_ : Bool -/
def result_prefix (root : (Vector (BitVec 8) 32)) (success : Bool) : SailM Unit := do
  (scratch_push_b256 root WORD_BYTE_LENGTH)
  (scratch_push_byte
    (if (success : Bool)
    then 0x01#8
    else 0x00#8))
  (scratch_push_byte 0x25#8)
  (scratch_push_byte 0x00#8)
  (scratch_push_byte 0x00#8)
  (scratch_push_byte 0x00#8)

/-- Serializes and commits the public validation result exactly once. -/
/- Type quantifiers: chain_config_dependentWitness1 : Nat, chain_config_dependentWitness0 : Nat, k_ex555168_
  : Bool, 0 ≤ chain_config_dependentWitness0 ∧
  0 ≤ chain_config_dependentWitness1 ∧
  (chain_config_dependentWitness0 + chain_config_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def commit_validation_result (root : (Vector (BitVec 8) 32)) (success : Bool) (chain_config : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM Unit := do
  let chain_config_dependentWitness0 := (chain_config).1
  let chain_config_dependentWitness1 := ((chain_config).2).1
  let chain_config := ((chain_config).2).2
  let fixed_length := (WORD_BYTE_LENGTH + RESULT_METADATA_LENGTH)
  let output_length ← do (scratch_length_add fixed_length chain_config.len)
  let start ← do (scratch_reserve output_length)
  (result_prefix root success)
  (stateless_input_scratch_push_slice ⟨_, ⟨_, chain_config⟩⟩)
  let ⟨_, ⟨_, output⟩⟩ ← do (scratch_finish start)
  let written ← do (public_output_write ⟨_, ⟨_, output⟩⟩)
  assert written "public output write"

/-- Emits the full public output for a decoded input: the request root
computed from the input itself, the verdict, and the input's chain
configuration echoed byte for byte. -/
/- Type quantifiers: k_ex555173_ : Bool -/
def write_validation_result (input_ref : StatelessInputRef) (success : Bool) : SailM Unit := do
  let root ← do (htr_new_payload_request input_ref)
  (commit_validation_result root success input_ref.chain_config)

/-- Emits the failure output for an undecodable input: a zero root, a
false verdict, and a default configuration frame. -/
def write_invalid_result (_ : Unit) : SailM Unit := do
  let start ← do (scratch_reserve 24)
  let default_chain_config := ZERO_HASH
  let default_chain_config : (Vector (BitVec 8) 32) := (vectorUpdate default_chain_config 8 0x0C#8)
  let default_chain_config : (Vector (BitVec 8) 32) := (vectorUpdate default_chain_config 12 0x04#8)
  let default_chain_config : (Vector (BitVec 8) 32) := (vectorUpdate default_chain_config 16 0x08#8)
  let default_chain_config : (Vector (BitVec 8) 32) := (vectorUpdate default_chain_config 20 0x08#8)
  (scratch_push_b256 default_chain_config 24)
  let ⟨_, ⟨_, chain_config⟩⟩ ← do (scratch_finish start)
  let fixed_length := (WORD_BYTE_LENGTH + RESULT_METADATA_LENGTH)
  let output_length ← do (scratch_length_add fixed_length chain_config.len)
  let output_start ← do (scratch_reserve output_length)
  (result_prefix ZERO_HASH false)
  (scratch_scratch_push_slice ⟨_, ⟨_, chain_config⟩⟩)
  let ⟨_, ⟨_, output⟩⟩ ← do (scratch_finish output_start)
  let written ← do (public_output_write ⟨_, ⟨_, output⟩⟩)
  assert written "public output write"

