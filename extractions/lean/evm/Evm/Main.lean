import Evm.Primitives.CycleScopes
import Evm.Host.CycleScopesDisabled
import Evm.Host.DebugDisabled
import Evm.Lib.Ssz.StatelessInput
import Evm.Executor.Stateless
import Evm.Executor.Result

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

/-! # The guest entry point

The zkVM guest's `main`: decode the stateless input, verify the payload,
and emit the public validation result. Any thrown `InvalidBlock` — even
during input decoding — resolves to an invalid verdict rather than a
crash: validation of an undecodable input is simply unsuccessful. -/

/-- Decodes, verifies, and reports: `write_validation_result` on a
decodable input (valid or not), `write_invalid_result` when decoding
itself fails. -/
def sail_main (_ : Unit) : SailM Unit := do
  let _ : Unit := (validation_debug_reset ())
  let _ : Unit := (cycle_scope_start SCOPE_STATELESS_VALIDATION)
  let result ← (( do
    sailTryCatch ((do
        let input_ref ← do (decode_stateless_input_ref (← (stateless_input ())))
        let validation ← (( do (verify_stateless_payload input_ref) ) : SailM
          StatelessValidationResult )
        let valid : Bool :=
          match validation with
          | .StatelessPayloadValid () => true
          | .StatelessPayloadInvalid failure =>
            (let _ : Unit := (validation_debug_record failure.scope failure.reason)
            false)
        (pure (some
            { input_ref := input_ref,
              valid := valid })))) (fun the_exception => 
      match the_exception with
        | .InvalidBlock reason =>
          (let _ : Unit := (validation_debug_record SCOPE_DECODE_INPUT reason)
          (pure none))) ) : SailM (Option GuestValidation) )
  let _ : Unit := (cycle_scope_end SCOPE_STATELESS_VALIDATION)
  match result with
  | .some result => (write_validation_result result.input_ref result.valid)
  | none => (write_invalid_result ())

