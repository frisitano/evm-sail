import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Primitives.Code
import Evm.Host.RegionAccess
import Evm.Primitives.Fork
import Evm.Host.Code
import Evm.Kernel.Environment
import Evm.Kernel.Code
import Evm.Kernel.Lifecycle
import Evm.Evm.Machine
import Evm.Evm.Gas
import Evm.Evm.Execute

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

/-! # The interpreter

The user-space EVM: it fetches and decodes bytecode (Yellow Paper §9),
drives the step loop, and enters sub-frames for the call and create
opcodes. This module specifies that machinery in three layers:

1. **Fetch/decode** — read the opcode at the carried program counter,
   decode PUSH immediates inline, and map every other byte to its AST
   node (an undefined byte decodes to `INVALID`). Reading past the end of
   code yields `STOP` (YP: implicit halt).
2. **Run loop** — [interpret][] steps fetch-then-execute until the active
   frame stops, threading the carried machine state (program counter,
   gas, operand-stack cursor, memory cursor) through every step: each
   step's arguments are supplied from the frame registers and the
   returned state is assigned back, so no handler ever reads or writes
   those registers. Popping a pending
   [FrameContinuation][type-FrameContinuation] resumes a completed child;
   [Empty][type-FrameContinuation] marks completion of the top-level
   frame.
3. **Message calls** — [run_call][] handles
   `CALL`/`CALLCODE`/`DELEGATECALL`/`STATICCALL` (multiplexed on `mode`)
   and [run_create][] handles `CREATE`/`CREATE2`. A sub-call publishes
   the parent's carried state to the frame registers, saves them through
   [frame_stack_push][], installs the child as the active frame, and
   returns the child's carried state to the single run loop. When the
   child halts, that loop restores and resumes the parent. There is no
   recursive interpreter invocation. All world effects go through kernel
   syscalls: [k_journal_checkpoint][] on entry, [k_transfer][] for value,
   and [k_journal_revert][] on failure — the kernel rolls the world back
   atomically on a reverting child. The applicable rules are EIP-150
   (63/64ths gas cap + stipend),
   EIP-214 (static-context write protection), EIP-2929 (cold/warm
   access), and EIP-7702 (delegated-code execution). -/

/-- Assembles an `n`-byte big-endian PUSH immediate from a local code cursor;
bytes past the end of code read as zero. -/
/- Type quantifiers: k_ex552938_ : Nat, k_ex552937_ : Nat, code_dependentWitness1 : Nat, code_dependentWitness0
  : Nat, 0 ≤ code_dependentWitness0 ∧
  0 ≤ code_dependentWitness1 ∧
  (code_dependentWitness0 + code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ code_dependentWitness1 ∧ (code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤ k_ex552937_
  ∧ k_ex552937_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex552938_ ∧ k_ex552938_ ≤ 32 -/
def read_push (code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeRegionSliceFields k_off k_len)))) (offset : Nat) (n : Nat) : SailM Nat := do
  let code_dependentWitness0 := (code).1
  let code_dependentWitness1 := ((code).2).1
  let code := ((code).2).2
  (code_slice_load_n ⟨_, ⟨_, code⟩⟩ offset n)

/-- Decodes one non-PUSH opcode byte to its AST node. The three
contiguous families fold to an arity argument — `DUP1`–`DUP16`
(0x80–0x8f), `SWAP1`–`SWAP16` (0x90–0x9f), `LOG0`–`LOG4` (0xa0–0xa4)
— and the remainder is a flat table. Any byte with no defined opcode
decodes to `INVALID`. -/
/- Type quantifiers: opcode : Nat, 0 ≤ opcode ∧ opcode ≤ 255 -/
def decode_simple (opcode : Nat) : SailM ast := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if (((128 ≤b opcode) && (opcode ≤b 143)) : Bool)
  then (pure (DUP (opcode - 127)))
  else
    (if (((144 ≤b opcode) && (opcode ≤b 159)) : Bool)
    then (pure (SWAP (opcode - 143)))
    else
      (if (((160 ≤b opcode) && (opcode ≤b 164)) : Bool)
      then (pure (LOG (opcode - 160)))
      else
        (match opcode with
        | 0 => (pure (STOP ()))
        | 1 => (pure (ADD ()))
        | 2 => (pure (MUL ()))
        | 3 => (pure (SUB ()))
        | 4 => (pure (DIV ()))
        | 5 => (pure (SDIV ()))
        | 6 => (pure (MOD ()))
        | 7 => (pure (SMOD ()))
        | 8 => (pure (ADDMOD ()))
        | 9 => (pure (MULMOD ()))
        | 10 => (pure (EXP ()))
        | 11 => (pure (SIGNEXTEND ()))
        | 16 => (pure (LT ()))
        | 17 => (pure (GT ()))
        | 18 => (pure (SLT ()))
        | 19 => (pure (SGT ()))
        | 20 => (pure (EQ ()))
        | 21 => (pure (ISZERO ()))
        | 22 => (pure (AND ()))
        | 23 => (pure (OR ()))
        | 24 => (pure (XOR ()))
        | 25 => (pure (NOT ()))
        | 26 => (pure (BYTE ()))
        | 27 => (pure (SHL ()))
        | 28 => (pure (SHR ()))
        | 29 => (pure (SAR ()))
        | 30 =>
          (if ((profile.fork ≥b Osaka) : Bool)
          then (pure (CLZ ()))
          else (pure (INVALID ())))
        | 32 => (pure (KECCAK256 ()))
        | 48 => (pure (ADDRESS ()))
        | 49 => (pure (BALANCE ()))
        | 50 => (pure (ORIGIN ()))
        | 51 => (pure (CALLER ()))
        | 52 => (pure (CALLVALUE ()))
        | 53 => (pure (CALLDATALOAD ()))
        | 54 => (pure (CALLDATASIZE ()))
        | 55 => (pure (CALLDATACOPY ()))
        | 56 => (pure (CODESIZE ()))
        | 57 => (pure (CODECOPY ()))
        | 58 => (pure (GASPRICE ()))
        | 59 => (pure (EXTCODESIZE ()))
        | 60 => (pure (EXTCODECOPY ()))
        | 61 => (pure (RETURNDATASIZE ()))
        | 62 => (pure (RETURNDATACOPY ()))
        | 63 => (pure (EXTCODEHASH ()))
        | 64 => (pure (BLOCKHASH ()))
        | 65 => (pure (COINBASE ()))
        | 66 => (pure (TIMESTAMP ()))
        | 67 => (pure (NUMBER ()))
        | 68 => (pure (PREVRANDAO ()))
        | 69 => (pure (GASLIMIT ()))
        | 70 => (pure (CHAINID ()))
        | 71 => (pure (SELFBALANCE ()))
        | 72 =>
          (if ((profile.fork ≥b London) : Bool)
          then (pure (BASEFEE ()))
          else (pure (INVALID ())))
        | 73 =>
          (if ((profile.fork ≥b Cancun) : Bool)
          then (pure (BLOBHASH ()))
          else (pure (INVALID ())))
        | 74 =>
          (if ((profile.fork ≥b Cancun) : Bool)
          then (pure (BLOBBASEFEE ()))
          else (pure (INVALID ())))
        | 75 =>
          (if ((profile.fork ≥b Amsterdam) : Bool)
          then (pure (SLOTNUM ()))
          else (pure (INVALID ())))
        | 80 => (pure (POP ()))
        | 81 => (pure (MLOAD ()))
        | 82 => (pure (MSTORE ()))
        | 83 => (pure (MSTORE8 ()))
        | 84 => (pure (SLOAD ()))
        | 85 => (pure (SSTORE ()))
        | 86 => (pure (JUMP ()))
        | 87 => (pure (JUMPI ()))
        | 88 => (pure (PC ()))
        | 89 => (pure (MSIZE ()))
        | 90 => (pure (GAS ()))
        | 91 => (pure (JUMPDEST ()))
        | 92 =>
          (if ((profile.fork ≥b Cancun) : Bool)
          then (pure (TLOAD ()))
          else (pure (INVALID ())))
        | 93 =>
          (if ((profile.fork ≥b Cancun) : Bool)
          then (pure (TSTORE ()))
          else (pure (INVALID ())))
        | 94 =>
          (if ((profile.fork ≥b Cancun) : Bool)
          then (pure (MCOPY ()))
          else (pure (INVALID ())))
        | 240 => (pure (opcode_CREATE ()))
        | 241 => (pure (CALL ()))
        | 242 => (pure (CALLCODE ()))
        | 243 => (pure (RETURN ()))
        | 244 => (pure (DELEGATECALL ()))
        | 245 => (pure (CREATE2 ()))
        | 250 => (pure (STATICCALL ()))
        | 253 => (pure (REVERT ()))
        | 255 => (pure (SELFDESTRUCT ()))
        | _ => (pure (INVALID ())))))

/-- Fetches and decodes the opcode at the carried program counter,
returning the counter advanced past the opcode and any immediate.
Past the end of code the frame implicitly executes `STOP` (YP).
`PUSH0`–`PUSH32` (0x5f–0x7f) carry an `n`-byte immediate; Amsterdam's
`DUPN`/`SWAPN`/`EXCHANGE` carry one byte, zero-padded at end of code.
Every other byte decodes via [decode_simple][]. -/
/- Type quantifiers: current : Nat, 0 ≤ current ∧ current ≤ (2 ^ 32 - 1) -/
def fetch (current : Nat) : SailM (Nat × ast) := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let ⟨_, ⟨_, analyzed⟩⟩ ← do readReg frame_code
  let code := (code_bytes analyzed)
  let code_length := code.len
  let past_end := (! (current <b code_length))
  if (past_end : Bool)
  then (pure (current, (STOP ())))
  else
    (do
      let opcode_byte ← do (code_slice_byte ⟨_, ⟨_, code⟩⟩ current)
      let opcode : Nat := (BitVec.toNatInt opcode_byte)
      let immediate_offset := (current + 1)
      if (((opcode == 95) && (profile.fork <b Shanghai)) : Bool)
      then (pure (immediate_offset, (INVALID ())))
      else
        (do
          if (((95 ≤b opcode) && (opcode ≤b 127)) : Bool)
          then
            (do
              let size : Nat := (opcode - 95)
              let value ← do (read_push ⟨_, ⟨_, code⟩⟩ immediate_offset size)
              let after_immediate : Nat := ((current + 1) + size)
              (pure (after_immediate, (PUSH (size, value)))))
          else
            (do
              if ((profile.fork ≥b Amsterdam) : Bool)
              then
                (do
                  let deep_operation := (deep_stack_operation opcode)
                  match deep_operation with
                  | .NotDeepStackOperation =>
                    (pure (immediate_offset, (← (decode_simple opcode))))
                  | operation =>
                    (do
                      let immediate ← do (code_slice_byte ⟨_, ⟨_, code⟩⟩ immediate_offset)
                      let immediate_valid :=
                        (deep_stack_operation_immediate_valid operation immediate)
                      let after_instruction : Nat :=
                        if (immediate_valid : Bool)
                        then (current + 2)
                        else (current + 1)
                      let instruction ← (( do
                        match operation with
                        | .DeepStackDuplicate => (pure (DUPN immediate))
                        | .DeepStackSwap => (pure (SWAPN immediate))
                        | .DeepStackExchange => (pure (EXCHANGE immediate))
                        | .NotDeepStackOperation => (decode_simple opcode) ) : SailM ast )
                      (pure (after_instruction, instruction))))
              else (pure (immediate_offset, (← (decode_simple opcode)))))))

/-- Returns the active frame's halt output. -/
def frame_output (_ : Unit) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))) := do
  match (← readReg frame_status) with
  | .Halted (.HaltReturn ⟨_, ⟨_, output⟩⟩) =>
    (pure ((⟨_, ⟨_, output⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))))
  | .Halted (.HaltRevert ⟨_, ⟨_, output⟩⟩) =>
    (pure ((⟨_, ⟨_, output⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))))
  | _ =>
    (pure ((⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))))

/-- Whether the just-finished frame ended successfully: a normal halt
succeeds; a `REVERT` and any exceptional halt do not (their world
effects are rolled back and `CALL`/`CREATE` reports failure). -/
def frame_succeeded (_ : Unit) : SailM Bool := do
  match (← readReg frame_status) with
  | .Halted (.HaltRevert ⟨_, ⟨_, _⟩⟩) => (pure false)
  | .Halted _ => (pure true)
  | .Running () => (pure true)
  | .Exceptional _ => (pure false)

/-- Restores a message-call parent and applies the child's outcome. The
interpreter has published the child's final carried state to the frame
registers; this boundary code works through them and republishes the
parent's stack cursor after pushing the result. -/
/- Type quantifiers: output_dependentWitness1 : Nat, output_dependentWitness0 : Nat, 0 ≤
  output_dependentWitness0 ∧
  0 ≤ output_dependentWitness1 ∧
  (output_dependentWitness0 + output_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def resume_call (continuation : CallContinuation) (output : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : SailM Unit := do
  let output_dependentWitness0 := (output).1
  let output_dependentWitness1 := ((output).2).1
  let output := ((output).2).2
  writeReg returndata ⟨_, ⟨_, output⟩⟩
  let checkpoint := continuation.checkpoint
  let succeeded ← do (frame_succeeded ())
  let child_left ← do readReg gas_remaining
  let child_state_left ← do readReg state_gas_remaining
  let child_state_spill ← do readReg state_gas_spilled
  let child_refund ← do readReg frame_refund
  (restore_frame checkpoint)
  writeReg gas_remaining (refund_gas (← readReg gas_remaining) child_left)
  (return_child_state_gas child_state_left child_state_spill)
  (returndata_copy_prefix continuation.return_offset continuation.return_length)
  if (succeeded : Bool)
  then
    (do
      (record_refund child_refund)
      (k_journal_commit ())
      writeReg stack_top (← (push_word (← readReg stack_top) WORD_ONE)))
  else
    (do
      (k_journal_revert ())
      if (continuation.new_account_charged : Bool)
      then
        writeReg gas_remaining (← (credit_state_gas_refund (← readReg gas_remaining)
            G_amsterdam_state_new_account))
      else (pure ())
      writeReg stack_top (← (push_word (← readReg stack_top) WORD_ZERO)))

/-- Restores a create parent and either deploys or rolls back the child. -/
/- Type quantifiers: output_dependentWitness1 : Nat, output_dependentWitness0 : Nat, 0 ≤
  output_dependentWitness0 ∧
  0 ≤ output_dependentWitness1 ∧
  (output_dependentWitness0 + output_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def resume_create (continuation : CreateContinuation) (output : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : SailM Unit := do
  let output_dependentWitness0 := (output).1
  let output_dependentWitness1 := ((output).2).1
  let output := ((output).2).2
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  writeReg returndata ⟨_, ⟨_, output⟩⟩
  let checkpoint := continuation.checkpoint
  let initcode_succeeded ← do (frame_succeeded ())
  let deployed_length ← do (returndata_size ())
  let deployed_size := deployed_length
  let frontier_empty_deposit : Bool := false
  if (initcode_succeeded : Bool)
  then
    (do
      let deployed_size_allowed ← do (deployed_code_size_allowed deployed_size)
      let invalid_deployed_size := (! deployed_size_allowed)
      let prohibited_prefix ← do
        if ((deployed_size != 0) : Bool)
        then
          (do
            let first_byte ← do
              (do
                  let dependentArg0 := (← readReg returndata)
                  (output_byte dependentArg0 0))
            (pure (first_byte == 0xEF#8)))
        else (pure false)
      if ((invalid_deployed_size || ((profile.fork ≥b London) && prohibited_prefix)) : Bool)
      then writeReg gas_remaining (← (exc_halt (← readReg gas_remaining) OutOfGas))
      else
        (do
          let deployment_charge ← do
            (code_deployment_execution_cost deployed_length (← readReg gas_remaining))
          if (deployment_charge.affordable : Bool)
          then
            (do
              let execution_deposit := deployment_charge.cost
              writeReg gas_remaining (gas_sub (← readReg gas_remaining) execution_deposit)
              let state_deposit ← do (code_deployment_state_cost deployed_length)
              let (_, deployment_gas) ← do
                (charge_deployment_state_gas (← readReg gas_remaining) state_deposit)
              writeReg gas_remaining deployment_gas)
          else writeReg gas_remaining (← (exc_halt (← readReg gas_remaining) OutOfGas))))
  else (pure ())
  let deploy_succeeds ← do
    if (initcode_succeeded : Bool)
    then
      (do
        (frame_succeeded ()))
    else (pure false)
  let child_left ← do readReg gas_remaining
  let child_state_left ← do readReg state_gas_remaining
  let child_state_spill ← do readReg state_gas_spilled
  let child_refund ← do readReg frame_refund
  (restore_frame checkpoint)
  writeReg gas_remaining (refund_gas (← readReg gas_remaining) child_left)
  (return_child_state_gas child_state_left child_state_spill)
  if (deploy_succeeds : Bool)
  then
    (do
      (record_refund child_refund)
      let ⟨_, ⟨_, deployed_bytes⟩⟩ ← (( do
        if _sailIf0 : (frontier_empty_deposit : Bool) = true
        then
          (pure ((⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩ : (Sigma fun (output_dependentWitness0 : Nat)
            =>
            (Sigma fun (output_dependentWitness1 : Nat) =>
            (OutputSliceFields output_dependentWitness0 output_dependentWitness1)))) : (Sigma fun
            (output_dependentWitness0 : Nat) =>
            (Sigma fun (output_dependentWitness1 : Nat) =>
            (OutputSliceFields output_dependentWitness0 output_dependentWitness1)))))
        else
          (do
            readReg returndata) ) : SailM
        (Sigma fun (output_dependentWitness0 : Nat) =>
        (Sigma fun (output_dependentWitness1 : Nat) =>
        (OutputSliceFields output_dependentWitness0 output_dependentWitness1))) )
      let ⟨_, ⟨_, deployed_code⟩⟩ ← do
        (code_db_intern_output ⟨_, ⟨_, deployed_bytes⟩⟩)
      (k_deploy_code continuation.address ⟨_, ⟨_, deployed_code⟩⟩)
      (k_journal_commit ())
      let deployed_address := (address_to_word continuation.address)
      writeReg stack_top (← (push_word (← readReg stack_top) deployed_address)))
  else
    (do
      (k_journal_revert ())
      if (continuation.new_account_charged : Bool)
      then
        writeReg gas_remaining (← (credit_state_gas_refund (← readReg gas_remaining)
            G_amsterdam_state_new_account))
      else (pure ())
      writeReg stack_top (← (push_word (← readReg stack_top) WORD_ZERO)))
  if (initcode_succeeded : Bool)
  then (returndata_clear ())
  else (pure ())

/-- Applies the pending operation for one completed child frame. -/
/- Type quantifiers: output_dependentWitness1 : Nat, output_dependentWitness0 : Nat, 0 ≤
  output_dependentWitness0 ∧
  0 ≤ output_dependentWitness1 ∧
  (output_dependentWitness0 + output_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def resume_frame (continuation : FrameContinuation) (output : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : SailM Unit := do
  let output_dependentWitness0 := (output).1
  let output_dependentWitness1 := ((output).2).1
  let output := ((output).2).2
  match continuation with
  | .Empty () => (fatal_error ExecutionInvalid)
  | .ResumeCall call => (resume_call call ⟨_, ⟨_, output⟩⟩)
  | .ResumeCreate create => (resume_create create ⟨_, ⟨_, output⟩⟩)

/-- The non-recursive step loop for one complete call tree. It executes
the active frame, resumes suspended parents through
[frame_stack_pop][] as children halt, and returns the top-level
frame's output. Each step's carried state is supplied from the frame
registers and its returned state is assigned back; the handlers
themselves never touch the registers. `STOP`, `SELFDESTRUCT`, and
exceptional halts return the empty slice; `RETURN` and `REVERT` carry
their frozen memory slice in the halt value. -/
def interpret (_ : Unit) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))) := do
  (frame_stack_reset ())
  let interpreting : Bool := true
  let result : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))) :=
    ((⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))))
  let initial_execution_gas ← do readReg gas_remaining
  let initial_state_gas ← do readReg state_gas_remaining
  let initial_call_tree_gas := (initial_execution_gas + initial_state_gas)
  let call_tree_steps_remaining : Int := ((3 *i initial_call_tree_gas) + 2)
  let (call_tree_steps_remaining, interpreting, result) ← (( do
    let loop_vars ← whileFuelM (fuel :=call_tree_steps_remaining) (fun (call_tree_steps_remaining, interpreting, result) => (pure interpreting)) (call_tree_steps_remaining, interpreting, result)
      fun (call_tree_steps_remaining, interpreting, result) => do
        assert true "loop dummy assert"
        let running ← do (is_running ())
        let (interpreting, result) ← (( do
          if _sailIf0 : (running : Bool) = true
          then
            (do
              let (fetched_pc, instruction) ← do (fetch (← readReg pc))
              let (next_pc, next_top, next_mem, next_gas) ← do
                (do
                    let dependentArg3 := (← readReg evm_memory)
                    (execute instruction fetched_pc (← readReg stack_top) dependentArg3
                    (← readReg gas_remaining)))
              writeReg pc next_pc
              writeReg stack_top next_top
              writeReg evm_memory next_mem
              writeReg gas_remaining next_gas
              (pure ((interpreting : Bool), (result : (Sigma fun (k_off : Nat) =>
                (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))))))
          else
            (do
              let ⟨_, ⟨_, output⟩⟩ ← do (frame_output ())
              let continuation ← do (frame_stack_pop ())
              let (interpreting, result) ← (( do
                match continuation with
                | .Empty () =>
                  (let result : (Sigma fun (k_off : Nat) =>
                    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))) :=
                    ((⟨_, ⟨_, output⟩⟩ : (Sigma fun (k_off : Nat) =>
                    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : (Sigma fun
                    (k_off : Nat) => (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))))
                  let interpreting : Bool := false
                  (pure ((interpreting : Bool), (result : (Sigma fun (k_off : Nat) =>
                    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))))))
                | continuation =>
                  (do
                    (resume_frame continuation ⟨_, ⟨_, output⟩⟩)
                    (pure ((interpreting : Bool), (result : (Sigma fun (k_off : Nat) =>
                      (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))))))) ) : SailM
                (Bool × (Sigma fun (k_off : Nat) =>
                (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) )
              (pure ((interpreting : Bool), (result : (Sigma fun (k_off : Nat) =>
                (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))))))) ) : SailM
          (Bool × (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) )
        let call_tree_steps_remaining : Int := (call_tree_steps_remaining -i 1)
        (pure ((call_tree_steps_remaining : Int), (interpreting : Bool), (result : (Sigma fun
          (k_off : Nat) => (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))))))
    (pure loop_vars) ) : SailM
    (Int × Bool × (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) )
  (pure (result : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))))

def undefined_CallSemantics (_ : Unit) : SailM CallSemantics := do
  (pure { takes_value := ← (undefined_bool ()),
          transfers_value := ← (undefined_bool ()),
          uses_target_address := ← (undefined_bool ()),
          inherits_caller_and_value := ← (undefined_bool ()),
          enters_static_context := ← (undefined_bool ()) })

def undefined_CreateSemantics (_ : Unit) : SailM CreateSemantics := do
  (pure { uses_salt := ← (undefined_bool ()) })

