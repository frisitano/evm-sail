import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
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

/-! # The interpreter

The user-space EVM: it fetches and decodes bytecode (Yellow Paper §9),
drives the step loop, and enters sub-frames for the call and create
opcodes. This module specifies that machinery in three layers:

1. **Fetch/decode** — read the opcode at `pc`, decode PUSH immediates
   inline, and map every other byte to its AST node (an undefined byte
   decodes to `INVALID`). Reading past the end of code yields `STOP`
   (YP: implicit halt).
2. **Run loop** — [interpret][] steps fetch-then-execute until the active
   frame stops. A completed child is resumed through its explicit
   [FrameContinuation][type-FrameContinuation]; a completed top-level
   frame ends interpretation.
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

/-- Assembles the `n`-byte big-endian immediate following a `PUSHn`
opcode into a word; bytes past the end of code read as zero. -/
def read_push (n : code_length) : SailM word := do
  (slice_load_n (← readReg frame_code).bytes (← readReg pc) n)

def advance_pc (delta : code_length) : SailM Unit := do
  writeReg pc (← (byte_quantity_add (← readReg pc) delta))

/-- Decodes one non-PUSH opcode byte to its AST node. The three
contiguous families fold to an arity argument — `DUP1`–`DUP16`
(0x80–0x8f), `SWAP1`–`SWAP16` (0x90–0x9f), `LOG0`–`LOG4` (0xa0–0xa4)
— and the remainder is a flat table. Any byte with no defined opcode
decodes to `INVALID`. -/
/- Type quantifiers: opcode : Nat, 0 ≤ opcode ∧ opcode ≤ 255 -/
def decode_simple (opcode : opcode) : SailM ast := do
  let opcode := (opcode).value
  if (((128 ≤b opcode) && (opcode ≤b 143)) : Bool)
  then (pure (DUP ⟨(opcode -i 127)⟩))
  else
    (do
      if (((144 ≤b opcode) && (opcode ≤b 159)) : Bool)
      then (pure (SWAP ⟨(opcode -i 143)⟩))
      else
        (do
          if (((160 ≤b opcode) && (opcode ≤b 164)) : Bool)
          then (pure (LOG ⟨(opcode -i 160)⟩))
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
immediate consumed here; every other byte decodes via
[decode_simple][]. -/
def fetch (_ : Unit) : SailM ast := do
  if ((! (byte_quantity_lt (← readReg pc) (← (frame_code_len ())))) : Bool)
  then (pure (STOP ()))
  else
    (do
      let opcode ← (( do
        (pure (BitVec.toNatInt (← (slice_byte (← readReg frame_code).bytes (← readReg pc)))))
        ) : SailM Nat )
      (advance_pc BYTE_ONE)
      if (((opcode == 95) && (fork_lt (← readReg k_fork) Shanghai)) : Bool)
      then (pure (INVALID ()))
      else
        (do
          if (((95 ≤b opcode) && (opcode ≤b 127)) : Bool)
          then
            (do
              let encoded_size : Nat := (opcode -i 95)
              let size : byte_quantity := (ByteQuantity encoded_size)
              let value ← do (read_push size)
              (advance_pc size)
              (pure (PUSH
                  ((fun (semanticValue0, semanticValue1) => (⟨semanticValue0⟩, semanticValue1)) ((encoded_size, value))))))
          else (decode_simple ⟨opcode⟩)))

/-- Returns the active frame's halt output. -/
def frame_output (_ : Unit) : SailM EvmByteSlice := do
  match (← readReg frame_status) with
  | .Halted (.HaltReturn output) => (pure output)
  | .Halted (.HaltRevert output) => (pure output)
  | _ => (pure EMPTY_SLICE)

/-- Whether the just-finished frame ended successfully: a normal halt
succeeds; a `REVERT` and any exceptional halt do not (their world
effects are rolled back and `CALL`/`CREATE` reports failure). -/
def frame_succeeded (_ : Unit) : SailM Bool := do
  match (← readReg frame_status) with
  | .Halted (.HaltRevert _) => (pure false)
  | .Halted _ => (pure true)
  | .Running () => (pure true)
  | .Exceptional _ => (pure false)

/-- Restores a message-call parent and applies the child's outcome. -/
def resume_call (continuation : CallContinuation) (output : EvmByteSlice) : SailM Unit := do
  writeReg returndata output
  let checkpoint := continuation.checkpoint
  let succeeded ← do (frame_succeeded ())
  let child_left ← do readReg gas_remaining
  let child_refund ← do readReg frame_refund
  (restore_frame checkpoint)
  (refund_gas child_left)
  (returndata_copy_prefix continuation.return_offset continuation.return_length)
  if (succeeded : Bool)
  then
    (do
      (record_refund child_refund)
      (push WORD_ONE))
  else
    (do
      (k_revert checkpoint.state)
      (push WORD_ZERO))

/-- Restores a create parent and either deploys or rolls back the child. -/
def resume_create (continuation : CreateContinuation) (output : EvmByteSlice) : SailM Unit := do
  writeReg returndata output
  let checkpoint := continuation.checkpoint
  let succeeded ← do (frame_succeeded ())
  let child_left ← do readReg gas_remaining
  let child_refund ← do readReg frame_refund
  let deployed_length ← do (returndata_size ())
  let code_ok ← do
    (pure ((byte_quantity_le deployed_length (← (max_code_size ()))) && ((fork_lt
            (← readReg k_fork) London) || ((deployed_length == BYTE_ZERO) || ((← (slice_byte
                  (← readReg returndata) BYTE_ZERO)) != 0xEF#8)))))
  let deposit := (gas_constant_scale_byte_quantity G_codedeposit deployed_length)
  (restore_frame checkpoint)
  (refund_gas child_left)
  if ((succeeded && (code_ok && (gas_cost_le_gas deposit child_left))) : Bool)
  then
    (do
      writeReg gas_remaining (← (gas_sub_cost_or_oog (← readReg gas_remaining) deposit))
      if ((← (is_running ())) : Bool)
      then
        (do
          (record_refund child_refund)
          (k_deploy_code continuation.address (← readReg returndata))
          (push (address_to_word continuation.address)))
      else (pure ()))
  else
    (do
      if (succeeded : Bool)
      then writeReg gas_remaining checkpoint.gas_remaining
      else (pure ())
      (k_revert checkpoint.state)
      (push WORD_ZERO))
  if (succeeded : Bool)
  then (returndata_clear ())
  else (pure ())

/-- Applies the pending operation for one completed child frame. -/
def resume_frame (continuation : FrameContinuation) (output : EvmByteSlice) : SailM Unit := do
  match continuation with
  | .ResumeCall call => (resume_call call output)
  | .ResumeCreate create => (resume_create create output)

/-- The non-recursive step loop for one complete call tree. It executes the
active frame, resumes suspended parents from [frame_stack][] as children
halt, and returns the top-level frame's output. `STOP`, `SELFDESTRUCT`,
and exceptional halts return the empty slice; `RETURN` and `REVERT` carry
their frozen memory slice in the halt value. -/
def interpret (_ : Unit) : SailM EvmByteSlice := do
  (frame_stack_reset ())
  let interpreting : Bool := true
  let result : EvmByteSlice := EMPTY_SLICE
  let (interpreting, result) ← (( do
    let loop_vars ← whileFuelM (fuel :=(2 *i ((GAS_MAX_VALUE).value + 1))) (fun (interpreting, result) => (pure interpreting)) (interpreting, result)
      fun (interpreting, result) => do
        assert true "loop dummy assert"
        let (interpreting, result) ← (( do
          if ((← (is_running ())) : Bool)
          then
            (do
              (execute (← (fetch ())))
              (pure (interpreting, result)))
          else
            (do
              let output ← do (frame_output ())
              let (interpreting, result) ← (( do
                if ((← (frame_stack_is_empty ())) : Bool)
                then
                  (let result : EvmByteSlice := output
                  let interpreting : Bool := false
                  (pure (interpreting, result)))
                else
                  (do
                    (resume_frame (← (frame_stack_pop ())) output)
                    (pure (interpreting, result))) ) : SailM (Bool × EvmByteSlice) )
              (pure (interpreting, result))) ) : SailM (Bool × EvmByteSlice) )
        (pure (interpreting, result))
    (pure loop_vars) ) : SailM (Bool × EvmByteSlice) )
  (pure result)

