import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Primitives.Code
import Evm.Host.EvmByteSlice
import Evm.Host.Code
import Evm.Host.Output
import Evm.Host.Kernel.Environment
import Evm.Host.Kernel.Storage
import Evm.Host.Kernel.Logs
import Evm.Host.Kernel.Accounts
import Evm.Host.Kernel.Code
import Evm.Host.Kernel.Selfdestruct
import Evm.Evm.Machine
import Evm.Evm.Gas
import Evm.Evm.Precompiles
import Evm.Evm.Instructions

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

/-! # Opcode semantics

The single-step transition function of the EVM (Yellow Paper §9): given
the decoded opcode, [execute][] charges its gas, consumes its stack
operands, and produces its result or effect. One match arm per opcode,
grouped by family (arithmetic, bitwise, keccak, environment, block,
stack/memory, storage, flow, push/dup/swap, log, system).

Pure compute is done here; every world effect is a kernel syscall
(`k_*`). All gas and policy stays in the EVM: it marks-and-prices access
via the kernel's returned warm bit (EIP-2929), decides whether an effect
happens, and issues the syscall only for real effects (a no-op `SSTORE`
charges gas but issues no host write). Memory-touching opcodes pay the
quadratic expansion cost via [charge_memory_range][] before acting.
Sub-calls and creates delegate to [run_call][] / [run_create][], which
install a child frame and save its parent continuation. The non-recursive
opcode bodies live in [execute_opcode][]. -/

/-- The storage owner (YP I_a): `SLOAD`, `SSTORE`, `LOG`, and
`SELFDESTRUCT` act on this account. -/
def self_addr (_ : Unit) : SailM address := do
  (pure (← readReg message).address)

/-- EIP-214 write protection: any state-modifying opcode in a
`STATICCALL` frame halts exceptionally. State-changing opcodes call
this first; `true` means already halted. -/
def guard_static (_ : Unit) : SailM Bool := do
  if ((← readReg message).is_static : Bool)
  then
    (do
      (exc_halt WriteProtection)
      (pure true))
  else (pure false)

/-- `JUMP`/`JUMPI` target validity: the destination must be in code range
and land on a `JUMPDEST` (the precomputed valid-destination set,
YP §9.4.3); otherwise an invalid-jump exceptional halt. PUSH-data
bytes that happen to equal `0x5b` are not valid. -/
/- Type quantifiers: destination_value : Nat, 0 ≤ destination_value ∧
  destination_value ≤ (2 ^ 256 - 1) -/
def do_jump (destination_value : word) : SailM Unit := do
  let destination_value := (destination_value).value
  let code_length ← do (frame_code_len ())
  if ((destination_value <b code_length) : Bool)
  then
    (do
      let destination : Nat := destination_value
      if ((← (frame_jumpdest_valid destination)) : Bool)
      then writeReg pc destination
      else (exc_halt InvalidJump))
  else (exc_halt InvalidJump)

/-- Pops `count` log topics from the stack. -/
/- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ 4 -/
def pop_log_topics (count : log_topic_count) : SailM (List word) := do
  let count := (count).value
  let publicResult ← do
    match count with
    | 0 => (pure [])
    | 1 => (pure [((← (pop ()))).value])
    | 2 =>
      (do
        let t0 ← do
          (do
              let publicResult ← (pop ())
              pure ((publicResult).value))
        let t1 ← do
          (do
              let publicResult ← (pop ())
              pure ((publicResult).value))
        (pure [t0, t1]))
    | 3 =>
      (do
        let t0 ← do
          (do
              let publicResult ← (pop ())
              pure ((publicResult).value))
        let t1 ← do
          (do
              let publicResult ← (pop ())
              pure ((publicResult).value))
        let t2 ← do
          (do
              let publicResult ← (pop ())
              pure ((publicResult).value))
        (pure [t0, t1, t2]))
    | 4 =>
      (do
        let t0 ← do
          (do
              let publicResult ← (pop ())
              pure ((publicResult).value))
        let t1 ← do
          (do
              let publicResult ← (pop ())
              pure ((publicResult).value))
        let t2 ← do
          (do
              let publicResult ← (pop ())
              pure ((publicResult).value))
        let t3 ← do
          (do
              let publicResult ← (pop ())
              pure ((publicResult).value))
        (pure [t0, t1, t2, t3]))
    | _ => (pure [])
  pure ((List.map (fun semanticValue => ⟨semanticValue⟩) (publicResult)))

/-- Executes the arithmetic, comparison, and bitwise opcode families. -/
def execute_arithmetic (op : ast) : SailM Unit := do
  match op with
  | .ADD () =>
    (do
      (charge (G_verylow).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_add ⟨a⟩ b)).value⟩))
  | .MUL () =>
    (do
      (charge (G_low).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨(alu_mul a b)⟩))
  | .SUB () =>
    (do
      (charge (G_verylow).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_sub ⟨a⟩ ⟨b⟩)).value⟩))
  | .DIV () =>
    (do
      (charge (G_low).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_div ⟨a⟩ ⟨b⟩)).value⟩))
  | .SDIV () =>
    (do
      (charge (G_low).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_sdiv ⟨a⟩ ⟨b⟩)).value⟩))
  | .MOD () =>
    (do
      (charge (G_low).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_mod ⟨a⟩ ⟨b⟩)).value⟩))
  | .SMOD () =>
    (do
      (charge (G_low).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_smod ⟨a⟩ ⟨b⟩)).value⟩))
  | .ADDMOD () =>
    (do
      (charge (G_mid).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let n ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_addmod a b n)).value⟩))
  | .MULMOD () =>
    (do
      (charge (G_mid).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let n ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_mulmod a b n)).value⟩))
  | .EXP () =>
    (do
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let e ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (charge (exp_gas ⟨e⟩))
      (push_word ⟨((alu_exp ⟨a⟩ ⟨e⟩)).value⟩))
  | .SIGNEXTEND () =>
    (do
      (charge (G_low).value)
      let bi ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let v ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_signextend ⟨bi⟩ ⟨v⟩)).value⟩))
  | .LT () =>
    (do
      (charge (G_verylow).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_lt ⟨a⟩ ⟨b⟩)).value⟩))
  | .GT () =>
    (do
      (charge (G_verylow).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_gt ⟨a⟩ ⟨b⟩)).value⟩))
  | .SLT () =>
    (do
      (charge (G_verylow).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_slt ⟨a⟩ ⟨b⟩)).value⟩))
  | .SGT () =>
    (do
      (charge (G_verylow).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_sgt ⟨a⟩ ⟨b⟩)).value⟩))
  | .EQ () =>
    (do
      (charge (G_verylow).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_eq ⟨a⟩ ⟨b⟩)).value⟩))
  | .ISZERO () =>
    (do
      (charge (G_verylow).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_iszero ⟨a⟩)).value⟩))
  | .AND () =>
    (do
      (charge (G_verylow).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_and ⟨a⟩ ⟨b⟩)).value⟩))
  | .OR () =>
    (do
      (charge (G_verylow).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_or ⟨a⟩ ⟨b⟩)).value⟩))
  | .XOR () =>
    (do
      (charge (G_verylow).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let b ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_xor ⟨a⟩ ⟨b⟩)).value⟩))
  | .NOT () =>
    (do
      (charge (G_verylow).value)
      let a ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_not ⟨a⟩)).value⟩))
  | .BYTE () =>
    (do
      (charge (G_verylow).value)
      let i ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let x ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_byte ⟨i⟩ ⟨x⟩)).value⟩))
  | .SHL () =>
    (do
      (charge (G_verylow).value)
      let s ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let v ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_shl ⟨s⟩ ⟨v⟩)).value⟩))
  | .SHR () =>
    (do
      (charge (G_verylow).value)
      let s ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let v ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_shr ⟨s⟩ ⟨v⟩)).value⟩))
  | .SAR () =>
    (do
      (charge (G_verylow).value)
      let s ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let v ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_sar ⟨s⟩ ⟨v⟩)).value⟩))
  | .CLZ () =>
    (do
      (charge (G_low).value)
      let x ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((alu_clz ⟨x⟩)).value⟩))
  | _ => assert false "sail/evm/execute.sail:255.21-255.22"

/-- Executes hashing and transaction/environment data access opcodes. -/
def execute_environment (op : ast) : SailM Unit := do
  match op with
  | .KECCAK256 () =>
    (do
      let offset_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let length_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (charge_keccak_gas ⟨length_word⟩)
      let ⟨_, ⟨_, range⟩⟩ ← do (charge_memory_range ⟨offset_word⟩ length_word)
      if ((← (is_running ())) : Bool)
      then (push_word ⟨((← (mem_keccak ⟨_, ⟨_, range⟩⟩))).value⟩)
      else (pure ()))
  | .ADDRESS () =>
    (do
      (charge (G_base).value)
      (push_word ⟨((address_to_word (← (self_addr ())))).value⟩))
  | .ORIGIN () =>
    (do
      (charge (G_base).value)
      (push_word ⟨((← (k_env F_Origin))).value⟩))
  | .CALLER () =>
    (do
      (charge (G_base).value)
      (push_word ⟨((address_to_word (← readReg message).caller)).value⟩))
  | .CALLVALUE () =>
    (do
      (charge (G_base).value)
      (push_word ⟨((← readReg message).value).value⟩))
  | .GASPRICE () =>
    (do
      (charge (G_base).value)
      (push_word ⟨((← (k_env F_GasPrice))).value⟩))
  | .CALLDATASIZE () =>
    (do
      (charge (G_base).value)
      let ⟨_, ⟨_, input⟩⟩ ← do pure ((← readReg calldata))
      (push_word ⟨((← (word_of_source_byte_count input.len))).value⟩))
  | .CALLDATALOAD () =>
    (do
      (charge (G_verylow).value)
      let offset_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      if ((← (is_running ())) : Bool)
      then
        (push_word
          ⟨((← (slice_load_word_offset (← readReg calldata) ⟨offset_word⟩))).value⟩)
      else (pure ()))
  | .CALLDATACOPY () =>
    (do
      (charge (G_verylow).value)
      let destination_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let source_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let length_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (charge_copy_gas ⟨length_word⟩)
      let ⟨_, ⟨_, range⟩⟩ ← do (charge_memory_range ⟨destination_word⟩ length_word)
      if ((← (is_running ())) : Bool)
      then (slice_copy_word_offset (← readReg calldata) range.off ⟨source_word⟩ range.len)
      else (pure ()))
  | .CODESIZE () =>
    (do
      (charge (G_base).value)
      (push_word ⟨((← (word_of_source_byte_count (← (frame_code_len ()))))).value⟩))
  | .CODECOPY () =>
    (do
      (charge (G_verylow).value)
      let destination_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let source_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let length_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (charge_copy_gas ⟨length_word⟩)
      let ⟨_, ⟨_, range⟩⟩ ← do (charge_memory_range ⟨destination_word⟩ length_word)
      if ((← (is_running ())) : Bool)
      then
        (slice_copy_word_offset
          (⟨_, ⟨_, (((← readReg frame_code).bytes).2).2⟩⟩ : (Sigma fun (k_off : Nat) =>
          (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) range.off ⟨source_word⟩
          range.len)
      else (pure ()))
  | .BALANCE () =>
    (do
      let a ← do (pure (word_to_address ⟨((← (pop ()))).value⟩))
      let warm ← do (k_access_account a)
      (charge ((← (account_cost warm))).value)
      if ((← (is_running ())) : Bool)
      then (push_word ⟨((← (k_get_balance a))).value⟩)
      else (pure ()))
  | .SELFBALANCE () =>
    (do
      (charge (G_low).value)
      (push_word ⟨((← (k_get_balance (← (self_addr ()))))).value⟩))
  | .EXTCODESIZE () =>
    (do
      let a ← do (pure (word_to_address ⟨((← (pop ()))).value⟩))
      let warm ← do (k_access_account a)
      (charge (((← (account_cost warm))).value + ((← (external_code_read_cost ()))).value))
      if ((← (is_running ())) : Bool)
      then (push_word ⟨((← (word_of_source_byte_count (← (k_get_code_size a))))).value⟩)
      else (pure ()))
  | .EXTCODECOPY () =>
    (do
      let a ← do (pure (word_to_address ⟨((← (pop ()))).value⟩))
      let destination_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let source_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let length_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let warm ← do (k_access_account a)
      (charge (((← (account_cost warm))).value + ((← (external_code_read_cost ()))).value))
      (charge_copy_gas ⟨length_word⟩)
      let ⟨_, ⟨_, range⟩⟩ ← do (charge_memory_range ⟨destination_word⟩ length_word)
      if ((← (is_running ())) : Bool)
      then (k_code_copy a range.off ⟨source_word⟩ range.len)
      else (pure ()))
  | .EXTCODEHASH () =>
    (do
      let a ← do (pure (word_to_address ⟨((← (pop ()))).value⟩))
      let warm ← do (k_access_account a)
      (charge ((← (account_cost warm))).value)
      if ((← (is_running ())) : Bool)
      then (push_word ⟨((hash_to_word (← (k_get_codehash a)))).value⟩)
      else (pure ()))
  | .RETURNDATASIZE () =>
    (do
      (charge (G_base).value)
      (push_word ⟨((← (word_of_source_byte_count (← (returndata_size ()))))).value⟩))
  | .RETURNDATACOPY () =>
    (do
      (charge (G_verylow).value)
      let destination_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let source_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let length_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (charge_copy_gas ⟨length_word⟩)
      let ⟨_, ⟨_, range⟩⟩ ← do (charge_memory_range ⟨destination_word⟩ length_word)
      if ((← (is_running ())) : Bool)
      then (returndata_copy_words range.off ⟨source_word⟩ ⟨length_word⟩)
      else (pure ()))
  | _ => assert false "sail/evm/execute.sail:383.21-383.22"

/-- Executes block-context opcodes. -/
def execute_block (op : ast) : SailM Unit := do
  match op with
  | .BLOCKHASH () =>
    (do
      (charge 20)
      (push_word ⟨((hash_to_word (← (k_blockhash ⟨((← (pop ()))).value⟩)))).value⟩))
  | .COINBASE () =>
    (do
      (charge (G_base).value)
      (push_word ⟨((← (k_env F_Coinbase))).value⟩))
  | .TIMESTAMP () =>
    (do
      (charge (G_base).value)
      (push_word ⟨((← (k_env F_Timestamp))).value⟩))
  | .NUMBER () =>
    (do
      (charge (G_base).value)
      (push_word ⟨((← (k_env F_Number))).value⟩))
  | .SLOTNUM () =>
    (do
      (charge (G_base).value)
      (push_word ⟨((← (k_env F_SlotNumber))).value⟩))
  | .PREVRANDAO () =>
    (do
      (charge (G_base).value)
      (push_word ⟨((← (k_env F_PrevRandao))).value⟩))
  | .GASLIMIT () =>
    (do
      (charge (G_base).value)
      (push_word ⟨((← (k_env F_GasLimit))).value⟩))
  | .CHAINID () =>
    (do
      (charge (G_base).value)
      (push_word ⟨((← (k_env F_ChainId))).value⟩))
  | .BASEFEE () =>
    (do
      (charge (G_base).value)
      (push_word ⟨((← (k_env F_BaseFee))).value⟩))
  | .BLOBBASEFEE () =>
    (do
      (charge (G_base).value)
      (push_word
        ⟨((← (blob_base_fee ⟨((← readReg k_header).excess_blob_gas).value⟩))).value⟩))
  | .BLOBHASH () =>
    (do
      (charge (G_verylow).value)
      (push_word ⟨((← (k_blobhash ⟨((← (pop ()))).value⟩))).value⟩))
  | _ => assert false "sail/evm/execute.sail:432.21-432.22"

/-- Executes stack and linear-memory opcodes. -/
def execute_memory (op : ast) : SailM Unit := do
  match op with
  | .POP () =>
    (do
      (charge (G_base).value)
      let _ ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (pure ()))
  | .MLOAD () =>
    (do
      (charge (G_verylow).value)
      let offset_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let ⟨_, ⟨_, range⟩⟩ ← do (charge_memory_range ⟨offset_word⟩ ((U256 32)).value)
      if ((← (is_running ())) : Bool)
      then (push_word ⟨((← (mem_load range.off))).value⟩)
      else (pure ()))
  | .MSTORE () =>
    (do
      (charge (G_verylow).value)
      let offset_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let v ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let ⟨_, ⟨_, range⟩⟩ ← do (charge_memory_range ⟨offset_word⟩ ((U256 32)).value)
      if ((← (is_running ())) : Bool)
      then (mem_store range.off ⟨v⟩)
      else (pure ()))
  | .MSTORE8 () =>
    (do
      (charge (G_verylow).value)
      let offset_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let v ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let ⟨_, ⟨_, range⟩⟩ ← do (charge_memory_range ⟨offset_word⟩ (WORD_ONE).value)
      if ((← (is_running ())) : Bool)
      then (mem_store_byte range.off ⟨v⟩)
      else (pure ()))
  | .MSIZE () =>
    (do
      (charge (G_base).value)
      (push_word
        ⟨((← (word_of_nat_byte_count
          ((memory_word_count (← (evm_memory_high_water ()))) *i 32)))).value⟩))
  | .MCOPY () =>
    (do
      (charge (G_verylow).value)
      let destination_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let source_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let length_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (charge_copy_gas ⟨length_word⟩)
      if ((← (is_running ())) : Bool)
      then
        (do
          let available ← do pure ((← readReg gas_remaining))
          let expansion ← do
            (memory_pair_expansion ⟨destination_word⟩ ⟨length_word⟩ ⟨source_word⟩
              ⟨length_word⟩ available)
          (charge expansion.cost)
          if ((← (is_running ())) : Bool)
          then
            (do
              let (destination, source) ← do (apply_memory_pair_expansion expansion)
              (mem_mcopy ((destination).2).2.off ((source).2).2.off ((destination).2).2.len))
          else (pure ()))
      else (pure ()))
  | _ => assert false "sail/evm/execute.sail:488.21-488.22"

/-- Executes persistent and transient storage opcodes. -/
def execute_storage (op : ast) : SailM Unit := do
  match op with
  | .SLOAD () =>
    (do
      let s ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let warm ← do (k_slot_is_warm (← (self_addr ())) ⟨s⟩)
      (charge ((← (sload_cost warm))).value)
      if ((← (is_running ())) : Bool)
      then (push_word ⟨((← (k_sload (← (self_addr ())) ⟨s⟩)).curr).value⟩)
      else (pure ()))
  | .SSTORE () =>
    (do
      if ((← (guard_static ())) : Bool)
      then (pure ())
      else
        (do
          if (((fork_lt (← readReg k_fork) Amsterdam) && (← do
                 (pure ((← readReg gas_remaining) ≤b G_callstipend)))) : Bool)
          then (exc_halt OutOfGas)
          else
            (do
              let s ← do
                (do
                    let publicResult ← (pop ())
                    pure ((publicResult).value))
              let v ← do
                (do
                    let publicResult ← (pop ())
                    pure ((publicResult).value))
              if ((← (is_running ())) : Bool)
              then
                (do
                  let warm ← do (k_slot_is_warm (← (self_addr ())) ⟨s⟩)
                  let cold := (! warm)
                  if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
                  then
                    (do
                      let access_cost := ((amsterdam_storage_access_cost cold)).value
                      let sentry_cost :=
                        if ((access_cost <b G_sstore_sentry) : Bool)
                        then G_sstore_sentry
                        else access_cost
                      (check_execution_gas sentry_cost))
                  else (pure ())
                  if ((! (← (is_running ()))) : Bool)
                  then (pure ())
                  else
                    (do
                      let entry ← do (k_sload (← (self_addr ())) ⟨s⟩)
                      let costs ← do
                        (sstore_costs ⟨(entry.orig).value⟩ ⟨(entry.curr).value⟩ ⟨v⟩ cold)
                      if (((costs.state_credit).value != 0) : Bool)
                      then (credit_state_gas_refund ⟨(costs.state_credit).value⟩)
                      else (pure ())
                      (charge costs.execution)
                      (charge_state_gas costs.state_charge)
                      if ((! ((costs.refund).value == (GAS_REFUND_ZERO).value)) : Bool)
                      then (record_refund (costs.refund).value)
                      else (pure ())
                      if (((← (is_running ())) && (((entry.curr).value != v) : Bool)) : Bool)
                      then
                        (k_sstore (← (self_addr ())) ⟨s⟩
                          { curr := ⟨v⟩,
                            orig := ⟨(entry.orig).value⟩ })
                      else (pure ())))
              else (pure ()))))
  | .TLOAD () =>
    (do
      (charge (G_warm_access).value)
      let s ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (push_word ⟨((← (k_tload (← (self_addr ())) ⟨s⟩))).value⟩))
  | .TSTORE () =>
    (do
      if ((← (guard_static ())) : Bool)
      then (pure ())
      else
        (do
          (charge (G_warm_access).value)
          let s ← do
            (do
                let publicResult ← (pop ())
                pure ((publicResult).value))
          let v ← do
            (do
                let publicResult ← (pop ())
                pure ((publicResult).value))
          (k_tstore (← (self_addr ())) ⟨s⟩ ⟨v⟩)))
  | _ => assert false "sail/evm/execute.sail:556.21-556.22"

/-- Executes control-flow and stack-shuffling opcodes. -/
def execute_control (op : ast) : SailM Unit := do
  match op with
  | .JUMP () =>
    (do
      (charge (G_mid).value)
      let dest ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      (do_jump ⟨dest⟩))
  | .JUMPI () =>
    (do
      (charge (G_high).value)
      let dest ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let cond ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      if ((word_is_zero cond) : Bool)
      then (pure ())
      else (do_jump ⟨dest⟩))
  | .PC () =>
    (do
      (charge (G_base).value)
      (push_word
        ⟨((alu_sub ⟨((← (word_of_source_byte_count (← readReg pc)))).value⟩
          ⟨(WORD_ONE).value⟩)).value⟩))
  | .GAS () =>
    (do
      (charge (G_base).value)
      (push_gas (← readReg gas_remaining)))
  | .JUMPDEST () => (charge (G_jumpdest).value)
  | .PUSH (n, v) =>
    let n := (n).value
    let v := (v).value
    (do
      if ((n == 0) : Bool)
      then (charge (G_base).value)
      else (charge (G_verylow).value)
      (push_word ⟨v⟩))
  | .DUP n =>
    let n := (n).value
    (do
      (charge (G_verylow).value)
      if ((((← (stack_height ()))).value <b n) : Bool)
      then (exc_halt StackUnderflow)
      else (push_word ⟨((← (peek ⟨(n - 1)⟩))).value⟩))
  | .SWAP n =>
    let n := (n).value
    (do
      (charge (G_verylow).value)
      if ((((← (stack_height ()))).value <b (n + 1)) : Bool)
      then (exc_halt StackUnderflow)
      else
        (do
          let top ← do
            (do
                let publicResult ← (peek ⟨0⟩)
                pure ((publicResult).value))
          let other ← do
            (do
                let publicResult ← (peek ⟨n⟩)
                pure ((publicResult).value))
          (stack_set ⟨0⟩ ⟨other⟩)
          (stack_set ⟨n⟩ ⟨top⟩)))
  | .DUPN immediate =>
    (do
      (charge (G_verylow).value)
      if ((← (is_running ())) : Bool)
      then
        (do
          if ((! (deep_stack_immediate_valid immediate)) : Bool)
          then (exc_halt InvalidOpcode)
          else
            (do
              let n ← do
                (do
                    let publicResult ← (decode_single_stack_index immediate)
                    pure ((publicResult).value))
              if ((((← (stack_height ()))).value <b n) : Bool)
              then (exc_halt StackUnderflow)
              else (push_word ⟨((← (peek ⟨(n - 1)⟩))).value⟩)))
      else (pure ()))
  | .SWAPN immediate =>
    (do
      (charge (G_verylow).value)
      if ((← (is_running ())) : Bool)
      then
        (do
          if ((! (deep_stack_immediate_valid immediate)) : Bool)
          then (exc_halt InvalidOpcode)
          else
            (do
              let n ← do
                (do
                    let publicResult ← (decode_single_stack_index immediate)
                    pure ((publicResult).value))
              if ((((← (stack_height ()))).value <b (n + 1)) : Bool)
              then (exc_halt StackUnderflow)
              else
                (do
                  let top ← do
                    (do
                        let publicResult ← (peek ⟨0⟩)
                        pure ((publicResult).value))
                  let other ← do
                    (do
                        let publicResult ← (peek ⟨n⟩)
                        pure ((publicResult).value))
                  (stack_set ⟨0⟩ ⟨other⟩)
                  (stack_set ⟨n⟩ ⟨top⟩))))
      else (pure ()))
  | .EXCHANGE immediate =>
    (do
      (charge (G_verylow).value)
      if ((← (is_running ())) : Bool)
      then
        (do
          if ((! (exchange_immediate_valid immediate)) : Bool)
          then (exc_halt InvalidOpcode)
          else
            (do
              let (n, m) ← do
                (do
                    let publicResult ← (decode_exchange_stack_indices immediate)
                    pure (((fun (semanticValue0, semanticValue1) => ((semanticValue0).value, (semanticValue1).value)) (publicResult))))
              if ((((← (stack_height ()))).value <b (m + 1)) : Bool)
              then (exc_halt StackUnderflow)
              else
                (do
                  let first ← do
                    (do
                        let publicResult ← (peek ⟨n⟩)
                        pure ((publicResult).value))
                  let second ← do
                    (do
                        let publicResult ← (peek ⟨m⟩)
                        pure ((publicResult).value))
                  (stack_set ⟨n⟩ ⟨second⟩)
                  (stack_set ⟨m⟩ ⟨first⟩))))
      else (pure ()))
  | _ => assert false "sail/evm/execute.sail:663.21-663.22"

/-- Executes the LOG family. -/
def execute_log (op : ast) : SailM Unit := do
  match op with
  | .LOG n =>
    let n := (n).value
    (do
      if ((← (guard_static ())) : Bool)
      then (pure ())
      else
        (do
          let offset_word ← do
            (do
                let publicResult ← (pop ())
                pure ((publicResult).value))
          let length_word ← do
            (do
                let publicResult ← (pop ())
                pure ((publicResult).value))
          let topics ← do
            (do
                let publicResult ← (pop_log_topics ⟨n⟩)
                pure ((List.map (fun semanticValue => (semanticValue).value) (publicResult))))
          (charge_log_gas ⟨n⟩ ⟨length_word⟩)
          let ⟨_, ⟨_, range⟩⟩ ← do (charge_memory_range ⟨offset_word⟩ length_word)
          if ((← (is_running ())) : Bool)
          then
            (k_log (← (self_addr ()))
              (List.map (fun semanticValue => ⟨semanticValue⟩) (topics))
              (BytesSlice
                (⟨_, ⟨_, (((← (memory_byte_slice range.off range.len))).2).2⟩⟩ : (Sigma
                fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))))
          else (pure ())))
  | _ => assert false "sail/evm/execute.sail:682.21-682.22"

/-- Executes frame-halting opcodes other than call and create. -/
def execute_halt (op : ast) : SailM Unit := do
  match op with
  | .STOP () => writeReg frame_status (Halted (HaltStop ()))
  | .RETURN () =>
    (do
      let offset_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let length_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let ⟨_, ⟨_, range⟩⟩ ← do (charge_memory_range ⟨offset_word⟩ length_word)
      if ((← (is_running ())) : Bool)
      then
        writeReg frame_status (Halted
          (HaltReturn
            (← (freeze_output
                (⟨_, ⟨_, (((← (memory_byte_slice range.off range.len))).2).2⟩⟩ : (Sigma
                fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))))))
      else (pure ()))
  | .REVERT () =>
    (do
      let offset_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let length_word ← do
        (do
            let publicResult ← (pop ())
            pure ((publicResult).value))
      let ⟨_, ⟨_, range⟩⟩ ← do (charge_memory_range ⟨offset_word⟩ length_word)
      if ((← (is_running ())) : Bool)
      then
        (do
          (refill_frame_state_gas ())
          writeReg frame_status (Halted
            (HaltRevert
              (← (freeze_output
                  (⟨_, ⟨_, (((← (memory_byte_slice range.off range.len))).2).2⟩⟩ : (Sigma
                  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))))))
      else (pure ()))
  | .INVALID () =>
    (do
      writeReg gas_remaining GAS_ZERO
      (exc_halt InvalidOpcode))
  | .SELFDESTRUCT () =>
    (do
      if ((← (guard_static ())) : Bool)
      then (pure ())
      else
        (do
          let beneficiary ← do (pure (word_to_address ⟨((← (pop ()))).value⟩))
          if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
          then
            (do
              let warm ← do (k_access_account beneficiary)
              let access_cost :=
                ((0 + (G_selfdestruct).value) + (if (warm : Bool)
                  then (G_zero).value
                  else (G_amsterdam_cold_account_access).value))
              (check_execution_gas access_cost)
              if ((← (is_running ())) : Bool)
              then
                (do
                  let bal ← do
                    (do
                        let publicResult ← (k_get_balance (← (self_addr ())))
                        pure ((publicResult).value))
                  let creates_account ← do
                    (pure ((word_nonzero bal) && (← (k_account_is_empty beneficiary))))
                  let execution_cost :=
                    if (creates_account : Bool)
                    then (access_cost + (G_amsterdam_account_write).value)
                    else access_cost
                  (charge execution_cost)
                  if (((← (is_running ())) && creates_account) : Bool)
                  then (charge_state_gas (G_amsterdam_state_new_account).value)
                  else (pure ())
                  if ((← (is_running ())) : Bool)
                  then
                    (do
                      (k_transfer (← (self_addr ())) beneficiary ⟨bal⟩)
                      if ((← (k_was_created (← (self_addr ())))) : Bool)
                      then (k_selfdestruct (← (self_addr ())))
                      else (pure ())
                      writeReg frame_status (Halted (HaltSelfDestruct ())))
                  else (pure ()))
              else (pure ()))
          else
            (do
              let bal ← do
                (do
                    let publicResult ← (k_get_balance (← (self_addr ())))
                    pure ((publicResult).value))
              let warm ← do (k_access_account beneficiary)
              (charge (G_selfdestruct).value)
              if ((! warm) : Bool)
              then (charge (G_cold_account).value)
              else (pure ())
              if (((← (is_running ())) && ((word_nonzero bal) && (← (k_account_is_empty
                         beneficiary)))) : Bool)
              then (charge (G_newaccount).value)
              else (pure ())
              if ((← (is_running ())) : Bool)
              then
                (do
                  let first_selfdestruct ← do
                    (pure (! (← (k_is_selfdestructed (← (self_addr ()))))))
                  if (((fork_lt (← readReg k_fork) London) && first_selfdestruct) : Bool)
                  then (record_refund (R_selfdestruct_pre_london).value)
                  else (pure ())
                  (k_transfer (← (self_addr ())) beneficiary ⟨bal⟩)
                  if ((fork_lt (← readReg k_fork) Cancun) : Bool)
                  then
                    (do
                      (k_zero_balance (← (self_addr ())))
                      (k_selfdestruct (← (self_addr ()))))
                  else
                    (do
                      if ((← (k_was_created (← (self_addr ())))) : Bool)
                      then
                        (do
                          (k_zero_balance (← (self_addr ())))
                          (k_selfdestruct (← (self_addr ()))))
                      else (pure ()))
                  writeReg frame_status (Halted (HaltSelfDestruct ())))
              else (pure ()))))
  | _ => assert false "sail/evm/execute.sail:771.21-771.22"

/-- Executes every opcode except the call and create families. The public
[execute][] dispatcher handles those six frame-entering cases directly
and only calls this function for the remaining constructors. -/
def execute_opcode (op : ast) : SailM Unit := do
  match op with
  | .STOP () => (execute_halt op)
  | .ADD () => (execute_arithmetic op)
  | .MUL () => (execute_arithmetic op)
  | .SUB () => (execute_arithmetic op)
  | .DIV () => (execute_arithmetic op)
  | .SDIV () => (execute_arithmetic op)
  | .MOD () => (execute_arithmetic op)
  | .SMOD () => (execute_arithmetic op)
  | .ADDMOD () => (execute_arithmetic op)
  | .MULMOD () => (execute_arithmetic op)
  | .EXP () => (execute_arithmetic op)
  | .SIGNEXTEND () => (execute_arithmetic op)
  | .LT () => (execute_arithmetic op)
  | .GT () => (execute_arithmetic op)
  | .SLT () => (execute_arithmetic op)
  | .SGT () => (execute_arithmetic op)
  | .EQ () => (execute_arithmetic op)
  | .ISZERO () => (execute_arithmetic op)
  | .AND () => (execute_arithmetic op)
  | .OR () => (execute_arithmetic op)
  | .XOR () => (execute_arithmetic op)
  | .NOT () => (execute_arithmetic op)
  | .BYTE () => (execute_arithmetic op)
  | .SHL () => (execute_arithmetic op)
  | .SHR () => (execute_arithmetic op)
  | .SAR () => (execute_arithmetic op)
  | .CLZ () => (execute_arithmetic op)
  | .KECCAK256 () => (execute_environment op)
  | .ADDRESS () => (execute_environment op)
  | .ORIGIN () => (execute_environment op)
  | .CALLER () => (execute_environment op)
  | .CALLVALUE () => (execute_environment op)
  | .GASPRICE () => (execute_environment op)
  | .CALLDATASIZE () => (execute_environment op)
  | .CALLDATALOAD () => (execute_environment op)
  | .CALLDATACOPY () => (execute_environment op)
  | .CODESIZE () => (execute_environment op)
  | .CODECOPY () => (execute_environment op)
  | .BALANCE () => (execute_environment op)
  | .SELFBALANCE () => (execute_environment op)
  | .EXTCODESIZE () => (execute_environment op)
  | .EXTCODECOPY () => (execute_environment op)
  | .EXTCODEHASH () => (execute_environment op)
  | .RETURNDATASIZE () => (execute_environment op)
  | .RETURNDATACOPY () => (execute_environment op)
  | .BLOCKHASH () => (execute_block op)
  | .COINBASE () => (execute_block op)
  | .TIMESTAMP () => (execute_block op)
  | .NUMBER () => (execute_block op)
  | .SLOTNUM () => (execute_block op)
  | .PREVRANDAO () => (execute_block op)
  | .GASLIMIT () => (execute_block op)
  | .CHAINID () => (execute_block op)
  | .BASEFEE () => (execute_block op)
  | .BLOBBASEFEE () => (execute_block op)
  | .BLOBHASH () => (execute_block op)
  | .POP () => (execute_memory op)
  | .MLOAD () => (execute_memory op)
  | .MSTORE () => (execute_memory op)
  | .MSTORE8 () => (execute_memory op)
  | .MSIZE () => (execute_memory op)
  | .MCOPY () => (execute_memory op)
  | .SLOAD () => (execute_storage op)
  | .SSTORE () => (execute_storage op)
  | .TLOAD () => (execute_storage op)
  | .TSTORE () => (execute_storage op)
  | .JUMP () => (execute_control op)
  | .JUMPI () => (execute_control op)
  | .PC () => (execute_control op)
  | .GAS () => (execute_control op)
  | .JUMPDEST () => (execute_control op)
  | .PUSH (_, _) => (execute_control op)
  | .DUP _ => (execute_control op)
  | .SWAP _ => (execute_control op)
  | .DUPN _ => (execute_control op)
  | .SWAPN _ => (execute_control op)
  | .EXCHANGE _ => (execute_control op)
  | .LOG _ => (execute_log op)
  | .CREATE () => assert false "sail/evm/execute.sail:867.28-867.29"
  | .CREATE2 () => assert false "sail/evm/execute.sail:868.29-868.30"
  | .CALL () => assert false "sail/evm/execute.sail:869.26-869.27"
  | .CALLCODE () => assert false "sail/evm/execute.sail:870.30-870.31"
  | .DELEGATECALL () => assert false "sail/evm/execute.sail:871.34-871.35"
  | .STATICCALL () => assert false "sail/evm/execute.sail:872.32-872.33"
  | .RETURN () => (execute_halt op)
  | .REVERT () => (execute_halt op)
  | .INVALID () => (execute_halt op)
  | .SELFDESTRUCT () => (execute_halt op)

/-- Whether a call kind inherits the parent caller and call value. -/
def call_is_delegate (kind : CallKind) : Bool :=
  match kind with
  | .DelegateCall => true
  | _ => false

/-- Whether a call kind introduces a read-only execution context. -/
def call_is_static (kind : CallKind) : Bool :=
  match kind with
  | .StaticCall => true
  | _ => false

/-- Whether a call kind consumes an explicit value stack operand. -/
def call_takes_value (kind : CallKind) : Bool :=
  match kind with
  | .Call => true
  | .CallCode => true
  | _ => false

/-- Whether a call kind transfers value between distinct accounts. -/
def call_transfers_value (kind : CallKind) : Bool :=
  match kind with
  | .Call => true
  | _ => false

/-- Whether a call kind executes with the target as its storage address. -/
def call_uses_target_address (kind : CallKind) : Bool :=
  match kind with
  | .Call => true
  | .StaticCall => true
  | _ => false

/-- Selects the code a frame actually executes (EIP-7702). A delegated
account runs the code at its delegation target, following exactly one
hop; a delegation whose target is a precompile (or has no code)
executes as empty code. An undelegated account runs its own code. -/
/- Type quantifiers: k_ex411018_ : Bool -/
def executable_code (target : address) (dele : Bool) (dtgt : address) : SailM Code := do
  if (dele : Bool)
  then
    (do
      let delegate_code ← do (code_db_resolve (← (k_code_key dtgt)))
      if ((((← (precompile_number dtgt))).value != 0) : Bool)
      then (pure EMPTY_CODE)
      else (pure delegate_code))
  else (code_db_resolve (← (k_code_key target)))

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
Pushes 1 on success, 0 on failure. -/
def run_call (kind : CallKind) : SailM Unit := do
  let caller ← do (self_addr ())
  let gas_request ← do
    (do
        let publicResult ← (pop ())
        pure ((publicResult).value))
  let target_word ← do
    (do
        let publicResult ← (pop ())
        pure ((publicResult).value))
  let target := (word_to_address ⟨target_word⟩)
  let value ← do
    if ((call_takes_value kind) : Bool)
    then
      (do
          let publicResult ← (pop ())
          pure ((publicResult).value))
    else (pure (WORD_ZERO).value)
  let args_off_word ← do
    (do
        let publicResult ← (pop ())
        pure ((publicResult).value))
  let args_len_word ← do
    (do
        let publicResult ← (pop ())
        pure ((publicResult).value))
  let ret_off_word ← do
    (do
        let publicResult ← (pop ())
        pure ((publicResult).value))
  let ret_len_word ← do
    (do
        let publicResult ← (pop ())
        pure ((publicResult).value))
  if ((! (← (is_running ()))) : Bool)
  then (pure ())
  else
    (do
      if (((call_transfers_value kind) && ((word_nonzero value) && (← readReg message).is_static)) : Bool)
      then
        (do
          (exc_halt WriteProtection))
      else
        (do
          let warm ← do (k_access_account target)
          let target_cost ← (( do
            (do
                let publicResult ← (account_cost warm)
                pure ((publicResult).value)) ) : SailM Nat )
          let transfer_cost ← (( do
            if ((word_nonzero value) : Bool)
            then
              (do
                  let publicResult ← (call_value_cost ())
                  pure ((publicResult).value))
            else (pure (GAS_CONSTANT_ZERO).value) ) : SailM Nat )
          let available ← do pure ((← readReg gas_remaining))
          let memory ← do
            (memory_pair_expansion ⟨args_off_word⟩ ⟨args_len_word⟩ ⟨ret_off_word⟩
              ⟨ret_len_word⟩ available)
          if ((! (← (is_running ()))) : Bool)
          then (pure ())
          else
            (do
              let memory_cost := memory.cost
              let static_base : Nat := (target_cost + transfer_cost)
              let before_static ← do pure ((← readReg gas_remaining))
              if ((before_static <b static_base) : Bool)
              then
                (do
                  (exc_halt OutOfGas))
              else
                (do
                  let after_static_base : Nat := (before_static - static_base)
                  if ((after_static_base <b memory_cost) : Bool)
                  then
                    (do
                      (exc_halt OutOfGas))
                  else
                    (do
                      let (tg_deleg, tg_target) ← do (k_deleg_target target)
                      let delegation_cost ← (( do
                        if (tg_deleg : Bool)
                        then
                          (do
                            let dw ← do (k_access_account tg_target)
                            (do
                                let publicResult ← (account_cost dw)
                                pure ((publicResult).value)))
                        else (pure (GAS_CONSTANT_ZERO).value) ) : SailM Nat )
                      let new_account_charged ← do
                        (pure ((fork_gteq (← readReg k_fork) Amsterdam) && ((word_nonzero value) && ((call_transfers_value
                                  kind) && (← (k_account_is_empty target))))))
                      let create_cost ← (( do
                        if (((fork_lt (← readReg k_fork) Amsterdam) && ((word_nonzero value) && ((call_transfers_value
                                   kind) && (← (k_account_is_empty target))))) : Bool)
                        then (pure (G_newaccount).value)
                        else (pure (GAS_CONSTANT_ZERO).value) ) : SailM Nat )
                      let additional_cost : Nat := (delegation_cost + create_cost)
                      let before_required ← do pure ((← readReg gas_remaining))
                      if ((before_required <b static_base) : Bool)
                      then
                        (do
                          (exc_halt OutOfGas))
                      else
                        (do
                          let after_required_base : Nat := (before_required - static_base)
                          if ((after_required_base <b memory_cost) : Bool)
                          then
                            (do
                              (exc_halt OutOfGas))
                          else
                            (do
                              let after_static_cost : Nat := (after_required_base - memory_cost)
                              if ((after_static_cost <b additional_cost) : Bool)
                              then
                                (do
                                  (exc_halt OutOfGas))
                              else
                                (do
                                  let after_required : Nat := (after_static_cost - additional_cost)
                                  let required ← (( do
                                    if ((after_required ≤b before_required) : Bool)
                                    then (pure (before_required - after_required))
                                    else sailThrow ((InvalidBlock ExecutionInvalid)) ) : SailM Nat )
                                  let stipend : Nat :=
                                    if ((word_nonzero value) : Bool)
                                    then G_callstipend
                                    else GAS_ZERO
                                  let base_child : Nat := GAS_ZERO
                                  let base_child ← (( do
                                    if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
                                    then
                                      (do
                                        (charge required)
                                        if (new_account_charged : Bool)
                                        then
                                          (charge_state_gas (G_amsterdam_state_new_account).value)
                                        else (pure ())
                                        if ((← (is_running ())) : Bool)
                                        then
                                          (do
                                            let base_child ←
                                              (call_gas_cap_word (← readReg gas_remaining)
                                                ⟨gas_request⟩)
                                            (charge base_child)
                                            (pure base_child))
                                        else (pure base_child))
                                    else
                                      (do
                                        let avail ← do pure ((← readReg gas_remaining))
                                        let base_child ←
                                          if ((avail <b required) : Bool)
                                          then (pure GAS_ZERO)
                                          else
                                            (do
                                              let available_after_cost ← do
                                                (gas_sub_or_oog avail required)
                                              (call_gas_cap_word available_after_cost
                                                ⟨gas_request⟩))
                                        (charge required)
                                        if ((← (is_running ())) : Bool)
                                        then (charge base_child)
                                        else (pure ())
                                        (pure base_child)) ) : SailM Nat )
                                  if ((! (← (is_running ()))) : Bool)
                                  then (pure ())
                                  else
                                    (do
                                      if (tg_deleg : Bool)
                                      then
                                        (do
                                          let _ ← do
                                            (code_db_resolve (← (k_code_key tg_target)))
                                          let _ ← do (k_aload tg_target)
                                          (pure ()))
                                      else (pure ())
                                      let (args, ret) ← do (apply_memory_pair_expansion memory)
                                      let child_gas : Nat := (conserved_gas_add base_child stipend)
                                      let _ ← do (k_aload target)
                                      if (((((← readReg call_depth)).value ≥b (DEPTH_LIMIT).value) || ((call_takes_value
                                               kind) && ((word_nonzero value) && (! (word_ule value
                                                   ((← (k_get_balance caller))).value))))) : Bool)
                                      then
                                        (do
                                          (returndata_clear ())
                                          (refund_gas child_gas)
                                          if (new_account_charged : Bool)
                                          then
                                            (credit_state_gas_refund
                                              ⟨(G_amsterdam_state_new_account).value⟩)
                                          else (pure ())
                                          (push_word ⟨(WORD_ZERO).value⟩))
                                      else
                                        (do
                                          let selected_precompile ← do
                                            (do
                                                let publicResult ← (precompile_number target)
                                                pure ((publicResult).value))
                                          if ((selected_precompile != 0) : Bool)
                                          then
                                            (do
                                              let number : Nat := selected_precompile
                                              let ⟨_, ⟨_, input⟩⟩ ← do
                                                (memory_byte_slice ((args).2).2.off ((args).2).2.len)
                                              match (← (precompile_gas ⟨number⟩
                                                  ⟨_, ⟨_, input⟩⟩ child_gas)) with
                                              | .some used =>
                                                (do
                                                  let result ← do
                                                    (run_precompile_slice ⟨number⟩
                                                      ⟨_, ⟨_, input⟩⟩)
                                                  if (result.success : Bool)
                                                  then
                                                    (do
                                                      writeReg returndata result.output
                                                      if (((call_transfers_value kind) && (word_nonzero
                                                             value)) : Bool)
                                                      then (k_transfer caller target ⟨value⟩)
                                                      else (pure ())
                                                      (returndata_copy_prefix ((ret).2).2.off
                                                        ((ret).2).2.len)
                                                      let unused ← do
                                                        (gas_sub_or_oog child_gas used)
                                                      if ((← (is_running ())) : Bool)
                                                      then
                                                        (do
                                                          (refund_gas unused)
                                                          (push_word ⟨(WORD_ONE).value⟩))
                                                      else (pure ()))
                                                  else
                                                    (do
                                                      (returndata_clear ())
                                                      if (new_account_charged : Bool)
                                                      then
                                                        (credit_state_gas_refund
                                                          ⟨(G_amsterdam_state_new_account).value⟩)
                                                      else (pure ())
                                                      (push_word ⟨(WORD_ZERO).value⟩)))
                                              | _ =>
                                                (do
                                                  (returndata_clear ())
                                                  if (new_account_charged : Bool)
                                                  then
                                                    (credit_state_gas_refund
                                                      ⟨(G_amsterdam_state_new_account).value⟩)
                                                  else (pure ())
                                                  (push_word ⟨(WORD_ZERO).value⟩)))
                                          else
                                            (do
                                              let child_code ← do
                                                (executable_code target tg_deleg tg_target)
                                              let child_addr : (Vector (BitVec 8) 20) :=
                                                if ((call_uses_target_address kind) : Bool)
                                                then target
                                                else caller
                                              let child_caller ← (( do
                                                if ((call_is_delegate kind) : Bool)
                                                then (pure (← readReg message).caller)
                                                else (pure caller) ) : SailM (Vector (BitVec 8) 20)
                                                )
                                              let child_value ← (( do
                                                if ((call_is_delegate kind) : Bool)
                                                then (pure ((← readReg message).value).value)
                                                else (pure value) ) : SailM Nat )
                                              let child_static ← (( do
                                                if ((call_is_static kind) : Bool)
                                                then (pure true)
                                                else (pure (← readReg message).is_static) ) :
                                                SailM Bool )
                                              let ⟨_, ⟨_, child_calldata⟩⟩ ← do
                                                if ((← (is_running ())) : Bool)
                                                then
                                                  (do
                                                    (memory_byte_slice ((args).2).2.off
                                                      ((args).2).2.len))
                                                else
                                                  (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun
                                                    (k_syn_off : Nat) =>
                                                    (Sigma fun (k_syn_len : Nat) =>
                                                    (EvmByteSliceFields k_syn_off k_syn_len)))) : (Sigma
                                                    fun (k_syn_off : Nat) =>
                                                    (Sigma fun (k_syn_len : Nat) =>
                                                    (EvmByteSliceFields k_syn_off k_syn_len)))))
                                              let child_state_gas ← do
                                                pure ((← readReg state_gas_remaining))
                                              let checkpoint ← do
                                                (pure { (← (suspend_frame ())) with state_gas_remaining := GAS_ZERO })
                                              (frame_stack_push
                                                (ResumeCall
                                                  { checkpoint := checkpoint,
                                                    return_offset := ((ret).2).2.off,
                                                    return_length := ((ret).2).2.len,
                                                    new_account_charged := new_account_charged }))
                                              if (((call_transfers_value kind) && (word_nonzero
                                                     value)) : Bool)
                                              then (k_transfer caller target ⟨value⟩)
                                              else (pure ())
                                              writeReg message { caller := child_caller,
                                                                 address := child_addr,
                                                                 code_address := target,
                                                                 value := ⟨child_value⟩,
                                                                 state_gas_reservoir := child_state_gas,
                                                                 is_static := child_static,
                                                                 depth := ← do
                                                                     let publicField ← (do
                                                                         let publicResult ← (frame_depth_increment
                                                                         ⟨(checkpoint.call_depth).value⟩)
                                                                         pure ((publicResult).value))
                                                                     pure (⟨publicField⟩) }
                                              (calldata_install ⟨_, ⟨_, child_calldata⟩⟩)
                                              writeReg pc 0
                                              writeReg gas_remaining child_gas
                                              writeReg state_gas_remaining child_state_gas
                                              writeReg state_gas_spilled ⟨(STATE_GAS_SPILL_ZERO).value⟩
                                              writeReg frame_status (Running ())
                                              (returndata_clear ())
                                              writeReg frame_code child_code
                                              writeReg call_depth ⟨((← (frame_depth_increment
                                                ⟨(checkpoint.call_depth).value⟩))).value⟩
                                              writeReg frame_refund ⟨(GAS_REFUND_ZERO).value⟩)))))))))))

/-- `CREATE` (`is2 = false`) and `CREATE2` (`is2 = true`, EIP-1014).
Both deploy a new contract by running the initcode supplied in
memory; the new address derives from `(creator, nonce)` for `CREATE`
or `(creator, salt, keccak256(initcode))` for `CREATE2`. Operand
layout: `value`, `offset`, `length`, `salt` (for `CREATE2`). Pushes
the new address on success, 0 on failure. -/
/- Type quantifiers: k_ex411019_ : Bool -/
def run_create (is2 : Bool) : SailM Unit := do
  let creator ← do (self_addr ())
  let value ← do
    (do
        let publicResult ← (pop ())
        pure ((publicResult).value))
  let off_word ← do
    (do
        let publicResult ← (pop ())
        pure ((publicResult).value))
  let len_word ← do
    (do
        let publicResult ← (pop ())
        pure ((publicResult).value))
  let salt ← do
    if (is2 : Bool)
    then
      (do
          let publicResult ← (pop ())
          pure ((publicResult).value))
    else (pure (WORD_ZERO).value)
  if ((! (← (is_running ()))) : Bool)
  then (pure ())
  else
    (do
      if ((← (guard_static ())) : Bool)
      then (pure ())
      else
        (do
          let ⟨_, ⟨_, initcode⟩⟩ ← do (charge_memory_range ⟨off_word⟩ len_word)
          if ((! (← (is_running ()))) : Bool)
          then (pure ())
          else
            (do
              (charge ((← (create_access_cost ()))).value)
              if ((fork_gteq (← readReg k_fork) Shanghai) : Bool)
              then
                (charge_word_scaled_gas ⟨G_initcode_word⟩
                  ⟨((memory_word_count_word ⟨len_word⟩)).value⟩)
              else (pure ())
              if (is2 : Bool)
              then
                (charge_word_scaled_gas ⟨G_keccak_word⟩
                  ⟨((memory_word_count_word ⟨len_word⟩)).value⟩)
              else (pure ())
              if ((! (← (is_running ()))) : Bool)
              then (pure ())
              else
                (do
                  if ((! (← (initcode_size_allowed initcode.len))) : Bool)
                  then (exc_halt InitCodeTooLarge)
                  else
                    (do
                      let nonce ← do
                        (do
                            let publicResult ← (k_get_nonce creator)
                            pure ((publicResult).value))
                      let new_addr ← (( do
                        if ((is2 && (← (is_running ()))) : Bool)
                        then
                          (k_create2_addr creator ⟨salt⟩
                            (word_to_hash
                              ⟨((← (mem_keccak ⟨_, ⟨_, initcode⟩⟩))).value⟩))
                        else (k_create_addr creator ⟨nonce⟩) ) : SailM (Vector (BitVec 8) 20) )
                      let child_gas : Nat := GAS_ZERO
                      let child_gas ← (( do
                        if ((fork_lt (← readReg k_fork) Amsterdam) : Bool)
                        then
                          (do
                            let avail ← do pure ((← readReg gas_remaining))
                            let retained_gas : Nat := (Int.ediv avail 64)
                            let child_gas ← (gas_sub_or_oog avail retained_gas)
                            writeReg gas_remaining retained_gas
                            (pure child_gas))
                        else (pure child_gas) ) : SailM Nat )
                      if ((! (← (is_running ()))) : Bool)
                      then (pure ())
                      else
                        (do
                          if ((← readReg message).is_static : Bool)
                          then
                            (do
                              (exc_halt WriteProtection))
                          else
                            (do
                              if (((((← readReg call_depth)).value ≥b (DEPTH_LIMIT).value) || ((! (word_ule
                                         value ((← (k_get_balance creator))).value)) || (nonce == ((2 ^i 64) - 1)))) : Bool)
                              then
                                (do
                                  (returndata_clear ())
                                  if ((fork_lt (← readReg k_fork) Amsterdam) : Bool)
                                  then (refund_gas child_gas)
                                  else (pure ())
                                  (push_word ⟨(WORD_ZERO).value⟩))
                              else
                                (do
                                  let _ ← do (k_access_account new_addr)
                                  let new_account_charged ← do
                                    (pure ((fork_gteq (← readReg k_fork) Amsterdam) && (← (k_account_is_empty
                                            new_addr))))
                                  if (new_account_charged : Bool)
                                  then (charge_state_gas (G_amsterdam_state_new_account).value)
                                  else (pure ())
                                  if ((! (← (is_running ()))) : Bool)
                                  then (pure ())
                                  else
                                    (do
                                      let child_gas ← (( do
                                        if ((fork_gteq (← readReg k_fork) Amsterdam) : Bool)
                                        then
                                          (do
                                            let avail ← do pure ((← readReg gas_remaining))
                                            let retained_gas : Nat := (Int.ediv avail 64)
                                            let child_gas ← (gas_sub_or_oog avail retained_gas)
                                            writeReg gas_remaining retained_gas
                                            (pure child_gas))
                                        else (pure child_gas) ) : SailM Nat )
                                      if ((! (← (is_running ()))) : Bool)
                                      then (pure ())
                                      else
                                        (do
                                          let occupied ← (( do (k_account_occupied new_addr) ) :
                                            SailM Bool )
                                          (returndata_clear ())
                                          (k_bump_nonce creator)
                                          if (occupied : Bool)
                                          then
                                            (do
                                              if (new_account_charged : Bool)
                                              then
                                                (credit_state_gas_refund
                                                  ⟨(G_amsterdam_state_new_account).value⟩)
                                              else (pure ())
                                              (push_word ⟨(WORD_ZERO).value⟩))
                                          else
                                            (do
                                              let ⟨_, ⟨_, initcode⟩⟩ ← do
                                                if ((← (is_running ())) : Bool)
                                                then
                                                  (do
                                                    (memory_code_slice initcode.off initcode.len))
                                                else
                                                  (pure ((⟨_, ⟨_, EMPTY_CODE_SLICE⟩⟩ : (Sigma
                                                    fun (k_syn_off : Nat) =>
                                                    (Sigma fun (k_syn_len : Nat) =>
                                                    (EvmByteSliceFields k_syn_off k_syn_len)))) : (Sigma
                                                    fun (k_syn_off : Nat) =>
                                                    (Sigma fun (k_syn_len : Nat) =>
                                                    (EvmByteSliceFields k_syn_off k_syn_len)))))
                                              let child_code ← do
                                                (code_db_resolve
                                                  (← (code_db_insert ⟨_, ⟨_, initcode⟩⟩
                                                      (← readReg k_fork))))
                                              let child_state_gas ← do
                                                pure ((← readReg state_gas_remaining))
                                              let checkpoint ← do
                                                (pure { (← (suspend_frame ())) with state_gas_remaining := GAS_ZERO })
                                              (frame_stack_push
                                                (ResumeCreate
                                                  { checkpoint := checkpoint,
                                                    address := new_addr,
                                                    new_account_charged := new_account_charged }))
                                              (k_mark_created new_addr)
                                              (k_clear_storage new_addr)
                                              (k_bump_nonce new_addr)
                                              (k_transfer creator new_addr ⟨value⟩)
                                              writeReg message { caller := creator,
                                                                 address := new_addr,
                                                                 code_address := new_addr,
                                                                 value := ⟨value⟩,
                                                                 state_gas_reservoir := child_state_gas,
                                                                 is_static := checkpoint.message.is_static,
                                                                 depth := ← do
                                                                     let publicField ← (do
                                                                         let publicResult ← (frame_depth_increment
                                                                         ⟨(checkpoint.call_depth).value⟩)
                                                                         pure ((publicResult).value))
                                                                     pure (⟨publicField⟩) }
                                              (calldata_install ⟨_, ⟨_, EMPTY_SLICE⟩⟩)
                                              writeReg pc 0
                                              writeReg gas_remaining child_gas
                                              writeReg state_gas_remaining child_state_gas
                                              writeReg state_gas_spilled ⟨(STATE_GAS_SPILL_ZERO).value⟩
                                              writeReg frame_status (Running ())
                                              (returndata_clear ())
                                              writeReg frame_code child_code
                                              writeReg call_depth ⟨((← (frame_depth_increment
                                                ⟨(checkpoint.call_depth).value⟩))).value⟩
                                              writeReg frame_refund ⟨(GAS_REFUND_ZERO).value⟩)))))))))))

/-- The single-step transition function: call/create opcodes install a child
frame; every other opcode executes in [execute_opcode][]. -/
def execute (op : ast) : SailM Unit := do
  match op with
  | .CREATE () => (run_create false)
  | .CREATE2 () => (run_create true)
  | .CALL () => (run_call Call)
  | .CALLCODE () => (run_call CallCode)
  | .DELEGATECALL () => (run_call DelegateCall)
  | .STATICCALL () => (run_call StaticCall)
  | _ => (execute_opcode op)

