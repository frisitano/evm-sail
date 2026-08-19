import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Primitives.Code
import Evm.Host.RegionAccess
import Evm.Primitives.Fork
import Evm.Host.Code
import Evm.Host.Stack
import Evm.Kernel.Environment
import Evm.Kernel.Storage
import Evm.Kernel.Accounts
import Evm.Kernel.Code
import Evm.Kernel.Selfdestruct
import Evm.Kernel.Lifecycle
import Evm.Evm.Machine
import Evm.Evm.Gas
import Evm.Evm.Precompiles
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
open StackValidation
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
open OpcodeOutcome
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
/- Type quantifiers: k_ex550922_ : Nat, k_ex550921_ : Nat, code_dependentWitness1 : Nat, code_dependentWitness0
  : Nat, 0 ≤ code_dependentWitness0 ∧
  0 ≤ code_dependentWitness1 ∧
  (code_dependentWitness0 + code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ code_dependentWitness1 ∧ (code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤ k_ex550921_
  ∧ k_ex550921_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550922_ ∧ k_ex550922_ ≤ 32 -/
def read_push (code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeRegionSliceFields k_off k_len)))) (offset : Nat) (n : Nat) : SailM Nat := do
  let code_dependentWitness0 := (code).1
  let code_dependentWitness1 := ((code).2).1
  let code := ((code).2).2
  (code_slice_load_n ⟨_, ⟨_, code⟩⟩ offset n)

/-- Reports whether an opcode byte is defined by the active fork. Undefined
opcode bytes remain available here and decode to `INVALID`; this predicate
contains only fork-dependent availability so every interpreter can share
the same deployment rules without duplicating them in its dispatch. -/
/- Type quantifiers: k_ex550924_ : Nat, k_ex550923_ : Nat, 0 ≤ k_ex550923_ ∧ k_ex550923_ ≤ 255, 0
  ≤ k_ex550924_ ∧ k_ex550924_ ≤ 16 -/
def opcode_available (opcode : Nat) (fork : Nat) : Bool :=
  match opcode with
  | 30 => (fork ≥b Osaka)
  | 72 => (fork ≥b London)
  | 73 => (fork ≥b Cancun)
  | 74 => (fork ≥b Cancun)
  | 75 => (fork ≥b Amsterdam)
  | 92 => (fork ≥b Cancun)
  | 93 => (fork ≥b Cancun)
  | 94 => (fork ≥b Cancun)
  | 95 => (fork ≥b Shanghai)
  | 230 => (fork ≥b Amsterdam)
  | 231 => (fork ≥b Amsterdam)
  | 232 => (fork ≥b Amsterdam)
  | _ => true

/-- Decodes one PUSH immediate and returns the semantic program counter after
all encoded immediate bytes. Missing code bytes contribute zero to the
value but still belong to the instruction encoding. -/
/- Type quantifiers: k_ex550933_ : Nat, k_ex550932_ : Nat, frame_code_dependentWitness1 : Nat, frame_code_dependentWitness0
  : Nat, 0 ≤ frame_code_dependentWitness0 ∧
  0 ≤ frame_code_dependentWitness1 ∧
  (frame_code_dependentWitness0 + frame_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ frame_code_dependentWitness1 ∧ (frame_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex550932_ ∧ k_ex550932_ ≤ (2 ^ 32 - 1 - 32), 0 ≤ k_ex550933_ ∧ k_ex550933_ ≤ 32 -/
def decode_push_immediate (frame_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (immediate_offset : Nat) (width : Nat) : SailM (Nat × Nat) := do
  let frame_code_dependentWitness0 := (frame_code).1
  let frame_code_dependentWitness1 := ((frame_code).2).1
  let frame_code := ((frame_code).2).2
  let bytes := (code_bytes frame_code)
  let value ← do (read_push ⟨_, ⟨_, bytes⟩⟩ immediate_offset width)
  (pure ((immediate_offset + width), value))

/-- Decodes the immediate of an Amsterdam deep-stack instruction. A valid
immediate advances the counter; an invalid immediate remains unconsumed so
the handler can report `InvalidOpcode` with the canonical instruction
boundary. Reads beyond code are zero-padded. -/
/- Type quantifiers: k_ex550941_ : Nat, frame_code_dependentWitness1 : Nat, frame_code_dependentWitness0
  : Nat, 0 ≤ frame_code_dependentWitness0 ∧
  0 ≤ frame_code_dependentWitness1 ∧
  (frame_code_dependentWitness0 + frame_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ frame_code_dependentWitness1 ∧ (frame_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex550941_ ∧ k_ex550941_ ≤ (2 ^ 32 - 1 - 32) -/
def decode_deep_immediate (frame_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (immediate_offset : Nat) (operation : DeepStackOperation) : SailM (Nat × (BitVec 8)) := do
  let frame_code_dependentWitness0 := (frame_code).1
  let frame_code_dependentWitness1 := ((frame_code).2).1
  let frame_code := ((frame_code).2).2
  let bytes := (code_bytes frame_code)
  let immediate ← do (code_slice_byte ⟨_, ⟨_, bytes⟩⟩ immediate_offset)
  let immediate_valid := (deep_stack_operation_immediate_valid operation immediate)
  let next_pc : Nat :=
    if (immediate_valid : Bool)
    then (immediate_offset + 1)
    else immediate_offset
  (pure (next_pc, immediate))

/-- Executes an encoded PUSH instruction from its immediate cursor. This is
the shared semantic boundary used by raw-byte interpreters: decoding, PC
progression, stack validation, gas charging, and the stack effect remain
generated from Sail. -/
/- Type quantifiers: k_ex550951_ : Nat, k_ex550950_ : Nat, k_ex550949_ : Nat, frame_code_dependentWitness1
  : Nat, frame_code_dependentWitness0 : Nat, 0 ≤ frame_code_dependentWitness0 ∧
  0 ≤ frame_code_dependentWitness1 ∧
  (frame_code_dependentWitness0 + frame_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ frame_code_dependentWitness1 ∧ (frame_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex550949_ ∧ k_ex550949_ ≤ 255, 0 ≤ k_ex550950_ ∧ k_ex550950_ ≤ (2 ^ 32 - 1 - 32), 0
  ≤ k_ex550951_ ∧ k_ex550951_ ≤ (2 ^ 64 - 1) -/
def execute_push_encoded (frame_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (opcode : Nat) (immediate_offset : Nat) (execution_gas : Nat) (sp : StackPointer) : SailM (Nat × Nat × StackPointer × OpcodeOutcome) := do
  let frame_code_dependentWitness0 := (frame_code).1
  let frame_code_dependentWitness1 := ((frame_code).2).1
  let frame_code := ((frame_code).2).2
  if (((95 ≤b opcode) && (opcode ≤b 127)) : Bool)
  then
    (do
      let width : Nat := (opcode - 95)
      let (next_pc, value) ← do
        (decode_push_immediate ⟨_, ⟨_, frame_code⟩⟩ immediate_offset width)
      let (gas_after, sp_after, status_after) ← do (execute_push execution_gas sp width value)
      (pure (next_pc, gas_after, sp_after, status_after)))
  else
    (let (gas_after, status_after) := (execute_invalid execution_gas)
    (pure (immediate_offset, gas_after, sp, status_after)))

/-- Executes one opcode from the `DUP1` through `DUP16` family. The raw-byte
interpreter routes the family here without reproducing its index
relationship. -/
/- Type quantifiers: k_ex550953_ : Nat, k_ex550952_ : Nat, 0 ≤ k_ex550952_ ∧ k_ex550952_ ≤ 255, 0
  ≤ k_ex550953_ ∧ k_ex550953_ ≤ (2 ^ 64 - 1) -/
def execute_dup_encoded (opcode : Nat) (execution_gas : Nat) (sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  if (((128 ≤b opcode) && (opcode ≤b 143)) : Bool)
  then (execute_dup execution_gas sp (opcode - 127))
  else
    (let (gas_after, status_after) := (execute_invalid execution_gas)
    (pure (gas_after, sp, status_after)))

/-- Executes one opcode from the `SWAP1` through `SWAP16` family. -/
/- Type quantifiers: k_ex550955_ : Nat, k_ex550954_ : Nat, 0 ≤ k_ex550954_ ∧ k_ex550954_ ≤ 255, 0
  ≤ k_ex550955_ ∧ k_ex550955_ ≤ (2 ^ 64 - 1) -/
def execute_swap_encoded (opcode : Nat) (execution_gas : Nat) (sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  if (((144 ≤b opcode) && (opcode ≤b 159)) : Bool)
  then (execute_swap execution_gas sp (opcode - 143))
  else
    (let (gas_after, status_after) := (execute_invalid execution_gas)
    (pure (gas_after, sp, status_after)))

/-- Executes one opcode from the `LOG0` through `LOG4` family. -/
/- Type quantifiers: k_ex550960_ : Nat, k_ex550959_ : Nat, k_ex550958_ : Nat, k_ex550957_ : Nat, k_ex550956_
  : Bool, 0 ≤ k_ex550957_ ∧ k_ex550957_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550958_ ∧
  k_ex550958_ ≤ 255, 0 ≤ k_ex550959_ ∧ k_ex550959_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex550960_ ∧
  k_ex550960_ ≤ (2 ^ 32 - 1) -/
def execute_log_encoded (carried_address : (Vector (BitVec 8) 20)) (carried_is_static : Bool) (memory_base : Nat) (opcode : Nat) (execution_gas : Nat) (sp : StackPointer) (memory : Nat) : SailM (Nat × StackPointer × Nat × OpcodeOutcome) := do
  if (((160 ≤b opcode) && (opcode ≤b 164)) : Bool)
  then
    (execute_log carried_address carried_is_static memory_base (opcode - 160) execution_gas sp
      memory)
  else
    (let (gas_after, status_after) := (execute_invalid execution_gas)
    (pure (gas_after, sp, memory, status_after)))

/-- Executes an encoded Amsterdam deep-stack instruction from its immediate
cursor. Opcode classification and immediate validity are specification
semantics; the raw-byte interpreter supplies only the opcode byte. -/
/- Type quantifiers: k_ex550970_ : Nat, k_ex550969_ : Nat, k_ex550968_ : Nat, frame_code_dependentWitness1
  : Nat, frame_code_dependentWitness0 : Nat, 0 ≤ frame_code_dependentWitness0 ∧
  0 ≤ frame_code_dependentWitness1 ∧
  (frame_code_dependentWitness0 + frame_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ frame_code_dependentWitness1 ∧ (frame_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex550968_ ∧ k_ex550968_ ≤ 255, 0 ≤ k_ex550969_ ∧ k_ex550969_ ≤ (2 ^ 32 - 1 - 32), 0
  ≤ k_ex550970_ ∧ k_ex550970_ ≤ (2 ^ 64 - 1) -/
def execute_deep_stack_encoded (frame_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (opcode : Nat) (immediate_offset : Nat) (execution_gas : Nat) (sp : StackPointer) : SailM (Nat × Nat × StackPointer × OpcodeOutcome) := do
  let frame_code_dependentWitness0 := (frame_code).1
  let frame_code_dependentWitness1 := ((frame_code).2).1
  let frame_code := ((frame_code).2).2
  let operation := (deep_stack_operation opcode)
  let (next_pc, immediate) ← do
    (decode_deep_immediate ⟨_, ⟨_, frame_code⟩⟩ immediate_offset operation)
  let result ← (( do
    match operation with
    | .DeepStackDuplicate => (execute_dupn execution_gas sp immediate)
    | .DeepStackSwap => (execute_swapn execution_gas sp immediate)
    | .DeepStackExchange => (execute_exchange execution_gas sp immediate)
    | .NotDeepStackOperation =>
      (let (gas_after, status_after) := (execute_invalid execution_gas)
      (pure (gas_after, sp, status_after))) ) : SailM (Nat × StackPointer × OpcodeOutcome) )
  let (gas_after, sp_after, status_after) := result
  (pure (next_pc, gas_after, sp_after, status_after))

/-- Decodes one non-PUSH opcode byte to its AST node. The three
contiguous families fold to an arity argument — `DUP1`–`DUP16`
(0x80–0x8f), `SWAP1`–`SWAP16` (0x90–0x9f), `LOG0`–`LOG4` (0xa0–0xa4)
— and the remainder is a flat table. Any byte with no defined opcode
decodes to `INVALID`. -/
/- Type quantifiers: k_ex550972_ : Nat, k_ex550971_ : Nat, 0 ≤ k_ex550971_ ∧ k_ex550971_ ≤ 255, 0
  ≤ k_ex550972_ ∧ k_ex550972_ ≤ 16 -/
def decode_simple (opcode : Nat) (fork : Nat) : ast :=
  let available := (opcode_available opcode fork)
  if ((! available) : Bool)
  then (INVALID ())
  else
    (if (((128 ≤b opcode) && (opcode ≤b 143)) : Bool)
    then (DUP (opcode - 127))
    else
      (if (((144 ≤b opcode) && (opcode ≤b 159)) : Bool)
      then (SWAP (opcode - 143))
      else
        (if (((160 ≤b opcode) && (opcode ≤b 164)) : Bool)
        then (LOG (opcode - 160))
        else
          (match opcode with
          | 0 => (STOP ())
          | 1 => (ADD ())
          | 2 => (MUL ())
          | 3 => (SUB ())
          | 4 => (DIV ())
          | 5 => (SDIV ())
          | 6 => (MOD ())
          | 7 => (SMOD ())
          | 8 => (ADDMOD ())
          | 9 => (MULMOD ())
          | 10 => (EXP ())
          | 11 => (SIGNEXTEND ())
          | 16 => (LT ())
          | 17 => (GT ())
          | 18 => (SLT ())
          | 19 => (SGT ())
          | 20 => (EQ ())
          | 21 => (ISZERO ())
          | 22 => (AND ())
          | 23 => (OR ())
          | 24 => (XOR ())
          | 25 => (NOT ())
          | 26 => (BYTE ())
          | 27 => (SHL ())
          | 28 => (SHR ())
          | 29 => (SAR ())
          | 30 => (CLZ ())
          | 32 => (KECCAK256 ())
          | 48 => (ADDRESS ())
          | 49 => (BALANCE ())
          | 50 => (ORIGIN ())
          | 51 => (CALLER ())
          | 52 => (CALLVALUE ())
          | 53 => (CALLDATALOAD ())
          | 54 => (CALLDATASIZE ())
          | 55 => (CALLDATACOPY ())
          | 56 => (CODESIZE ())
          | 57 => (CODECOPY ())
          | 58 => (GASPRICE ())
          | 59 => (EXTCODESIZE ())
          | 60 => (EXTCODECOPY ())
          | 61 => (RETURNDATASIZE ())
          | 62 => (RETURNDATACOPY ())
          | 63 => (EXTCODEHASH ())
          | 64 => (BLOCKHASH ())
          | 65 => (COINBASE ())
          | 66 => (TIMESTAMP ())
          | 67 => (NUMBER ())
          | 68 => (PREVRANDAO ())
          | 69 => (GASLIMIT ())
          | 70 => (CHAINID ())
          | 71 => (SELFBALANCE ())
          | 72 => (BASEFEE ())
          | 73 => (BLOBHASH ())
          | 74 => (BLOBBASEFEE ())
          | 75 => (SLOTNUM ())
          | 80 => (POP ())
          | 81 => (MLOAD ())
          | 82 => (MSTORE ())
          | 83 => (MSTORE8 ())
          | 84 => (SLOAD ())
          | 85 => (SSTORE ())
          | 86 => (JUMP ())
          | 87 => (JUMPI ())
          | 88 => (PC ())
          | 89 => (MSIZE ())
          | 90 => (GAS ())
          | 91 => (JUMPDEST ())
          | 92 => (TLOAD ())
          | 93 => (TSTORE ())
          | 94 => (MCOPY ())
          | 240 => (opcode_CREATE ())
          | 241 => (CALL ())
          | 242 => (CALLCODE ())
          | 243 => (RETURN ())
          | 244 => (DELEGATECALL ())
          | 245 => (CREATE2 ())
          | 250 => (STATICCALL ())
          | 253 => (REVERT ())
          | 255 => (SELFDESTRUCT ())
          | _ => (INVALID ())))))

/-- Fetches and decodes the opcode at the carried program counter,
returning the counter advanced past the opcode and any immediate.
Past the end of code the frame implicitly executes `STOP` (YP).
`PUSH0`–`PUSH32` (0x5f–0x7f) carry an `n`-byte immediate; Amsterdam's
`DUPN`/`SWAPN`/`EXCHANGE` carry one byte, zero-padded at end of code.
Every other byte decodes via [decode_simple][]. -/
/- Type quantifiers: k_ex550981_ : Nat, k_ex550980_ : Nat, frame_code_dependentWitness1 : Nat, frame_code_dependentWitness0
  : Nat, 0 ≤ frame_code_dependentWitness0 ∧
  0 ≤ frame_code_dependentWitness1 ∧
  (frame_code_dependentWitness0 + frame_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ frame_code_dependentWitness1 ∧ (frame_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex550980_ ∧ k_ex550980_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550981_ ∧ k_ex550981_ ≤ 16 -/
def fetch (frame_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (current : Nat) (fork : Nat) : SailM (Nat × ast) := do
  let frame_code_dependentWitness0 := (frame_code).1
  let frame_code_dependentWitness1 := ((frame_code).2).1
  let frame_code := ((frame_code).2).2
  let analyzed := frame_code
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
      let available := (opcode_available opcode fork)
      if ((! available) : Bool)
      then (pure (immediate_offset, (INVALID ())))
      else
        (do
          if (((95 ≤b opcode) && (opcode ≤b 127)) : Bool)
          then
            (do
              let size : Nat := (opcode - 95)
              let (after_immediate, value) ← do
                (decode_push_immediate ⟨_, ⟨_, frame_code⟩⟩ immediate_offset size)
              (pure (after_immediate, (PUSH (size, value)))))
          else
            (do
              let deep_operation := (deep_stack_operation opcode)
              match deep_operation with
              | .NotDeepStackOperation => (pure (immediate_offset, (decode_simple opcode fork)))
              | operation =>
                (do
                  let (after_instruction, immediate) ← do
                    (decode_deep_immediate ⟨_, ⟨_, frame_code⟩⟩ immediate_offset operation)
                  let instruction : ast :=
                    match operation with
                    | .DeepStackDuplicate => (DUPN immediate)
                    | .DeepStackSwap => (SWAPN immediate)
                    | .DeepStackExchange => (EXCHANGE immediate)
                    | .NotDeepStackOperation => (decode_simple opcode fork)
                  (pure (after_instruction, instruction))))))

/-- Returns the active frame's halt output. -/
def frame_output (frame_status : FrameStatus) : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))) :=
  match frame_status with
  | .Halted (.HaltReturn ⟨_, ⟨_, output⟩⟩) =>
    ((⟨_, ⟨_, output⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))))
  | .Halted (.HaltRevert ⟨_, ⟨_, output⟩⟩) =>
    ((⟨_, ⟨_, output⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))))
  | _ =>
    ((⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩ : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
    (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len))))

/-- Whether the just-finished frame ended successfully: a normal halt
succeeds; a `REVERT` and any exceptional halt do not (their world
effects are rolled back and `CALL`/`CREATE` reports failure). -/
def frame_succeeded (frame_status : FrameStatus) : Bool :=
  match frame_status with
  | .Halted (.HaltRevert ⟨_, ⟨_, _⟩⟩) => false
  | .Halted _ => true
  | .Running () => true
  | .Exceptional _ => false

/-- Restores a message-call parent and applies the child's outcome. -/
/- Type quantifiers: k_ex550994_ : Int, k_ex550993_ : Nat, k_ex550992_ : Nat, k_ex550991_ : Nat, k_ex550990_
  : Nat, output_dependentWitness1 : Nat, output_dependentWitness0 : Nat, 0 ≤
  output_dependentWitness0 ∧
  0 ≤ output_dependentWitness1 ∧
  (output_dependentWitness0 + output_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ k_ex550990_ ∧
  k_ex550990_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex550991_ ∧ k_ex550991_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex550992_ ∧ k_ex550992_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex550993_ ∧ k_ex550993_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1))))
  ≤ k_ex550994_ ∧ k_ex550994_ ≤ (199 * (2 ^ 64 - 1)) -/
def resume_call (continuation : CallContinuation) (output : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) (child_memory_base : Nat) (child_gas : Nat) (child_state_gas : Nat) (child_state_spill : Nat) (child_refund : Int) (child_status : FrameStatus) : SailM FrameTransition := do
  let output_dependentWitness0 := (output).1
  let output_dependentWitness1 := ((output).2).1
  let output := ((output).2).2
  let checkpoint := continuation.checkpoint
  let succeeded := (frame_succeeded child_status)
  (operand_stack_pop_frame ())
  let parent_memory_base ← do (memory_parent_base child_memory_base checkpoint.memory_height)
  let parent_gas ← do (refund_gas checkpoint.gas_remaining child_gas)
  let parent_state_gas : Nat := checkpoint.state_gas_remaining
  let parent_state_spill : Nat := checkpoint.state_gas_spilled
  let (tup__0, tup__1) ← do
    (return_child_state_gas parent_state_gas parent_state_spill child_state_gas child_state_spill)
  let parent_state_gas : Nat := tup__0
  let parent_state_spill : Nat := tup__1
  (pure ())
  let parent_refund : Int := checkpoint.refund
  let parent_sp : StackPointer := checkpoint.stack_top
  let return_destination ← do (memory_absolute parent_memory_base continuation.return_offset)
  (returndata_copy_prefix ⟨_, ⟨_, output⟩⟩ return_destination continuation.return_length)
  let (parent_gas, parent_refund, parent_sp, parent_state_gas, parent_state_spill) ← (( do
    if (succeeded : Bool)
    then
      (do
        let parent_refund ← (record_refund parent_refund child_refund)
        (k_journal_commit ())
        let parent_sp ← (stack_top_advance parent_sp 1)
        (write_stack_word parent_sp WORD_ONE)
        (pure (parent_gas, parent_refund, parent_sp, parent_state_gas, parent_state_spill)))
    else
      (do
        (k_journal_revert ())
        let (parent_gas, parent_state_gas, parent_state_spill) ← (( do
          if (continuation.new_account_charged : Bool)
          then
            (do
              let (tup__0, tup__1, tup__2) ← do
                (credit_state_gas_refund parent_gas parent_state_gas parent_state_spill
                  G_amsterdam_state_new_account)
              let parent_gas : Nat := tup__0
              let parent_state_gas : Nat := tup__1
              let parent_state_spill : Nat := tup__2
              (pure (parent_gas, parent_state_gas, parent_state_spill)))
          else (pure (parent_gas, parent_state_gas, parent_state_spill)) ) : SailM
          (Nat × Nat × Nat) )
        let parent_sp ← (stack_top_advance parent_sp 1)
        (write_stack_word parent_sp WORD_ZERO)
        (pure (parent_gas, parent_refund, parent_sp, parent_state_gas, parent_state_spill))) ) :
    SailM (Nat × Int × StackPointer × Nat × Nat) )
  (pure { pc := checkpoint.pc,
          gas_remaining := parent_gas,
          state_gas_remaining := parent_state_gas,
          state_gas_spilled := parent_state_spill,
          refund := parent_refund,
          status := checkpoint.status,
          stack_top := parent_sp,
          memory_base := parent_memory_base,
          memory_height := checkpoint.memory_height,
          message := checkpoint.message,
          code := checkpoint.code,
          calldata := checkpoint.calldata,
          returndata := ⟨_, ⟨_, output⟩⟩ })

/-- Restores a create parent and either deploys or rolls back the child. -/
/- Type quantifiers: k_ex551006_ : Nat, k_ex551005_ : Int, k_ex551004_ : Nat, k_ex551003_ : Nat, k_ex551002_
  : Nat, k_ex551001_ : Nat, output_dependentWitness1 : Nat, output_dependentWitness0 : Nat, 0 ≤
  output_dependentWitness0 ∧
  0 ≤ output_dependentWitness1 ∧
  (output_dependentWitness0 + output_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ k_ex551001_ ∧
  k_ex551001_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex551002_ ∧ k_ex551002_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex551003_ ∧ k_ex551003_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551004_ ∧ k_ex551004_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1))))
  ≤ k_ex551005_ ∧ k_ex551005_ ≤ (199 * (2 ^ 64 - 1)), 0 ≤ k_ex551006_ ∧
  k_ex551006_ ≤ (2 ^ 64 - 1) -/
def resume_create (continuation : CreateContinuation) (output : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) (child_memory_base : Nat) (child_gas : Nat) (child_state_gas : Nat) (child_state_spill : Nat) (child_refund : Int) (child_status : FrameStatus) (child_state_gas_reservoir : Nat) : SailM FrameTransition := do
  let output_dependentWitness0 := (output).1
  let output_dependentWitness1 := ((output).2).1
  let output := ((output).2).2
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let checkpoint := continuation.checkpoint
  let initcode_succeeded := (frame_succeeded child_status)
  let deployed_length := (returndata_size ⟨_, ⟨_, output⟩⟩)
  let deployed_size := deployed_length
  let frontier_empty_deposit : Bool := false
  let settled_child_gas : Nat := child_gas
  let settled_child_state_gas : Nat := child_state_gas
  let settled_child_state_spill : Nat := child_state_spill
  let settled_child_status : FrameStatus := child_status
  let (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status) ← (( do
    if (initcode_succeeded : Bool)
    then
      (do
        let deployed_size_allowed ← do (deployed_code_size_allowed deployed_size)
        let invalid_deployed_size := (! deployed_size_allowed)
        let prohibited_prefix ← do
          if ((deployed_size != 0) : Bool)
          then
            (do
              let first_byte ← do (output_byte ⟨_, ⟨_, output⟩⟩ 0)
              (pure (first_byte == 0xEF#8)))
          else (pure false)
        let (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status) ← (( do
          if ((invalid_deployed_size || ((profile.fork ≥b London) && prohibited_prefix)) : Bool)
          then
            (do
              let settled_child_gas : Nat := GAS_ZERO
              let exceptional ← do
                (exceptional_state settled_child_state_gas settled_child_state_spill
                  child_state_gas_reservoir OutOfGas)
              let settled_child_state_gas : Nat := exceptional.state_gas_remaining
              let settled_child_state_spill : Nat := exceptional.state_gas_spilled
              let settled_child_status : FrameStatus := exceptional.status
              (pure (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status)))
          else
            (do
              let deployment_charge ← do
                (code_deployment_execution_cost deployed_length settled_child_gas)
              let (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status) ← (( do
                if (deployment_charge.affordable : Bool)
                then
                  (do
                    let execution_deposit := deployment_charge.cost
                    let settled_child_gas : Nat := (gas_sub settled_child_gas execution_deposit)
                    let state_deposit ← do (code_deployment_state_cost deployed_length)
                    let deployment_halt : Bool := false
                    let (tup__0, tup__1, tup__2, tup__3) ← do
                      (charge_state_gas settled_child_gas settled_child_state_gas
                        settled_child_state_spill state_deposit)
                    let deployment_halt : Bool := tup__0
                    let settled_child_gas : Nat := tup__1
                    let settled_child_state_gas : Nat := tup__2
                    let settled_child_state_spill : Nat := tup__3
                    (pure ())
                    let (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status) ← (( do
                      if (deployment_halt : Bool)
                      then
                        (do
                          let settled_child_gas : Nat := GAS_ZERO
                          let exceptional ← do
                            (exceptional_state settled_child_state_gas settled_child_state_spill
                              child_state_gas_reservoir OutOfGas)
                          let settled_child_state_gas : Nat := exceptional.state_gas_remaining
                          let settled_child_state_spill : Nat := exceptional.state_gas_spilled
                          let settled_child_status : FrameStatus := exceptional.status
                          (pure (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status)))
                      else
                        (pure (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status))
                      ) : SailM (Nat × Nat × Nat × FrameStatus) )
                    (pure (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status)))
                else
                  (do
                    let settled_child_gas : Nat := GAS_ZERO
                    let exceptional ← do
                      (exceptional_state settled_child_state_gas settled_child_state_spill
                        child_state_gas_reservoir OutOfGas)
                    let settled_child_state_gas : Nat := exceptional.state_gas_remaining
                    let settled_child_state_spill : Nat := exceptional.state_gas_spilled
                    let settled_child_status : FrameStatus := exceptional.status
                    (pure (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status)))
                ) : SailM (Nat × Nat × Nat × FrameStatus) )
              (pure (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status)))
          ) : SailM (Nat × Nat × Nat × FrameStatus) )
        (pure (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status)))
    else
      (pure (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status))
    ) : SailM (Nat × Nat × Nat × FrameStatus) )
  let deploy_succeeds : Bool := false
  let deploy_succeeds : Bool :=
    if (initcode_succeeded : Bool)
    then (frame_succeeded settled_child_status)
    else deploy_succeeds
  (operand_stack_pop_frame ())
  let parent_memory_base ← do (memory_parent_base child_memory_base checkpoint.memory_height)
  let parent_gas ← do (refund_gas checkpoint.gas_remaining settled_child_gas)
  let parent_state_gas : Nat := checkpoint.state_gas_remaining
  let parent_state_spill : Nat := checkpoint.state_gas_spilled
  let (tup__0, tup__1) ← do
    (return_child_state_gas parent_state_gas parent_state_spill settled_child_state_gas
      settled_child_state_spill)
  let parent_state_gas : Nat := tup__0
  let parent_state_spill : Nat := tup__1
  (pure ())
  let parent_refund : Int := checkpoint.refund
  let parent_sp : StackPointer := checkpoint.stack_top
  let (parent_gas, parent_refund, parent_sp, parent_state_gas, parent_state_spill) ← (( do
    if (deploy_succeeds : Bool)
    then
      (do
        let parent_refund ← (record_refund parent_refund child_refund)
        let ⟨_, ⟨_, deployed_bytes⟩⟩ : (Sigma fun (output_dependentWitness0 : Nat) =>
          (Sigma fun (output_dependentWitness1 : Nat) =>
          (OutputSliceFields output_dependentWitness0 output_dependentWitness1))) :=
          if _sailIf0 : (frontier_empty_deposit : Bool) = true
          then
            ((⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩ : (Sigma fun (output_dependentWitness0 : Nat) =>
            (Sigma fun (output_dependentWitness1 : Nat) =>
            (OutputSliceFields output_dependentWitness0 output_dependentWitness1)))) : (Sigma fun
            (output_dependentWitness0 : Nat) =>
            (Sigma fun (output_dependentWitness1 : Nat) =>
            (OutputSliceFields output_dependentWitness0 output_dependentWitness1))))
          else
            ((⟨_, ⟨_, output⟩⟩ : (Sigma fun (output_dependentWitness0 : Nat) =>
            (Sigma fun (output_dependentWitness1 : Nat) =>
            (OutputSliceFields output_dependentWitness0 output_dependentWitness1)))) : (Sigma fun
            (output_dependentWitness0 : Nat) =>
            (Sigma fun (output_dependentWitness1 : Nat) =>
            (OutputSliceFields output_dependentWitness0 output_dependentWitness1))))
        let ⟨_, ⟨_, deployed_code⟩⟩ ← do
          (code_db_intern_output ⟨_, ⟨_, deployed_bytes⟩⟩)
        (k_deploy_code continuation.address ⟨_, ⟨_, deployed_code⟩⟩)
        (k_journal_commit ())
        let deployed_address := (address_to_word continuation.address)
        let parent_sp ← (stack_top_advance parent_sp 1)
        (write_stack_word parent_sp deployed_address)
        (pure (parent_gas, parent_refund, parent_sp, parent_state_gas, parent_state_spill)))
    else
      (do
        (k_journal_revert ())
        let (parent_gas, parent_state_gas, parent_state_spill) ← (( do
          if (continuation.new_account_charged : Bool)
          then
            (do
              let (tup__0, tup__1, tup__2) ← do
                (credit_state_gas_refund parent_gas parent_state_gas parent_state_spill
                  G_amsterdam_state_new_account)
              let parent_gas : Nat := tup__0
              let parent_state_gas : Nat := tup__1
              let parent_state_spill : Nat := tup__2
              (pure (parent_gas, parent_state_gas, parent_state_spill)))
          else (pure (parent_gas, parent_state_gas, parent_state_spill)) ) : SailM
          (Nat × Nat × Nat) )
        let parent_sp ← (stack_top_advance parent_sp 1)
        (write_stack_word parent_sp WORD_ZERO)
        (pure (parent_gas, parent_refund, parent_sp, parent_state_gas, parent_state_spill))) ) :
    SailM (Nat × Int × StackPointer × Nat × Nat) )
  let ⟨_, ⟨_, parent_returndata⟩⟩ :=
    if _sailIf0 : (initcode_succeeded : Bool) = true
    then
      ((returndata_clear ()) : (Sigma fun (output_dependentWitness0 : Nat) =>
      (Sigma fun (output_dependentWitness1 : Nat) =>
      (OutputSliceFields output_dependentWitness0 output_dependentWitness1))))
    else
      ((⟨_, ⟨_, output⟩⟩ : (Sigma fun (output_dependentWitness0 : Nat) =>
      (Sigma fun (output_dependentWitness1 : Nat) =>
      (OutputSliceFields output_dependentWitness0 output_dependentWitness1)))) : (Sigma fun
      (output_dependentWitness0 : Nat) =>
      (Sigma fun (output_dependentWitness1 : Nat) =>
      (OutputSliceFields output_dependentWitness0 output_dependentWitness1))))
  (pure { pc := checkpoint.pc,
          gas_remaining := parent_gas,
          state_gas_remaining := parent_state_gas,
          state_gas_spilled := parent_state_spill,
          refund := parent_refund,
          status := checkpoint.status,
          stack_top := parent_sp,
          memory_base := parent_memory_base,
          memory_height := checkpoint.memory_height,
          message := checkpoint.message,
          code := checkpoint.code,
          calldata := checkpoint.calldata,
          returndata := ⟨_, ⟨_, parent_returndata⟩⟩ })

/-- Applies the pending operation for one completed child frame. -/
/- Type quantifiers: k_ex551018_ : Nat, k_ex551017_ : Int, k_ex551016_ : Nat, k_ex551015_ : Nat, k_ex551014_
  : Nat, k_ex551013_ : Nat, output_dependentWitness1 : Nat, output_dependentWitness0 : Nat, 0 ≤
  output_dependentWitness0 ∧
  0 ≤ output_dependentWitness1 ∧
  (output_dependentWitness0 + output_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ k_ex551013_ ∧
  k_ex551013_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex551014_ ∧ k_ex551014_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex551015_ ∧ k_ex551015_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551016_ ∧ k_ex551016_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1))))
  ≤ k_ex551017_ ∧ k_ex551017_ ≤ (199 * (2 ^ 64 - 1)), 0 ≤ k_ex551018_ ∧
  k_ex551018_ ≤ (2 ^ 64 - 1) -/
def resume_frame (continuation : FrameContinuation) (output : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) (child_memory_base : Nat) (child_gas : Nat) (child_state_gas : Nat) (child_state_spill : Nat) (child_refund : Int) (child_status : FrameStatus) (child_state_gas_reservoir : Nat) : SailM FrameTransition := do
  let output_dependentWitness0 := (output).1
  let output_dependentWitness1 := ((output).2).1
  let output := ((output).2).2
  match continuation with
  | .Empty () => (fatal_error ExecutionInvalid)
  | .ResumeCall call =>
    (resume_call call ⟨_, ⟨_, output⟩⟩ child_memory_base child_gas child_state_gas
      child_state_spill child_refund child_status)
  | .ResumeCreate create =>
    (resume_create create ⟨_, ⟨_, output⟩⟩ child_memory_base child_gas child_state_gas
      child_state_spill child_refund child_status child_state_gas_reservoir)

/-- Defunctionalizes each CALL-family opcode into the data consumed by the
shared message-call interpreter. -/
def call_semantics (kind : CallKind) : CallSemantics :=
  match kind with
  | .Call =>
    { takes_value := true,
      transfers_value := true,
      uses_target_address := true,
      inherits_caller_and_value := false,
      enters_static_context := false }
  | .CallCode =>
    { takes_value := true,
      transfers_value := false,
      uses_target_address := false,
      inherits_caller_and_value := false,
      enters_static_context := false }
  | .DelegateCall =>
    { takes_value := false,
      transfers_value := false,
      uses_target_address := false,
      inherits_caller_and_value := true,
      enters_static_context := false }
  | .StaticCall =>
    { takes_value := false,
      transfers_value := false,
      uses_target_address := true,
      inherits_caller_and_value := false,
      enters_static_context := true }

/-- The four call opcodes, multiplexed on `mode`.

- `0` — `CALL`: a new frame at `target`, may transfer value.
- `1` — `CALLCODE`: runs the target's code in the caller's account,
  may transfer.
- `2` — `DELEGATECALL` (EIP-7): runs the target's code in the
  caller's account, inheriting the parent's caller/value/static
  context.
- `3` — `STATICCALL` (EIP-214): `CALL` with value 0 and a forced
  static context.

Operand layout (top of stack first): `gas`, `target`, `value` (for
`CALL`/`CALLCODE`), `argsOffset`, `argsLen`, `retOffset`, `retLen`.
Pushes 1 on success, 0 on failure. Takes the parent's carried machine
state; returns the parent's updated state on the non-entering paths
and the freshly installed child's state after a frame entry. -/
def call_stack_inputs (kind : CallKind) : Nat :=
  match kind with
  | .Call => 7
  | .CallCode => 7
  | .DelegateCall => 6
  | .StaticCall => 6

/-- Selects the code a frame actually executes (EIP-7702). A delegated
account runs the code at its delegation target, following exactly one
hop; a delegation whose target is a precompile (or has no code)
executes as empty code. An undelegated account runs its own code. -/
/- Type quantifiers: k_ex551022_ : Bool -/
def executable_code (target : (Vector (BitVec 8) 20)) (dele : Bool) (dtgt : (Vector (BitVec 8) 20)) : SailM (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))) := do
  if _sailIf0 : (dele : Bool) = true
  then
    (do
      let delegate_key ← do (k_code_key dtgt)
      let ⟨_, ⟨_, delegate_code⟩⟩ ← do (code_db_resolve delegate_key)
      let delegate_precompile ← do (precompile_id_for_address dtgt)
      if _sailIf1 : ((bne delegate_precompile NotPrecompile) : Bool) = true
      then
        (pure ((⟨_, ⟨_, ((EMPTY_CODE).2).2⟩⟩ : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))))
      else
        (pure ((⟨_, ⟨_, delegate_code⟩⟩ : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (CodeFields k_off k_len))))))
  else
    (do
      let target_key ← do (k_code_key target)
      (code_db_resolve target_key))

/-- Executes a message-call instruction through its non-entering failure paths
or installs the child frame and returns its initial carried machine state. -/
/- Type quantifiers: carried_returndata_dependentWitness1 : Nat, carried_returndata_dependentWitness0
  : Nat, carried_code_dependentWitness1 : Nat, carried_code_dependentWitness0 : Nat, k_ex551038_ :
  Nat, k_ex551037_ : Bool, k_ex551036_ : Nat, k_ex551035_ : Nat, k_ex551034_ : Nat, k_ex551033_ :
  Nat, k_ex551032_ : Int, k_ex551031_ : Nat, k_ex551030_ : Nat, k_ex551029_ : Nat, k_ex551028_ : Nat, 0
  ≤ k_ex551028_ ∧ k_ex551028_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex551029_ ∧
  k_ex551029_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551030_ ∧ k_ex551030_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex551031_ ∧ k_ex551031_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1)))) ≤ k_ex551032_ ∧
  k_ex551032_ ≤ (199 * (2 ^ 64 - 1)), 0 ≤ k_ex551033_ ∧ k_ex551033_ ≤ (2 ^ 32 - 1), 0 ≤
  k_ex551034_ ∧ k_ex551034_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex551035_ ∧ k_ex551035_ ≤ (2 ^ 256 - 1), 0
  ≤ k_ex551036_ ∧ k_ex551036_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551038_ ∧ k_ex551038_ ≤ 1024, 0
  ≤ carried_code_dependentWitness0 ∧
  0 ≤ carried_code_dependentWitness1 ∧
  (carried_code_dependentWitness0 + carried_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ carried_code_dependentWitness1 ∧ (carried_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0
  ≤ carried_returndata_dependentWitness0 ∧
  0 ≤ carried_returndata_dependentWitness1 ∧
  (carried_returndata_dependentWitness0 + carried_returndata_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def run_call (carried_pc : Nat) (carried_gas : Nat) (carried_state_gas : Nat) (carried_state_spill : Nat) (carried_refund : Int) (carried_sp : StackPointer) (carried_memory_base : Nat) (carried_memory_height : Nat) (carried_caller : (Vector (BitVec 8) 20)) (carried_address : (Vector (BitVec 8) 20)) (carried_code_address : (Vector (BitVec 8) 20)) (carried_value : Nat) (carried_state_gas_reservoir : Nat) (carried_is_static : Bool) (carried_depth : Nat) (carried_code : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (carried_calldata : CalldataSlice) (carried_returndata : (Sigma
  fun (carried_code_dependentWitness0 : Nat) =>
  (Sigma fun (carried_code_dependentWitness1 : Nat) =>
  (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) (kind : CallKind) : SailM FrameTransition := SailME.run do
  let carried_code_dependentWitness0 := (carried_code).1
  let carried_code_dependentWitness1 := ((carried_code).2).1
  let carried_code := ((carried_code).2).2
  let carried_returndata_dependentWitness0 := (carried_returndata).1
  let carried_returndata_dependentWitness1 := ((carried_returndata).2).1
  let carried_returndata := ((carried_returndata).2).2
  let stack_inputs := (call_stack_inputs kind)
  let stack_status := (guard_stack carried_sp stack_inputs 1)
  match stack_status with
  | .Failed halt_kind =>
    (do
      let exceptional ← do
        (exceptional_state carried_state_gas carried_state_spill carried_state_gas_reservoir
          halt_kind)
      let state_gas_after := exceptional.state_gas_remaining
      let state_spill_after := exceptional.state_gas_spilled
      let status_after := exceptional.status
      (pure { pc := carried_pc,
              gas_remaining := GAS_ZERO,
              state_gas_remaining := state_gas_after,
              state_gas_spilled := state_spill_after,
              refund := carried_refund,
              status := status_after,
              stack_top := carried_sp,
              memory_base := carried_memory_base,
              memory_height := carried_memory_height,
              message := { caller := carried_caller,
                           address := carried_address,
                           code_address := carried_code_address,
                           value := carried_value,
                           state_gas_reservoir := carried_state_gas_reservoir,
                           is_static := carried_is_static,
                           depth := carried_depth },
              code := ⟨_, ⟨_, carried_code⟩⟩,
              calldata := carried_calldata,
              returndata := ⟨_, ⟨_, carried_returndata⟩⟩ }))
  | .Continue () =>
    (do
      let pc_after : Nat := carried_pc
      let gas_after : Nat := carried_gas
      let state_gas_after : Nat := carried_state_gas
      let state_spill_after : Nat := carried_state_spill
      let status_after : FrameStatus := (Running ())
      let sp_after : StackPointer := carried_sp
      let memory_after : Nat := carried_memory_height
      let returndata_after : (Sigma fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
        ((⟨_, ⟨_, carried_returndata⟩⟩ : (Sigma fun
        (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
      let parent_message : Message :=
        { caller := carried_caller,
          address := carried_address,
          code_address := carried_code_address,
          value := carried_value,
          state_gas_reservoir := carried_state_gas_reservoir,
          is_static := carried_is_static,
          depth := carried_depth }
      let semantics := (call_semantics kind)
      let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
        readReg k_execution_profile
      let profile := execution_profile.protocol
      let current_depth := carried_depth
      let caller := carried_address
      let gas_request ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let target_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let target := (word_to_address target_word)
      let (value, next_sp) ← (( do
        if (semantics.takes_value : Bool)
        then
          (do
            let value ← do (read_stack_word sp_after)
            (pure (value, (← (stack_top_retreat sp_after 1)))))
        else (pure (WORD_ZERO, sp_after)) ) : SailME FrameTransition (Nat × StackPointer) )
      let sp_after : StackPointer := next_sp
      let value_nonzero := (word_nonzero value)
      let args_off_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let args_len_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let ret_off_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let ret_len_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      if ((semantics.transfers_value && (value_nonzero && carried_is_static)) : Bool)
      then
        (do
          let gas_after : Nat := GAS_ZERO
          let exceptional ← do
            (exceptional_state state_gas_after state_spill_after carried_state_gas_reservoir
              WriteProtection)
          let state_gas_after : Nat := exceptional.state_gas_remaining
          let state_spill_after : Nat := exceptional.state_gas_spilled
          let status_after : FrameStatus := exceptional.status
          (pure { pc := pc_after,
                  gas_remaining := gas_after,
                  state_gas_remaining := state_gas_after,
                  state_gas_spilled := state_spill_after,
                  refund := carried_refund,
                  status := status_after,
                  stack_top := sp_after,
                  memory_base := carried_memory_base,
                  memory_height := memory_after,
                  message := parent_message,
                  code := ⟨_, ⟨_, carried_code⟩⟩,
                  calldata := carried_calldata,
                  returndata := returndata_after }))
      else
        (do
          let warm ← do (k_account_is_warm target)
          let target_cost ← (( do (account_cost warm) ) : SailME FrameTransition Nat )
          let transfer_cost ← (( do
            if (value_nonzero : Bool)
            then (call_value_cost ())
            else (pure GAS_CONSTANT_ZERO) ) : SailME FrameTransition Nat )
          let args_requested_height := (memory_requested_height args_off_word args_len_word)
          let ret_requested_height := (memory_requested_height ret_off_word ret_len_word)
          let requested_height :=
            if ((args_requested_height <b ret_requested_height) : Bool)
            then ret_requested_height
            else args_requested_height
          let expansion_cost := (memory_expansion_gas_cost memory_after requested_height gas_after)
          if ((! expansion_cost.affordable) : Bool)
          then
            (do
              let gas_after : Nat := GAS_ZERO
              let exceptional ← do
                (exceptional_state state_gas_after state_spill_after carried_state_gas_reservoir
                  OutOfGas)
              let state_gas_after : Nat := exceptional.state_gas_remaining
              let state_spill_after : Nat := exceptional.state_gas_spilled
              let status_after : FrameStatus := exceptional.status
              (pure { pc := pc_after,
                      gas_remaining := gas_after,
                      state_gas_remaining := state_gas_after,
                      state_gas_spilled := state_spill_after,
                      refund := carried_refund,
                      status := status_after,
                      stack_top := sp_after,
                      memory_base := carried_memory_base,
                      memory_height := memory_after,
                      message := parent_message,
                      code := ⟨_, ⟨_, carried_code⟩⟩,
                      calldata := carried_calldata,
                      returndata := returndata_after }))
          else
            (do
              let gas_after : Nat := (gas_sub gas_after expansion_cost.cost)
              let static_base : Nat := (target_cost + transfer_cost)
              if ((gas_after <b static_base) : Bool)
              then
                (do
                  let gas_after : Nat := GAS_ZERO
                  let exceptional ← do
                    (exceptional_state state_gas_after state_spill_after carried_state_gas_reservoir
                      OutOfGas)
                  let state_gas_after : Nat := exceptional.state_gas_remaining
                  let state_spill_after : Nat := exceptional.state_gas_spilled
                  let status_after : FrameStatus := exceptional.status
                  (pure { pc := pc_after,
                          gas_remaining := gas_after,
                          state_gas_remaining := state_gas_after,
                          state_gas_spilled := state_spill_after,
                          refund := carried_refund,
                          status := status_after,
                          stack_top := sp_after,
                          memory_base := carried_memory_base,
                          memory_height := memory_after,
                          message := parent_message,
                          code := ⟨_, ⟨_, carried_code⟩⟩,
                          calldata := carried_calldata,
                          returndata := returndata_after }))
              else
                (do
                  let gas_after : Nat := (gas_sub gas_after static_base)
                  (k_account_mark_warm target)
                  let (tg_deleg, tg_target) ← do (k_deleg_target target)
                  let delegation_cost ← (( do
                    if (tg_deleg : Bool)
                    then
                      (do
                        let dw ← do (k_account_is_warm tg_target)
                        (account_cost dw))
                    else (pure GAS_CONSTANT_ZERO) ) : SailME FrameTransition Nat )
                  let target_empty ← do (k_account_is_empty target)
                  let new_account_charged :=
                    ((profile.fork ≥b Amsterdam) && (value_nonzero && (semantics.transfers_value && target_empty)))
                  let create_cost : Nat :=
                    if (((profile.fork <b Amsterdam) && (value_nonzero && (semantics.transfers_value && target_empty))) : Bool)
                    then G_newaccount
                    else GAS_CONSTANT_ZERO
                  let additional_cost : Nat := (delegation_cost + create_cost)
                  if ((gas_after <b additional_cost) : Bool)
                  then
                    (do
                      let gas_after : Nat := GAS_ZERO
                      let exceptional ← do
                        (exceptional_state state_gas_after state_spill_after
                          carried_state_gas_reservoir OutOfGas)
                      let state_gas_after : Nat := exceptional.state_gas_remaining
                      let state_spill_after : Nat := exceptional.state_gas_spilled
                      let status_after : FrameStatus := exceptional.status
                      (pure { pc := pc_after,
                              gas_remaining := gas_after,
                              state_gas_remaining := state_gas_after,
                              state_gas_spilled := state_spill_after,
                              refund := carried_refund,
                              status := status_after,
                              stack_top := sp_after,
                              memory_base := carried_memory_base,
                              memory_height := memory_after,
                              message := parent_message,
                              code := ⟨_, ⟨_, carried_code⟩⟩,
                              calldata := carried_calldata,
                              returndata := returndata_after }))
                  else
                    (do
                      let gas_after : Nat := (gas_sub gas_after additional_cost)
                      let stipend : Nat :=
                        if (value_nonzero : Bool)
                        then G_callstipend
                        else GAS_ZERO
                      let base_child : Nat := GAS_ZERO
                      let (base_child, gas_after, state_gas_after, state_spill_after, status_after) ← (( do
                        if ((profile.fork ≥b Amsterdam) : Bool)
                        then
                          (do
                            let (gas_after, state_gas_after, state_spill_after, status_after) ← (( do
                              if (new_account_charged : Bool)
                              then
                                (do
                                  let (state_gas_halt, next_gas, next_state_gas, next_state_spill) ← do
                                    (charge_state_gas gas_after state_gas_after state_spill_after
                                      G_amsterdam_state_new_account)
                                  let gas_after : Nat := next_gas
                                  let state_gas_after : Nat := next_state_gas
                                  let state_spill_after : Nat := next_state_spill
                                  let (gas_after, state_gas_after, state_spill_after, status_after) ← (( do
                                    if (state_gas_halt : Bool)
                                    then
                                      (do
                                        let a__303 ← (( do
                                          let gas_after : Nat := GAS_ZERO
                                          let exceptional ← do
                                            (exceptional_state state_gas_after state_spill_after
                                              carried_state_gas_reservoir OutOfGas)
                                          let state_gas_after : Nat :=
                                            exceptional.state_gas_remaining
                                          let state_spill_after : Nat :=
                                            exceptional.state_gas_spilled
                                          let status_after : FrameStatus := exceptional.status
                                          (pure { pc := pc_after,
                                                  gas_remaining := gas_after,
                                                  state_gas_remaining := state_gas_after,
                                                  state_gas_spilled := state_spill_after,
                                                  refund := carried_refund,
                                                  status := status_after,
                                                  stack_top := sp_after,
                                                  memory_base := carried_memory_base,
                                                  memory_height := memory_after,
                                                  message := parent_message,
                                                  code := ⟨_, ⟨_, carried_code⟩⟩,
                                                  calldata := carried_calldata,
                                                  returndata := returndata_after }) ) : SailME
                                          FrameTransition FrameTransition )
                                        SailME.throw a__303)
                                    else
                                      (pure (gas_after, state_gas_after, state_spill_after, status_after))
                                    ) : SailME FrameTransition (Nat × Nat × Nat × FrameStatus) )
                                  (pure (gas_after, state_gas_after, state_spill_after, status_after)))
                              else
                                (pure (gas_after, state_gas_after, state_spill_after, status_after))
                              ) : SailME FrameTransition (Nat × Nat × Nat × FrameStatus) )
                            let base_child : Nat := (call_gas_cap_word gas_after gas_request)
                            let (gas_after, state_gas_after, state_spill_after, status_after) ← (( do
                              if ((gas_after <b base_child) : Bool)
                              then
                                (do
                                  let a__302 ← (( do
                                    let gas_after : Nat := GAS_ZERO
                                    let exceptional ← do
                                      (exceptional_state state_gas_after state_spill_after
                                        carried_state_gas_reservoir OutOfGas)
                                    let state_gas_after : Nat := exceptional.state_gas_remaining
                                    let state_spill_after : Nat := exceptional.state_gas_spilled
                                    let status_after : FrameStatus := exceptional.status
                                    (pure { pc := pc_after,
                                            gas_remaining := gas_after,
                                            state_gas_remaining := state_gas_after,
                                            state_gas_spilled := state_spill_after,
                                            refund := carried_refund,
                                            status := status_after,
                                            stack_top := sp_after,
                                            memory_base := carried_memory_base,
                                            memory_height := memory_after,
                                            message := parent_message,
                                            code := ⟨_, ⟨_, carried_code⟩⟩,
                                            calldata := carried_calldata,
                                            returndata := returndata_after }) ) : SailME
                                    FrameTransition FrameTransition )
                                  SailME.throw a__302)
                              else
                                (let gas_after : Nat := (gas_sub gas_after base_child)
                                (pure (gas_after, state_gas_after, state_spill_after, status_after)))
                              ) : SailME FrameTransition (Nat × Nat × Nat × FrameStatus) )
                            (pure (base_child, gas_after, state_gas_after, state_spill_after, status_after)))
                        else
                          (do
                            let base_child : Nat := (call_gas_cap_word gas_after gas_request)
                            let (gas_after, state_gas_after, state_spill_after, status_after) ← (( do
                              if ((gas_after <b base_child) : Bool)
                              then
                                (do
                                  let a__304 ← (( do
                                    let gas_after : Nat := GAS_ZERO
                                    let exceptional ← do
                                      (exceptional_state state_gas_after state_spill_after
                                        carried_state_gas_reservoir OutOfGas)
                                    let state_gas_after : Nat := exceptional.state_gas_remaining
                                    let state_spill_after : Nat := exceptional.state_gas_spilled
                                    let status_after : FrameStatus := exceptional.status
                                    (pure { pc := pc_after,
                                            gas_remaining := gas_after,
                                            state_gas_remaining := state_gas_after,
                                            state_gas_spilled := state_spill_after,
                                            refund := carried_refund,
                                            status := status_after,
                                            stack_top := sp_after,
                                            memory_base := carried_memory_base,
                                            memory_height := memory_after,
                                            message := parent_message,
                                            code := ⟨_, ⟨_, carried_code⟩⟩,
                                            calldata := carried_calldata,
                                            returndata := returndata_after }) ) : SailME
                                    FrameTransition FrameTransition )
                                  SailME.throw a__304)
                              else
                                (let gas_after : Nat := (gas_sub gas_after base_child)
                                (pure (gas_after, state_gas_after, state_spill_after, status_after)))
                              ) : SailME FrameTransition (Nat × Nat × Nat × FrameStatus) )
                            (pure (base_child, gas_after, state_gas_after, state_spill_after, status_after)))
                        ) : SailME FrameTransition (Nat × Nat × Nat × Nat × FrameStatus) )
                      if (tg_deleg : Bool)
                      then (k_account_mark_warm tg_target)
                      else (pure ())
                      if (tg_deleg : Bool)
                      then
                        (do
                          let delegate_key ← do (k_code_key tg_target)
                          let ⟨_, ⟨_, _⟩⟩ ← do (code_db_resolve delegate_key)
                          let _ ← do (k_aload tg_target)
                          (pure ()))
                      else (pure ())
                      let ⟨_, ⟨_, ⟨_, args_access⟩⟩⟩ ← do
                        (memory_access args_off_word args_len_word)
                      let ⟨_, ⟨_, ⟨_, ret_access⟩⟩⟩ ← do
                        (memory_access ret_off_word ret_len_word)
                      let materialized_required_size :=
                        if ((args_access.requested_height <b ret_access.requested_height) : Bool)
                        then ret_access.requested_height
                        else args_access.requested_height
                      let mem1 ← do
                        (expand_memory carried_memory_base memory_after materialized_required_size)
                      let args := args_access.range
                      let ret := ret_access.range
                      let child_gas ← (( do (conserved_gas_add base_child stipend) ) : SailME
                        FrameTransition Nat )
                      let _ ← do (k_aload target)
                      let insufficient_balance ← (( do
                        if ((semantics.takes_value && value_nonzero) : Bool)
                        then
                          (do
                            let caller_balance ← do (k_get_balance caller)
                            let transfer_affordable := (word_ule value caller_balance)
                            (pure (! transfer_affordable)))
                        else (pure false) ) : SailME FrameTransition Bool )
                      let depth_limit := 1024
                      if ((insufficient_balance || (current_depth == depth_limit)) : Bool)
                      then
                        (do
                          let ⟨_, ⟨_, returndata_after⟩⟩ : (Sigma fun
                            (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                            ((returndata_clear ()) : (Sigma fun
                            (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                          let gas_after ← (refund_gas gas_after child_gas)
                          let (gas_after, state_gas_after, state_spill_after) ← (( do
                            if (new_account_charged : Bool)
                            then
                              (do
                                let (tup__0, tup__1, tup__2) ← do
                                  (credit_state_gas_refund gas_after state_gas_after
                                    state_spill_after G_amsterdam_state_new_account)
                                let gas_after : Nat := tup__0
                                let state_gas_after : Nat := tup__1
                                let state_spill_after : Nat := tup__2
                                (pure (gas_after, state_gas_after, state_spill_after)))
                            else (pure (gas_after, state_gas_after, state_spill_after)) ) : SailME
                            FrameTransition (Nat × Nat × Nat) )
                          let sp_after ← (stack_top_advance sp_after 1)
                          (write_stack_word sp_after WORD_ZERO)
                          let memory_after : Nat := mem1
                          (pure { pc := pc_after,
                                  gas_remaining := gas_after,
                                  state_gas_remaining := state_gas_after,
                                  state_gas_spilled := state_spill_after,
                                  refund := carried_refund,
                                  status := status_after,
                                  stack_top := sp_after,
                                  memory_base := carried_memory_base,
                                  memory_height := memory_after,
                                  message := parent_message,
                                  code := ⟨_, ⟨_, carried_code⟩⟩,
                                  calldata := carried_calldata,
                                  returndata := ⟨_, ⟨_, returndata_after⟩⟩ }))
                      else
                        (do
                          let selected_precompile ← do (precompile_id_for_address target)
                          if ((bne selected_precompile NotPrecompile) : Bool)
                          then
                            (do
                              let ⟨_, ⟨_, input_memory⟩⟩ ← do
                                (active_memory_slice carried_memory_base mem1 args.off args.len)
                              let input := (MemoryCalldata ⟨_, ⟨_, input_memory⟩⟩)
                              let precompile_charge ← do
                                (precompile_gas selected_precompile input child_gas)
                              if (precompile_charge.affordable : Bool)
                              then
                                (do
                                  let used := precompile_charge.cost
                                  let result ← do (run_precompile_slice selected_precompile input)
                                  if (result.success : Bool)
                                  then
                                    (do
                                      let ⟨_, ⟨_, returndata_after⟩⟩ : (Sigma fun
                                        (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                                        (result.output : (Sigma fun
                                        (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                      if ((semantics.transfers_value && value_nonzero) : Bool)
                                      then (k_transfer caller target value)
                                      else (pure ())
                                      let return_destination ← do
                                        (memory_absolute carried_memory_base ret.off)
                                      (returndata_copy_prefix ⟨_, ⟨_, returndata_after⟩⟩
                                        return_destination ret.len)
                                      let unused : Nat := (gas_sub child_gas used)
                                      let gas_after ← (refund_gas gas_after unused)
                                      let sp_after ← (stack_top_advance sp_after 1)
                                      (write_stack_word sp_after WORD_ONE)
                                      let memory_after : Nat := mem1
                                      (pure { pc := pc_after,
                                              gas_remaining := gas_after,
                                              state_gas_remaining := state_gas_after,
                                              state_gas_spilled := state_spill_after,
                                              refund := carried_refund,
                                              status := status_after,
                                              stack_top := sp_after,
                                              memory_base := carried_memory_base,
                                              memory_height := memory_after,
                                              message := parent_message,
                                              code := ⟨_, ⟨_, carried_code⟩⟩,
                                              calldata := carried_calldata,
                                              returndata := ⟨_, ⟨_, returndata_after⟩⟩ }))
                                  else
                                    (do
                                      let ⟨_, ⟨_, returndata_after⟩⟩ : (Sigma fun
                                        (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                                        ((returndata_clear ()) : (Sigma fun
                                        (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                      let (gas_after, state_gas_after, state_spill_after) ← (( do
                                        if (new_account_charged : Bool)
                                        then
                                          (do
                                            let (tup__0, tup__1, tup__2) ← do
                                              (credit_state_gas_refund gas_after state_gas_after
                                                state_spill_after G_amsterdam_state_new_account)
                                            let gas_after : Nat := tup__0
                                            let state_gas_after : Nat := tup__1
                                            let state_spill_after : Nat := tup__2
                                            (pure (gas_after, state_gas_after, state_spill_after)))
                                        else (pure (gas_after, state_gas_after, state_spill_after))
                                        ) : SailME FrameTransition (Nat × Nat × Nat) )
                                      let sp_after ← (stack_top_advance sp_after 1)
                                      (write_stack_word sp_after WORD_ZERO)
                                      let memory_after : Nat := mem1
                                      (pure { pc := pc_after,
                                              gas_remaining := gas_after,
                                              state_gas_remaining := state_gas_after,
                                              state_gas_spilled := state_spill_after,
                                              refund := carried_refund,
                                              status := status_after,
                                              stack_top := sp_after,
                                              memory_base := carried_memory_base,
                                              memory_height := memory_after,
                                              message := parent_message,
                                              code := ⟨_, ⟨_, carried_code⟩⟩,
                                              calldata := carried_calldata,
                                              returndata := ⟨_, ⟨_, returndata_after⟩⟩ })))
                              else
                                (do
                                  let ⟨_, ⟨_, returndata_after⟩⟩ : (Sigma fun
                                    (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                                    ((returndata_clear ()) : (Sigma fun
                                    (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                  let (gas_after, state_gas_after, state_spill_after) ← (( do
                                    if (new_account_charged : Bool)
                                    then
                                      (do
                                        let (tup__0, tup__1, tup__2) ← do
                                          (credit_state_gas_refund gas_after state_gas_after
                                            state_spill_after G_amsterdam_state_new_account)
                                        let gas_after : Nat := tup__0
                                        let state_gas_after : Nat := tup__1
                                        let state_spill_after : Nat := tup__2
                                        (pure (gas_after, state_gas_after, state_spill_after)))
                                    else (pure (gas_after, state_gas_after, state_spill_after)) ) :
                                    SailME FrameTransition (Nat × Nat × Nat) )
                                  let sp_after ← (stack_top_advance sp_after 1)
                                  (write_stack_word sp_after WORD_ZERO)
                                  let memory_after : Nat := mem1
                                  (pure { pc := pc_after,
                                          gas_remaining := gas_after,
                                          state_gas_remaining := state_gas_after,
                                          state_gas_spilled := state_spill_after,
                                          refund := carried_refund,
                                          status := status_after,
                                          stack_top := sp_after,
                                          memory_base := carried_memory_base,
                                          memory_height := memory_after,
                                          message := parent_message,
                                          code := ⟨_, ⟨_, carried_code⟩⟩,
                                          calldata := carried_calldata,
                                          returndata := ⟨_, ⟨_, returndata_after⟩⟩ })))
                          else
                            (do
                              let child_depth : Nat := (current_depth + 1)
                              let ⟨_, ⟨_, child_code⟩⟩ ← do
                                (executable_code target tg_deleg tg_target)
                              let child_addr : (Vector (BitVec 8) 20) :=
                                if (semantics.uses_target_address : Bool)
                                then target
                                else caller
                              let child_caller : (Vector (BitVec 8) 20) :=
                                if (semantics.inherits_caller_and_value : Bool)
                                then carried_caller
                                else caller
                              let child_value : Nat :=
                                if (semantics.inherits_caller_and_value : Bool)
                                then carried_value
                                else value
                              let child_static : Bool :=
                                if (semantics.enters_static_context : Bool)
                                then true
                                else carried_is_static
                              let ⟨_, ⟨_, bytes⟩⟩ ← do
                                (active_memory_slice carried_memory_base mem1 args.off args.len)
                              let child_memory := (evm_memory_slice bytes.bytes bytes.len)
                              let child_calldata := (MemoryCalldata ⟨_, ⟨_, child_memory⟩⟩)
                              let child_state_gas := state_gas_after
                              let running := (Running ())
                              let (checkpoint, child_stack, child_memory_base, child_memory_height) ← do
                                (suspend_frame pc_after gas_after sp_after carried_memory_base mem1
                                  STATE_GAS_ZERO state_spill_after carried_refund running
                                  parent_message ⟨_, ⟨_, carried_code⟩⟩ carried_calldata)
                              let call_continuation : CallContinuation :=
                                { checkpoint := checkpoint,
                                  return_offset := ret.off,
                                  return_length := ret.len,
                                  new_account_charged := new_account_charged }
                              let continuation := (ResumeCall call_continuation)
                              (frame_stack_push continuation)
                              if ((semantics.transfers_value && value_nonzero) : Bool)
                              then (k_transfer caller target value)
                              else (pure ())
                              let ⟨_, ⟨_, child_returndata⟩⟩ := (returndata_clear ())
                              (pure { pc := 0,
                                      gas_remaining := child_gas,
                                      state_gas_remaining := child_state_gas,
                                      state_gas_spilled := STATE_GAS_SPILL_ZERO,
                                      refund := GAS_REFUND_ZERO,
                                      status := running,
                                      stack_top := child_stack,
                                      memory_base := child_memory_base,
                                      memory_height := child_memory_height,
                                      message := { caller := child_caller,
                                                   address := child_addr,
                                                   code_address := target,
                                                   value := child_value,
                                                   state_gas_reservoir := child_state_gas,
                                                   is_static := child_static,
                                                   depth := child_depth },
                                      code := ⟨_, ⟨_, child_code⟩⟩,
                                      calldata := child_calldata,
                                      returndata := ⟨_, ⟨_, child_returndata⟩⟩ }))))))))

/-- Defunctionalizes each CREATE-family opcode into the data consumed by the
shared contract-creation interpreter. -/
def create_semantics (kind : CreateKind) : CreateSemantics :=
  match kind with
  | .CreateByNonce => { uses_salt := false }
  | .CreateBySalt => { uses_salt := true }

/-- `CREATE` (`CreateByNonce`) and `CREATE2` (`CreateBySalt`, EIP-1014).
Both deploy a new contract by running the initcode supplied in
memory; the new address derives from `(creator, nonce)` for `CREATE`
or `(creator, salt, keccak256(initcode))` for `CREATE2`. Operand
layout: `value`, `offset`, `length`, `salt` (for `CREATE2`). Pushes
the new address on success, 0 on failure. Takes the parent's carried
machine state; returns the parent's updated state on the non-entering
paths and the freshly installed child's state after a frame entry. -/
def create_stack_inputs (kind : CreateKind) : Nat :=
  match kind with
  | .CreateByNonce => 3
  | .CreateBySalt => 4

/-- Executes a contract-creation instruction through its non-entering failure
paths or installs the initcode child frame and returns its initial state. -/
/- Type quantifiers: carried_returndata_dependentWitness1 : Nat, carried_returndata_dependentWitness0
  : Nat, carried_code_dependentWitness1 : Nat, carried_code_dependentWitness0 : Nat, k_ex551062_ :
  Nat, k_ex551061_ : Bool, k_ex551060_ : Nat, k_ex551059_ : Nat, k_ex551058_ : Nat, k_ex551057_ :
  Nat, k_ex551056_ : Int, k_ex551055_ : Nat, k_ex551054_ : Nat, k_ex551053_ : Nat, k_ex551052_ : Nat, 0
  ≤ k_ex551052_ ∧ k_ex551052_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex551053_ ∧
  k_ex551053_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551054_ ∧ k_ex551054_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex551055_ ∧ k_ex551055_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1)))) ≤ k_ex551056_ ∧
  k_ex551056_ ≤ (199 * (2 ^ 64 - 1)), 0 ≤ k_ex551057_ ∧ k_ex551057_ ≤ (2 ^ 32 - 1), 0 ≤
  k_ex551058_ ∧ k_ex551058_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex551059_ ∧ k_ex551059_ ≤ (2 ^ 256 - 1), 0
  ≤ k_ex551060_ ∧ k_ex551060_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551062_ ∧ k_ex551062_ ≤ 1024, 0
  ≤ carried_code_dependentWitness0 ∧
  0 ≤ carried_code_dependentWitness1 ∧
  (carried_code_dependentWitness0 + carried_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ carried_code_dependentWitness1 ∧ (carried_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0
  ≤ carried_returndata_dependentWitness0 ∧
  0 ≤ carried_returndata_dependentWitness1 ∧
  (carried_returndata_dependentWitness0 + carried_returndata_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def run_create (carried_pc : Nat) (carried_gas : Nat) (carried_state_gas : Nat) (carried_state_spill : Nat) (carried_refund : Int) (carried_sp : StackPointer) (carried_memory_base : Nat) (carried_memory_height : Nat) (carried_caller : (Vector (BitVec 8) 20)) (carried_address : (Vector (BitVec 8) 20)) (carried_code_address : (Vector (BitVec 8) 20)) (carried_value : Nat) (carried_state_gas_reservoir : Nat) (carried_is_static : Bool) (carried_depth : Nat) (carried_code : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (carried_calldata : CalldataSlice) (carried_returndata : (Sigma
  fun (carried_code_dependentWitness0 : Nat) =>
  (Sigma fun (carried_code_dependentWitness1 : Nat) =>
  (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) (kind : CreateKind) : SailM FrameTransition := SailME.run do
  let carried_code_dependentWitness0 := (carried_code).1
  let carried_code_dependentWitness1 := ((carried_code).2).1
  let carried_code := ((carried_code).2).2
  let carried_returndata_dependentWitness0 := (carried_returndata).1
  let carried_returndata_dependentWitness1 := ((carried_returndata).2).1
  let carried_returndata := ((carried_returndata).2).2
  let stack_inputs := (create_stack_inputs kind)
  let stack_status := (guard_stack carried_sp stack_inputs 1)
  match stack_status with
  | .Failed halt_kind =>
    (do
      let exceptional ← do
        (exceptional_state carried_state_gas carried_state_spill carried_state_gas_reservoir
          halt_kind)
      let state_gas_after := exceptional.state_gas_remaining
      let state_spill_after := exceptional.state_gas_spilled
      let status_after := exceptional.status
      (pure { pc := carried_pc,
              gas_remaining := GAS_ZERO,
              state_gas_remaining := state_gas_after,
              state_gas_spilled := state_spill_after,
              refund := carried_refund,
              status := status_after,
              stack_top := carried_sp,
              memory_base := carried_memory_base,
              memory_height := carried_memory_height,
              message := { caller := carried_caller,
                           address := carried_address,
                           code_address := carried_code_address,
                           value := carried_value,
                           state_gas_reservoir := carried_state_gas_reservoir,
                           is_static := carried_is_static,
                           depth := carried_depth },
              code := ⟨_, ⟨_, carried_code⟩⟩,
              calldata := carried_calldata,
              returndata := ⟨_, ⟨_, carried_returndata⟩⟩ }))
  | .Continue () =>
    (do
      let pc_after : Nat := carried_pc
      let gas_after : Nat := carried_gas
      let state_gas_after : Nat := carried_state_gas
      let state_spill_after : Nat := carried_state_spill
      let status_after : FrameStatus := (Running ())
      let sp_after : StackPointer := carried_sp
      let memory_after : Nat := carried_memory_height
      let returndata_after : (Sigma fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
        ((⟨_, ⟨_, carried_returndata⟩⟩ : (Sigma fun
        (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
      let parent_message : Message :=
        { caller := carried_caller,
          address := carried_address,
          code_address := carried_code_address,
          value := carried_value,
          state_gas_reservoir := carried_state_gas_reservoir,
          is_static := carried_is_static,
          depth := carried_depth }
      let semantics := (create_semantics kind)
      let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
        readReg k_execution_profile
      let profile := execution_profile.protocol
      let current_depth := carried_depth
      let creator := carried_address
      let value ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let off_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let len_word ← do (read_stack_word sp_after)
      let sp_after ← (stack_top_retreat sp_after 1)
      let (salt, next_sp) ← (( do
        if (semantics.uses_salt : Bool)
        then
          (do
            let salt ← do (read_stack_word sp_after)
            (pure (salt, (← (stack_top_retreat sp_after 1)))))
        else (pure (WORD_ZERO, sp_after)) ) : SailME FrameTransition (Nat × StackPointer) )
      let sp_after : StackPointer := next_sp
      if (carried_is_static : Bool)
      then
        (do
          let gas_after : Nat := GAS_ZERO
          let exceptional ← do
            (exceptional_state state_gas_after state_spill_after carried_state_gas_reservoir
              WriteProtection)
          let state_gas_after : Nat := exceptional.state_gas_remaining
          let state_spill_after : Nat := exceptional.state_gas_spilled
          let status_after : FrameStatus := exceptional.status
          (pure { pc := pc_after,
                  gas_remaining := gas_after,
                  state_gas_remaining := state_gas_after,
                  state_gas_spilled := state_spill_after,
                  refund := carried_refund,
                  status := status_after,
                  stack_top := sp_after,
                  memory_base := carried_memory_base,
                  memory_height := memory_after,
                  message := parent_message,
                  code := ⟨_, ⟨_, carried_code⟩⟩,
                  calldata := carried_calldata,
                  returndata := returndata_after }))
      else
        (do
          let requested_height := (memory_requested_height off_word len_word)
          let expansion_cost := (memory_expansion_gas_cost memory_after requested_height gas_after)
          if ((! expansion_cost.affordable) : Bool)
          then
            (do
              let gas_after : Nat := GAS_ZERO
              let exceptional ← do
                (exceptional_state state_gas_after state_spill_after carried_state_gas_reservoir
                  OutOfGas)
              let state_gas_after : Nat := exceptional.state_gas_remaining
              let state_spill_after : Nat := exceptional.state_gas_spilled
              let status_after : FrameStatus := exceptional.status
              (pure { pc := pc_after,
                      gas_remaining := gas_after,
                      state_gas_remaining := state_gas_after,
                      state_gas_spilled := state_spill_after,
                      refund := carried_refund,
                      status := status_after,
                      stack_top := sp_after,
                      memory_base := carried_memory_base,
                      memory_height := memory_after,
                      message := parent_message,
                      code := ⟨_, ⟨_, carried_code⟩⟩,
                      calldata := carried_calldata,
                      returndata := returndata_after }))
          else
            (do
              let gas_after : Nat := (gas_sub gas_after expansion_cost.cost)
              let ⟨_, ⟨_, ⟨_, initcode_access⟩⟩⟩ ← do
                (memory_access off_word len_word)
              let mem1 ← do
                (expand_memory carried_memory_base memory_after initcode_access.requested_height)
              let initcode := initcode_access.range
              let access_cost ← do (create_access_cost ())
              if ((gas_after <b access_cost) : Bool)
              then
                (do
                  let memory_after : Nat := mem1
                  let gas_after : Nat := GAS_ZERO
                  let exceptional ← do
                    (exceptional_state state_gas_after state_spill_after carried_state_gas_reservoir
                      OutOfGas)
                  let state_gas_after : Nat := exceptional.state_gas_remaining
                  let state_spill_after : Nat := exceptional.state_gas_spilled
                  let status_after : FrameStatus := exceptional.status
                  (pure { pc := pc_after,
                          gas_remaining := gas_after,
                          state_gas_remaining := state_gas_after,
                          state_gas_spilled := state_spill_after,
                          refund := carried_refund,
                          status := status_after,
                          stack_top := sp_after,
                          memory_base := carried_memory_base,
                          memory_height := memory_after,
                          message := parent_message,
                          code := ⟨_, ⟨_, carried_code⟩⟩,
                          calldata := carried_calldata,
                          returndata := returndata_after }))
              else
                (do
                  let gas_after : Nat := (gas_sub gas_after access_cost)
                  let initcode_word_count := (memory_word_count_word len_word)
                  let (gas_after, memory_after, state_gas_after, state_spill_after, status_after) ← (( do
                    if ((profile.fork ≥b Shanghai) : Bool)
                    then
                      (do
                        let initcode_cost :=
                          (word_scaled_gas_cost G_initcode_word initcode_word_count gas_after)
                        let (gas_after, memory_after, state_gas_after, state_spill_after, status_after) ← (( do
                          if ((! initcode_cost.affordable) : Bool)
                          then
                            (do
                              let a__290 ← (( do
                                let memory_after : Nat := mem1
                                let gas_after : Nat := GAS_ZERO
                                let exceptional ← do
                                  (exceptional_state state_gas_after state_spill_after
                                    carried_state_gas_reservoir OutOfGas)
                                let state_gas_after : Nat := exceptional.state_gas_remaining
                                let state_spill_after : Nat := exceptional.state_gas_spilled
                                let status_after : FrameStatus := exceptional.status
                                (pure { pc := pc_after,
                                        gas_remaining := gas_after,
                                        state_gas_remaining := state_gas_after,
                                        state_gas_spilled := state_spill_after,
                                        refund := carried_refund,
                                        status := status_after,
                                        stack_top := sp_after,
                                        memory_base := carried_memory_base,
                                        memory_height := memory_after,
                                        message := parent_message,
                                        code := ⟨_, ⟨_, carried_code⟩⟩,
                                        calldata := carried_calldata,
                                        returndata := returndata_after }) ) : SailME FrameTransition
                                FrameTransition )
                              SailME.throw a__290)
                          else
                            (let gas_after : Nat := (gas_sub gas_after initcode_cost.cost)
                            (pure (gas_after, memory_after, state_gas_after, state_spill_after, status_after)))
                          ) : SailME FrameTransition (Nat × Nat × Nat × Nat × FrameStatus) )
                        (pure (gas_after, memory_after, state_gas_after, state_spill_after, status_after)))
                    else
                      (pure (gas_after, memory_after, state_gas_after, state_spill_after, status_after))
                    ) : SailME FrameTransition (Nat × Nat × Nat × Nat × FrameStatus) )
                  let (gas_after, memory_after, state_gas_after, state_spill_after, status_after) ← (( do
                    if (semantics.uses_salt : Bool)
                    then
                      (do
                        let hashing_cost :=
                          (word_scaled_gas_cost G_keccak_word initcode_word_count gas_after)
                        let (gas_after, memory_after, state_gas_after, state_spill_after, status_after) ← (( do
                          if ((! hashing_cost.affordable) : Bool)
                          then
                            (do
                              let a__289 ← (( do
                                let memory_after : Nat := mem1
                                let gas_after : Nat := GAS_ZERO
                                let exceptional ← do
                                  (exceptional_state state_gas_after state_spill_after
                                    carried_state_gas_reservoir OutOfGas)
                                let state_gas_after : Nat := exceptional.state_gas_remaining
                                let state_spill_after : Nat := exceptional.state_gas_spilled
                                let status_after : FrameStatus := exceptional.status
                                (pure { pc := pc_after,
                                        gas_remaining := gas_after,
                                        state_gas_remaining := state_gas_after,
                                        state_gas_spilled := state_spill_after,
                                        refund := carried_refund,
                                        status := status_after,
                                        stack_top := sp_after,
                                        memory_base := carried_memory_base,
                                        memory_height := memory_after,
                                        message := parent_message,
                                        code := ⟨_, ⟨_, carried_code⟩⟩,
                                        calldata := carried_calldata,
                                        returndata := returndata_after }) ) : SailME FrameTransition
                                FrameTransition )
                              SailME.throw a__289)
                          else
                            (let gas_after : Nat := (gas_sub gas_after hashing_cost.cost)
                            (pure (gas_after, memory_after, state_gas_after, state_spill_after, status_after)))
                          ) : SailME FrameTransition (Nat × Nat × Nat × Nat × FrameStatus) )
                        (pure (gas_after, memory_after, state_gas_after, state_spill_after, status_after)))
                    else
                      (pure (gas_after, memory_after, state_gas_after, state_spill_after, status_after))
                    ) : SailME FrameTransition (Nat × Nat × Nat × Nat × FrameStatus) )
                  let valid_initcode_size ← do (initcode_size_allowed initcode.len)
                  let invalid_initcode_size := (! valid_initcode_size)
                  if (invalid_initcode_size : Bool)
                  then
                    (do
                      let memory_after : Nat := mem1
                      let gas_after : Nat := GAS_ZERO
                      let exceptional ← do
                        (exceptional_state state_gas_after state_spill_after
                          carried_state_gas_reservoir InitCodeTooLarge)
                      let state_gas_after : Nat := exceptional.state_gas_remaining
                      let state_spill_after : Nat := exceptional.state_gas_spilled
                      let status_after : FrameStatus := exceptional.status
                      (pure { pc := pc_after,
                              gas_remaining := gas_after,
                              state_gas_remaining := state_gas_after,
                              state_gas_spilled := state_spill_after,
                              refund := carried_refund,
                              status := status_after,
                              stack_top := sp_after,
                              memory_base := carried_memory_base,
                              memory_height := memory_after,
                              message := parent_message,
                              code := ⟨_, ⟨_, carried_code⟩⟩,
                              calldata := carried_calldata,
                              returndata := returndata_after }))
                  else
                    (do
                      let nonce ← do (k_get_nonce creator)
                      let new_addr ← (( do
                        if (semantics.uses_salt : Bool)
                        then
                          (do
                            let initcode_digest_word ← do
                              (mem_keccak carried_memory_base mem1 ⟨_, ⟨_, initcode⟩⟩)
                            let initcode_digest := (word_to_hash initcode_digest_word)
                            (k_create2_addr creator salt initcode_digest))
                        else (k_create_addr creator nonce) ) : SailME FrameTransition
                        (Vector (BitVec 8) 20) )
                      let child_gas : Nat := GAS_ZERO
                      let (child_gas, gas_after) : (Nat × Nat) :=
                        if ((profile.fork <b Amsterdam) : Bool)
                        then
                          (let avail := gas_after
                          let retained_gas : Nat := (avail / 64)
                          let child_gas : Nat := (gas_sub avail retained_gas)
                          let gas_after : Nat := retained_gas
                          (child_gas, gas_after))
                        else (child_gas, gas_after)
                      let creator_balance ← do (k_get_balance creator)
                      let endowment_affordable := (word_ule value creator_balance)
                      let insufficient_balance := (! endowment_affordable)
                      let nonce_limit := ((2 ^i 64) - 1)
                      let depth_limit := 1024
                      if ((insufficient_balance || ((nonce == nonce_limit) || (current_depth == depth_limit))) : Bool)
                      then
                        (do
                          let ⟨_, ⟨_, returndata_after⟩⟩ : (Sigma fun
                            (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                            ((returndata_clear ()) : (Sigma fun
                            (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                          let gas_after ←
                            if ((profile.fork <b Amsterdam) : Bool)
                            then (refund_gas gas_after child_gas)
                            else (pure gas_after)
                          let sp_after ← (stack_top_advance sp_after 1)
                          (write_stack_word sp_after WORD_ZERO)
                          let memory_after : Nat := mem1
                          (pure { pc := pc_after,
                                  gas_remaining := gas_after,
                                  state_gas_remaining := state_gas_after,
                                  state_gas_spilled := state_spill_after,
                                  refund := carried_refund,
                                  status := status_after,
                                  stack_top := sp_after,
                                  memory_base := carried_memory_base,
                                  memory_height := memory_after,
                                  message := parent_message,
                                  code := ⟨_, ⟨_, carried_code⟩⟩,
                                  calldata := carried_calldata,
                                  returndata := ⟨_, ⟨_, returndata_after⟩⟩ }))
                      else
                        (do
                          let child_depth : Nat := (current_depth + 1)
                          (k_account_mark_warm new_addr)
                          let new_account_charged : Bool := false
                          let new_account_charged ← (( do
                            if ((profile.fork ≥b Amsterdam) : Bool)
                            then
                              (do
                                (k_account_is_empty new_addr))
                            else (pure new_account_charged) ) : SailME FrameTransition Bool )
                          let (gas_after, memory_after, state_gas_after, state_spill_after, status_after) ← (( do
                            if (new_account_charged : Bool)
                            then
                              (do
                                let (state_gas_halt, next_gas, next_state_gas, next_state_spill) ← do
                                  (charge_state_gas gas_after state_gas_after state_spill_after
                                    G_amsterdam_state_new_account)
                                let gas_after : Nat := next_gas
                                let state_gas_after : Nat := next_state_gas
                                let state_spill_after : Nat := next_state_spill
                                let (gas_after, memory_after, state_gas_after, state_spill_after, status_after) ← (( do
                                  if (state_gas_halt : Bool)
                                  then
                                    (do
                                      let a__288 ← (( do
                                        let memory_after : Nat := mem1
                                        let gas_after : Nat := GAS_ZERO
                                        let exceptional ← do
                                          (exceptional_state state_gas_after state_spill_after
                                            carried_state_gas_reservoir OutOfGas)
                                        let state_gas_after : Nat := exceptional.state_gas_remaining
                                        let state_spill_after : Nat := exceptional.state_gas_spilled
                                        let status_after : FrameStatus := exceptional.status
                                        (pure { pc := pc_after,
                                                gas_remaining := gas_after,
                                                state_gas_remaining := state_gas_after,
                                                state_gas_spilled := state_spill_after,
                                                refund := carried_refund,
                                                status := status_after,
                                                stack_top := sp_after,
                                                memory_base := carried_memory_base,
                                                memory_height := memory_after,
                                                message := parent_message,
                                                code := ⟨_, ⟨_, carried_code⟩⟩,
                                                calldata := carried_calldata,
                                                returndata := returndata_after }) ) : SailME
                                        FrameTransition FrameTransition )
                                      SailME.throw a__288)
                                  else
                                    (pure (gas_after, memory_after, state_gas_after, state_spill_after, status_after))
                                  ) : SailME FrameTransition
                                  (Nat × Nat × Nat × Nat × FrameStatus) )
                                (pure (gas_after, memory_after, state_gas_after, state_spill_after, status_after)))
                            else
                              (pure (gas_after, memory_after, state_gas_after, state_spill_after, status_after))
                            ) : SailME FrameTransition (Nat × Nat × Nat × Nat × FrameStatus) )
                          let (child_gas, gas_after) : (Nat × Nat) :=
                            if ((profile.fork ≥b Amsterdam) : Bool)
                            then
                              (let avail := gas_after
                              let retained_gas : Nat := (avail / 64)
                              let child_gas : Nat := (gas_sub avail retained_gas)
                              let gas_after : Nat := retained_gas
                              (child_gas, gas_after))
                            else (child_gas, gas_after)
                          let occupied ← (( do (k_account_occupied new_addr) ) : SailME
                            FrameTransition Bool )
                          let ⟨_, ⟨_, returndata_after⟩⟩ : (Sigma fun
                            (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                            ((returndata_clear ()) : (Sigma fun
                            (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                          (k_bump_nonce creator)
                          if (occupied : Bool)
                          then
                            (do
                              let (gas_after, state_gas_after, state_spill_after) ← (( do
                                if (new_account_charged : Bool)
                                then
                                  (do
                                    let (tup__0, tup__1, tup__2) ← do
                                      (credit_state_gas_refund gas_after state_gas_after
                                        state_spill_after G_amsterdam_state_new_account)
                                    let gas_after : Nat := tup__0
                                    let state_gas_after : Nat := tup__1
                                    let state_spill_after : Nat := tup__2
                                    (pure (gas_after, state_gas_after, state_spill_after)))
                                else (pure (gas_after, state_gas_after, state_spill_after)) ) :
                                SailME FrameTransition (Nat × Nat × Nat) )
                              let sp_after ← (stack_top_advance sp_after 1)
                              (write_stack_word sp_after WORD_ZERO)
                              let memory_after : Nat := mem1
                              (pure { pc := pc_after,
                                      gas_remaining := gas_after,
                                      state_gas_remaining := state_gas_after,
                                      state_gas_spilled := state_spill_after,
                                      refund := carried_refund,
                                      status := status_after,
                                      stack_top := sp_after,
                                      memory_base := carried_memory_base,
                                      memory_height := memory_after,
                                      message := parent_message,
                                      code := ⟨_, ⟨_, carried_code⟩⟩,
                                      calldata := carried_calldata,
                                      returndata := ⟨_, ⟨_, returndata_after⟩⟩ }))
                          else
                            (do
                              let ⟨_, ⟨_, initcode_bytes⟩⟩ ← do
                                (memory_code_slice carried_memory_base mem1 initcode.off
                                  initcode.len)
                              let child_code_id ← do
                                (code_db_insert ⟨_, ⟨_, initcode_bytes⟩⟩ profile.fork)
                              let ⟨_, ⟨_, child_code⟩⟩ ← do
                                (code_db_resolve child_code_id)
                              let child_state_gas := state_gas_after
                              let running := (Running ())
                              let (checkpoint, child_stack, child_memory_base, child_memory_height) ← do
                                (suspend_frame pc_after gas_after sp_after carried_memory_base mem1
                                  STATE_GAS_ZERO state_spill_after carried_refund running
                                  parent_message ⟨_, ⟨_, carried_code⟩⟩ carried_calldata)
                              let create_continuation : CreateContinuation :=
                                { checkpoint := checkpoint,
                                  address := new_addr,
                                  new_account_charged := new_account_charged }
                              let continuation := (ResumeCreate create_continuation)
                              (frame_stack_push continuation)
                              (k_mark_created new_addr)
                              (k_clear_storage new_addr)
                              (k_bump_nonce new_addr)
                              (k_transfer creator new_addr value)
                              let ⟨_, ⟨_, child_returndata⟩⟩ := (returndata_clear ())
                              (pure { pc := 0,
                                      gas_remaining := child_gas,
                                      state_gas_remaining := child_state_gas,
                                      state_gas_spilled := STATE_GAS_SPILL_ZERO,
                                      refund := GAS_REFUND_ZERO,
                                      status := running,
                                      stack_top := child_stack,
                                      memory_base := child_memory_base,
                                      memory_height := child_memory_height,
                                      message := { caller := creator,
                                                   address := new_addr,
                                                   code_address := new_addr,
                                                   value := value,
                                                   state_gas_reservoir := child_state_gas,
                                                   is_static := carried_is_static,
                                                   depth := child_depth },
                                      code := ⟨_, ⟨_, child_code⟩⟩,
                                      calldata := EMPTY_CALLDATA,
                                      returndata := ⟨_, ⟨_, child_returndata⟩⟩ }))))))))

/-- The non-recursive step loop for one complete call tree. It executes
the active frame, resumes suspended parents through
[frame_stack_pop][] as children halt, and returns the top-level
frame's output. Each step's carried state is supplied from the frame
registers and its returned state is assigned back; the handlers
themselves never touch the registers. `STOP`, `SELFDESTRUCT`, and
exceptional halts return the empty slice; `RETURN` and `REVERT` carry
their frozen memory slice in the halt value. -/
/- Type quantifiers: initial_code_dependentWitness1 : Nat, initial_code_dependentWitness0 : Nat, k_ex551085_
  : Nat, k_ex551084_ : Bool, k_ex551083_ : Nat, k_ex551082_ : Nat, k_ex551081_ : Nat, k_ex551080_ :
  Nat, k_ex551079_ : Int, k_ex551078_ : Nat, k_ex551077_ : Nat, k_ex551076_ : Nat, 0 ≤ k_ex551076_
  ∧ k_ex551076_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551077_ ∧ k_ex551077_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex551078_ ∧ k_ex551078_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1)))) ≤ k_ex551079_ ∧
  k_ex551079_ ≤ (199 * (2 ^ 64 - 1)), 0 ≤ k_ex551080_ ∧ k_ex551080_ ≤ (2 ^ 32 - 1), 0 ≤
  k_ex551081_ ∧ k_ex551081_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex551082_ ∧ k_ex551082_ ≤ (2 ^ 256 - 1), 0
  ≤ k_ex551083_ ∧ k_ex551083_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551085_ ∧ k_ex551085_ ≤ 1024, 0
  ≤ initial_code_dependentWitness0 ∧
  0 ≤ initial_code_dependentWitness1 ∧
  (initial_code_dependentWitness0 + initial_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ initial_code_dependentWitness1 ∧ (initial_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1) -/
def interpret (initial_gas : Nat) (initial_state_gas : Nat) (initial_state_spill : Nat) (initial_refund : Int) (initial_sp : StackPointer) (initial_memory_base : Nat) (initial_memory_height : Nat) (initial_caller : (Vector (BitVec 8) 20)) (initial_address : (Vector (BitVec 8) 20)) (initial_code_address : (Vector (BitVec 8) 20)) (initial_value : Nat) (initial_state_gas_reservoir : Nat) (initial_is_static : Bool) (initial_depth : Nat) (initial_code : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (initial_calldata : CalldataSlice) : SailM (Nat × Nat × Nat × Int × FrameStatus × (Sigma
  fun (initial_code_dependentWitness0 : Nat) =>
  (Sigma fun (initial_code_dependentWitness1 : Nat) =>
  (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) := do
  let initial_code_dependentWitness0 := (initial_code).1
  let initial_code_dependentWitness1 := ((initial_code).2).1
  let initial_code := ((initial_code).2).2
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  let fork := profile.fork
  let blob_fee ← do
    (blob_base_fee fork profile.blob_schedule profile.excess_blob_gas_limit
      (← readReg k_header).excess_blob_gas)
  (frame_stack_reset ())
  let interpreting : Bool := true
  let result : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
    ((⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩ : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
    fun (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
  let carried_pc : Nat := 0
  let carried_sp : StackPointer := initial_sp
  let carried_memory_base : Nat := initial_memory_base
  let carried_memory_height : Nat := initial_memory_height
  let carried_gas : Nat := initial_gas
  let carried_state_gas : Nat := initial_state_gas
  let carried_state_spill : Nat := initial_state_spill
  let carried_refund : Int := initial_refund
  let carried_status : FrameStatus := (Running ())
  let carried_caller : (Vector (BitVec 8) 20) := initial_caller
  let carried_address : (Vector (BitVec 8) 20) := initial_address
  let carried_account_context : AccountExecutionContext :=
    (account_execution_context initial_address)
  let carried_code_address : (Vector (BitVec 8) 20) := initial_code_address
  let carried_value : Nat := initial_value
  let carried_state_gas_reservoir : Nat := initial_state_gas_reservoir
  let carried_is_static : Bool := initial_is_static
  let carried_depth : Nat := initial_depth
  let carried_code : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
    ((⟨_, ⟨_, initial_code⟩⟩ : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma fun
    (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
  let carried_calldata : CalldataSlice := initial_calldata
  let carried_returndata : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
    ((⟨_, ⟨_, EMPTY_OUTPUT_SLICE⟩⟩ : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
    fun (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
  let initial_call_tree_gas := (initial_gas + initial_state_gas)
  let call_tree_steps_remaining : Nat := ((3 *i initial_call_tree_gas) + 2)
  let (call_tree_steps_remaining, carried_account_context, carried_address, carried_calldata, carried_caller, carried_code, carried_code_address, carried_depth, carried_gas, carried_is_static, carried_memory_base, carried_memory_height, carried_pc, carried_refund, carried_returndata, carried_sp, carried_state_gas, carried_state_gas_reservoir, carried_state_spill, carried_status, carried_value, interpreting, result) ← (( do
    let loop_vars ← whileFuelM (fuel :=call_tree_steps_remaining) (fun (call_tree_steps_remaining, carried_account_context, carried_address, carried_calldata, carried_caller, carried_code, carried_code_address, carried_depth, carried_gas, carried_is_static, carried_memory_base, carried_memory_height, carried_pc, carried_refund, carried_returndata, carried_sp, carried_state_gas, carried_state_gas_reservoir, carried_state_spill, carried_status, carried_value, interpreting, result) => (pure interpreting)) (call_tree_steps_remaining, carried_account_context, carried_address, carried_calldata, carried_caller, carried_code, carried_code_address, carried_depth, carried_gas, carried_is_static, carried_memory_base, carried_memory_height, carried_pc, carried_refund, carried_returndata, carried_sp, carried_state_gas, carried_state_gas_reservoir, carried_state_spill, carried_status, carried_value, interpreting, result)
      fun (call_tree_steps_remaining, carried_account_context, carried_address, carried_calldata, carried_caller, carried_code, carried_code_address, carried_depth, carried_gas, carried_is_static, carried_memory_base, carried_memory_height, carried_pc, carried_refund, carried_returndata, carried_sp, carried_state_gas, carried_state_gas_reservoir, carried_state_spill, carried_status, carried_value, interpreting, result) => do
        assert true "loop dummy assert"
        let running := (is_running carried_status)
        let (carried_account_context, carried_address, carried_calldata, carried_caller, carried_code, carried_code_address, carried_depth, carried_gas, carried_is_static, carried_memory_base, carried_memory_height, carried_pc, carried_refund, carried_returndata, carried_sp, carried_state_gas, carried_state_gas_reservoir, carried_state_spill, carried_status, carried_value, interpreting, result) ← (( do
          if _sailIf0 : (running : Bool) = true
          then
            (do
              let (fetched_pc, instruction) ← do (fetch carried_code carried_pc fork)
              let carried_pc : Nat := fetched_pc
              let (carried_account_context, carried_address, carried_calldata, carried_caller, carried_code, carried_code_address, carried_depth, carried_gas, carried_is_static, carried_memory_base, carried_memory_height, carried_pc, carried_refund, carried_returndata, carried_sp, carried_state_gas, carried_state_gas_reservoir, carried_state_spill, carried_status, carried_value) ← (( do
                match instruction with
                | .opcode_CREATE () =>
                  (do
                    let previous_address := carried_address
                    let transition ← do
                      (run_create carried_pc carried_gas carried_state_gas carried_state_spill
                        carried_refund carried_sp carried_memory_base carried_memory_height
                        carried_caller carried_address carried_code_address carried_value
                        carried_state_gas_reservoir carried_is_static carried_depth carried_code
                        carried_calldata carried_returndata CreateByNonce)
                    let carried_pc : Nat := transition.pc
                    let carried_gas : Nat := transition.gas_remaining
                    let carried_state_gas : Nat := transition.state_gas_remaining
                    let carried_state_spill : Nat := transition.state_gas_spilled
                    let carried_refund : Int := transition.refund
                    let carried_status : FrameStatus := transition.status
                    let carried_sp : StackPointer := transition.stack_top
                    let carried_memory_base : Nat := transition.memory_base
                    let carried_memory_height : Nat := transition.memory_height
                    let carried_caller : (Vector (BitVec 8) 20) := transition.message.caller
                    let carried_address : (Vector (BitVec 8) 20) := transition.message.address
                    let carried_code_address : (Vector (BitVec 8) 20) :=
                      transition.message.code_address
                    let carried_value : Nat := transition.message.value
                    let carried_state_gas_reservoir : Nat := transition.message.state_gas_reservoir
                    let carried_is_static : Bool := transition.message.is_static
                    let carried_depth : Nat := transition.message.depth
                    let carried_code : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (transition.code : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_calldata : CalldataSlice := transition.calldata
                    let carried_returndata : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (transition.returndata : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_account_context : AccountExecutionContext :=
                      (refresh_account_execution_context carried_account_context previous_address
                        carried_address)
                    (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory_base : Nat), (carried_memory_height : Nat), (carried_pc : Nat), (carried_refund : Int), ((⟨_, ⟨_, carried_returndata⟩⟩ : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat))))
                | .CREATE2 () =>
                  (do
                    let previous_address := carried_address
                    let transition ← do
                      (run_create carried_pc carried_gas carried_state_gas carried_state_spill
                        carried_refund carried_sp carried_memory_base carried_memory_height
                        carried_caller carried_address carried_code_address carried_value
                        carried_state_gas_reservoir carried_is_static carried_depth carried_code
                        carried_calldata carried_returndata CreateBySalt)
                    let carried_pc : Nat := transition.pc
                    let carried_gas : Nat := transition.gas_remaining
                    let carried_state_gas : Nat := transition.state_gas_remaining
                    let carried_state_spill : Nat := transition.state_gas_spilled
                    let carried_refund : Int := transition.refund
                    let carried_status : FrameStatus := transition.status
                    let carried_sp : StackPointer := transition.stack_top
                    let carried_memory_base : Nat := transition.memory_base
                    let carried_memory_height : Nat := transition.memory_height
                    let carried_caller : (Vector (BitVec 8) 20) := transition.message.caller
                    let carried_address : (Vector (BitVec 8) 20) := transition.message.address
                    let carried_code_address : (Vector (BitVec 8) 20) :=
                      transition.message.code_address
                    let carried_value : Nat := transition.message.value
                    let carried_state_gas_reservoir : Nat := transition.message.state_gas_reservoir
                    let carried_is_static : Bool := transition.message.is_static
                    let carried_depth : Nat := transition.message.depth
                    let carried_code : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (transition.code : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_calldata : CalldataSlice := transition.calldata
                    let carried_returndata : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (transition.returndata : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_account_context : AccountExecutionContext :=
                      (refresh_account_execution_context carried_account_context previous_address
                        carried_address)
                    (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory_base : Nat), (carried_memory_height : Nat), (carried_pc : Nat), (carried_refund : Int), ((⟨_, ⟨_, carried_returndata⟩⟩ : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat))))
                | .CALL () =>
                  (do
                    let previous_address := carried_address
                    let transition ← do
                      (run_call carried_pc carried_gas carried_state_gas carried_state_spill
                        carried_refund carried_sp carried_memory_base carried_memory_height
                        carried_caller carried_address carried_code_address carried_value
                        carried_state_gas_reservoir carried_is_static carried_depth carried_code
                        carried_calldata carried_returndata Call)
                    let carried_pc : Nat := transition.pc
                    let carried_gas : Nat := transition.gas_remaining
                    let carried_state_gas : Nat := transition.state_gas_remaining
                    let carried_state_spill : Nat := transition.state_gas_spilled
                    let carried_refund : Int := transition.refund
                    let carried_status : FrameStatus := transition.status
                    let carried_sp : StackPointer := transition.stack_top
                    let carried_memory_base : Nat := transition.memory_base
                    let carried_memory_height : Nat := transition.memory_height
                    let carried_caller : (Vector (BitVec 8) 20) := transition.message.caller
                    let carried_address : (Vector (BitVec 8) 20) := transition.message.address
                    let carried_code_address : (Vector (BitVec 8) 20) :=
                      transition.message.code_address
                    let carried_value : Nat := transition.message.value
                    let carried_state_gas_reservoir : Nat := transition.message.state_gas_reservoir
                    let carried_is_static : Bool := transition.message.is_static
                    let carried_depth : Nat := transition.message.depth
                    let carried_code : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (transition.code : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_calldata : CalldataSlice := transition.calldata
                    let carried_returndata : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (transition.returndata : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_account_context : AccountExecutionContext :=
                      (refresh_account_execution_context carried_account_context previous_address
                        carried_address)
                    (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory_base : Nat), (carried_memory_height : Nat), (carried_pc : Nat), (carried_refund : Int), ((⟨_, ⟨_, carried_returndata⟩⟩ : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat))))
                | .CALLCODE () =>
                  (do
                    let previous_address := carried_address
                    let transition ← do
                      (run_call carried_pc carried_gas carried_state_gas carried_state_spill
                        carried_refund carried_sp carried_memory_base carried_memory_height
                        carried_caller carried_address carried_code_address carried_value
                        carried_state_gas_reservoir carried_is_static carried_depth carried_code
                        carried_calldata carried_returndata CallCode)
                    let carried_pc : Nat := transition.pc
                    let carried_gas : Nat := transition.gas_remaining
                    let carried_state_gas : Nat := transition.state_gas_remaining
                    let carried_state_spill : Nat := transition.state_gas_spilled
                    let carried_refund : Int := transition.refund
                    let carried_status : FrameStatus := transition.status
                    let carried_sp : StackPointer := transition.stack_top
                    let carried_memory_base : Nat := transition.memory_base
                    let carried_memory_height : Nat := transition.memory_height
                    let carried_caller : (Vector (BitVec 8) 20) := transition.message.caller
                    let carried_address : (Vector (BitVec 8) 20) := transition.message.address
                    let carried_code_address : (Vector (BitVec 8) 20) :=
                      transition.message.code_address
                    let carried_value : Nat := transition.message.value
                    let carried_state_gas_reservoir : Nat := transition.message.state_gas_reservoir
                    let carried_is_static : Bool := transition.message.is_static
                    let carried_depth : Nat := transition.message.depth
                    let carried_code : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (transition.code : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_calldata : CalldataSlice := transition.calldata
                    let carried_returndata : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (transition.returndata : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_account_context : AccountExecutionContext :=
                      (refresh_account_execution_context carried_account_context previous_address
                        carried_address)
                    (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory_base : Nat), (carried_memory_height : Nat), (carried_pc : Nat), (carried_refund : Int), ((⟨_, ⟨_, carried_returndata⟩⟩ : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat))))
                | .DELEGATECALL () =>
                  (do
                    let previous_address := carried_address
                    let transition ← do
                      (run_call carried_pc carried_gas carried_state_gas carried_state_spill
                        carried_refund carried_sp carried_memory_base carried_memory_height
                        carried_caller carried_address carried_code_address carried_value
                        carried_state_gas_reservoir carried_is_static carried_depth carried_code
                        carried_calldata carried_returndata DelegateCall)
                    let carried_pc : Nat := transition.pc
                    let carried_gas : Nat := transition.gas_remaining
                    let carried_state_gas : Nat := transition.state_gas_remaining
                    let carried_state_spill : Nat := transition.state_gas_spilled
                    let carried_refund : Int := transition.refund
                    let carried_status : FrameStatus := transition.status
                    let carried_sp : StackPointer := transition.stack_top
                    let carried_memory_base : Nat := transition.memory_base
                    let carried_memory_height : Nat := transition.memory_height
                    let carried_caller : (Vector (BitVec 8) 20) := transition.message.caller
                    let carried_address : (Vector (BitVec 8) 20) := transition.message.address
                    let carried_code_address : (Vector (BitVec 8) 20) :=
                      transition.message.code_address
                    let carried_value : Nat := transition.message.value
                    let carried_state_gas_reservoir : Nat := transition.message.state_gas_reservoir
                    let carried_is_static : Bool := transition.message.is_static
                    let carried_depth : Nat := transition.message.depth
                    let carried_code : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (transition.code : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_calldata : CalldataSlice := transition.calldata
                    let carried_returndata : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (transition.returndata : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_account_context : AccountExecutionContext :=
                      (refresh_account_execution_context carried_account_context previous_address
                        carried_address)
                    (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory_base : Nat), (carried_memory_height : Nat), (carried_pc : Nat), (carried_refund : Int), ((⟨_, ⟨_, carried_returndata⟩⟩ : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat))))
                | .STATICCALL () =>
                  (do
                    let previous_address := carried_address
                    let transition ← do
                      (run_call carried_pc carried_gas carried_state_gas carried_state_spill
                        carried_refund carried_sp carried_memory_base carried_memory_height
                        carried_caller carried_address carried_code_address carried_value
                        carried_state_gas_reservoir carried_is_static carried_depth carried_code
                        carried_calldata carried_returndata StaticCall)
                    let carried_pc : Nat := transition.pc
                    let carried_gas : Nat := transition.gas_remaining
                    let carried_state_gas : Nat := transition.state_gas_remaining
                    let carried_state_spill : Nat := transition.state_gas_spilled
                    let carried_refund : Int := transition.refund
                    let carried_status : FrameStatus := transition.status
                    let carried_sp : StackPointer := transition.stack_top
                    let carried_memory_base : Nat := transition.memory_base
                    let carried_memory_height : Nat := transition.memory_height
                    let carried_caller : (Vector (BitVec 8) 20) := transition.message.caller
                    let carried_address : (Vector (BitVec 8) 20) := transition.message.address
                    let carried_code_address : (Vector (BitVec 8) 20) :=
                      transition.message.code_address
                    let carried_value : Nat := transition.message.value
                    let carried_state_gas_reservoir : Nat := transition.message.state_gas_reservoir
                    let carried_is_static : Bool := transition.message.is_static
                    let carried_depth : Nat := transition.message.depth
                    let carried_code : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (transition.code : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_calldata : CalldataSlice := transition.calldata
                    let carried_returndata : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (transition.returndata : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_account_context : AccountExecutionContext :=
                      (refresh_account_execution_context carried_account_context previous_address
                        carried_address)
                    (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory_base : Nat), (carried_memory_height : Nat), (carried_pc : Nat), (carried_refund : Int), ((⟨_, ⟨_, carried_returndata⟩⟩ : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat))))
                | _ =>
                  (do
                    let result ← (( do
                      match instruction with
                      | .STOP () =>
                        (let status_after := (execute_stop ())
                        (pure (carried_pc, carried_gas, carried_state_gas, carried_state_spill, carried_refund, carried_sp, carried_memory_height, status_after)))
                      | .ADD () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_add carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .MUL () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_mul carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .SUB () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_sub carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .DIV () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_div carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .SDIV () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_sdiv carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .MOD () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_mod carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .SMOD () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_smod carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .ADDMOD () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_addmod carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .MULMOD () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_mulmod carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .EXP () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_exp carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .SIGNEXTEND () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_signextend carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .LT () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_lt carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .GT () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_gt carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .SLT () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_slt carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .SGT () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_sgt carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .EQ () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_eq carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .ISZERO () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_iszero carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .AND () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_and carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .OR () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_or carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .XOR () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_xor carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .NOT () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_not carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .BYTE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_byte carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .SHL () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_shl carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .SHR () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_shr carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .SAR () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_sar carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .CLZ () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_clz carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .KECCAK256 () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_keccak256 carried_memory_base carried_gas carried_sp
                              carried_memory_height)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, memory_after, (opcode_frame_status
                              status_after))))
                      | .ADDRESS () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_address carried_address carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .BALANCE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_balance carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .ORIGIN () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_origin carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .CALLER () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_caller carried_caller carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .CALLVALUE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_callvalue carried_value carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .CALLDATALOAD () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_calldataload carried_calldata carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .CALLDATASIZE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_calldatasize carried_calldata carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .CALLDATACOPY () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_calldatacopy carried_calldata carried_memory_base carried_gas
                              carried_sp carried_memory_height)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, memory_after, (opcode_frame_status
                              status_after))))
                      | .CODESIZE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_codesize carried_code carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .CODECOPY () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_codecopy carried_code carried_memory_base carried_gas
                              carried_sp carried_memory_height)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, memory_after, (opcode_frame_status
                              status_after))))
                      | .GASPRICE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_gasprice carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .EXTCODESIZE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_extcodesize carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .EXTCODECOPY () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_extcodecopy carried_memory_base carried_gas carried_sp
                              carried_memory_height)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, memory_after, (opcode_frame_status
                              status_after))))
                      | .RETURNDATASIZE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_returndatasize carried_returndata carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .RETURNDATACOPY () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_returndatacopy carried_returndata carried_memory_base
                              carried_gas carried_sp carried_memory_height)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, memory_after, (opcode_frame_status
                              status_after))))
                      | .EXTCODEHASH () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_extcodehash carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .BLOCKHASH () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_blockhash carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .COINBASE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_coinbase carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .TIMESTAMP () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_timestamp carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .NUMBER () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_number carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .SLOTNUM () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_slotnum carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .PREVRANDAO () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_prevrandao carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .GASLIMIT () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_gaslimit carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .CHAINID () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_chainid carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .SELFBALANCE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_selfbalance carried_address carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .BASEFEE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_basefee carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .BLOBHASH () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_blobhash carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .BLOBBASEFEE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_blobbasefee blob_fee carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .POP () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_pop carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .MLOAD () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_mload carried_memory_base carried_gas carried_sp
                              carried_memory_height)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, memory_after, (opcode_frame_status
                              status_after))))
                      | .MSTORE () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_mstore carried_memory_base carried_gas carried_sp
                              carried_memory_height)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, memory_after, (opcode_frame_status
                              status_after))))
                      | .MSTORE8 () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_mstore8 carried_memory_base carried_gas carried_sp
                              carried_memory_height)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, memory_after, (opcode_frame_status
                              status_after))))
                      | .SLOAD () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_sload carried_account_context carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .SSTORE () =>
                        (do
                          let (gas_after, state_gas_after, state_spill_after, refund_after, sp_after, status_after) ← do
                            (execute_sstore carried_account_context fork carried_is_static
                              carried_gas carried_state_gas carried_state_spill carried_refund
                              carried_sp)
                          (pure (carried_pc, gas_after, state_gas_after, state_spill_after, refund_after, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .JUMP () =>
                        (do
                          let (pc_after, gas_after, sp_after, status_after) ← do
                            (execute_jump carried_code carried_pc carried_gas carried_sp)
                          (pure (pc_after, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .JUMPI () =>
                        (do
                          let (pc_after, gas_after, sp_after, status_after) ← do
                            (execute_jumpi carried_code carried_pc carried_gas carried_sp)
                          (pure (pc_after, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .PC () =>
                        (do
                          let (pc_after, gas_after, sp_after, status_after) ← do
                            (execute_pc carried_pc carried_gas carried_sp)
                          (pure (pc_after, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .MSIZE () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_msize carried_gas carried_sp carried_memory_height)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, memory_after, (opcode_frame_status
                              status_after))))
                      | .GAS () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_gas carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .JUMPDEST () =>
                        (let (gas_after, status_after) := (execute_jumpdest carried_gas)
                        (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, carried_sp, carried_memory_height, (opcode_frame_status
                            status_after))))
                      | .TLOAD () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_tload carried_address carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .TSTORE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_tstore carried_address carried_is_static carried_gas carried_sp)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .MCOPY () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_mcopy carried_memory_base carried_gas carried_sp
                              carried_memory_height)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, memory_after, (opcode_frame_status
                              status_after))))
                      | .PUSH (n, value) =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_push carried_gas carried_sp n value)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .DUP n =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_dup carried_gas carried_sp n)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .SWAP n =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_swap carried_gas carried_sp n)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .DUPN immediate =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_dupn carried_gas carried_sp immediate)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .SWAPN immediate =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_swapn carried_gas carried_sp immediate)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .EXCHANGE immediate =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_exchange carried_gas carried_sp immediate)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, carried_memory_height, (opcode_frame_status
                              status_after))))
                      | .LOG n =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_log carried_address carried_is_static carried_memory_base n
                              carried_gas carried_sp carried_memory_height)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, memory_after, (opcode_frame_status
                              status_after))))
                      | .opcode_CREATE () => (fatal_error ExecutionInvalid)
                      | .CREATE2 () => (fatal_error ExecutionInvalid)
                      | .CALL () => (fatal_error ExecutionInvalid)
                      | .CALLCODE () => (fatal_error ExecutionInvalid)
                      | .DELEGATECALL () => (fatal_error ExecutionInvalid)
                      | .STATICCALL () => (fatal_error ExecutionInvalid)
                      | .RETURN () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_return carried_memory_base carried_gas carried_sp
                              carried_memory_height)
                          (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, sp_after, memory_after, status_after)))
                      | .REVERT () =>
                        (do
                          let (gas_after, state_gas_after, state_spill_after, sp_after, memory_after, status_after) ← do
                            (execute_revert carried_state_gas_reservoir carried_memory_base
                              carried_gas carried_state_gas carried_state_spill carried_sp
                              carried_memory_height)
                          (pure (carried_pc, gas_after, state_gas_after, state_spill_after, carried_refund, sp_after, memory_after, status_after)))
                      | .INVALID () =>
                        (let (gas_after, status_after) := (execute_invalid carried_gas)
                        (pure (carried_pc, gas_after, carried_state_gas, carried_state_spill, carried_refund, carried_sp, carried_memory_height, (opcode_frame_status
                            status_after))))
                      | .SELFDESTRUCT () =>
                        (do
                          let (gas_after, state_gas_after, state_spill_after, refund_after, sp_after, status_after) ← do
                            (execute_selfdestruct carried_address fork carried_is_static carried_gas
                              carried_state_gas carried_state_spill carried_refund carried_sp)
                          (pure (carried_pc, gas_after, state_gas_after, state_spill_after, refund_after, sp_after, carried_memory_height, status_after)))
                      ) : SailM
                      (Nat × Nat × Nat × Nat × Int × StackPointer × Nat × FrameStatus) )
                    let (tup__0, tup__1, tup__2, tup__3, tup__4, tup__5, tup__6, tup__7) ← do
                      match result with
                      | (pc_after, _, state_gas_after, state_spill_after, refund_after, sp_after, memory_after, .Exceptional kind) =>
                        (do
                          let exceptional ← do
                            (exceptional_state state_gas_after state_spill_after
                              carried_state_gas_reservoir kind)
                          let state_gas_after := exceptional.state_gas_remaining
                          let state_spill_after := exceptional.state_gas_spilled
                          let status_after := exceptional.status
                          (pure (pc_after, GAS_ZERO, state_gas_after, state_spill_after, refund_after, sp_after, memory_after, status_after)))
                      | _ => (pure result)
                    let carried_pc : Nat := tup__0
                    let carried_gas : Nat := tup__1
                    let carried_state_gas : Nat := tup__2
                    let carried_state_spill : Nat := tup__3
                    let carried_refund : Int := tup__4
                    let carried_sp : StackPointer := tup__5
                    let carried_memory_height : Nat := tup__6
                    let carried_status : FrameStatus := tup__7
                    (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), (carried_code : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory_base : Nat), (carried_memory_height : Nat), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat))))
                ) : SailM
                (AccountExecutionContext × (Vector (BitVec 8) 20) × CalldataSlice × (Vector (BitVec 8) 20) × (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × Nat × Nat × Int × (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × StackPointer × Nat × Nat × Nat × FrameStatus × Nat)
                )
              (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), (carried_code : (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory_base : Nat), (carried_memory_height : Nat), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat), (interpreting : Bool), (result : (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))))))
          else
            (do
              let ⟨_, ⟨_, output⟩⟩ := (frame_output carried_status)
              let continuation ← do (frame_stack_pop ())
              let (carried_account_context, carried_address, carried_calldata, carried_caller, carried_code, carried_code_address, carried_depth, carried_gas, carried_is_static, carried_memory_base, carried_memory_height, carried_pc, carried_refund, carried_returndata, carried_sp, carried_state_gas, carried_state_gas_reservoir, carried_state_spill, carried_status, carried_value, interpreting, result) ← (( do
                match continuation with
                | .Empty () =>
                  (let result : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                    (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                    ((⟨_, ⟨_, output⟩⟩ : (Sigma fun (initial_code_dependentWitness0 : Nat)
                    =>
                    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                    (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
                    fun (initial_code_dependentWitness0 : Nat) =>
                    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                    (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                  let interpreting : Bool := false
                  (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), (carried_code : (Sigma
                    fun (initial_code_dependentWitness0 : Nat) =>
                    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                    (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory_base : Nat), (carried_memory_height : Nat), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
                    fun (initial_code_dependentWitness0 : Nat) =>
                    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                    (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat), (interpreting : Bool), (result : (Sigma
                    fun (initial_code_dependentWitness0 : Nat) =>
                    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                    (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))))))
                | continuation =>
                  (do
                    let previous_address := carried_address
                    let transition ← do
                      (resume_frame continuation ⟨_, ⟨_, output⟩⟩ carried_memory_base
                        carried_gas carried_state_gas carried_state_spill carried_refund
                        carried_status carried_state_gas_reservoir)
                    let carried_pc : Nat := transition.pc
                    let carried_gas : Nat := transition.gas_remaining
                    let carried_state_gas : Nat := transition.state_gas_remaining
                    let carried_state_spill : Nat := transition.state_gas_spilled
                    let carried_refund : Int := transition.refund
                    let carried_status : FrameStatus := transition.status
                    let carried_sp : StackPointer := transition.stack_top
                    let carried_memory_base : Nat := transition.memory_base
                    let carried_memory_height : Nat := transition.memory_height
                    let carried_caller : (Vector (BitVec 8) 20) := transition.message.caller
                    let carried_address : (Vector (BitVec 8) 20) := transition.message.address
                    let carried_code_address : (Vector (BitVec 8) 20) :=
                      transition.message.code_address
                    let carried_value : Nat := transition.message.value
                    let carried_state_gas_reservoir : Nat := transition.message.state_gas_reservoir
                    let carried_is_static : Bool := transition.message.is_static
                    let carried_depth : Nat := transition.message.depth
                    let carried_code : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (transition.code : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_calldata : CalldataSlice := transition.calldata
                    let carried_returndata : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (transition.returndata : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_account_context : AccountExecutionContext :=
                      (refresh_account_execution_context carried_account_context previous_address
                        carried_address)
                    (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory_base : Nat), (carried_memory_height : Nat), (carried_pc : Nat), (carried_refund : Int), ((⟨_, ⟨_, carried_returndata⟩⟩ : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat), (interpreting : Bool), (result : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))))))
                ) : SailM
                (AccountExecutionContext × (Vector (BitVec 8) 20) × CalldataSlice × (Vector (BitVec 8) 20) × (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × Nat × Nat × Int × (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × StackPointer × Nat × Nat × Nat × FrameStatus × Nat × Bool × (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                )
              (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), (carried_code : (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory_base : Nat), (carried_memory_height : Nat), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat), (interpreting : Bool), (result : (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))))))
          ) : SailM
          (AccountExecutionContext × (Vector (BitVec 8) 20) × CalldataSlice × (Vector (BitVec 8) 20) × (Sigma
          fun (initial_code_dependentWitness0 : Nat) =>
          (Sigma fun (initial_code_dependentWitness1 : Nat) =>
          (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × Nat × Nat × Int × (Sigma
          fun (initial_code_dependentWitness0 : Nat) =>
          (Sigma fun (initial_code_dependentWitness1 : Nat) =>
          (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × StackPointer × Nat × Nat × Nat × FrameStatus × Nat × Bool × (Sigma
          fun (initial_code_dependentWitness0 : Nat) =>
          (Sigma fun (initial_code_dependentWitness1 : Nat) =>
          (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) )
        let remaining_steps := call_tree_steps_remaining
        let call_tree_steps_remaining : Nat :=
          if ((remaining_steps == 0) : Bool)
          then 0
          else (remaining_steps - 1)
        (pure ((call_tree_steps_remaining : Nat), (carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), (carried_code : (Sigma
          fun (initial_code_dependentWitness0 : Nat) =>
          (Sigma fun (initial_code_dependentWitness1 : Nat) =>
          (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory_base : Nat), (carried_memory_height : Nat), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
          fun (initial_code_dependentWitness0 : Nat) =>
          (Sigma fun (initial_code_dependentWitness1 : Nat) =>
          (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat), (interpreting : Bool), (result : (Sigma
          fun (initial_code_dependentWitness0 : Nat) =>
          (Sigma fun (initial_code_dependentWitness1 : Nat) =>
          (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))))
    (pure loop_vars) ) : SailM
    (Nat × AccountExecutionContext × (Vector (BitVec 8) 20) × CalldataSlice × (Vector (BitVec 8) 20) × (Sigma
    fun (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × Nat × Nat × Int × (Sigma
    fun (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × StackPointer × Nat × Nat × Nat × FrameStatus × Nat × Bool × (Sigma
    fun (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) )
  (pure ((carried_gas : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (carried_status : FrameStatus), (result : (Sigma
    fun (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))))

def undefined_CallSemantics (_ : Unit) : SailM CallSemantics := do
  (pure { takes_value := ← (undefined_bool ()),
          transfers_value := ← (undefined_bool ()),
          uses_target_address := ← (undefined_bool ()),
          inherits_caller_and_value := ← (undefined_bool ()),
          enters_static_context := ← (undefined_bool ()) })

def undefined_CreateSemantics (_ : Unit) : SailM CreateSemantics := do
  (pure { uses_salt := ← (undefined_bool ()) })

/-- Selects the canonical frame-entering operation from its encoded opcode.
The optimized threaded dispatcher therefore owns only label routing; CALL
and CREATE classification remains part of the executable specification. -/
/- Type quantifiers: k_ex551114_ : Nat, carried_returndata_dependentWitness1 : Nat, carried_returndata_dependentWitness0
  : Nat, carried_code_dependentWitness1 : Nat, carried_code_dependentWitness0 : Nat, k_ex551105_ :
  Nat, k_ex551104_ : Bool, k_ex551103_ : Nat, k_ex551102_ : Nat, k_ex551101_ : Nat, k_ex551100_ :
  Nat, k_ex551099_ : Int, k_ex551098_ : Nat, k_ex551097_ : Nat, k_ex551096_ : Nat, k_ex551095_ : Nat, 0
  ≤ k_ex551095_ ∧ k_ex551095_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex551096_ ∧
  k_ex551096_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551097_ ∧ k_ex551097_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex551098_ ∧ k_ex551098_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1)))) ≤ k_ex551099_ ∧
  k_ex551099_ ≤ (199 * (2 ^ 64 - 1)), 0 ≤ k_ex551100_ ∧ k_ex551100_ ≤ (2 ^ 32 - 1), 0 ≤
  k_ex551101_ ∧ k_ex551101_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex551102_ ∧ k_ex551102_ ≤ (2 ^ 256 - 1), 0
  ≤ k_ex551103_ ∧ k_ex551103_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex551105_ ∧ k_ex551105_ ≤ 1024, 0
  ≤ carried_code_dependentWitness0 ∧
  0 ≤ carried_code_dependentWitness1 ∧
  (carried_code_dependentWitness0 + carried_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ carried_code_dependentWitness1 ∧ (carried_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0
  ≤ carried_returndata_dependentWitness0 ∧
  0 ≤ carried_returndata_dependentWitness1 ∧
  (carried_returndata_dependentWitness0 + carried_returndata_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex551114_ ∧ k_ex551114_ ≤ 255 -/
def run_frame_entry_encoded (carried_pc : Nat) (carried_gas : Nat) (carried_state_gas : Nat) (carried_state_spill : Nat) (carried_refund : Int) (carried_sp : StackPointer) (carried_memory_base : Nat) (carried_memory_height : Nat) (carried_caller : (Vector (BitVec 8) 20)) (carried_address : (Vector (BitVec 8) 20)) (carried_code_address : (Vector (BitVec 8) 20)) (carried_value : Nat) (carried_state_gas_reservoir : Nat) (carried_is_static : Bool) (carried_depth : Nat) (carried_code : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (carried_calldata : CalldataSlice) (carried_returndata : (Sigma
  fun (carried_code_dependentWitness0 : Nat) =>
  (Sigma fun (carried_code_dependentWitness1 : Nat) =>
  (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) (opcode : Nat) : SailM FrameTransition := do
  let carried_code_dependentWitness0 := (carried_code).1
  let carried_code_dependentWitness1 := ((carried_code).2).1
  let carried_code := ((carried_code).2).2
  let carried_returndata_dependentWitness0 := (carried_returndata).1
  let carried_returndata_dependentWitness1 := ((carried_returndata).2).1
  let carried_returndata := ((carried_returndata).2).2
  match opcode with
  | 240 =>
    (run_create carried_pc carried_gas carried_state_gas carried_state_spill carried_refund
      carried_sp carried_memory_base carried_memory_height carried_caller carried_address
      carried_code_address carried_value carried_state_gas_reservoir carried_is_static carried_depth
      ⟨_, ⟨_, carried_code⟩⟩ carried_calldata ⟨_, ⟨_, carried_returndata⟩⟩
      CreateByNonce)
  | 241 =>
    (run_call carried_pc carried_gas carried_state_gas carried_state_spill carried_refund carried_sp
      carried_memory_base carried_memory_height carried_caller carried_address carried_code_address
      carried_value carried_state_gas_reservoir carried_is_static carried_depth
      ⟨_, ⟨_, carried_code⟩⟩ carried_calldata ⟨_, ⟨_, carried_returndata⟩⟩ Call)
  | 242 =>
    (run_call carried_pc carried_gas carried_state_gas carried_state_spill carried_refund carried_sp
      carried_memory_base carried_memory_height carried_caller carried_address carried_code_address
      carried_value carried_state_gas_reservoir carried_is_static carried_depth
      ⟨_, ⟨_, carried_code⟩⟩ carried_calldata ⟨_, ⟨_, carried_returndata⟩⟩ CallCode)
  | 244 =>
    (run_call carried_pc carried_gas carried_state_gas carried_state_spill carried_refund carried_sp
      carried_memory_base carried_memory_height carried_caller carried_address carried_code_address
      carried_value carried_state_gas_reservoir carried_is_static carried_depth
      ⟨_, ⟨_, carried_code⟩⟩ carried_calldata ⟨_, ⟨_, carried_returndata⟩⟩
      DelegateCall)
  | 245 =>
    (run_create carried_pc carried_gas carried_state_gas carried_state_spill carried_refund
      carried_sp carried_memory_base carried_memory_height carried_caller carried_address
      carried_code_address carried_value carried_state_gas_reservoir carried_is_static carried_depth
      ⟨_, ⟨_, carried_code⟩⟩ carried_calldata ⟨_, ⟨_, carried_returndata⟩⟩
      CreateBySalt)
  | 250 =>
    (run_call carried_pc carried_gas carried_state_gas carried_state_spill carried_refund carried_sp
      carried_memory_base carried_memory_height carried_caller carried_address carried_code_address
      carried_value carried_state_gas_reservoir carried_is_static carried_depth
      ⟨_, ⟨_, carried_code⟩⟩ carried_calldata ⟨_, ⟨_, carried_returndata⟩⟩
      StaticCall)
  | _ =>
    (do
      let exceptional ← do
        (exceptional_state carried_state_gas carried_state_spill carried_state_gas_reservoir
          InvalidOpcode)
      let state_gas_after := exceptional.state_gas_remaining
      let state_spill_after := exceptional.state_gas_spilled
      let status_after := exceptional.status
      (pure { pc := carried_pc,
              gas_remaining := GAS_ZERO,
              state_gas_remaining := state_gas_after,
              state_gas_spilled := state_spill_after,
              refund := carried_refund,
              status := status_after,
              stack_top := carried_sp,
              memory_base := carried_memory_base,
              memory_height := carried_memory_height,
              message := { caller := carried_caller,
                           address := carried_address,
                           code_address := carried_code_address,
                           value := carried_value,
                           state_gas_reservoir := carried_state_gas_reservoir,
                           is_static := carried_is_static,
                           depth := carried_depth },
              code := ⟨_, ⟨_, carried_code⟩⟩,
              calldata := carried_calldata,
              returndata := ⟨_, ⟨_, carried_returndata⟩⟩ }))

