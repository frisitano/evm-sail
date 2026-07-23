import Evm.Prelude
import Evm.Primitives.CycleScopes
import Evm.Host.CycleScopesDisabled
import Evm.Primitives.Bytes
import Evm.Lib.Bytes
import Evm.Host.Kernel.Scratch
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

/-! # The public output

The guest's public output: the `hash_tree_root` of the new-payload
request, the validation verdict, and the echoed chain configuration. -/

def RESULT_METADATA_LENGTH : Nat := 5

/-- Writes the request root and validation metadata evm_prefix. -/
/- Type quantifiers: k_ex409490_ : Bool -/
def result_prefix (root : hash) (success : Bool) : SailM Unit := do
  (scratch_push_bytes (hash_to_bytes32 root) WORD_BYTE_LENGTH)
  (scratch_push_bytes
    [(if (success : Bool)
    then 0x01#8
    else 0x00#8), 0x25#8, 0x00#8, 0x00#8, 0x00#8] RESULT_METADATA_LENGTH)

/-- Serializes and commits the public validation result exactly once. -/
/- Type quantifiers: k_ex409495_ : Nat, k_ex409494_ : Nat, k_ex409491_ : Bool, 0 ≤ k_ex409494_ ∧
  0 ≤ k_ex409495_ -/
def commit_validation_result (root : hash) (success : Bool) (chain_config : EvmByteSlice) : SailM Unit := do
  let chain_config := ((chain_config).2).2
  let start ← do (scratch_begin ())
  (result_prefix root success)
  (scratch_push_slice ⟨_, ⟨_, chain_config⟩⟩)
  assert (← (public_output_write (← (scratch_finish start)))) "public output write"

/-- Emits the full public output for a decoded input: the request root
computed from the input itself, the verdict, and the input's chain
configuration echoed byte for byte. -/
/- Type quantifiers: k_ex409496_ : Bool -/
def write_validation_result (input_ref : StatelessInputRef) (success : Bool) : SailM Unit := do
  let _ : Unit := (cycle_scope_start SCOPE_COMPUTE_OUTPUT_ROOT)
  let root ← do (htr_new_payload_request input_ref)
  let _ : Unit := (cycle_scope_end SCOPE_COMPUTE_OUTPUT_ROOT)
  let _ : Unit := (cycle_scope_start SCOPE_SERIALIZE_OUTPUT)
  (commit_validation_result root success input_ref.chain_config)
  (pure (cycle_scope_end SCOPE_SERIALIZE_OUTPUT))

/-- Emits the failure output for an undecodable input: a zero root, a
false verdict, and a default configuration frame. -/
def write_invalid_result (_ : Unit) : SailM Unit := do
  let _ : Unit := (cycle_scope_start SCOPE_SERIALIZE_OUTPUT)
  let start ← do (scratch_begin ())
  (scratch_push_bytes
    [0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x00#8, 0x0C#8, 0x00#8, 0x00#8, 0x00#8, 0x04#8, 0x00#8, 0x00#8, 0x00#8, 0x08#8, 0x00#8, 0x00#8, 0x00#8, 0x08#8, 0x00#8, 0x00#8, 0x00#8]
    24)
  let ⟨_, ⟨_, chain_config⟩⟩ ← do (scratch_finish start)
  (commit_validation_result ZERO_HASH false ⟨_, ⟨_, chain_config⟩⟩)
  (pure (cycle_scope_end SCOPE_SERIALIZE_OUTPUT))

