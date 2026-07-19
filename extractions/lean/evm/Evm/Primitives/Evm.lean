import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Primitives.Code

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

/-! # EVM execution types

The per-transaction environment, the transaction validity and frame-result
records, and the per-frame call [Message][type-Message]. Pure data — no
registers, no externs. -/

def undefined_CallKind (_ : Unit) : SailM CallKind := do
  (internal_pick [Call, CallCode, DelegateCall, StaticCall])

/- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 3 -/
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

/-- The zero message; frame registers reset to it between transactions. -/
def DEFAULT_MESSAGE : Message :=
  { caller := ZERO_ADDR,
    address := ZERO_ADDR,
    code_address := ZERO_ADDR,
    value := ZERO_WORD,
    is_static := false,
    depth := ⟨0⟩ }

def DEFAULT_FRAME_CHECKPOINT : FrameCheckpoint :=
  { state := (StateCheckpoint ⟨0⟩),
    pc := BYTE_ZERO,
    gas_remaining := GAS_ZERO,
    refund := GAS_REFUND_ZERO,
    status := (Running ()),
    message := DEFAULT_MESSAGE,
    call_depth := ⟨0⟩,
    code := EMPTY_CODE,
    calldata := EMPTY_SLICE,
    memory := EMPTY_SLICE }

def DEFAULT_FRAME_CONTINUATION : FrameContinuation :=
  (ResumeCall
    { checkpoint := DEFAULT_FRAME_CHECKPOINT,
      return_offset := BYTE_ZERO,
      return_length := BYTE_ZERO })

