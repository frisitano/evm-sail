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
/- Type quantifiers: k_ex610174_ : Nat, k_ex610173_ : Nat, code_dependentWitness1 : Nat, code_dependentWitness0
  : Nat, 0 ≤ code_dependentWitness0 ∧
  0 ≤ code_dependentWitness1 ∧
  (code_dependentWitness0 + code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ code_dependentWitness1 ∧ (code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤ k_ex610173_
  ∧ k_ex610173_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex610174_ ∧ k_ex610174_ ≤ 32 -/
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
/- Type quantifiers: k_ex610176_ : Nat, k_ex610175_ : Nat, 0 ≤ k_ex610175_ ∧ k_ex610175_ ≤ 255, 0
  ≤ k_ex610176_ ∧ k_ex610176_ ≤ 16 -/
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
/- Type quantifiers: k_ex610185_ : Nat, k_ex610184_ : Nat, frame_code_dependentWitness1 : Nat, frame_code_dependentWitness0
  : Nat, 0 ≤ frame_code_dependentWitness0 ∧
  0 ≤ frame_code_dependentWitness1 ∧
  (frame_code_dependentWitness0 + frame_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ frame_code_dependentWitness1 ∧ (frame_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex610184_ ∧ k_ex610184_ ≤ (2 ^ 32 - 1 - 32), 0 ≤ k_ex610185_ ∧ k_ex610185_ ≤ 32 -/
def decode_push_immediate (frame_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (immediate_offset : Nat) (width : Nat) : SailM (Nat × Nat) := do
  let frame_code_dependentWitness0 := (frame_code).1
  let frame_code_dependentWitness1 := ((frame_code).2).1
  let frame_code := ((frame_code).2).2
  let value ← do (read_push ⟨_, ⟨_, (code_bytes frame_code)⟩⟩ immediate_offset width)
  (pure ((immediate_offset + width), value))

/-- Decodes the immediate of an Amsterdam deep-stack instruction. A valid
immediate advances the counter; an invalid immediate remains unconsumed so
the handler can report `InvalidOpcode` with the canonical instruction
boundary. Reads beyond code are zero-padded. -/
/- Type quantifiers: k_ex610193_ : Nat, frame_code_dependentWitness1 : Nat, frame_code_dependentWitness0
  : Nat, 0 ≤ frame_code_dependentWitness0 ∧
  0 ≤ frame_code_dependentWitness1 ∧
  (frame_code_dependentWitness0 + frame_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ frame_code_dependentWitness1 ∧ (frame_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex610193_ ∧ k_ex610193_ ≤ (2 ^ 32 - 1 - 32) -/
def decode_deep_immediate (frame_code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (immediate_offset : Nat) (operation : DeepStackOperation) : SailM (Nat × (BitVec 8)) := do
  let frame_code_dependentWitness0 := (frame_code).1
  let frame_code_dependentWitness1 := ((frame_code).2).1
  let frame_code := ((frame_code).2).2
  let immediate ← do (code_slice_byte ⟨_, ⟨_, (code_bytes frame_code)⟩⟩ immediate_offset)
  let next_pc : Nat :=
    if ((deep_stack_operation_immediate_valid operation immediate) : Bool)
    then (immediate_offset + 1)
    else immediate_offset
  (pure (next_pc, immediate))

/-- Executes an encoded PUSH instruction from its immediate cursor. This is
the shared semantic boundary used by raw-byte interpreters: decoding, PC
progression, stack validation, gas charging, and the stack effect remain
generated from Sail. -/
/- Type quantifiers: k_ex610203_ : Nat, k_ex610202_ : Nat, k_ex610201_ : Nat, frame_code_dependentWitness1
  : Nat, frame_code_dependentWitness0 : Nat, 0 ≤ frame_code_dependentWitness0 ∧
  0 ≤ frame_code_dependentWitness1 ∧
  (frame_code_dependentWitness0 + frame_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ frame_code_dependentWitness1 ∧ (frame_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex610201_ ∧ k_ex610201_ ≤ 255, 0 ≤ k_ex610202_ ∧ k_ex610202_ ≤ (2 ^ 32 - 1 - 32), 0
  ≤ k_ex610203_ ∧ k_ex610203_ ≤ (2 ^ 64 - 1) -/
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
/- Type quantifiers: k_ex610205_ : Nat, k_ex610204_ : Nat, 0 ≤ k_ex610204_ ∧ k_ex610204_ ≤ 255, 0
  ≤ k_ex610205_ ∧ k_ex610205_ ≤ (2 ^ 64 - 1) -/
def execute_dup_encoded (opcode : Nat) (execution_gas : Nat) (sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  if (((128 ≤b opcode) && (opcode ≤b 143)) : Bool)
  then (execute_dup execution_gas sp (opcode - 127))
  else
    (let (gas_after, status_after) := (execute_invalid execution_gas)
    (pure (gas_after, sp, status_after)))

/-- Executes one opcode from the `SWAP1` through `SWAP16` family. -/
/- Type quantifiers: k_ex610207_ : Nat, k_ex610206_ : Nat, 0 ≤ k_ex610206_ ∧ k_ex610206_ ≤ 255, 0
  ≤ k_ex610207_ ∧ k_ex610207_ ≤ (2 ^ 64 - 1) -/
def execute_swap_encoded (opcode : Nat) (execution_gas : Nat) (sp : StackPointer) : SailM (Nat × StackPointer × OpcodeOutcome) := do
  if (((144 ≤b opcode) && (opcode ≤b 159)) : Bool)
  then (execute_swap execution_gas sp (opcode - 143))
  else
    (let (gas_after, status_after) := (execute_invalid execution_gas)
    (pure (gas_after, sp, status_after)))

/-- Executes one opcode from the `LOG0` through `LOG4` family. -/
/- Type quantifiers: k_ex610218_ : Nat, memory_dependentWitness1 : Nat, memory_dependentWitness0 :
  Nat, k_ex610213_ : Nat, k_ex610212_ : Bool, 0 ≤ k_ex610213_ ∧ k_ex610213_ ≤ (2 ^ 64 - 1), 0
  ≤ memory_dependentWitness0 ∧
  0 ≤ memory_dependentWitness1 ∧
  (memory_dependentWitness0 + memory_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ k_ex610218_ ∧
  k_ex610218_ ≤ 255 -/
def execute_log_encoded (carried_address : (Vector (BitVec 8) 20)) (carried_is_static : Bool) (execution_gas : Nat) (sp : StackPointer) (memory : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (opcode : Nat) : SailM (Nat × StackPointer × (Sigma
  fun (memory_dependentWitness0 : Nat) =>
  (Sigma fun (memory_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields memory_dependentWitness0 memory_dependentWitness1))) × OpcodeOutcome) := do
  let memory_dependentWitness0 := (memory).1
  let memory_dependentWitness1 := ((memory).2).1
  let memory := ((memory).2).2
  if _sailIf0 : (((160 ≤b opcode) && (opcode ≤b 164)) : Bool) = true
  then
    (do
      (execute_log carried_address carried_is_static execution_gas sp ⟨_, ⟨_, memory⟩⟩
        (opcode - 160)))
  else
    (let (gas_after, status_after) := (execute_invalid execution_gas)
    (pure ((gas_after : Nat), (sp : StackPointer), ((⟨_, ⟨_, memory⟩⟩ : (Sigma fun
      (memory_dependentWitness0 : Nat) =>
      (Sigma fun (memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields memory_dependentWitness0 memory_dependentWitness1)))) : (Sigma fun
      (memory_dependentWitness0 : Nat) =>
      (Sigma fun (memory_dependentWitness1 : Nat) =>
      (EvmMemorySliceFields memory_dependentWitness0 memory_dependentWitness1)))), (status_after : OpcodeOutcome))))

/-- Executes an encoded Amsterdam deep-stack instruction from its immediate
cursor. Opcode classification and immediate validity are specification
semantics; the raw-byte interpreter supplies only the opcode byte. -/
/- Type quantifiers: k_ex610228_ : Nat, k_ex610227_ : Nat, k_ex610226_ : Nat, frame_code_dependentWitness1
  : Nat, frame_code_dependentWitness0 : Nat, 0 ≤ frame_code_dependentWitness0 ∧
  0 ≤ frame_code_dependentWitness1 ∧
  (frame_code_dependentWitness0 + frame_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ frame_code_dependentWitness1 ∧ (frame_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex610226_ ∧ k_ex610226_ ≤ 255, 0 ≤ k_ex610227_ ∧ k_ex610227_ ≤ (2 ^ 32 - 1 - 32), 0
  ≤ k_ex610228_ ∧ k_ex610228_ ≤ (2 ^ 64 - 1) -/
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
/- Type quantifiers: k_ex610230_ : Nat, k_ex610229_ : Nat, 0 ≤ k_ex610229_ ∧ k_ex610229_ ≤ 255, 0
  ≤ k_ex610230_ ∧ k_ex610230_ ≤ 16 -/
def decode_simple (opcode : Nat) (fork : Nat) : ast :=
  if ((! (opcode_available opcode fork)) : Bool)
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
/- Type quantifiers: k_ex610239_ : Nat, k_ex610238_ : Nat, frame_code_dependentWitness1 : Nat, frame_code_dependentWitness0
  : Nat, 0 ≤ frame_code_dependentWitness0 ∧
  0 ≤ frame_code_dependentWitness1 ∧
  (frame_code_dependentWitness0 + frame_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ frame_code_dependentWitness1 ∧ (frame_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex610238_ ∧ k_ex610238_ ≤ (2 ^ 32 - 1), 0 ≤ k_ex610239_ ∧ k_ex610239_ ≤ 16 -/
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
      if ((! (opcode_available opcode fork)) : Bool)
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
/- Type quantifiers: k_ex610258_ : Int, k_ex610257_ : Nat, k_ex610256_ : Nat, k_ex610255_ : Nat, output_dependentWitness1
  : Nat, output_dependentWitness0 : Nat, 0 ≤ output_dependentWitness0 ∧
  0 ≤ output_dependentWitness1 ∧
  (output_dependentWitness0 + output_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ k_ex610255_ ∧
  k_ex610255_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex610256_ ∧ k_ex610256_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex610257_ ∧ k_ex610257_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1)))) ≤ k_ex610258_ ∧
  k_ex610258_ ≤ (199 * (2 ^ 64 - 1)) -/
def resume_call (continuation : CallContinuation) (output : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) (child_gas : Nat) (child_state_gas : Nat) (child_state_spill : Nat) (child_refund : Int) (child_status : FrameStatus) : SailM (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
  fun (output_dependentWitness0 : Nat) =>
  (Sigma fun (output_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields output_dependentWitness0 output_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
  fun (output_dependentWitness0 : Nat) =>
  (Sigma fun (output_dependentWitness1 : Nat) =>
  (CodeFields output_dependentWitness0 output_dependentWitness1))) × CalldataSlice × (Sigma fun
  (output_dependentWitness0 : Nat) =>
  (Sigma fun (output_dependentWitness1 : Nat) =>
  (OutputSliceFields output_dependentWitness0 output_dependentWitness1)))) := do
  let output_dependentWitness0 := (output).1
  let output_dependentWitness1 := ((output).2).1
  let output := ((output).2).2
  let checkpoint := continuation.checkpoint
  let succeeded := (frame_succeeded child_status)
  let (parent_pc, restored_gas, restored_sp, parent_memory, restored_state_gas, restored_state_spill, restored_refund, restored_status, parent_message, parent_code, parent_calldata) ← do
    (restore_frame checkpoint)
  let parent_gas ← do (refund_gas restored_gas child_gas)
  let parent_state_gas : Nat := restored_state_gas
  let parent_state_spill : Nat := restored_state_spill
  let (tup__0, tup__1) ← do
    (return_child_state_gas parent_state_gas parent_state_spill child_state_gas child_state_spill)
  let parent_state_gas : Nat := tup__0
  let parent_state_spill : Nat := tup__1
  (pure ())
  let parent_refund : Int := restored_refund
  let parent_sp : StackPointer := restored_sp
  (returndata_copy_prefix ⟨_, ⟨_, output⟩⟩ continuation.return_offset
    continuation.return_length)
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
  (pure ((parent_pc : Nat), (parent_gas : Nat), (parent_state_gas : Nat), (parent_state_spill : Nat), (parent_refund : Int), (restored_status : FrameStatus), (parent_sp : StackPointer), (parent_memory : (Sigma
    fun (output_dependentWitness0 : Nat) =>
    (Sigma fun (output_dependentWitness1 : Nat) =>
    (EvmMemorySliceFields output_dependentWitness0 output_dependentWitness1)))), (parent_message.caller : (Vector (BitVec 8) 20)), (parent_message.address : (Vector (BitVec 8) 20)), (parent_message.code_address : (Vector (BitVec 8) 20)), (parent_message.value : Nat), (parent_message.state_gas_reservoir : Nat), (parent_message.is_static : Bool), (parent_message.depth : Nat), (parent_code : (Sigma
    fun (output_dependentWitness0 : Nat) =>
    (Sigma fun (output_dependentWitness1 : Nat) =>
    (CodeFields output_dependentWitness0 output_dependentWitness1)))), (parent_calldata : CalldataSlice), ((⟨_, ⟨_, output⟩⟩ : (Sigma
    fun (output_dependentWitness0 : Nat) =>
    (Sigma fun (output_dependentWitness1 : Nat) =>
    (OutputSliceFields output_dependentWitness0 output_dependentWitness1)))) : (Sigma fun
    (output_dependentWitness0 : Nat) =>
    (Sigma fun (output_dependentWitness1 : Nat) =>
    (OutputSliceFields output_dependentWitness0 output_dependentWitness1))))))

/-- Restores a create parent and either deploys or rolls back the child. -/
/- Type quantifiers: k_ex610276_ : Nat, k_ex610275_ : Int, k_ex610274_ : Nat, k_ex610273_ : Nat, k_ex610272_
  : Nat, output_dependentWitness1 : Nat, output_dependentWitness0 : Nat, 0 ≤
  output_dependentWitness0 ∧
  0 ≤ output_dependentWitness1 ∧
  (output_dependentWitness0 + output_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ k_ex610272_ ∧
  k_ex610272_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex610273_ ∧ k_ex610273_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex610274_ ∧ k_ex610274_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1)))) ≤ k_ex610275_ ∧
  k_ex610275_ ≤ (199 * (2 ^ 64 - 1)), 0 ≤ k_ex610276_ ∧ k_ex610276_ ≤ (2 ^ 64 - 1) -/
def resume_create (continuation : CreateContinuation) (output : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) (child_gas : Nat) (child_state_gas : Nat) (child_state_spill : Nat) (child_refund : Int) (child_status : FrameStatus) (child_state_gas_reservoir : Nat) : SailM (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
  fun (output_dependentWitness0 : Nat) =>
  (Sigma fun (output_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields output_dependentWitness0 output_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
  fun (output_dependentWitness0 : Nat) =>
  (Sigma fun (output_dependentWitness1 : Nat) =>
  (CodeFields output_dependentWitness0 output_dependentWitness1))) × CalldataSlice × (Sigma fun
  (output_dependentWitness0 : Nat) =>
  (Sigma fun (output_dependentWitness1 : Nat) =>
  (OutputSliceFields output_dependentWitness0 output_dependentWitness1)))) := do
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
              let (tup__0, tup__1, tup__2) ← do
                (exceptional_state settled_child_state_gas settled_child_state_spill
                  child_state_gas_reservoir OutOfGas)
              let settled_child_state_gas : Nat := tup__0
              let settled_child_state_spill : Nat := tup__1
              let settled_child_status : FrameStatus := tup__2
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
                    let (deployment_halt, deployment_gas, deployment_state_gas, deployment_state_spill) ← do
                      (charge_deployment_state_gas settled_child_gas settled_child_state_gas
                        settled_child_state_spill state_deposit)
                    let settled_child_gas : Nat := deployment_gas
                    let settled_child_state_gas : Nat := deployment_state_gas
                    let settled_child_state_spill : Nat := deployment_state_spill
                    let (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status) ← (( do
                      if (deployment_halt : Bool)
                      then
                        (do
                          let settled_child_gas : Nat := GAS_ZERO
                          let (tup__0, tup__1, tup__2) ← do
                            (exceptional_state settled_child_state_gas settled_child_state_spill
                              child_state_gas_reservoir OutOfGas)
                          let settled_child_state_gas : Nat := tup__0
                          let settled_child_state_spill : Nat := tup__1
                          let settled_child_status : FrameStatus := tup__2
                          (pure (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status)))
                      else
                        (pure (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status))
                      ) : SailM (Nat × Nat × Nat × FrameStatus) )
                    (pure (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status)))
                else
                  (do
                    let settled_child_gas : Nat := GAS_ZERO
                    let (tup__0, tup__1, tup__2) ← do
                      (exceptional_state settled_child_state_gas settled_child_state_spill
                        child_state_gas_reservoir OutOfGas)
                    let settled_child_state_gas : Nat := tup__0
                    let settled_child_state_spill : Nat := tup__1
                    let settled_child_status : FrameStatus := tup__2
                    (pure (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status)))
                ) : SailM (Nat × Nat × Nat × FrameStatus) )
              (pure (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status)))
          ) : SailM (Nat × Nat × Nat × FrameStatus) )
        (pure (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status)))
    else
      (pure (settled_child_gas, settled_child_state_gas, settled_child_state_spill, settled_child_status))
    ) : SailM (Nat × Nat × Nat × FrameStatus) )
  let deploy_succeeds := (initcode_succeeded && (frame_succeeded settled_child_status))
  let (parent_pc, restored_gas, restored_sp, parent_memory, restored_state_gas, restored_state_spill, restored_refund, restored_status, parent_message, parent_code, parent_calldata) ← do
    (restore_frame checkpoint)
  let parent_gas ← do (refund_gas restored_gas settled_child_gas)
  let parent_state_gas : Nat := restored_state_gas
  let parent_state_spill : Nat := restored_state_spill
  let (tup__0, tup__1) ← do
    (return_child_state_gas parent_state_gas parent_state_spill settled_child_state_gas
      settled_child_state_spill)
  let parent_state_gas : Nat := tup__0
  let parent_state_spill : Nat := tup__1
  (pure ())
  let parent_refund : Int := restored_refund
  let parent_sp : StackPointer := restored_sp
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
  (pure ((parent_pc : Nat), (parent_gas : Nat), (parent_state_gas : Nat), (parent_state_spill : Nat), (parent_refund : Int), (restored_status : FrameStatus), (parent_sp : StackPointer), (parent_memory : (Sigma
    fun (output_dependentWitness0 : Nat) =>
    (Sigma fun (output_dependentWitness1 : Nat) =>
    (EvmMemorySliceFields output_dependentWitness0 output_dependentWitness1)))), (parent_message.caller : (Vector (BitVec 8) 20)), (parent_message.address : (Vector (BitVec 8) 20)), (parent_message.code_address : (Vector (BitVec 8) 20)), (parent_message.value : Nat), (parent_message.state_gas_reservoir : Nat), (parent_message.is_static : Bool), (parent_message.depth : Nat), (parent_code : (Sigma
    fun (output_dependentWitness0 : Nat) =>
    (Sigma fun (output_dependentWitness1 : Nat) =>
    (CodeFields output_dependentWitness0 output_dependentWitness1)))), (parent_calldata : CalldataSlice), ((⟨_, ⟨_, parent_returndata⟩⟩ : (Sigma
    fun (output_dependentWitness0 : Nat) =>
    (Sigma fun (output_dependentWitness1 : Nat) =>
    (OutputSliceFields output_dependentWitness0 output_dependentWitness1)))) : (Sigma fun
    (output_dependentWitness0 : Nat) =>
    (Sigma fun (output_dependentWitness1 : Nat) =>
    (OutputSliceFields output_dependentWitness0 output_dependentWitness1))))))

/-- Applies the pending operation for one completed child frame. -/
/- Type quantifiers: k_ex610294_ : Nat, k_ex610293_ : Int, k_ex610292_ : Nat, k_ex610291_ : Nat, k_ex610290_
  : Nat, output_dependentWitness1 : Nat, output_dependentWitness0 : Nat, 0 ≤
  output_dependentWitness0 ∧
  0 ≤ output_dependentWitness1 ∧
  (output_dependentWitness0 + output_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤ k_ex610290_ ∧
  k_ex610290_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex610291_ ∧ k_ex610291_ ≤ (2 ^ 64 - 1), 0 ≤
  k_ex610292_ ∧ k_ex610292_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1)))) ≤ k_ex610293_ ∧
  k_ex610293_ ≤ (199 * (2 ^ 64 - 1)), 0 ≤ k_ex610294_ ∧ k_ex610294_ ≤ (2 ^ 64 - 1) -/
def resume_frame (continuation : FrameContinuation) (output : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (OutputSliceFields k_off k_len)))) (child_gas : Nat) (child_state_gas : Nat) (child_state_spill : Nat) (child_refund : Int) (child_status : FrameStatus) (child_state_gas_reservoir : Nat) : SailM (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
  fun (output_dependentWitness0 : Nat) =>
  (Sigma fun (output_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields output_dependentWitness0 output_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
  fun (output_dependentWitness0 : Nat) =>
  (Sigma fun (output_dependentWitness1 : Nat) =>
  (CodeFields output_dependentWitness0 output_dependentWitness1))) × CalldataSlice × (Sigma fun
  (output_dependentWitness0 : Nat) =>
  (Sigma fun (output_dependentWitness1 : Nat) =>
  (OutputSliceFields output_dependentWitness0 output_dependentWitness1)))) := do
  let output_dependentWitness0 := (output).1
  let output_dependentWitness1 := ((output).2).1
  let output := ((output).2).2
  match continuation with
  | .Empty () =>
    (do
      (fatal_error ExecutionInvalid))
  | .ResumeCall call =>
    (do
      (resume_call call ⟨_, ⟨_, output⟩⟩ child_gas child_state_gas child_state_spill
        child_refund child_status))
  | .ResumeCreate create =>
    (do
      (resume_create create ⟨_, ⟨_, output⟩⟩ child_gas child_state_gas child_state_spill
        child_refund child_status child_state_gas_reservoir))

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
/- Type quantifiers: k_ex610298_ : Bool -/
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

/- Type quantifiers: carried_returndata_dependentWitness1 : Nat, carried_returndata_dependentWitness0
  : Nat, carried_code_dependentWitness1 : Nat, carried_code_dependentWitness0 : Nat, k_ex610325_ :
  Nat, k_ex610324_ : Bool, k_ex610323_ : Nat, k_ex610322_ : Nat, carried_memory_dependentWitness1 :
  Nat, carried_memory_dependentWitness0 : Nat, k_ex610317_ : Int, k_ex610316_ : Nat, k_ex610315_ :
  Nat, k_ex610314_ : Nat, k_ex610313_ : Nat, 0 ≤ k_ex610313_ ∧ k_ex610313_ ≤ (2 ^ 32 - 1), 0
  ≤ k_ex610314_ ∧ k_ex610314_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex610315_ ∧
  k_ex610315_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex610316_ ∧ k_ex610316_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1))))
  ≤ k_ex610317_ ∧ k_ex610317_ ≤ (199 * (2 ^ 64 - 1)), 0 ≤ carried_memory_dependentWitness0
  ∧
  0 ≤ carried_memory_dependentWitness1 ∧
  (carried_memory_dependentWitness0 + carried_memory_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex610322_ ∧ k_ex610322_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex610323_ ∧ k_ex610323_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex610325_ ∧ k_ex610325_ ≤ 1024, 0 ≤ carried_code_dependentWitness0 ∧
  0 ≤ carried_code_dependentWitness1 ∧
  (carried_code_dependentWitness0 + carried_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ carried_code_dependentWitness1 ∧ (carried_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0
  ≤ carried_returndata_dependentWitness0 ∧
  0 ≤ carried_returndata_dependentWitness1 ∧
  (carried_returndata_dependentWitness0 + carried_returndata_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def run_call (carried_pc : Nat) (carried_gas : Nat) (carried_state_gas : Nat) (carried_state_spill : Nat) (carried_refund : Int) (carried_sp : StackPointer) (carried_memory : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (carried_caller : (Vector (BitVec 8) 20)) (carried_address : (Vector (BitVec 8) 20)) (carried_code_address : (Vector (BitVec 8) 20)) (carried_value : Nat) (carried_state_gas_reservoir : Nat) (carried_is_static : Bool) (carried_depth : Nat) (carried_code : (Sigma
  fun (carried_memory_dependentWitness0 : Nat) =>
  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
  (CodeFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) (carried_calldata : CalldataSlice) (carried_returndata : (Sigma
  fun (carried_code_dependentWitness0 : Nat) =>
  (Sigma fun (carried_code_dependentWitness1 : Nat) =>
  (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) (kind : CallKind) : SailM (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
  fun (carried_returndata_dependentWitness0 : Nat) =>
  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
  fun (carried_returndata_dependentWitness0 : Nat) =>
  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
  (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
  fun (carried_returndata_dependentWitness0 : Nat) =>
  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
  (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) := SailME.run do
  let carried_memory_dependentWitness0 := (carried_memory).1
  let carried_memory_dependentWitness1 := ((carried_memory).2).1
  let carried_memory := ((carried_memory).2).2
  let carried_code_dependentWitness0 := (carried_code).1
  let carried_code_dependentWitness1 := ((carried_code).2).1
  let carried_code := ((carried_code).2).2
  let carried_returndata_dependentWitness0 := (carried_returndata).1
  let carried_returndata_dependentWitness1 := ((carried_returndata).2).1
  let carried_returndata := ((carried_returndata).2).2
  match (guard_stack carried_sp (call_stack_inputs kind) 1) with
  | .Failed halt_kind =>
    (do
      let (state_gas_after, state_spill_after, status_after) ← do
        (exceptional_state carried_state_gas carried_state_spill carried_state_gas_reservoir
          halt_kind)
      (pure ((carried_pc : Nat), (GAS_ZERO : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (carried_sp : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), ((⟨_, ⟨_, carried_returndata⟩⟩ : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))
  | .Continue () =>
    (do
      let pc_after : Nat := carried_pc
      let gas_after : Nat := carried_gas
      let state_gas_after : Nat := carried_state_gas
      let state_spill_after : Nat := carried_state_spill
      let status_after : FrameStatus := (Running ())
      let sp_after : StackPointer := carried_sp
      let memory_after : (Sigma fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
        ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma fun (carried_returndata_dependentWitness0 : Nat)
        =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
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
        else (pure (WORD_ZERO, sp_after)) ) : SailME
        (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma fun
        (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
        (Nat × StackPointer) )
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
      if _sailIf0 : ((semantics.transfers_value && (value_nonzero && carried_is_static)) : Bool) = true
      then
        (do
          let gas_after : Nat := GAS_ZERO
          let (tup__0, tup__1, tup__2) ← do
            (exceptional_state state_gas_after state_spill_after carried_state_gas_reservoir
              WriteProtection)
          let state_gas_after : Nat := tup__0
          let state_spill_after : Nat := tup__1
          let status_after : FrameStatus := tup__2
          (pure ())
          (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), (memory_after : (Sigma
            fun (carried_returndata_dependentWitness0 : Nat) =>
            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
            fun (carried_returndata_dependentWitness0 : Nat) =>
            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
            (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
            fun (carried_returndata_dependentWitness0 : Nat) =>
            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
            (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), (returndata_after : (Sigma
            fun (carried_returndata_dependentWitness0 : Nat) =>
            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))
      else
        (do
          let warm ← do (k_account_is_warm target)
          let target_cost ← (( do (account_cost warm) ) : SailME
            (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma fun
            (carried_returndata_dependentWitness0 : Nat) =>
            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
            fun (carried_returndata_dependentWitness0 : Nat) =>
            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
            (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
            fun (carried_returndata_dependentWitness0 : Nat) =>
            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
            Nat )
          let transfer_cost ← (( do
            if (value_nonzero : Bool)
            then (call_value_cost ())
            else (pure GAS_CONSTANT_ZERO) ) : SailME
            (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma fun
            (carried_returndata_dependentWitness0 : Nat) =>
            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
            fun (carried_returndata_dependentWitness0 : Nat) =>
            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
            (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
            fun (carried_returndata_dependentWitness0 : Nat) =>
            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
            Nat )
          let args_required := (memory_required_size args_off_word args_len_word)
          let ret_required := (memory_required_size ret_off_word ret_len_word)
          let required_size :=
            if ((args_required <b ret_required) : Bool)
            then ret_required
            else args_required
          let (expansion_halt, gas_after_expansion) :=
            (charge_memory_expansion gas_after memory_after required_size)
          let gas_after : Nat := gas_after_expansion
          if _sailIf1 : (expansion_halt : Bool) = true
          then
            (do
              let gas_after : Nat := GAS_ZERO
              let (tup__0, tup__1, tup__2) ← do
                (exceptional_state state_gas_after state_spill_after carried_state_gas_reservoir
                  OutOfGas)
              let state_gas_after : Nat := tup__0
              let state_spill_after : Nat := tup__1
              let status_after : FrameStatus := tup__2
              (pure ())
              (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), (memory_after : (Sigma
                fun (carried_returndata_dependentWitness0 : Nat) =>
                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                fun (carried_returndata_dependentWitness0 : Nat) =>
                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                fun (carried_returndata_dependentWitness0 : Nat) =>
                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), (returndata_after : (Sigma
                fun (carried_returndata_dependentWitness0 : Nat) =>
                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))
          else
            (do
              let static_base : Nat := (target_cost + transfer_cost)
              let (static_base_halt, gas_after_static_base) := (charge gas_after static_base)
              let gas_after : Nat := gas_after_static_base
              if _sailIf2 : (static_base_halt : Bool) = true
              then
                (do
                  let gas_after : Nat := GAS_ZERO
                  let (tup__0, tup__1, tup__2) ← do
                    (exceptional_state state_gas_after state_spill_after carried_state_gas_reservoir
                      OutOfGas)
                  let state_gas_after : Nat := tup__0
                  let state_spill_after : Nat := tup__1
                  let status_after : FrameStatus := tup__2
                  (pure ())
                  (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), (memory_after : (Sigma
                    fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                    fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                    fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), (returndata_after : (Sigma
                    fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))
              else
                (do
                  (k_account_mark_warm target)
                  let (tg_deleg, tg_target) ← do (k_deleg_target target)
                  let delegation_cost ← (( do
                    if (tg_deleg : Bool)
                    then
                      (do
                        let dw ← do (k_account_is_warm tg_target)
                        (account_cost dw))
                    else (pure GAS_CONSTANT_ZERO) ) : SailME
                    (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma fun
                    (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                    fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                    fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                    Nat )
                  let target_empty ← do (k_account_is_empty target)
                  let new_account_charged :=
                    ((profile.fork ≥b Amsterdam) && (value_nonzero && (semantics.transfers_value && target_empty)))
                  let create_cost : Nat :=
                    if (((profile.fork <b Amsterdam) && (value_nonzero && (semantics.transfers_value && target_empty))) : Bool)
                    then G_newaccount
                    else GAS_CONSTANT_ZERO
                  let additional_cost : Nat := (delegation_cost + create_cost)
                  let (additional_cost_halt, gas_after_additional_cost) :=
                    (charge gas_after additional_cost)
                  let gas_after : Nat := gas_after_additional_cost
                  if _sailIf3 : (additional_cost_halt : Bool) = true
                  then
                    (do
                      let gas_after : Nat := GAS_ZERO
                      let (tup__0, tup__1, tup__2) ← do
                        (exceptional_state state_gas_after state_spill_after
                          carried_state_gas_reservoir OutOfGas)
                      let state_gas_after : Nat := tup__0
                      let state_spill_after : Nat := tup__1
                      let status_after : FrameStatus := tup__2
                      (pure ())
                      (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), (memory_after : (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), (returndata_after : (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))
                  else
                    (do
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
                                        let a__301 ← (( do
                                          let gas_after : Nat := GAS_ZERO
                                          let (tup__0, tup__1, tup__2) ← do
                                            (exceptional_state state_gas_after state_spill_after
                                              carried_state_gas_reservoir OutOfGas)
                                          let state_gas_after : Nat := tup__0
                                          let state_spill_after : Nat := tup__1
                                          let status_after : FrameStatus := tup__2
                                          (pure ())
                                          (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), (memory_after : (Sigma
                                            fun (carried_returndata_dependentWitness0 : Nat) =>
                                            (Sigma fun (carried_returndata_dependentWitness1 : Nat)
                                            =>
                                            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                                            fun (carried_returndata_dependentWitness0 : Nat) =>
                                            (Sigma fun (carried_returndata_dependentWitness1 : Nat)
                                            =>
                                            (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                            fun (carried_returndata_dependentWitness0 : Nat) =>
                                            (Sigma fun (carried_returndata_dependentWitness1 : Nat)
                                            =>
                                            (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), (returndata_after : (Sigma
                                            fun (carried_returndata_dependentWitness0 : Nat) =>
                                            (Sigma fun (carried_returndata_dependentWitness1 : Nat)
                                            =>
                                            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))))
                                          ) : SailME
                                          (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                          fun (carried_returndata_dependentWitness0 : Nat) =>
                                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                          (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                          fun (carried_returndata_dependentWitness0 : Nat) =>
                                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                          (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                          fun (carried_returndata_dependentWitness0 : Nat) =>
                                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                          (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                          (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                          fun (carried_returndata_dependentWitness0 : Nat) =>
                                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                          (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                          fun (carried_returndata_dependentWitness0 : Nat) =>
                                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                          (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                          fun (carried_returndata_dependentWitness0 : Nat) =>
                                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                          (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                          )
                                        SailME.throw ((fun (dependentValue0, dependentValue1, dependentValue2, dependentValue3, dependentValue4, dependentValue5, dependentValue6, dependentValue7, dependentValue8, dependentValue9, dependentValue10, dependentValue11, dependentValue12, dependentValue13, dependentValue14, dependentValue15, dependentValue16, dependentValue17) => (dependentValue0, dependentValue1, dependentValue2, dependentValue3, dependentValue4, dependentValue5, dependentValue6, dependentValue7, dependentValue8, dependentValue9, dependentValue10, dependentValue11, dependentValue12, dependentValue13, dependentValue14, dependentValue15, dependentValue16, dependentValue17)) (a__301)))
                                    else
                                      (pure (gas_after, state_gas_after, state_spill_after, status_after))
                                    ) : SailME
                                    (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                    (Nat × Nat × Nat × FrameStatus) )
                                  (pure (gas_after, state_gas_after, state_spill_after, status_after)))
                              else
                                (pure (gas_after, state_gas_after, state_spill_after, status_after))
                              ) : SailME
                              (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                              fun (carried_returndata_dependentWitness0 : Nat) =>
                              (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                              (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                              fun (carried_returndata_dependentWitness0 : Nat) =>
                              (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                              (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                              fun (carried_returndata_dependentWitness0 : Nat) =>
                              (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                              (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                              (Nat × Nat × Nat × FrameStatus) )
                            let base_child : Nat := (call_gas_cap_word gas_after gas_request)
                            let (child_gas_halt, child_charged_gas) := (charge gas_after base_child)
                            let gas_after : Nat := child_charged_gas
                            let (gas_after, state_gas_after, state_spill_after, status_after) ← (( do
                              if (child_gas_halt : Bool)
                              then
                                (do
                                  let a__300 ← (( do
                                    let gas_after : Nat := GAS_ZERO
                                    let (tup__0, tup__1, tup__2) ← do
                                      (exceptional_state state_gas_after state_spill_after
                                        carried_state_gas_reservoir OutOfGas)
                                    let state_gas_after : Nat := tup__0
                                    let state_spill_after : Nat := tup__1
                                    let status_after : FrameStatus := tup__2
                                    (pure ())
                                    (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), (memory_after : (Sigma
                                      fun (carried_returndata_dependentWitness0 : Nat) =>
                                      (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                      (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                                      fun (carried_returndata_dependentWitness0 : Nat) =>
                                      (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                      (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                      fun (carried_returndata_dependentWitness0 : Nat) =>
                                      (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                      (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), (returndata_after : (Sigma
                                      fun (carried_returndata_dependentWitness0 : Nat) =>
                                      (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                      (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))))
                                    ) : SailME
                                    (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                    (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                    )
                                  SailME.throw ((fun (dependentValue0, dependentValue1, dependentValue2, dependentValue3, dependentValue4, dependentValue5, dependentValue6, dependentValue7, dependentValue8, dependentValue9, dependentValue10, dependentValue11, dependentValue12, dependentValue13, dependentValue14, dependentValue15, dependentValue16, dependentValue17) => (dependentValue0, dependentValue1, dependentValue2, dependentValue3, dependentValue4, dependentValue5, dependentValue6, dependentValue7, dependentValue8, dependentValue9, dependentValue10, dependentValue11, dependentValue12, dependentValue13, dependentValue14, dependentValue15, dependentValue16, dependentValue17)) (a__300)))
                              else
                                (pure (gas_after, state_gas_after, state_spill_after, status_after))
                              ) : SailME
                              (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                              fun (carried_returndata_dependentWitness0 : Nat) =>
                              (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                              (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                              fun (carried_returndata_dependentWitness0 : Nat) =>
                              (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                              (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                              fun (carried_returndata_dependentWitness0 : Nat) =>
                              (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                              (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                              (Nat × Nat × Nat × FrameStatus) )
                            (pure (base_child, gas_after, state_gas_after, state_spill_after, status_after)))
                        else
                          (do
                            let base_child : Nat := (call_gas_cap_word gas_after gas_request)
                            let (child_gas_halt, child_charged_gas) := (charge gas_after base_child)
                            let gas_after : Nat := child_charged_gas
                            let (gas_after, state_gas_after, state_spill_after, status_after) ← (( do
                              if (child_gas_halt : Bool)
                              then
                                (do
                                  let a__302 ← (( do
                                    let gas_after : Nat := GAS_ZERO
                                    let (tup__0, tup__1, tup__2) ← do
                                      (exceptional_state state_gas_after state_spill_after
                                        carried_state_gas_reservoir OutOfGas)
                                    let state_gas_after : Nat := tup__0
                                    let state_spill_after : Nat := tup__1
                                    let status_after : FrameStatus := tup__2
                                    (pure ())
                                    (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), (memory_after : (Sigma
                                      fun (carried_returndata_dependentWitness0 : Nat) =>
                                      (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                      (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                                      fun (carried_returndata_dependentWitness0 : Nat) =>
                                      (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                      (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                      fun (carried_returndata_dependentWitness0 : Nat) =>
                                      (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                      (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), (returndata_after : (Sigma
                                      fun (carried_returndata_dependentWitness0 : Nat) =>
                                      (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                      (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))))
                                    ) : SailME
                                    (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                    (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                    )
                                  SailME.throw ((fun (dependentValue0, dependentValue1, dependentValue2, dependentValue3, dependentValue4, dependentValue5, dependentValue6, dependentValue7, dependentValue8, dependentValue9, dependentValue10, dependentValue11, dependentValue12, dependentValue13, dependentValue14, dependentValue15, dependentValue16, dependentValue17) => (dependentValue0, dependentValue1, dependentValue2, dependentValue3, dependentValue4, dependentValue5, dependentValue6, dependentValue7, dependentValue8, dependentValue9, dependentValue10, dependentValue11, dependentValue12, dependentValue13, dependentValue14, dependentValue15, dependentValue16, dependentValue17)) (a__302)))
                              else
                                (pure (gas_after, state_gas_after, state_spill_after, status_after))
                              ) : SailME
                              (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                              fun (carried_returndata_dependentWitness0 : Nat) =>
                              (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                              (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                              fun (carried_returndata_dependentWitness0 : Nat) =>
                              (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                              (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                              fun (carried_returndata_dependentWitness0 : Nat) =>
                              (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                              (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                              (Nat × Nat × Nat × FrameStatus) )
                            (pure (base_child, gas_after, state_gas_after, state_spill_after, status_after)))
                        ) : SailME
                        (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                        (Nat × Nat × Nat × Nat × FrameStatus) )
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
                        if ((args_access.required_size <b ret_access.required_size) : Bool)
                        then ret_access.required_size
                        else args_access.required_size
                      let ⟨_, ⟨_, mem1⟩⟩ ← do
                        (expand_memory memory_after materialized_required_size)
                      let args := args_access.range
                      let ret := ret_access.range
                      let child_gas ← (( do (conserved_gas_add base_child stipend) ) : SailME
                        (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                        Nat )
                      let _ ← do (k_aload target)
                      let insufficient_balance ← (( do
                        if ((semantics.takes_value && value_nonzero) : Bool)
                        then
                          (do
                            let caller_balance ← do (k_get_balance caller)
                            let transfer_affordable := (word_ule value caller_balance)
                            (pure (! transfer_affordable)))
                        else (pure false) ) : SailME
                        (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                        Bool )
                      let depth_limit := 1024
                      if _sailIf4 : ((insufficient_balance || (current_depth == depth_limit)) : Bool) = true
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
                            (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                            (Nat × Nat × Nat) )
                          let sp_after ← (stack_top_advance sp_after 1)
                          (write_stack_word sp_after WORD_ZERO)
                          let memory_after : (Sigma fun (carried_returndata_dependentWitness0 : Nat)
                            =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                            ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
                            (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                          (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), (memory_after : (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), ((⟨_, ⟨_, returndata_after⟩⟩ : (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))
                      else
                        (do
                          let selected_precompile ← do (precompile_id_for_address target)
                          if _sailIf5 : ((bne selected_precompile NotPrecompile) : Bool) = true
                          then
                            (do
                              let (input_memory, mem2) ← do
                                (active_memory_slice ⟨_, ⟨_, mem1⟩⟩ args.off args.len)
                              let input := (MemoryCalldata input_memory)
                              let precompile_charge ← do
                                (precompile_gas selected_precompile input child_gas)
                              if _sailIf6 : (precompile_charge.affordable : Bool) = true
                              then
                                (do
                                  let used := precompile_charge.cost
                                  let result ← do (run_precompile_slice selected_precompile input)
                                  if _sailIf7 : (result.success : Bool) = true
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
                                      (returndata_copy_prefix ⟨_, ⟨_, returndata_after⟩⟩
                                        ret.off ret.len)
                                      let unused : Nat := (gas_sub child_gas used)
                                      let gas_after ← (refund_gas gas_after unused)
                                      let sp_after ← (stack_top_advance sp_after 1)
                                      (write_stack_word sp_after WORD_ONE)
                                      let ⟨_, ⟨_, memory_after⟩⟩ : (Sigma fun
                                        (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                                        (mem2 : (Sigma fun
                                        (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                      (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), ((⟨_, ⟨_, memory_after⟩⟩ : (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), ((⟨_, ⟨_, returndata_after⟩⟩ : (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))
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
                                        ) : SailME
                                        (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                        (Nat × Nat × Nat) )
                                      let sp_after ← (stack_top_advance sp_after 1)
                                      (write_stack_word sp_after WORD_ZERO)
                                      let ⟨_, ⟨_, memory_after⟩⟩ : (Sigma fun
                                        (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                                        (mem2 : (Sigma fun
                                        (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                      (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), ((⟨_, ⟨_, memory_after⟩⟩ : (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), ((⟨_, ⟨_, returndata_after⟩⟩ : (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))))))
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
                                    SailME
                                    (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                    (Nat × Nat × Nat) )
                                  let sp_after ← (stack_top_advance sp_after 1)
                                  (write_stack_word sp_after WORD_ZERO)
                                  let ⟨_, ⟨_, memory_after⟩⟩ : (Sigma fun
                                    (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                                    (mem2 : (Sigma fun (carried_returndata_dependentWitness0 : Nat)
                                    =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                  (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), ((⟨_, ⟨_, memory_after⟩⟩ : (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), ((⟨_, ⟨_, returndata_after⟩⟩ : (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                    fun (carried_returndata_dependentWitness0 : Nat) =>
                                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                    (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))))))
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
                              let (bytes, mem2) ← do
                                (active_memory_slice ⟨_, ⟨_, mem1⟩⟩ args.off args.len)
                              let child_memory :=
                                (evm_memory_slice ((bytes).2).2.bytes ((bytes).2).2.len)
                              let child_calldata := (MemoryCalldata ⟨_, ⟨_, child_memory⟩⟩)
                              let child_state_gas := state_gas_after
                              let (checkpoint, child_stack, child_frame_memory) ← do
                                (suspend_frame pc_after gas_after sp_after mem2 STATE_GAS_ZERO
                                  state_spill_after carried_refund (Running ()) parent_message
                                  ⟨_, ⟨_, carried_code⟩⟩ carried_calldata)
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
                              (pure ((0 : Nat), (child_gas : Nat), (child_state_gas : Nat), (STATE_GAS_SPILL_ZERO : Nat), (GAS_REFUND_ZERO : Int), ((Running
                                  ()) : FrameStatus), (child_stack : StackPointer), (child_frame_memory : (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (child_caller : (Vector (BitVec 8) 20)), (child_addr : (Vector (BitVec 8) 20)), (target : (Vector (BitVec 8) 20)), (child_value : Nat), (child_state_gas : Nat), (child_static : Bool), (child_depth : Nat), ((⟨_, ⟨_, child_code⟩⟩ : (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (child_calldata : CalldataSlice), ((⟨_, ⟨_, child_returndata⟩⟩ : (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))))))))

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

/- Type quantifiers: carried_returndata_dependentWitness1 : Nat, carried_returndata_dependentWitness0
  : Nat, carried_code_dependentWitness1 : Nat, carried_code_dependentWitness0 : Nat, k_ex610360_ :
  Nat, k_ex610359_ : Bool, k_ex610358_ : Nat, k_ex610357_ : Nat, carried_memory_dependentWitness1 :
  Nat, carried_memory_dependentWitness0 : Nat, k_ex610352_ : Int, k_ex610351_ : Nat, k_ex610350_ :
  Nat, k_ex610349_ : Nat, k_ex610348_ : Nat, 0 ≤ k_ex610348_ ∧ k_ex610348_ ≤ (2 ^ 32 - 1), 0
  ≤ k_ex610349_ ∧ k_ex610349_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex610350_ ∧
  k_ex610350_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex610351_ ∧ k_ex610351_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1))))
  ≤ k_ex610352_ ∧ k_ex610352_ ≤ (199 * (2 ^ 64 - 1)), 0 ≤ carried_memory_dependentWitness0
  ∧
  0 ≤ carried_memory_dependentWitness1 ∧
  (carried_memory_dependentWitness0 + carried_memory_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex610357_ ∧ k_ex610357_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex610358_ ∧ k_ex610358_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex610360_ ∧ k_ex610360_ ≤ 1024, 0 ≤ carried_code_dependentWitness0 ∧
  0 ≤ carried_code_dependentWitness1 ∧
  (carried_code_dependentWitness0 + carried_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ carried_code_dependentWitness1 ∧ (carried_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0
  ≤ carried_returndata_dependentWitness0 ∧
  0 ≤ carried_returndata_dependentWitness1 ∧
  (carried_returndata_dependentWitness0 + carried_returndata_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def run_create (carried_pc : Nat) (carried_gas : Nat) (carried_state_gas : Nat) (carried_state_spill : Nat) (carried_refund : Int) (carried_sp : StackPointer) (carried_memory : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (carried_caller : (Vector (BitVec 8) 20)) (carried_address : (Vector (BitVec 8) 20)) (carried_code_address : (Vector (BitVec 8) 20)) (carried_value : Nat) (carried_state_gas_reservoir : Nat) (carried_is_static : Bool) (carried_depth : Nat) (carried_code : (Sigma
  fun (carried_memory_dependentWitness0 : Nat) =>
  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
  (CodeFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) (carried_calldata : CalldataSlice) (carried_returndata : (Sigma
  fun (carried_code_dependentWitness0 : Nat) =>
  (Sigma fun (carried_code_dependentWitness1 : Nat) =>
  (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) (kind : CreateKind) : SailM (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
  fun (carried_returndata_dependentWitness0 : Nat) =>
  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
  fun (carried_returndata_dependentWitness0 : Nat) =>
  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
  (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
  fun (carried_returndata_dependentWitness0 : Nat) =>
  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
  (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) := SailME.run do
  let carried_memory_dependentWitness0 := (carried_memory).1
  let carried_memory_dependentWitness1 := ((carried_memory).2).1
  let carried_memory := ((carried_memory).2).2
  let carried_code_dependentWitness0 := (carried_code).1
  let carried_code_dependentWitness1 := ((carried_code).2).1
  let carried_code := ((carried_code).2).2
  let carried_returndata_dependentWitness0 := (carried_returndata).1
  let carried_returndata_dependentWitness1 := ((carried_returndata).2).1
  let carried_returndata := ((carried_returndata).2).2
  match (guard_stack carried_sp (create_stack_inputs kind) 1) with
  | .Failed halt_kind =>
    (do
      let (state_gas_after, state_spill_after, status_after) ← do
        (exceptional_state carried_state_gas carried_state_spill carried_state_gas_reservoir
          halt_kind)
      (pure ((carried_pc : Nat), (GAS_ZERO : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (carried_sp : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), ((⟨_, ⟨_, carried_returndata⟩⟩ : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))
  | .Continue () =>
    (do
      let pc_after : Nat := carried_pc
      let gas_after : Nat := carried_gas
      let state_gas_after : Nat := carried_state_gas
      let state_spill_after : Nat := carried_state_spill
      let status_after : FrameStatus := (Running ())
      let sp_after : StackPointer := carried_sp
      let memory_after : (Sigma fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
        ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma fun (carried_returndata_dependentWitness0 : Nat)
        =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
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
        else (pure (WORD_ZERO, sp_after)) ) : SailME
        (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma fun
        (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
        (Nat × StackPointer) )
      let sp_after : StackPointer := next_sp
      if _sailIf0 : (carried_is_static : Bool) = true
      then
        (do
          let gas_after : Nat := GAS_ZERO
          let (tup__0, tup__1, tup__2) ← do
            (exceptional_state state_gas_after state_spill_after carried_state_gas_reservoir
              WriteProtection)
          let state_gas_after : Nat := tup__0
          let state_spill_after : Nat := tup__1
          let status_after : FrameStatus := tup__2
          (pure ())
          (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), (memory_after : (Sigma
            fun (carried_returndata_dependentWitness0 : Nat) =>
            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
            fun (carried_returndata_dependentWitness0 : Nat) =>
            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
            (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
            fun (carried_returndata_dependentWitness0 : Nat) =>
            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
            (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), (returndata_after : (Sigma
            fun (carried_returndata_dependentWitness0 : Nat) =>
            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))
      else
        (do
          let required_size := (memory_required_size off_word len_word)
          let (expansion_halt, gas_after_expansion) :=
            (charge_memory_expansion gas_after memory_after required_size)
          let gas_after : Nat := gas_after_expansion
          if _sailIf1 : (expansion_halt : Bool) = true
          then
            (do
              let gas_after : Nat := GAS_ZERO
              let (tup__0, tup__1, tup__2) ← do
                (exceptional_state state_gas_after state_spill_after carried_state_gas_reservoir
                  OutOfGas)
              let state_gas_after : Nat := tup__0
              let state_spill_after : Nat := tup__1
              let status_after : FrameStatus := tup__2
              (pure ())
              (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), (memory_after : (Sigma
                fun (carried_returndata_dependentWitness0 : Nat) =>
                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                fun (carried_returndata_dependentWitness0 : Nat) =>
                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                fun (carried_returndata_dependentWitness0 : Nat) =>
                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), (returndata_after : (Sigma
                fun (carried_returndata_dependentWitness0 : Nat) =>
                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))
          else
            (do
              let ⟨_, ⟨_, ⟨_, initcode_access⟩⟩⟩ ← do
                (memory_access off_word len_word)
              let ⟨_, ⟨_, mem1⟩⟩ ← do
                (expand_memory memory_after initcode_access.required_size)
              let initcode := initcode_access.range
              let access_cost ← do (create_access_cost ())
              let (access_halt, gas_after_access) := (charge gas_after access_cost)
              let gas_after : Nat := gas_after_access
              if _sailIf2 : (access_halt : Bool) = true
              then
                (do
                  let memory_after : (Sigma fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                    ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
                    (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                    fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                  let gas_after : Nat := GAS_ZERO
                  let (tup__0, tup__1, tup__2) ← do
                    (exceptional_state state_gas_after state_spill_after carried_state_gas_reservoir
                      OutOfGas)
                  let state_gas_after : Nat := tup__0
                  let state_spill_after : Nat := tup__1
                  let status_after : FrameStatus := tup__2
                  (pure ())
                  (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), (memory_after : (Sigma
                    fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                    fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                    fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), (returndata_after : (Sigma
                    fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))
              else
                (do
                  let initcode_word_count := (memory_word_count_word len_word)
                  let (gas_after, memory_after, state_gas_after, state_spill_after, status_after) ← (( do
                    if _sailIf3 : ((profile.fork ≥b Shanghai) : Bool) = true
                    then
                      (do
                        let (initcode_charge_halt, initcode_gas) :=
                          (charge_word_scaled_gas gas_after G_initcode_word initcode_word_count)
                        let gas_after : Nat := initcode_gas
                        let (gas_after, memory_after, state_gas_after, state_spill_after, status_after) ← (( do
                          if _sailIf4 : (initcode_charge_halt : Bool) = true
                          then
                            (do
                              let a__288 ← (( do
                                let memory_after : (Sigma fun
                                  (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                                  ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
                                  (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                  fun (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                let gas_after : Nat := GAS_ZERO
                                let (tup__0, tup__1, tup__2) ← do
                                  (exceptional_state state_gas_after state_spill_after
                                    carried_state_gas_reservoir OutOfGas)
                                let state_gas_after : Nat := tup__0
                                let state_spill_after : Nat := tup__1
                                let status_after : FrameStatus := tup__2
                                (pure ())
                                (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), (memory_after : (Sigma
                                  fun (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                                  fun (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                  fun (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), (returndata_after : (Sigma
                                  fun (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))))
                                ) : SailME
                                (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                )
                              SailME.throw ((fun (dependentValue0, dependentValue1, dependentValue2, dependentValue3, dependentValue4, dependentValue5, dependentValue6, dependentValue7, dependentValue8, dependentValue9, dependentValue10, dependentValue11, dependentValue12, dependentValue13, dependentValue14, dependentValue15, dependentValue16, dependentValue17) => (dependentValue0, dependentValue1, dependentValue2, dependentValue3, dependentValue4, dependentValue5, dependentValue6, dependentValue7, dependentValue8, dependentValue9, dependentValue10, dependentValue11, dependentValue12, dependentValue13, dependentValue14, dependentValue15, dependentValue16, dependentValue17)) (a__288)))
                          else
                            (pure ((gas_after : Nat), (memory_after : (Sigma fun
                              (carried_returndata_dependentWitness0 : Nat) =>
                              (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                              (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus)))
                          ) : SailME
                          (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                          fun (carried_returndata_dependentWitness0 : Nat) =>
                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                          fun (carried_returndata_dependentWitness0 : Nat) =>
                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                          (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                          fun (carried_returndata_dependentWitness0 : Nat) =>
                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                          (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                          (Nat × (Sigma fun (carried_returndata_dependentWitness0 : Nat) =>
                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × Nat × Nat × FrameStatus)
                          )
                        (pure ((gas_after : Nat), (memory_after : (Sigma fun
                          (carried_returndata_dependentWitness0 : Nat) =>
                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus))))
                    else
                      (pure ((gas_after : Nat), (memory_after : (Sigma fun
                        (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus)))
                    ) : SailME
                    (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma fun
                    (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                    fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                    fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                    (Nat × (Sigma fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × Nat × Nat × FrameStatus)
                    )
                  let (gas_after, memory_after, state_gas_after, state_spill_after, status_after) ← (( do
                    if _sailIf3 : (semantics.uses_salt : Bool) = true
                    then
                      (do
                        let (hashing_halt, hashing_gas) :=
                          (charge_word_scaled_gas gas_after G_keccak_word initcode_word_count)
                        let gas_after : Nat := hashing_gas
                        let (gas_after, memory_after, state_gas_after, state_spill_after, status_after) ← (( do
                          if _sailIf4 : (hashing_halt : Bool) = true
                          then
                            (do
                              let a__287 ← (( do
                                let memory_after : (Sigma fun
                                  (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                                  ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
                                  (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                  fun (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                let gas_after : Nat := GAS_ZERO
                                let (tup__0, tup__1, tup__2) ← do
                                  (exceptional_state state_gas_after state_spill_after
                                    carried_state_gas_reservoir OutOfGas)
                                let state_gas_after : Nat := tup__0
                                let state_spill_after : Nat := tup__1
                                let status_after : FrameStatus := tup__2
                                (pure ())
                                (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), (memory_after : (Sigma
                                  fun (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                                  fun (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                  fun (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), (returndata_after : (Sigma
                                  fun (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))))
                                ) : SailME
                                (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                )
                              SailME.throw ((fun (dependentValue0, dependentValue1, dependentValue2, dependentValue3, dependentValue4, dependentValue5, dependentValue6, dependentValue7, dependentValue8, dependentValue9, dependentValue10, dependentValue11, dependentValue12, dependentValue13, dependentValue14, dependentValue15, dependentValue16, dependentValue17) => (dependentValue0, dependentValue1, dependentValue2, dependentValue3, dependentValue4, dependentValue5, dependentValue6, dependentValue7, dependentValue8, dependentValue9, dependentValue10, dependentValue11, dependentValue12, dependentValue13, dependentValue14, dependentValue15, dependentValue16, dependentValue17)) (a__287)))
                          else
                            (pure ((gas_after : Nat), (memory_after : (Sigma fun
                              (carried_returndata_dependentWitness0 : Nat) =>
                              (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                              (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus)))
                          ) : SailME
                          (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                          fun (carried_returndata_dependentWitness0 : Nat) =>
                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                          fun (carried_returndata_dependentWitness0 : Nat) =>
                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                          (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                          fun (carried_returndata_dependentWitness0 : Nat) =>
                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                          (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                          (Nat × (Sigma fun (carried_returndata_dependentWitness0 : Nat) =>
                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × Nat × Nat × FrameStatus)
                          )
                        (pure ((gas_after : Nat), (memory_after : (Sigma fun
                          (carried_returndata_dependentWitness0 : Nat) =>
                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus))))
                    else
                      (pure ((gas_after : Nat), (memory_after : (Sigma fun
                        (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus)))
                    ) : SailME
                    (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma fun
                    (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                    fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                    fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                    (Nat × (Sigma fun (carried_returndata_dependentWitness0 : Nat) =>
                    (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × Nat × Nat × FrameStatus)
                    )
                  let valid_initcode_size ← do (initcode_size_allowed initcode.len)
                  let invalid_initcode_size := (! valid_initcode_size)
                  if _sailIf3 : (invalid_initcode_size : Bool) = true
                  then
                    (do
                      let memory_after : (Sigma fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                        ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
                        (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                      let gas_after : Nat := GAS_ZERO
                      let (tup__0, tup__1, tup__2) ← do
                        (exceptional_state state_gas_after state_spill_after
                          carried_state_gas_reservoir InitCodeTooLarge)
                      let state_gas_after : Nat := tup__0
                      let state_spill_after : Nat := tup__1
                      let status_after : FrameStatus := tup__2
                      (pure ())
                      (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), (memory_after : (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), (returndata_after : (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))
                  else
                    (do
                      let nonce ← do (k_get_nonce creator)
                      let mem2 : (Sigma fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                        ((⟨_, ⟨_, mem1⟩⟩ : (Sigma fun
                        (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                      let (new_addr, mem2) ← (( do
                        if _sailIf4 : (semantics.uses_salt : Bool) = true
                        then
                          (do
                            let (initcode_digest_word, hashed_mem) ← do
                              (mem_keccak ⟨_, ⟨_, mem1⟩⟩ ⟨_, ⟨_, initcode⟩⟩)
                            let ⟨_, ⟨_, mem2⟩⟩ : (Sigma fun
                              (carried_returndata_dependentWitness0 : Nat) =>
                              (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                              (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                              (hashed_mem : (Sigma fun (carried_returndata_dependentWitness0 : Nat)
                              =>
                              (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                              (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                            let initcode_digest := (word_to_hash initcode_digest_word)
                            (pure ((← do
                                (k_create2_addr creator salt initcode_digest)), ((⟨_, ⟨_, mem2⟩⟩ : (Sigma
                              fun (carried_returndata_dependentWitness0 : Nat) =>
                              (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                              (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                              fun (carried_returndata_dependentWitness0 : Nat) =>
                              (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                              (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))
                        else
                          (do
                            (pure ((← do
                                (k_create_addr creator nonce)), (mem2 : (Sigma fun
                              (carried_returndata_dependentWitness0 : Nat) =>
                              (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                              (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))
                        ) : SailME
                        (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                        fun (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                        ((Vector (BitVec 8) 20) × (Sigma fun
                        (carried_returndata_dependentWitness0 : Nat) =>
                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                        )
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
                      if _sailIf4 : ((insufficient_balance || ((nonce == nonce_limit) || (current_depth == depth_limit))) : Bool) = true
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
                          let ⟨_, ⟨_, memory_after⟩⟩ : (Sigma fun
                            (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                            (mem2 : (Sigma fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                          (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), ((⟨_, ⟨_, memory_after⟩⟩ : (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), ((⟨_, ⟨_, returndata_after⟩⟩ : (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))
                      else
                        (do
                          let child_depth : Nat := (current_depth + 1)
                          (k_account_mark_warm new_addr)
                          let new_account_charged ← do
                            if ((profile.fork ≥b Amsterdam) : Bool)
                            then
                              (do
                                (k_account_is_empty new_addr))
                            else (pure false)
                          let (gas_after, memory_after, state_gas_after, state_spill_after, status_after) ← (( do
                            if _sailIf5 : (new_account_charged : Bool) = true
                            then
                              (do
                                let (state_gas_halt, next_gas, next_state_gas, next_state_spill) ← do
                                  (charge_state_gas gas_after state_gas_after state_spill_after
                                    G_amsterdam_state_new_account)
                                let gas_after : Nat := next_gas
                                let state_gas_after : Nat := next_state_gas
                                let state_spill_after : Nat := next_state_spill
                                let (gas_after, memory_after, state_gas_after, state_spill_after, status_after) ← (( do
                                  if _sailIf6 : (state_gas_halt : Bool) = true
                                  then
                                    (do
                                      let a__286 ← (( do
                                        let ⟨_, ⟨_, memory_after⟩⟩ : (Sigma fun
                                          (carried_returndata_dependentWitness0 : Nat) =>
                                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                          (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                                          (mem2 : (Sigma fun
                                          (carried_returndata_dependentWitness0 : Nat) =>
                                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                          (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                        let gas_after : Nat := GAS_ZERO
                                        let (tup__0, tup__1, tup__2) ← do
                                          (exceptional_state state_gas_after state_spill_after
                                            carried_state_gas_reservoir OutOfGas)
                                        let state_gas_after : Nat := tup__0
                                        let state_spill_after : Nat := tup__1
                                        let status_after : FrameStatus := tup__2
                                        (pure ())
                                        (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), ((⟨_, ⟨_, memory_after⟩⟩ : (Sigma
                                          fun (carried_returndata_dependentWitness0 : Nat) =>
                                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                          (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                          fun (carried_returndata_dependentWitness0 : Nat) =>
                                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                          (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                                          fun (carried_returndata_dependentWitness0 : Nat) =>
                                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                          (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                          fun (carried_returndata_dependentWitness0 : Nat) =>
                                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                          (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), (returndata_after : (Sigma
                                          fun (carried_returndata_dependentWitness0 : Nat) =>
                                          (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                          (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))))
                                        ) : SailME
                                        (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                        (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                        fun (carried_returndata_dependentWitness0 : Nat) =>
                                        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                        )
                                      SailME.throw ((fun (dependentValue0, dependentValue1, dependentValue2, dependentValue3, dependentValue4, dependentValue5, dependentValue6, dependentValue7, dependentValue8, dependentValue9, dependentValue10, dependentValue11, dependentValue12, dependentValue13, dependentValue14, dependentValue15, dependentValue16, dependentValue17) => (dependentValue0, dependentValue1, dependentValue2, dependentValue3, dependentValue4, dependentValue5, dependentValue6, dependentValue7, dependentValue8, dependentValue9, dependentValue10, dependentValue11, dependentValue12, dependentValue13, dependentValue14, dependentValue15, dependentValue16, dependentValue17)) (a__286)))
                                  else
                                    (pure ((gas_after : Nat), (memory_after : (Sigma fun
                                      (carried_returndata_dependentWitness0 : Nat) =>
                                      (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                      (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus)))
                                  ) : SailME
                                  (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                  fun (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                  fun (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                  fun (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                  (Nat × (Sigma fun (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × Nat × Nat × FrameStatus)
                                  )
                                (pure ((gas_after : Nat), (memory_after : (Sigma fun
                                  (carried_returndata_dependentWitness0 : Nat) =>
                                  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                  (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus))))
                            else
                              (pure ((gas_after : Nat), (memory_after : (Sigma fun
                                (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (state_gas_after : Nat), (state_spill_after : Nat), (status_after : FrameStatus)))
                            ) : SailME
                            (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                            (Nat × (Sigma fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × Nat × Nat × FrameStatus)
                            )
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
                            (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                            fun (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                            Bool )
                          let ⟨_, ⟨_, returndata_after⟩⟩ : (Sigma fun
                            (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                            ((returndata_clear ()) : (Sigma fun
                            (carried_returndata_dependentWitness0 : Nat) =>
                            (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                            (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                          (k_bump_nonce creator)
                          if _sailIf5 : (occupied : Bool) = true
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
                                SailME
                                (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                                (Nat × Nat × Nat) )
                              let sp_after ← (stack_top_advance sp_after 1)
                              (write_stack_word sp_after WORD_ZERO)
                              let ⟨_, ⟨_, memory_after⟩⟩ : (Sigma fun
                                (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) :=
                                (mem2 : (Sigma fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))))
                              (pure ((pc_after : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (sp_after : StackPointer), ((⟨_, ⟨_, memory_after⟩⟩ : (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), ((⟨_, ⟨_, returndata_after⟩⟩ : (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))
                          else
                            (do
                              let (initcode_bytes, mem3) ← do
                                (memory_code_slice mem2 initcode.off initcode.len)
                              let child_code_id ← do (code_db_insert initcode_bytes profile.fork)
                              let ⟨_, ⟨_, child_code⟩⟩ ← do
                                (code_db_resolve child_code_id)
                              let child_state_gas := state_gas_after
                              let (checkpoint, child_stack, child_frame_memory) ← do
                                (suspend_frame pc_after gas_after sp_after mem3 STATE_GAS_ZERO
                                  state_spill_after carried_refund (Running ()) parent_message
                                  ⟨_, ⟨_, carried_code⟩⟩ carried_calldata)
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
                              (pure ((0 : Nat), (child_gas : Nat), (child_state_gas : Nat), (STATE_GAS_SPILL_ZERO : Nat), (GAS_REFUND_ZERO : Int), ((Running
                                  ()) : FrameStatus), (child_stack : StackPointer), (child_frame_memory : (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (creator : (Vector (BitVec 8) 20)), (new_addr : (Vector (BitVec 8) 20)), (new_addr : (Vector (BitVec 8) 20)), (value : Nat), (child_state_gas : Nat), (carried_is_static : Bool), (child_depth : Nat), ((⟨_, ⟨_, child_code⟩⟩ : (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (EMPTY_CALLDATA : CalldataSlice), ((⟨_, ⟨_, child_returndata⟩⟩ : (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
                                fun (carried_returndata_dependentWitness0 : Nat) =>
                                (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
                                (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))))))))

/-- The non-recursive step loop for one complete call tree. It executes
the active frame, resumes suspended parents through
[frame_stack_pop][] as children halt, and returns the top-level
frame's output. Each step's carried state is supplied from the frame
registers and its returned state is assigned back; the handlers
themselves never touch the registers. `STOP`, `SELFDESTRUCT`, and
exceptional halts return the empty slice; `RETURN` and `REVERT` carry
their frozen memory slice in the halt value. -/
/- Type quantifiers: initial_code_dependentWitness1 : Nat, initial_code_dependentWitness0 : Nat, k_ex610387_
  : Nat, k_ex610386_ : Bool, k_ex610385_ : Nat, k_ex610384_ : Nat, initial_memory_dependentWitness1
  : Nat, initial_memory_dependentWitness0 : Nat, k_ex610379_ : Int, k_ex610378_ : Nat, k_ex610377_ :
  Nat, k_ex610376_ : Nat, 0 ≤ k_ex610376_ ∧ k_ex610376_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex610377_ ∧
  k_ex610377_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex610378_ ∧ k_ex610378_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1))))
  ≤ k_ex610379_ ∧ k_ex610379_ ≤ (199 * (2 ^ 64 - 1)), 0 ≤ initial_memory_dependentWitness0
  ∧
  0 ≤ initial_memory_dependentWitness1 ∧
  (initial_memory_dependentWitness0 + initial_memory_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex610384_ ∧ k_ex610384_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex610385_ ∧ k_ex610385_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex610387_ ∧ k_ex610387_ ≤ 1024, 0 ≤ initial_code_dependentWitness0 ∧
  0 ≤ initial_code_dependentWitness1 ∧
  (initial_code_dependentWitness0 + initial_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ initial_code_dependentWitness1 ∧ (initial_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1) -/
def interpret (initial_gas : Nat) (initial_state_gas : Nat) (initial_state_spill : Nat) (initial_refund : Int) (initial_sp : StackPointer) (initial_memory : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (initial_caller : (Vector (BitVec 8) 20)) (initial_address : (Vector (BitVec 8) 20)) (initial_code_address : (Vector (BitVec 8) 20)) (initial_value : Nat) (initial_state_gas_reservoir : Nat) (initial_is_static : Bool) (initial_depth : Nat) (initial_code : (Sigma
  fun (initial_memory_dependentWitness0 : Nat) =>
  (Sigma fun (initial_memory_dependentWitness1 : Nat) =>
  (CodeFields initial_memory_dependentWitness0 initial_memory_dependentWitness1)))) (initial_calldata : CalldataSlice) : SailM (Nat × Nat × Nat × Int × FrameStatus × (Sigma
  fun (initial_code_dependentWitness0 : Nat) =>
  (Sigma fun (initial_code_dependentWitness1 : Nat) =>
  (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) := do
  let initial_memory_dependentWitness0 := (initial_memory).1
  let initial_memory_dependentWitness1 := ((initial_memory).2).1
  let initial_memory := ((initial_memory).2).2
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
  let carried_memory : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
    ((⟨_, ⟨_, initial_memory⟩⟩ : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
    fun (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
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
  let (call_tree_steps_remaining, carried_account_context, carried_address, carried_calldata, carried_caller, carried_code, carried_code_address, carried_depth, carried_gas, carried_is_static, carried_memory, carried_pc, carried_refund, carried_returndata, carried_sp, carried_state_gas, carried_state_gas_reservoir, carried_state_spill, carried_status, carried_value, interpreting, result) ← (( do
    let loop_vars ← whileFuelM (fuel :=call_tree_steps_remaining) (fun (call_tree_steps_remaining, carried_account_context, carried_address, carried_calldata, carried_caller, carried_code, carried_code_address, carried_depth, carried_gas, carried_is_static, carried_memory, carried_pc, carried_refund, carried_returndata, carried_sp, carried_state_gas, carried_state_gas_reservoir, carried_state_spill, carried_status, carried_value, interpreting, result) => (pure interpreting)) (call_tree_steps_remaining, carried_account_context, carried_address, carried_calldata, carried_caller, carried_code, carried_code_address, carried_depth, carried_gas, carried_is_static, carried_memory, carried_pc, carried_refund, carried_returndata, carried_sp, carried_state_gas, carried_state_gas_reservoir, carried_state_spill, carried_status, carried_value, interpreting, result)
      fun (call_tree_steps_remaining, carried_account_context, carried_address, carried_calldata, carried_caller, carried_code, carried_code_address, carried_depth, carried_gas, carried_is_static, carried_memory, carried_pc, carried_refund, carried_returndata, carried_sp, carried_state_gas, carried_state_gas_reservoir, carried_state_spill, carried_status, carried_value, interpreting, result) => do
        assert true "loop dummy assert"
        let (carried_account_context, carried_address, carried_calldata, carried_caller, carried_code, carried_code_address, carried_depth, carried_gas, carried_is_static, carried_memory, carried_pc, carried_refund, carried_returndata, carried_sp, carried_state_gas, carried_state_gas_reservoir, carried_state_spill, carried_status, carried_value, interpreting, result) ← (( do
          if _sailIf0 : ((is_running carried_status) : Bool) = true
          then
            (do
              let (fetched_pc, instruction) ← do (fetch carried_code carried_pc fork)
              let carried_pc : Nat := fetched_pc
              let (carried_account_context, carried_address, carried_calldata, carried_caller, carried_code, carried_code_address, carried_depth, carried_gas, carried_is_static, carried_memory, carried_pc, carried_refund, carried_returndata, carried_sp, carried_state_gas, carried_state_gas_reservoir, carried_state_spill, carried_status, carried_value) ← (( do
                match instruction with
                | .opcode_CREATE () =>
                  (do
                    let previous_address := carried_address
                    let (tup__0, tup__1, tup__2, tup__3, tup__4, tup__5, tup__6, tup__7, tup__8, tup__9, tup__10, tup__11, tup__12, tup__13, tup__14, tup__15, tup__16, tup__17) ← do
                      (run_create carried_pc carried_gas carried_state_gas carried_state_spill
                        carried_refund carried_sp carried_memory carried_caller carried_address
                        carried_code_address carried_value carried_state_gas_reservoir
                        carried_is_static carried_depth carried_code carried_calldata
                        carried_returndata CreateByNonce)
                    let carried_pc : Nat := tup__0
                    let carried_gas : Nat := tup__1
                    let carried_state_gas : Nat := tup__2
                    let carried_state_spill : Nat := tup__3
                    let carried_refund : Int := tup__4
                    let carried_status : FrameStatus := tup__5
                    let carried_sp : StackPointer := tup__6
                    let carried_memory : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__7 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_caller : (Vector (BitVec 8) 20) := tup__8
                    let carried_address : (Vector (BitVec 8) 20) := tup__9
                    let carried_code_address : (Vector (BitVec 8) 20) := tup__10
                    let carried_value : Nat := tup__11
                    let carried_state_gas_reservoir : Nat := tup__12
                    let carried_is_static : Bool := tup__13
                    let carried_depth : Nat := tup__14
                    let carried_code : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__15 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_calldata : CalldataSlice := tup__16
                    let carried_returndata : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__17 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    (pure ())
                    let carried_account_context : AccountExecutionContext :=
                      (refresh_account_execution_context carried_account_context previous_address
                        carried_address)
                    (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), (carried_code : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat))))
                | .CREATE2 () =>
                  (do
                    let previous_address := carried_address
                    let (tup__0, tup__1, tup__2, tup__3, tup__4, tup__5, tup__6, tup__7, tup__8, tup__9, tup__10, tup__11, tup__12, tup__13, tup__14, tup__15, tup__16, tup__17) ← do
                      (run_create carried_pc carried_gas carried_state_gas carried_state_spill
                        carried_refund carried_sp carried_memory carried_caller carried_address
                        carried_code_address carried_value carried_state_gas_reservoir
                        carried_is_static carried_depth carried_code carried_calldata
                        carried_returndata CreateBySalt)
                    let carried_pc : Nat := tup__0
                    let carried_gas : Nat := tup__1
                    let carried_state_gas : Nat := tup__2
                    let carried_state_spill : Nat := tup__3
                    let carried_refund : Int := tup__4
                    let carried_status : FrameStatus := tup__5
                    let carried_sp : StackPointer := tup__6
                    let carried_memory : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__7 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_caller : (Vector (BitVec 8) 20) := tup__8
                    let carried_address : (Vector (BitVec 8) 20) := tup__9
                    let carried_code_address : (Vector (BitVec 8) 20) := tup__10
                    let carried_value : Nat := tup__11
                    let carried_state_gas_reservoir : Nat := tup__12
                    let carried_is_static : Bool := tup__13
                    let carried_depth : Nat := tup__14
                    let carried_code : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__15 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_calldata : CalldataSlice := tup__16
                    let carried_returndata : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__17 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    (pure ())
                    let carried_account_context : AccountExecutionContext :=
                      (refresh_account_execution_context carried_account_context previous_address
                        carried_address)
                    (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), (carried_code : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat))))
                | .CALL () =>
                  (do
                    let previous_address := carried_address
                    let (tup__0, tup__1, tup__2, tup__3, tup__4, tup__5, tup__6, tup__7, tup__8, tup__9, tup__10, tup__11, tup__12, tup__13, tup__14, tup__15, tup__16, tup__17) ← do
                      (run_call carried_pc carried_gas carried_state_gas carried_state_spill
                        carried_refund carried_sp carried_memory carried_caller carried_address
                        carried_code_address carried_value carried_state_gas_reservoir
                        carried_is_static carried_depth carried_code carried_calldata
                        carried_returndata Call)
                    let carried_pc : Nat := tup__0
                    let carried_gas : Nat := tup__1
                    let carried_state_gas : Nat := tup__2
                    let carried_state_spill : Nat := tup__3
                    let carried_refund : Int := tup__4
                    let carried_status : FrameStatus := tup__5
                    let carried_sp : StackPointer := tup__6
                    let carried_memory : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__7 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_caller : (Vector (BitVec 8) 20) := tup__8
                    let carried_address : (Vector (BitVec 8) 20) := tup__9
                    let carried_code_address : (Vector (BitVec 8) 20) := tup__10
                    let carried_value : Nat := tup__11
                    let carried_state_gas_reservoir : Nat := tup__12
                    let carried_is_static : Bool := tup__13
                    let carried_depth : Nat := tup__14
                    let carried_code : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__15 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_calldata : CalldataSlice := tup__16
                    let carried_returndata : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__17 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    (pure ())
                    let carried_account_context : AccountExecutionContext :=
                      (refresh_account_execution_context carried_account_context previous_address
                        carried_address)
                    (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), (carried_code : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat))))
                | .CALLCODE () =>
                  (do
                    let previous_address := carried_address
                    let (tup__0, tup__1, tup__2, tup__3, tup__4, tup__5, tup__6, tup__7, tup__8, tup__9, tup__10, tup__11, tup__12, tup__13, tup__14, tup__15, tup__16, tup__17) ← do
                      (run_call carried_pc carried_gas carried_state_gas carried_state_spill
                        carried_refund carried_sp carried_memory carried_caller carried_address
                        carried_code_address carried_value carried_state_gas_reservoir
                        carried_is_static carried_depth carried_code carried_calldata
                        carried_returndata CallCode)
                    let carried_pc : Nat := tup__0
                    let carried_gas : Nat := tup__1
                    let carried_state_gas : Nat := tup__2
                    let carried_state_spill : Nat := tup__3
                    let carried_refund : Int := tup__4
                    let carried_status : FrameStatus := tup__5
                    let carried_sp : StackPointer := tup__6
                    let carried_memory : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__7 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_caller : (Vector (BitVec 8) 20) := tup__8
                    let carried_address : (Vector (BitVec 8) 20) := tup__9
                    let carried_code_address : (Vector (BitVec 8) 20) := tup__10
                    let carried_value : Nat := tup__11
                    let carried_state_gas_reservoir : Nat := tup__12
                    let carried_is_static : Bool := tup__13
                    let carried_depth : Nat := tup__14
                    let carried_code : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__15 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_calldata : CalldataSlice := tup__16
                    let carried_returndata : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__17 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    (pure ())
                    let carried_account_context : AccountExecutionContext :=
                      (refresh_account_execution_context carried_account_context previous_address
                        carried_address)
                    (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), (carried_code : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat))))
                | .DELEGATECALL () =>
                  (do
                    let previous_address := carried_address
                    let (tup__0, tup__1, tup__2, tup__3, tup__4, tup__5, tup__6, tup__7, tup__8, tup__9, tup__10, tup__11, tup__12, tup__13, tup__14, tup__15, tup__16, tup__17) ← do
                      (run_call carried_pc carried_gas carried_state_gas carried_state_spill
                        carried_refund carried_sp carried_memory carried_caller carried_address
                        carried_code_address carried_value carried_state_gas_reservoir
                        carried_is_static carried_depth carried_code carried_calldata
                        carried_returndata DelegateCall)
                    let carried_pc : Nat := tup__0
                    let carried_gas : Nat := tup__1
                    let carried_state_gas : Nat := tup__2
                    let carried_state_spill : Nat := tup__3
                    let carried_refund : Int := tup__4
                    let carried_status : FrameStatus := tup__5
                    let carried_sp : StackPointer := tup__6
                    let carried_memory : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__7 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_caller : (Vector (BitVec 8) 20) := tup__8
                    let carried_address : (Vector (BitVec 8) 20) := tup__9
                    let carried_code_address : (Vector (BitVec 8) 20) := tup__10
                    let carried_value : Nat := tup__11
                    let carried_state_gas_reservoir : Nat := tup__12
                    let carried_is_static : Bool := tup__13
                    let carried_depth : Nat := tup__14
                    let carried_code : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__15 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_calldata : CalldataSlice := tup__16
                    let carried_returndata : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__17 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    (pure ())
                    let carried_account_context : AccountExecutionContext :=
                      (refresh_account_execution_context carried_account_context previous_address
                        carried_address)
                    (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), (carried_code : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat))))
                | .STATICCALL () =>
                  (do
                    let previous_address := carried_address
                    let (tup__0, tup__1, tup__2, tup__3, tup__4, tup__5, tup__6, tup__7, tup__8, tup__9, tup__10, tup__11, tup__12, tup__13, tup__14, tup__15, tup__16, tup__17) ← do
                      (run_call carried_pc carried_gas carried_state_gas carried_state_spill
                        carried_refund carried_sp carried_memory carried_caller carried_address
                        carried_code_address carried_value carried_state_gas_reservoir
                        carried_is_static carried_depth carried_code carried_calldata
                        carried_returndata StaticCall)
                    let carried_pc : Nat := tup__0
                    let carried_gas : Nat := tup__1
                    let carried_state_gas : Nat := tup__2
                    let carried_state_spill : Nat := tup__3
                    let carried_refund : Int := tup__4
                    let carried_status : FrameStatus := tup__5
                    let carried_sp : StackPointer := tup__6
                    let carried_memory : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__7 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_caller : (Vector (BitVec 8) 20) := tup__8
                    let carried_address : (Vector (BitVec 8) 20) := tup__9
                    let carried_code_address : (Vector (BitVec 8) 20) := tup__10
                    let carried_value : Nat := tup__11
                    let carried_state_gas_reservoir : Nat := tup__12
                    let carried_is_static : Bool := tup__13
                    let carried_depth : Nat := tup__14
                    let carried_code : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__15 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_calldata : CalldataSlice := tup__16
                    let carried_returndata : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__17 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    (pure ())
                    let carried_account_context : AccountExecutionContext :=
                      (refresh_account_execution_context carried_account_context previous_address
                        carried_address)
                    (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), (carried_code : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat))))
                | _ =>
                  (do
                    let result ← (( do
                      match instruction with
                      | .STOP () =>
                        (let status_after := (execute_stop ())
                        (pure ((carried_pc : Nat), (carried_gas : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (carried_sp : StackPointer), (carried_memory : (Sigma
                          fun (initial_code_dependentWitness0 : Nat) =>
                          (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (status_after : FrameStatus))))
                      | .ADD () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_add carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .MUL () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_mul carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .SUB () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_sub carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .DIV () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_div carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .SDIV () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_sdiv carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .MOD () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_mod carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .SMOD () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_smod carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .ADDMOD () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_addmod carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .MULMOD () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_mulmod carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .EXP () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_exp carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .SIGNEXTEND () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_signextend carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .LT () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_lt carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .GT () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_gt carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .SLT () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_slt carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .SGT () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_sgt carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .EQ () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_eq carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .ISZERO () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_iszero carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .AND () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_and carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .OR () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_or carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .XOR () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_xor carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .NOT () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_not carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .BYTE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_byte carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .SHL () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_shl carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .SHR () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_shr carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .SAR () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_sar carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .CLZ () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_clz carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .KECCAK256 () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_keccak256 carried_gas carried_sp carried_memory)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (memory_after : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .ADDRESS () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_address carried_address carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .BALANCE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_balance carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .ORIGIN () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_origin carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .CALLER () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_caller carried_caller carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .CALLVALUE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_callvalue carried_value carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .CALLDATALOAD () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_calldataload carried_calldata carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .CALLDATASIZE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_calldatasize carried_calldata carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .CALLDATACOPY () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_calldatacopy carried_calldata carried_gas carried_sp
                              carried_memory)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (memory_after : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .CODESIZE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_codesize carried_code carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .CODECOPY () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_codecopy carried_code carried_gas carried_sp carried_memory)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (memory_after : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .GASPRICE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_gasprice carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .EXTCODESIZE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_extcodesize carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .EXTCODECOPY () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_extcodecopy carried_gas carried_sp carried_memory)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (memory_after : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .RETURNDATASIZE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_returndatasize carried_returndata carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .RETURNDATACOPY () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_returndatacopy carried_returndata carried_gas carried_sp
                              carried_memory)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (memory_after : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .EXTCODEHASH () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_extcodehash carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .BLOCKHASH () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_blockhash carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .COINBASE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_coinbase carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .TIMESTAMP () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_timestamp carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .NUMBER () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_number carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .SLOTNUM () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_slotnum carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .PREVRANDAO () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_prevrandao carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .GASLIMIT () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_gaslimit carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .CHAINID () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_chainid carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .SELFBALANCE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_selfbalance carried_address carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .BASEFEE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_basefee carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .BLOBHASH () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_blobhash carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .BLOBBASEFEE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_blobbasefee blob_fee carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .POP () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_pop carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .MLOAD () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_mload carried_gas carried_sp carried_memory)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (memory_after : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .MSTORE () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_mstore carried_gas carried_sp carried_memory)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (memory_after : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .MSTORE8 () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_mstore8 carried_gas carried_sp carried_memory)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (memory_after : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .SLOAD () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_sload carried_account_context carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .SSTORE () =>
                        (do
                          let (gas_after, state_gas_after, state_spill_after, refund_after, sp_after, status_after) ← do
                            (execute_sstore carried_account_context fork carried_is_static
                              carried_gas carried_state_gas carried_state_spill carried_refund
                              carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (refund_after : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .JUMP () =>
                        (do
                          let (pc_after, gas_after, sp_after, status_after) ← do
                            (execute_jump carried_code carried_pc carried_gas carried_sp)
                          (pure ((pc_after : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .JUMPI () =>
                        (do
                          let (pc_after, gas_after, sp_after, status_after) ← do
                            (execute_jumpi carried_code carried_pc carried_gas carried_sp)
                          (pure ((pc_after : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .PC () =>
                        (do
                          let (pc_after, gas_after, sp_after, status_after) ← do
                            (execute_pc carried_pc carried_gas carried_sp)
                          (pure ((pc_after : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .MSIZE () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_msize carried_gas carried_sp carried_memory)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (memory_after : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .GAS () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_gas carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .JUMPDEST () =>
                        (let (gas_after, status_after) := (execute_jumpdest carried_gas)
                        (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (carried_sp : StackPointer), (carried_memory : (Sigma
                          fun (initial_code_dependentWitness0 : Nat) =>
                          (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                            status_after) : FrameStatus))))
                      | .TLOAD () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_tload carried_address carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .TSTORE () =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_tstore carried_address carried_is_static carried_gas carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .MCOPY () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_mcopy carried_gas carried_sp carried_memory)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (memory_after : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .PUSH (n, value) =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_push carried_gas carried_sp n value)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .DUP n =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_dup carried_gas carried_sp n)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .SWAP n =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_swap carried_gas carried_sp n)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .DUPN immediate =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_dupn carried_gas carried_sp immediate)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .SWAPN immediate =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_swapn carried_gas carried_sp immediate)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .EXCHANGE immediate =>
                        (do
                          let (gas_after, sp_after, status_after) ← do
                            (execute_exchange carried_gas carried_sp immediate)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .LOG n =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_log carried_address carried_is_static carried_gas carried_sp
                              carried_memory n)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (memory_after : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                              status_after) : FrameStatus))))
                      | .opcode_CREATE () =>
                        (do
                          (fatal_error ExecutionInvalid))
                      | .CREATE2 () =>
                        (do
                          (fatal_error ExecutionInvalid))
                      | .CALL () =>
                        (do
                          (fatal_error ExecutionInvalid))
                      | .CALLCODE () =>
                        (do
                          (fatal_error ExecutionInvalid))
                      | .DELEGATECALL () =>
                        (do
                          (fatal_error ExecutionInvalid))
                      | .STATICCALL () =>
                        (do
                          (fatal_error ExecutionInvalid))
                      | .RETURN () =>
                        (do
                          let (gas_after, sp_after, memory_after, status_after) ← do
                            (execute_return carried_gas carried_sp carried_memory)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (sp_after : StackPointer), (memory_after : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (status_after : FrameStatus))))
                      | .REVERT () =>
                        (do
                          let (gas_after, state_gas_after, state_spill_after, sp_after, memory_after, status_after) ← do
                            (execute_revert carried_state_gas_reservoir carried_gas
                              carried_state_gas carried_state_spill carried_sp carried_memory)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (sp_after : StackPointer), (memory_after : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (status_after : FrameStatus))))
                      | .INVALID () =>
                        (let (gas_after, status_after) := (execute_invalid carried_gas)
                        (pure ((carried_pc : Nat), (gas_after : Nat), (carried_state_gas : Nat), (carried_state_spill : Nat), (carried_refund : Int), (carried_sp : StackPointer), (carried_memory : (Sigma
                          fun (initial_code_dependentWitness0 : Nat) =>
                          (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), ((opcode_frame_status
                            status_after) : FrameStatus))))
                      | .SELFDESTRUCT () =>
                        (do
                          let (gas_after, state_gas_after, state_spill_after, refund_after, sp_after, status_after) ← do
                            (execute_selfdestruct carried_address fork carried_is_static carried_gas
                              carried_state_gas carried_state_spill carried_refund carried_sp)
                          (pure ((carried_pc : Nat), (gas_after : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (refund_after : Int), (sp_after : StackPointer), (carried_memory : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (status_after : FrameStatus))))
                      ) : SailM
                      (Nat × Nat × Nat × Nat × Int × StackPointer × (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × FrameStatus)
                      )
                    let (tup__0, tup__1, tup__2, tup__3, tup__4, tup__5, tup__6, tup__7) ← do
                      match result with
                      | (pc_after, _, state_gas_after, state_spill_after, refund_after, sp_after, memory_after, .Exceptional kind) =>
                        (do
                          let (state_gas_after, state_spill_after, status_after) ← do
                            (exceptional_state state_gas_after state_spill_after
                              carried_state_gas_reservoir kind)
                          (pure ((pc_after : Nat), (GAS_ZERO : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (refund_after : Int), (sp_after : StackPointer), (memory_after : (Sigma
                            fun (initial_code_dependentWitness0 : Nat) =>
                            (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                            (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (status_after : FrameStatus))))
                      | _ =>
                        (pure ((((fun (dependentValue0, dependentValue1, dependentValue2, dependentValue3, dependentValue4, dependentValue5, dependentValue6, dependentValue7) => (dependentValue0, dependentValue1, dependentValue2, dependentValue3, dependentValue4, dependentValue5, dependentValue6, dependentValue7)) result) : (Nat × Nat × Nat × Nat × Int × StackPointer × (Sigma
                          fun (initial_code_dependentWitness0 : Nat) =>
                          (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × FrameStatus)) : (Nat × Nat × Nat × Nat × Int × StackPointer × (Sigma
                          fun (initial_code_dependentWitness0 : Nat) =>
                          (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                          (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × FrameStatus)))
                    let carried_pc : Nat := tup__0
                    let carried_gas : Nat := tup__1
                    let carried_state_gas : Nat := tup__2
                    let carried_state_spill : Nat := tup__3
                    let carried_refund : Int := tup__4
                    let carried_sp : StackPointer := tup__5
                    let ⟨_, ⟨_, carried_memory⟩⟩ : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__6 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_status : FrameStatus := tup__7
                    (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), (carried_code : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))) : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat))))
                ) : SailM
                (AccountExecutionContext × (Vector (BitVec 8) 20) × CalldataSlice × (Vector (BitVec 8) 20) × (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × Nat × Int × (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × StackPointer × Nat × Nat × Nat × FrameStatus × Nat)
                )
              (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), (carried_code : (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory : (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
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
              let (carried_account_context, carried_address, carried_calldata, carried_caller, carried_code, carried_code_address, carried_depth, carried_gas, carried_is_static, carried_memory, carried_pc, carried_refund, carried_returndata, carried_sp, carried_state_gas, carried_state_gas_reservoir, carried_state_spill, carried_status, carried_value, interpreting, result) ← (( do
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
                    (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory : (Sigma
                    fun (initial_code_dependentWitness0 : Nat) =>
                    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                    (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
                    fun (initial_code_dependentWitness0 : Nat) =>
                    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                    (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_sp : StackPointer), (carried_state_gas : Nat), (carried_state_gas_reservoir : Nat), (carried_state_spill : Nat), (carried_status : FrameStatus), (carried_value : Nat), (interpreting : Bool), (result : (Sigma
                    fun (initial_code_dependentWitness0 : Nat) =>
                    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                    (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))))))
                | continuation =>
                  (do
                    let previous_address := carried_address
                    let (tup__0, tup__1, tup__2, tup__3, tup__4, tup__5, tup__6, tup__7, tup__8, tup__9, tup__10, tup__11, tup__12, tup__13, tup__14, tup__15, tup__16, tup__17) ← do
                      (resume_frame continuation ⟨_, ⟨_, output⟩⟩ carried_gas
                        carried_state_gas carried_state_spill carried_refund carried_status
                        carried_state_gas_reservoir)
                    let carried_pc : Nat := tup__0
                    let carried_gas : Nat := tup__1
                    let carried_state_gas : Nat := tup__2
                    let carried_state_spill : Nat := tup__3
                    let carried_refund : Int := tup__4
                    let carried_status : FrameStatus := tup__5
                    let carried_sp : StackPointer := tup__6
                    let carried_memory : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__7 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_caller : (Vector (BitVec 8) 20) := tup__8
                    let carried_address : (Vector (BitVec 8) 20) := tup__9
                    let carried_code_address : (Vector (BitVec 8) 20) := tup__10
                    let carried_value : Nat := tup__11
                    let carried_state_gas_reservoir : Nat := tup__12
                    let carried_is_static : Bool := tup__13
                    let carried_depth : Nat := tup__14
                    let carried_code : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__15 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    let carried_calldata : CalldataSlice := tup__16
                    let carried_returndata : (Sigma fun
                      (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) :=
                      (tup__17 : (Sigma fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (OutputSliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))))
                    (pure ())
                    let carried_account_context : AccountExecutionContext :=
                      (refresh_account_execution_context carried_account_context previous_address
                        carried_address)
                    (pure ((carried_account_context : AccountExecutionContext), (carried_address : (Vector (BitVec 8) 20)), (carried_calldata : CalldataSlice), (carried_caller : (Vector (BitVec 8) 20)), (carried_code : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory : (Sigma
                      fun (initial_code_dependentWitness0 : Nat) =>
                      (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                      (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
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
                (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × Nat × Int × (Sigma
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
                (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory : (Sigma
                fun (initial_code_dependentWitness0 : Nat) =>
                (Sigma fun (initial_code_dependentWitness1 : Nat) =>
                (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
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
          (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × (Sigma
          fun (initial_code_dependentWitness0 : Nat) =>
          (Sigma fun (initial_code_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × Nat × Int × (Sigma
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
          (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_code_address : (Vector (BitVec 8) 20)), (carried_depth : Nat), (carried_gas : Nat), (carried_is_static : Bool), (carried_memory : (Sigma
          fun (initial_code_dependentWitness0 : Nat) =>
          (Sigma fun (initial_code_dependentWitness1 : Nat) =>
          (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1)))), (carried_pc : Nat), (carried_refund : Int), (carried_returndata : (Sigma
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
    (CodeFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × (Sigma
    fun (initial_code_dependentWitness0 : Nat) =>
    (Sigma fun (initial_code_dependentWitness1 : Nat) =>
    (EvmMemorySliceFields initial_code_dependentWitness0 initial_code_dependentWitness1))) × Nat × Int × (Sigma
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
/- Type quantifiers: k_ex610427_ : Nat, carried_returndata_dependentWitness1 : Nat, carried_returndata_dependentWitness0
  : Nat, carried_code_dependentWitness1 : Nat, carried_code_dependentWitness0 : Nat, k_ex610418_ :
  Nat, k_ex610417_ : Bool, k_ex610416_ : Nat, k_ex610415_ : Nat, carried_memory_dependentWitness1 :
  Nat, carried_memory_dependentWitness0 : Nat, k_ex610410_ : Int, k_ex610409_ : Nat, k_ex610408_ :
  Nat, k_ex610407_ : Nat, k_ex610406_ : Nat, 0 ≤ k_ex610406_ ∧ k_ex610406_ ≤ (2 ^ 32 - 1), 0
  ≤ k_ex610407_ ∧ k_ex610407_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex610408_ ∧
  k_ex610408_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex610409_ ∧ k_ex610409_ ≤ (2 ^ 24), ((- (199 * (2 ^ 64 - 1))))
  ≤ k_ex610410_ ∧ k_ex610410_ ≤ (199 * (2 ^ 64 - 1)), 0 ≤ carried_memory_dependentWitness0
  ∧
  0 ≤ carried_memory_dependentWitness1 ∧
  (carried_memory_dependentWitness0 + carried_memory_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤
  k_ex610415_ ∧ k_ex610415_ ≤ (2 ^ 256 - 1), 0 ≤ k_ex610416_ ∧ k_ex610416_ ≤ (2 ^ 64 - 1), 0
  ≤ k_ex610418_ ∧ k_ex610418_ ≤ 1024, 0 ≤ carried_code_dependentWitness0 ∧
  0 ≤ carried_code_dependentWitness1 ∧
  (carried_code_dependentWitness0 + carried_code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ carried_code_dependentWitness1 ∧ (carried_code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1), 0
  ≤ carried_returndata_dependentWitness0 ∧
  0 ≤ carried_returndata_dependentWitness1 ∧
  (carried_returndata_dependentWitness0 + carried_returndata_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex610427_ ∧ k_ex610427_ ≤ 255 -/
def run_frame_entry_encoded (carried_pc : Nat) (carried_gas : Nat) (carried_state_gas : Nat) (carried_state_spill : Nat) (carried_refund : Int) (carried_sp : StackPointer) (carried_memory : (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmMemorySliceFields k_off k_len)))) (carried_caller : (Vector (BitVec 8) 20)) (carried_address : (Vector (BitVec 8) 20)) (carried_code_address : (Vector (BitVec 8) 20)) (carried_value : Nat) (carried_state_gas_reservoir : Nat) (carried_is_static : Bool) (carried_depth : Nat) (carried_code : (Sigma
  fun (carried_memory_dependentWitness0 : Nat) =>
  (Sigma fun (carried_memory_dependentWitness1 : Nat) =>
  (CodeFields carried_memory_dependentWitness0 carried_memory_dependentWitness1)))) (carried_calldata : CalldataSlice) (carried_returndata : (Sigma
  fun (carried_code_dependentWitness0 : Nat) =>
  (Sigma fun (carried_code_dependentWitness1 : Nat) =>
  (OutputSliceFields carried_code_dependentWitness0 carried_code_dependentWitness1)))) (opcode : Nat) : SailM (Nat × Nat × Nat × Nat × Int × FrameStatus × StackPointer × (Sigma
  fun (carried_returndata_dependentWitness0 : Nat) =>
  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
  (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × (Vector (BitVec 8) 20) × Nat × Nat × Bool × Nat × (Sigma
  fun (carried_returndata_dependentWitness0 : Nat) =>
  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
  (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1))) × CalldataSlice × (Sigma
  fun (carried_returndata_dependentWitness0 : Nat) =>
  (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
  (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) := do
  let carried_memory_dependentWitness0 := (carried_memory).1
  let carried_memory_dependentWitness1 := ((carried_memory).2).1
  let carried_memory := ((carried_memory).2).2
  let carried_code_dependentWitness0 := (carried_code).1
  let carried_code_dependentWitness1 := ((carried_code).2).1
  let carried_code := ((carried_code).2).2
  let carried_returndata_dependentWitness0 := (carried_returndata).1
  let carried_returndata_dependentWitness1 := ((carried_returndata).2).1
  let carried_returndata := ((carried_returndata).2).2
  match opcode with
  | 240 =>
    (do
      (run_create carried_pc carried_gas carried_state_gas carried_state_spill carried_refund
        carried_sp ⟨_, ⟨_, carried_memory⟩⟩ carried_caller carried_address
        carried_code_address carried_value carried_state_gas_reservoir carried_is_static
        carried_depth ⟨_, ⟨_, carried_code⟩⟩ carried_calldata
        ⟨_, ⟨_, carried_returndata⟩⟩ CreateByNonce))
  | 241 =>
    (do
      (run_call carried_pc carried_gas carried_state_gas carried_state_spill carried_refund
        carried_sp ⟨_, ⟨_, carried_memory⟩⟩ carried_caller carried_address
        carried_code_address carried_value carried_state_gas_reservoir carried_is_static
        carried_depth ⟨_, ⟨_, carried_code⟩⟩ carried_calldata
        ⟨_, ⟨_, carried_returndata⟩⟩ Call))
  | 242 =>
    (do
      (run_call carried_pc carried_gas carried_state_gas carried_state_spill carried_refund
        carried_sp ⟨_, ⟨_, carried_memory⟩⟩ carried_caller carried_address
        carried_code_address carried_value carried_state_gas_reservoir carried_is_static
        carried_depth ⟨_, ⟨_, carried_code⟩⟩ carried_calldata
        ⟨_, ⟨_, carried_returndata⟩⟩ CallCode))
  | 244 =>
    (do
      (run_call carried_pc carried_gas carried_state_gas carried_state_spill carried_refund
        carried_sp ⟨_, ⟨_, carried_memory⟩⟩ carried_caller carried_address
        carried_code_address carried_value carried_state_gas_reservoir carried_is_static
        carried_depth ⟨_, ⟨_, carried_code⟩⟩ carried_calldata
        ⟨_, ⟨_, carried_returndata⟩⟩ DelegateCall))
  | 245 =>
    (do
      (run_create carried_pc carried_gas carried_state_gas carried_state_spill carried_refund
        carried_sp ⟨_, ⟨_, carried_memory⟩⟩ carried_caller carried_address
        carried_code_address carried_value carried_state_gas_reservoir carried_is_static
        carried_depth ⟨_, ⟨_, carried_code⟩⟩ carried_calldata
        ⟨_, ⟨_, carried_returndata⟩⟩ CreateBySalt))
  | 250 =>
    (do
      (run_call carried_pc carried_gas carried_state_gas carried_state_spill carried_refund
        carried_sp ⟨_, ⟨_, carried_memory⟩⟩ carried_caller carried_address
        carried_code_address carried_value carried_state_gas_reservoir carried_is_static
        carried_depth ⟨_, ⟨_, carried_code⟩⟩ carried_calldata
        ⟨_, ⟨_, carried_returndata⟩⟩ StaticCall))
  | _ =>
    (do
      let (state_gas_after, state_spill_after, status_after) ← do
        (exceptional_state carried_state_gas carried_state_spill carried_state_gas_reservoir
          InvalidOpcode)
      (pure ((carried_pc : Nat), (GAS_ZERO : Nat), (state_gas_after : Nat), (state_spill_after : Nat), (carried_refund : Int), (status_after : FrameStatus), (carried_sp : StackPointer), ((⟨_, ⟨_, carried_memory⟩⟩ : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (EvmMemorySliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_caller : (Vector (BitVec 8) 20)), (carried_address : (Vector (BitVec 8) 20)), (carried_code_address : (Vector (BitVec 8) 20)), (carried_value : Nat), (carried_state_gas_reservoir : Nat), (carried_is_static : Bool), (carried_depth : Nat), ((⟨_, ⟨_, carried_code⟩⟩ : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (CodeFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))), (carried_calldata : CalldataSlice), ((⟨_, ⟨_, carried_returndata⟩⟩ : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))) : (Sigma
        fun (carried_returndata_dependentWitness0 : Nat) =>
        (Sigma fun (carried_returndata_dependentWitness1 : Nat) =>
        (OutputSliceFields carried_returndata_dependentWitness0 carried_returndata_dependentWitness1)))))))

