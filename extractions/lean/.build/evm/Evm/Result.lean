import Evm.Flow
import Evm.Quantities
import Evm.Bytes
import Evm.Bytes0
import Evm.Scratch
import Evm.CycleScopes
import Evm.CycleScopesDisabled
import Evm.Htr

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

def RESULT_METADATA_LENGTH : byte_length := (ByteQuantity 5)

/-- Type quantifiers: k_ex173415_ : Bool -/
def result_prefix (root : (BitVec 256)) (success : Bool) : SailM Unit := do
  (scratch_push_bytes (word_to_bytes32 root) WORD_BYTE_LENGTH)
  (scratch_push_bytes
    [if (success : Bool)
    then 0x01#8
    else 0x00#8, 0x25#8, 0x00#8, 0x00#8, 0x00#8] RESULT_METADATA_LENGTH)

/-- Type quantifiers: k_ex173417_ : Bool -/
def commit_validation_result (root : (BitVec 256)) (success : Bool) (chain_config : ByteSlice) : SailM Unit := do
  let start ← do (scratch_begin ())
  (result_prefix root success)
  (scratch_push_slice chain_config)
  assert (← (public_output_write (← (scratch_finish start)))) "public output write"

/-- Type quantifiers: k_ex173419_ : Bool -/
def write_validation_result (input_ref : StatelessInputRef) (success : Bool) : SailM Unit := do
  let _ : Unit := (cycle_scope_start SCOPE_COMPUTE_OUTPUT_ROOT)
  let root ← do (htr_new_payload_request input_ref)
  let _ : Unit := (cycle_scope_end SCOPE_COMPUTE_OUTPUT_ROOT)
  let _ : Unit := (cycle_scope_start SCOPE_SERIALIZE_OUTPUT)
  (commit_validation_result root success input_ref.chain_config)
  (pure (cycle_scope_end SCOPE_SERIALIZE_OUTPUT))

def write_invalid_result (_ : Unit) : SailM Unit := do
  let _ : Unit := (cycle_scope_start SCOPE_SERIALIZE_OUTPUT)
  let start ← do (scratch_begin ())
  let loop_i_lower := 0
  let loop_i_upper := 35
  let mut loop_vars := ()
  for i in [loop_i_lower:loop_i_upper:1]i do
    let () := loop_vars
    loop_vars ← do
      let b : byte :=
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
  (commit_validation_result 0x0000000000000000000000000000000000000000000000000000000000000000#256
    false chain_config)
  (pure (cycle_scope_end SCOPE_SERIALIZE_OUTPUT))

