import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Bytes
import Evm.Lib.Bytes
import Evm.Host.Kernel.Scratch
import Evm.Primitives.CycleScopes
import Evm.Host.CycleScopesDisabled
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

/-! # The public output

The guest's public output: the `hash_tree_root` of the new-payload
request, the validation verdict, and the echoed chain configuration. -/

def RESULT_METADATA_LENGTH : byte_length := (ByteQuantity 5)

/-- Writes the request root and validation metadata evm_prefix. -/
/- Type quantifiers: k_ex161676_ : Bool -/
def result_prefix (root : hash) (success : Bool) : SailM Unit := do
  (scratch_push_bytes (hash_to_bytes32 root) WORD_BYTE_LENGTH)
  (scratch_push_bytes
    [(if (success : Bool)
    then 0x01#8
    else 0x00#8), 0x25#8, 0x00#8, 0x00#8, 0x00#8] RESULT_METADATA_LENGTH)

/-- Serializes and commits the public validation result exactly once. -/
/- Type quantifiers: k_ex161677_ : Bool -/
def commit_validation_result (root : hash) (success : Bool) (chain_config : EvmByteSlice) : SailM Unit := do
  let start ← do (scratch_begin ())
  (result_prefix root success)
  (scratch_push_slice chain_config)
  assert (← (public_output_write (← (scratch_finish start)))) "public output write"

/-- Emits the full public output for a decoded input: the request root
computed from the input itself, the verdict, and the input's chain
configuration echoed byte for byte. -/
/- Type quantifiers: k_ex161678_ : Bool -/
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
  let loop_i_lower := 0
  let loop_i_upper := 35
  let mut loop_vars := ()
  for i in [loop_i_lower:loop_i_upper:1]i do
    let () := loop_vars
    loop_vars ← do
      let b : (BitVec 8) :=
        match i with
        | 8 => 0x0C#8
        | 20 => 0x10#8
        | 24 => 0x18#8
        | 28 => 0x08#8
        | 32 => 0x08#8
        | _ => 0x00#8
      (scratch_push_bytes [b] BYTE_ONE)
  (pure loop_vars)
  let chain_config ← do (scratch_finish start)
  (commit_validation_result ZERO_HASH false chain_config)
  (pure (cycle_scope_end SCOPE_SERIALIZE_OUTPUT))

