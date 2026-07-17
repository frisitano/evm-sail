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

def undefined_CallKind (_ : Unit) : SailM CallKind := do
  (internal_pick [Call, CallCode, DelegateCall, StaticCall])

/-- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 3 -/
def CallKind_of_num (arg_ : Nat) : CallKind :=
  match arg_ with
  | 0 => Call
  | 1 => CallCode
  | 2 => DelegateCall
  | _ => StaticCall

def num_of_CallKind (arg_ : CallKind) : Int :=
  match arg_ with
  | .Call => 0
  | .CallCode => 1
  | .DelegateCall => 2
  | .StaticCall => 3

def undefined_Message (_ : Unit) : SailM Message := do
  (pure { caller := ← (undefined_bitvector 160),
          code_address := ← (undefined_bitvector 160),
          address := ← (undefined_bitvector 160),
          value := ← (undefined_bitvector 256),
          is_static := ← (undefined_bool ()),
          depth := ← do
              let semanticField ← (undefined_range 0 1024)
              pure (⟨semanticField⟩) })

def DEFAULT_MESSAGE : Message :=
  { caller := ZERO_ADDR,
    address := ZERO_ADDR,
    code_address := ZERO_ADDR,
    value := ZERO_WORD,
    is_static := false,
    depth := ⟨0⟩ }

