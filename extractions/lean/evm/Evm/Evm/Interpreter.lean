import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Primitives.Code
import Evm.Host.EvmByteSlice
import Evm.Host.Kernel.Environment
import Evm.Host.Kernel.Code
import Evm.Host.Kernel.Lifecycle
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
open BalIterEntry

/-! # The interpreter

The user-space EVM: it fetches and decodes bytecode (Yellow Paper §9),
drives the step loop, and enters sub-frames for the call and create
opcodes. This module specifies that machinery in three layers:

1. **Fetch/decode** — read the opcode at `pc`, decode PUSH immediates
   inline, and map every other byte to its AST node (an undefined byte
   decodes to `INVALID`). Reading past the end of code yields `STOP`
   (YP: implicit halt).
2. **Run loop** — [interpret][] steps fetch-then-execute until the active
   frame stops. Popping a pending [FrameContinuation][type-FrameContinuation]
   resumes a completed child; [Empty][type-FrameContinuation] marks completion
   of the top-level frame.
3. **Message calls** — [run_call][] handles
   `CALL`/`CALLCODE`/`DELEGATECALL`/`STATICCALL` (multiplexed on `mode`)
   and [run_create][] handles `CREATE`/`CREATE2`. A sub-call saves the
   parent's user-space frame registers in [frame_stack][], installs the
   child as the active frame, and returns to the single run loop. When the
   child halts, that loop restores and resumes the parent. There is no
   recursive interpreter invocation. All world effects go through kernel syscalls:
   [k_state_checkpoint][] on entry, [k_transfer][] for value, and [k_revert][] on
   failure — the kernel rolls the world back atomically on a reverting
   child. The applicable rules are EIP-150 (63/64ths gas cap + stipend),
   EIP-214 (static-context write protection), EIP-2929 (cold/warm
   access), and EIP-7702 (delegated-code execution). -/

/-- Assembles an `n`-byte big-endian PUSH immediate from a local code cursor;
bytes past the end of code read as zero. -/
/- Type quantifiers: k_ex415796_ : Nat, k_ex415795_ : Nat, code_dependentWitness1 : Nat, code_dependentWitness0
  : Nat, 0 ≤ code_dependentWitness0 ∧ 0 ≤ code_dependentWitness1 ∧
  0 ≤ code_dependentWitness1, 0 ≤ k_ex415795_, 0 ≤ k_ex415796_ ∧ k_ex415796_ ≤ 32 -/
def read_push (code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) (offset : Nat) (n : Nat) : SailM Nat := do
  let code_dependentWitness0 := (code).1
  let code_dependentWitness1 := ((code).2).1
  let code := ((code).2).2
  (slice_load_n ⟨_, ⟨_, code⟩⟩ offset n)

/-- Decodes one non-PUSH opcode byte to its AST node. The three
contiguous families fold to an arity argument — `DUP1`–`DUP16`
(0x80–0x8f), `SWAP1`–`SWAP16` (0x90–0x9f), `LOG0`–`LOG4` (0xa0–0xa4)
— and the remainder is a flat table. Any byte with no defined opcode
decodes to `INVALID`. -/
/- Type quantifiers: opcode : Nat, 0 ≤ opcode ∧ opcode ≤ 255 -/
def decode_simple (opcode : Nat) : SailM ast := do
  if (((128 ≤b opcode) && (opcode ≤b 143)) : Bool)
  then (pure (DUP (opcode - 127)))
  else
    (do
      if (((144 ≤b opcode) && (opcode ≤b 159)) : Bool)
      then (pure (SWAP (opcode - 143)))
      else
        (do
          if (((160 ≤b opcode) && (opcode ≤b 164)) : Bool)
          then (pure (LOG (opcode - 160)))
          else
            (do
              match opcode with
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
                (do
                  if ((fork_gteq (← readReg k_fork) Osaka) : Bool)
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
                (do
                  if ((fork_gteq (← readReg k_fork) London) : Bool)
                  then (pure (BASEFEE ()))
                  else (pure (INVALID ())))
              | 73 =>
                (do
                  if ((fork_gteq (← readReg k_fork) Cancun) : Bool)
                  then (pure (BLOBHASH ()))
                  else (pure (INVALID ())))
              | 74 =>
                (do
                  if ((fork_gteq (← readReg k_fork) Cancun) : Bool)
                  then (pure (BLOBBASEFEE ()))
                  else (pure (INVALID ())))
              | 75 =>
                (do
                  if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
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
                (do
                  if ((fork_gteq (← readReg k_fork) Cancun) : Bool)
                  then (pure (TLOAD ()))
                  else (pure (INVALID ())))
              | 93 =>
                (do
                  if ((fork_gteq (← readReg k_fork) Cancun) : Bool)
                  then (pure (TSTORE ()))
                  else (pure (INVALID ())))
              | 94 =>
                (do
                  if ((fork_gteq (← readReg k_fork) Cancun) : Bool)
                  then (pure (MCOPY ()))
                  else (pure (INVALID ())))
              | 240 => (pure (CREATE ()))
              | 241 => (pure (CALL ()))
              | 242 => (pure (CALLCODE ()))
              | 243 => (pure (RETURN ()))
              | 244 => (pure (DELEGATECALL ()))
              | 245 => (pure (CREATE2 ()))
              | 250 => (pure (STATICCALL ()))
              | 253 => (pure (REVERT ()))
              | 255 => (pure (SELFDESTRUCT ()))
              | _ => (pure (INVALID ())))))

/-- Fetches and decodes the opcode at `pc`, advancing `pc` past the
opcode and any immediate. Past the end of code the frame implicitly
executes `STOP` (YP). `PUSH0`–`PUSH32` (0x5f–0x7f) carry an `n`-byte
immediate; Amsterdam's `DUPN`/`SWAPN`/`EXCHANGE` carry one byte,
zero-padded at end of code. Every other byte decodes via
[decode_simple][]. -/
def fetch (_ : Unit) : SailM ast := do
  let current ← do readReg pc
  let ⟨_, ⟨_, code⟩⟩ ← do (pure (← readReg frame_code).bytes)
  let code_length := code.len
  if ((! (current <b code_length)) : Bool)
  then (pure (STOP ()))
  else
    (do
      let opcode ← (( do
        (pure (BitVec.toNatInt (← (slice_byte ⟨_, ⟨_, code⟩⟩ current)))) ) : SailM Nat )
      let immediate_offset := (current + 1)
      let decoded ← (( do
        if ((← if ((opcode == 95) : Bool)
             then
               (do
                 (pure (fork_lt (← readReg k_fork) Shanghai)))
             else (pure false)) : Bool)
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
                if (((fork_gteq (← readReg k_fork) Amsterdam) && ((230 ≤b opcode) && (opcode ≤b 232))) : Bool)
                then
                  (do
                    let immediate ← do (slice_byte ⟨_, ⟨_, code⟩⟩ immediate_offset)
                    let immediate_valid : Bool :=
                      match opcode with
                      | 230 => (deep_stack_immediate_valid immediate)
                      | 231 => (deep_stack_immediate_valid immediate)
                      | 232 => (exchange_immediate_valid immediate)
                      | _ => false
                    let after_instruction : Nat :=
                      if (immediate_valid : Bool)
                      then (current + 2)
                      else (current + 1)
                    let instruction : ast :=
                      match opcode with
                      | 230 => (DUPN immediate)
                      | 231 => (SWAPN immediate)
                      | 232 => (EXCHANGE immediate)
                      | _ => (INVALID ())
                    (pure (after_instruction, instruction)))
                else (pure (immediate_offset, (← (decode_simple opcode)))))) ) : SailM
        (Nat × ast) )
      let (next_pc, instruction) := decoded
      writeReg pc next_pc
      (pure instruction))

/-- Returns the active frame's halt output. -/
def frame_output (_ : Unit) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  match (← readReg frame_status) with
  | .Halted (.HaltReturn ⟨_, ⟨_, output⟩⟩) =>
    (pure ((⟨_, ⟨_, output⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  | .Halted (.HaltRevert ⟨_, ⟨_, output⟩⟩) =>
    (pure ((⟨_, ⟨_, output⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  | _ =>
    (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))

/-- Whether the just-finished frame ended successfully: a normal halt
succeeds; a `REVERT` and any exceptional halt do not (their world
effects are rolled back and `CALL`/`CREATE` reports failure). -/
def frame_succeeded (_ : Unit) : SailM Bool := do
  match (← readReg frame_status) with
  | .Halted (.HaltRevert ⟨_, ⟨_, _⟩⟩) => (pure false)
  | .Halted _ => (pure true)
  | .Running () => (pure true)
  | .Exceptional _ => (pure false)

/-- Restores a message-call parent and applies the child's outcome. -/
/- Type quantifiers: output_dependentWitness1 : Nat, output_dependentWitness0 : Nat, 0 ≤
  output_dependentWitness0 ∧ 0 ≤ output_dependentWitness1 -/
def resume_call (continuation : CallContinuation) (output : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM Unit := do
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
  (refund_gas child_left)
  (return_child_state_gas child_state_left child_state_spill)
  (returndata_copy_prefix continuation.return_offset continuation.return_length)
  if (succeeded : Bool)
  then
    (do
      (record_refund child_refund)
      (push_word WORD_ONE))
  else
    (do
      (k_revert checkpoint.state)
      if (continuation.new_account_charged : Bool)
      then (credit_state_gas_refund G_amsterdam_state_new_account)
      else (pure ())
      (push_word WORD_ZERO))

/-- Restores a create parent and either deploys or rolls back the child. -/
/- Type quantifiers: output_dependentWitness1 : Nat, output_dependentWitness0 : Nat, 0 ≤
  output_dependentWitness0 ∧ 0 ≤ output_dependentWitness1 -/
def resume_create (continuation : CreateContinuation) (output : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM Unit := do
  let output_dependentWitness0 := (output).1
  let output_dependentWitness1 := ((output).2).1
  let output := ((output).2).2
  writeReg returndata ⟨_, ⟨_, output⟩⟩
  let checkpoint := continuation.checkpoint
  let initcode_succeeded ← do (frame_succeeded ())
  let deployed_length ← do (returndata_size ())
  let deployed_size := deployed_length
  let frontier_empty_deposit : Bool := false
  let frontier_empty_deposit ← (( do
    if (initcode_succeeded : Bool)
    then
      (do
        if ((← if ((! (← (deployed_code_size_allowed deployed_size))) : Bool)
             then (pure true)
             else
               (do
                 if ((fork_gteq (← readReg k_fork) London) : Bool)
                 then
                   (do
                     if ((deployed_size != 0) : Bool)
                     then
                       (do
                         (pure ((← do
                                 let dependentArg0 := (← readReg returndata)
                                 (slice_byte dependentArg0 0)) == 0xEF#8)))
                     else (pure false))
                 else (pure false))) : Bool)
        then
          (do
            (exc_halt OutOfGas)
            (pure frontier_empty_deposit))
        else
          (do
            match (← (code_deployment_execution_cost deployed_length (← readReg gas_remaining))) with
            | .some execution_deposit =>
              (do
                writeReg gas_remaining (← (gas_sub_or_oog (← readReg gas_remaining)
                    execution_deposit))
                (charge_deployment_state_gas (← (code_deployment_state_cost deployed_length)))
                (pure frontier_empty_deposit))
            | none =>
              (do
                if ((fork_lt (← readReg k_fork) Homestead) : Bool)
                then
                  (do
                    writeReg gas_remaining GAS_ZERO
                    let frontier_empty_deposit : Bool := true
                    (pure frontier_empty_deposit))
                else
                  (do
                    (exc_halt OutOfGas)
                    (pure frontier_empty_deposit)))))
    else (pure frontier_empty_deposit) ) : SailM Bool )
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
  (refund_gas child_left)
  (return_child_state_gas child_state_left child_state_spill)
  if (deploy_succeeds : Bool)
  then
    (do
      (record_refund child_refund)
      let ⟨_, ⟨_, deployed_bytes⟩⟩ ← (( do
        if _sailIf0 : (frontier_empty_deposit : Bool) = true
        then
          (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (output_dependentWitness0 : Nat) =>
            (Sigma fun (output_dependentWitness1 : Nat) =>
            (EvmByteSliceFields output_dependentWitness0 output_dependentWitness1)))) : (Sigma fun
            (output_dependentWitness0 : Nat) =>
            (Sigma fun (output_dependentWitness1 : Nat) =>
            (EvmByteSliceFields output_dependentWitness0 output_dependentWitness1)))))
        else
          (do
            readReg returndata) ) : SailM
        (Sigma fun (output_dependentWitness0 : Nat) =>
        (Sigma fun (output_dependentWitness1 : Nat) =>
        (EvmByteSliceFields output_dependentWitness0 output_dependentWitness1))) )
      let ⟨_, ⟨_, deployed_code⟩⟩ := (validated_code_slice ⟨_, ⟨_, deployed_bytes⟩⟩)
      (k_deploy_code continuation.address ⟨_, ⟨_, deployed_code⟩⟩)
      (push_word (address_to_word continuation.address)))
  else
    (do
      (k_revert checkpoint.state)
      if (continuation.new_account_charged : Bool)
      then (credit_state_gas_refund G_amsterdam_state_new_account)
      else (pure ())
      (push_word WORD_ZERO))
  if (initcode_succeeded : Bool)
  then (returndata_clear ())
  else (pure ())

/-- Applies the pending operation for one completed child frame. -/
/- Type quantifiers: output_dependentWitness1 : Nat, output_dependentWitness0 : Nat, 0 ≤
  output_dependentWitness0 ∧ 0 ≤ output_dependentWitness1 -/
def resume_frame (continuation : FrameContinuation) (output : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM Unit := do
  let output_dependentWitness0 := (output).1
  let output_dependentWitness1 := ((output).2).1
  let output := ((output).2).2
  match continuation with
  | .Empty () => sailThrow ((InvalidBlock ExecutionInvalid))
  | .ResumeCall call => (resume_call call ⟨_, ⟨_, output⟩⟩)
  | .ResumeCreate create => (resume_create create ⟨_, ⟨_, output⟩⟩)

/-- The non-recursive step loop for one complete call tree. It executes the
active frame, resumes suspended parents from [frame_stack][] as children
halt, and returns the top-level frame's output. `STOP`, `SELFDESTRUCT`,
and exceptional halts return the empty slice; `RETURN` and `REVERT` carry
their frozen memory slice in the halt value. -/
def interpret (_ : Unit) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  (frame_stack_reset ())
  let interpreting : Bool := true
  let result : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) :=
    ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))
  let (interpreting, result) ← (( do
    let loop_vars ← whileFuelM (fuel :=(2 ^i 64)) (fun (interpreting, result) => (pure interpreting)) (interpreting, result)
      fun (interpreting, result) => do
        assert true "loop dummy assert"
        let (interpreting, result) ← (( do
          if _sailIf0 : ((← (is_running ())) : Bool) = true
          then
            (do
              (execute (← (fetch ())))
              (pure ((interpreting, result) : (Bool × (Sigma fun (k_off : Nat) =>
                (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))))
          else
            (do
              let ⟨_, ⟨_, output⟩⟩ ← do (frame_output ())
              let (interpreting, result) ← (( do
                match (← (frame_stack_pop ())) with
                | .Empty () =>
                  (let result : (Sigma fun (k_off : Nat) =>
                    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) :=
                    ((⟨_, ⟨_, output⟩⟩ : (Sigma fun (k_off : Nat) =>
                    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
                    (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))
                  let interpreting : Bool := false
                  (pure ((interpreting, result) : (Bool × (Sigma fun (k_off : Nat) =>
                    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))))
                | continuation =>
                  (do
                    (resume_frame continuation ⟨_, ⟨_, output⟩⟩)
                    (pure ((interpreting, result) : (Bool × (Sigma fun (k_off : Nat) =>
                      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))))) ) : SailM
                (Bool × (Sigma fun (k_off : Nat) =>
                (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) )
              (pure ((interpreting, result) : (Bool × (Sigma fun (k_off : Nat) =>
                (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))))) ) : SailM
          (Bool × (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) )
        (pure ((interpreting, result) : (Bool × (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))))
    (pure loop_vars) ) : SailM
    (Bool × (Sigma fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))
    )
  (pure (result : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))

