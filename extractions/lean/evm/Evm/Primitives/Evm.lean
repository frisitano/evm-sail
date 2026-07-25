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

/-! # EVM execution types

The per-transaction environment, the transaction validity and frame-result
records, and the per-frame call [Message][type-Message]. Pure data — no
registers, no externs. -/

def undefined_TxValidity (_ : Unit) : SailM TxValidity := do
  (pure { sender := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          nonce_before := ← (undefined_range 0 ((2 ^i 64) - 1)),
          gas_limit := ← (undefined_range 0 ((2 ^i 64) - 1)),
          intrinsic_execution_gas := ← (undefined_nat ()),
          intrinsic_state_gas := ← (undefined_nat ()),
          calldata_floor := ← (undefined_nat ()),
          blob_fee := ← (undefined_range 0 ((2 ^i 256) - 1)),
          gas_price := ← (undefined_range 0 ((2 ^i 256) - 1)),
          priority_fee := ← (undefined_range 0 ((2 ^i 256) - 1)) })

def undefined_TxFrameResult (_ : Unit) : SailM TxFrameResult := do
  (pure { success := ← (undefined_bool ()),
          execution_gas_remaining := ← (undefined_nat ()),
          state_gas_remaining := ← (undefined_nat ()),
          state_gas_used := ← (undefined_int ()),
          refund := ← (undefined_range (Neg.neg (199 *i ((2 ^i 64) - 1))) (199 *i ((2 ^i 64) - 1))) })

def undefined_CallKind (_ : Unit) : SailM CallKind := do
  (internal_pick [Call, CallCode, DelegateCall, StaticCall])

/- Type quantifiers: arg_ : Nat, 0 ≤ arg_ ∧ arg_ ≤ 3 -/
def CallKind_of_num (arg_ : Nat) : CallKind :=
  match arg_ with
  | 0 => Call
  | 1 => CallCode
  | 2 => DelegateCall
  | _ => StaticCall

def num_of_CallKind (arg_ : CallKind) : Nat :=
  match arg_ with
  | .Call => 0
  | .CallCode => 1
  | .DelegateCall => 2
  | .StaticCall => 3

def undefined_Message (_ : Unit) : SailM Message := do
  (pure { caller := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          code_address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          address := ← (undefined_vector 20 (← (undefined_bitvector 8))),
          value := ← (undefined_range 0 ((2 ^i 256) - 1)),
          state_gas_reservoir := ← (undefined_nat ()),
          is_static := ← (undefined_bool ()),
          depth := ← (undefined_range 0 1024) })

/-- The zero message; frame registers reset to it between transactions. -/
def DEFAULT_MESSAGE : Message :=
  { caller := ZERO_ADDRESS,
    address := ZERO_ADDRESS,
    code_address := ZERO_ADDRESS,
    value := ZERO_WORD,
    state_gas_reservoir := GAS_ZERO,
    is_static := false,
    depth := 0 }

def DEFAULT_FRAME_CHECKPOINT : FrameCheckpoint :=
  { state := 0,
    pc := 0,
    gas_remaining := GAS_ZERO,
    state_gas_remaining := GAS_ZERO,
    state_gas_spilled := STATE_GAS_SPILL_ZERO,
    refund := GAS_REFUND_ZERO,
    status := (Running ()),
    message := DEFAULT_MESSAGE,
    call_depth := 0,
    code := EMPTY_CODE,
    calldata := ⟨_, ⟨_, EMPTY_SLICE⟩⟩,
    memory := ⟨_, ⟨_, EMPTY_SLICE⟩⟩ }

def DEFAULT_FRAME_CONTINUATION : FrameContinuation :=
  (ResumeCall
    { checkpoint := DEFAULT_FRAME_CHECKPOINT,
      return_offset := 0,
      return_length := 0,
      new_account_charged := false })

