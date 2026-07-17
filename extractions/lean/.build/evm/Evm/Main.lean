import Evm.CycleScopes
import Evm.CycleScopesDisabled
import Evm.DebugDisabled
import Evm.StatelessInput0
import Evm.Stateless
import Evm.Result

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
            { input_ref := input_ref
              valid := valid })))) (fun the_exception => 
      match the_exception with
        | .InvalidBlock reason =>
          (let _ : Unit := (validation_debug_record SCOPE_DECODE_INPUT reason)
          (pure none))) ) : SailM (Option GuestValidation) )
  let _ : Unit := (cycle_scope_end SCOPE_STATELESS_VALIDATION)
  match result with
  | .some result => (write_validation_result result.input_ref result.valid)
  | none => (write_invalid_result ())

