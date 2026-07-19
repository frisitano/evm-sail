import Evm.Flow
import Evm.Arith
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
quadratic expansion cost via [charge_memory_expansion][] before acting.
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
def do_jump (dest_word : word) : SailM Unit := do
  if ((word_ult dest_word (← (word_of_byte_quantity (← (frame_code_len ()))))) : Bool)
  then
    (do
      match (word_to_limb dest_word) with
      | .some destination_bits =>
        (do
          let destination := (ByteQuantity (BitVec.toNatInt destination_bits))
          if ((← (frame_jumpdest_valid destination)) : Bool)
          then writeReg pc destination
          else (exc_halt InvalidJump))
      | none => (exc_halt InvalidJump))
  else (exc_halt InvalidJump)

/-- Pops `count` log topics from the stack. -/
/- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ 4 -/
def pop_log_topics (count : log_topic_count) : SailM (List word) := do
  let count := (count).value
  match count with
  | 0 => (pure [])
  | 1 => (pure [(← (pop ()))])
  | 2 =>
    (do
      let t0 ← do (pop ())
      let t1 ← do (pop ())
      (pure [t0, t1]))
  | 3 =>
    (do
      let t0 ← do (pop ())
      let t1 ← do (pop ())
      let t2 ← do (pop ())
      (pure [t0, t1, t2]))
  | 4 =>
    (do
      let t0 ← do (pop ())
      let t1 ← do (pop ())
      let t2 ← do (pop ())
      let t3 ← do (pop ())
      (pure [t0, t1, t2, t3]))
  | _ => (pure [])

/-- Executes the arithmetic, comparison, and bitwise opcode families. -/
def execute_arithmetic (op : ast) : SailM Unit := do
  match op with
  | .ADD () =>
    (do
      (charge_constant G_verylow)
      let a ← do (pop ())
      let b ← do (pop ())
      (push (alu_add a b)))
  | .MUL () =>
    (do
      (charge_constant G_low)
      let a ← do (pop ())
      let b ← do (pop ())
      (push (alu_mul a b)))
  | .SUB () =>
    (do
      (charge_constant G_verylow)
      let a ← do (pop ())
      let b ← do (pop ())
      (push (alu_sub a b)))
  | .DIV () =>
    (do
      (charge_constant G_low)
      let a ← do (pop ())
      let b ← do (pop ())
      (push (alu_div a b)))
  | .SDIV () =>
    (do
      (charge_constant G_low)
      let a ← do (pop ())
      let b ← do (pop ())
      (push (alu_sdiv a b)))
  | .MOD () =>
    (do
      (charge_constant G_low)
      let a ← do (pop ())
      let b ← do (pop ())
      (push (alu_mod a b)))
  | .SMOD () =>
    (do
      (charge_constant G_low)
      let a ← do (pop ())
      let b ← do (pop ())
      (push (alu_smod a b)))
  | .ADDMOD () =>
    (do
      (charge_constant G_mid)
      let a ← do (pop ())
      let b ← do (pop ())
      let n ← do (pop ())
      (push (alu_addmod a b n)))
  | .MULMOD () =>
    (do
      (charge_constant G_mid)
      let a ← do (pop ())
      let b ← do (pop ())
      let n ← do (pop ())
      (push (alu_mulmod a b n)))
  | .EXP () =>
    (do
      let a ← do (pop ())
      let e ← do (pop ())
      (charge_cost (← (exp_gas e)))
      (push (alu_exp a e)))
  | .SIGNEXTEND () =>
    (do
      (charge_constant G_low)
      let bi ← do (pop ())
      let v ← do (pop ())
      (push (alu_signextend bi v)))
  | .LT () =>
    (do
      (charge_constant G_verylow)
      let a ← do (pop ())
      let b ← do (pop ())
      (push (alu_lt a b)))
  | .GT () =>
    (do
      (charge_constant G_verylow)
      let a ← do (pop ())
      let b ← do (pop ())
      (push (alu_gt a b)))
  | .SLT () =>
    (do
      (charge_constant G_verylow)
      let a ← do (pop ())
      let b ← do (pop ())
      (push (alu_slt a b)))
  | .SGT () =>
    (do
      (charge_constant G_verylow)
      let a ← do (pop ())
      let b ← do (pop ())
      (push (alu_sgt a b)))
  | .EQ () =>
    (do
      (charge_constant G_verylow)
      let a ← do (pop ())
      let b ← do (pop ())
      (push (alu_eq a b)))
  | .ISZERO () =>
    (do
      (charge_constant G_verylow)
      let a ← do (pop ())
      (push (alu_iszero a)))
  | .AND () =>
    (do
      (charge_constant G_verylow)
      let a ← do (pop ())
      let b ← do (pop ())
      (push (alu_and a b)))
  | .OR () =>
    (do
      (charge_constant G_verylow)
      let a ← do (pop ())
      let b ← do (pop ())
      (push (alu_or a b)))
  | .XOR () =>
    (do
      (charge_constant G_verylow)
      let a ← do (pop ())
      let b ← do (pop ())
      (push (alu_xor a b)))
  | .NOT () =>
    (do
      (charge_constant G_verylow)
      let a ← do (pop ())
      (push (alu_not a)))
  | .BYTE () =>
    (do
      (charge_constant G_verylow)
      let i ← do (pop ())
      let x ← do (pop ())
      (push (alu_byte i x)))
  | .SHL () =>
    (do
      (charge_constant G_verylow)
      let s ← do (pop ())
      let v ← do (pop ())
      (push (alu_shl s v)))
  | .SHR () =>
    (do
      (charge_constant G_verylow)
      let s ← do (pop ())
      let v ← do (pop ())
      (push (alu_shr s v)))
  | .SAR () =>
    (do
      (charge_constant G_verylow)
      let s ← do (pop ())
      let v ← do (pop ())
      (push (alu_sar s v)))
  | .CLZ () =>
    (do
      (charge_constant G_low)
      let x ← do (pop ())
      (push (← (alu_clz x))))
  | _ => assert false "sail/evm/execute.sail:259.21-259.22"

/-- Executes hashing and transaction/environment data access opcodes. -/
def execute_environment (op : ast) : SailM Unit := do
  match op with
  | .KECCAK256 () =>
    (do
      let offset_word ← do (pop ())
      let length_word ← do (pop ())
      if ((← (is_running ())) : Bool)
      then
        (do
          let (offset, length) := (word_byte_range_or_oog offset_word length_word)
          if ((← (is_running ())) : Bool)
          then
            (do
              (charge_keccak_gas length)
              (charge_memory_expansion offset_word length_word))
          else (pure ())
          if ((← (is_running ())) : Bool)
          then (push (← (mem_keccak offset length)))
          else (pure ()))
      else (pure ()))
  | .ADDRESS () =>
    (do
      (charge_constant G_base)
      (push (address_to_word (← (self_addr ())))))
  | .ORIGIN () =>
    (do
      (charge_constant G_base)
      (push (← (k_env F_Origin))))
  | .CALLER () =>
    (do
      (charge_constant G_base)
      (push (address_to_word (← readReg message).caller)))
  | .CALLVALUE () =>
    (do
      (charge_constant G_base)
      (push (← readReg message).value))
  | .GASPRICE () =>
    (do
      (charge_constant G_base)
      (push (← (k_env F_GasPrice))))
  | .CALLDATASIZE () =>
    (do
      (charge_constant G_base)
      (push (← (word_of_byte_quantity (← readReg calldata).len))))
  | .CALLDATALOAD () =>
    (do
      (charge_constant G_verylow)
      let offset_word ← do (pop ())
      if ((← (is_running ())) : Bool)
      then (push (← (slice_load_word_offset (← readReg calldata) offset_word)))
      else (pure ()))
  | .CALLDATACOPY () =>
    (do
      (charge_constant G_verylow)
      let destination_word ← do (pop ())
      let source_word ← do (pop ())
      let length_word ← do (pop ())
      if ((← (is_running ())) : Bool)
      then
        (do
          let (destination, length) := (word_byte_range_or_oog destination_word length_word)
          if ((← (is_running ())) : Bool)
          then
            (do
              (charge_copy_gas length)
              (charge_memory_expansion destination_word length_word))
          else (pure ())
          if ((← (is_running ())) : Bool)
          then (slice_copy_word_offset (← readReg calldata) destination source_word length)
          else (pure ()))
      else (pure ()))
  | .CODESIZE () =>
    (do
      (charge_constant G_base)
      (push (← (word_of_byte_quantity (← (frame_code_len ()))))))
  | .CODECOPY () =>
    (do
      (charge_constant G_verylow)
      let destination_word ← do (pop ())
      let source_word ← do (pop ())
      let length_word ← do (pop ())
      if ((← (is_running ())) : Bool)
      then
        (do
          let (destination, length) := (word_byte_range_or_oog destination_word length_word)
          if ((← (is_running ())) : Bool)
          then
            (do
              (charge_copy_gas length)
              (charge_memory_expansion destination_word length_word))
          else (pure ())
          if ((← (is_running ())) : Bool)
          then
            (slice_copy_word_offset (← readReg frame_code).bytes destination source_word length)
          else (pure ()))
      else (pure ()))
  | .BALANCE () =>
    (do
      let a ← do (pure (word_to_address (← (pop ()))))
      let warm ← do (k_access_account a)
      (charge_constant (account_cost warm))
      if ((← (is_running ())) : Bool)
      then (push (← (k_get_balance a)))
      else (pure ()))
  | .SELFBALANCE () =>
    (do
      (charge_constant G_low)
      (push (← (k_get_balance (← (self_addr ()))))))
  | .EXTCODESIZE () =>
    (do
      let a ← do (pure (word_to_address (← (pop ()))))
      let warm ← do (k_access_account a)
      (charge_constant (account_cost warm))
      if ((← (is_running ())) : Bool)
      then (push (← (word_of_byte_quantity (← (k_get_code_size a)))))
      else (pure ()))
  | .EXTCODECOPY () =>
    (do
      let a ← do (pure (word_to_address (← (pop ()))))
      let destination_word ← do (pop ())
      let source_word ← do (pop ())
      let length_word ← do (pop ())
      let warm ← do (k_access_account a)
      (charge_constant (account_cost warm))
      if ((← (is_running ())) : Bool)
      then
        (do
          let (destination, length) := (word_byte_range_or_oog destination_word length_word)
          if ((← (is_running ())) : Bool)
          then
            (do
              (charge_copy_gas length)
              (charge_memory_expansion destination_word length_word))
          else (pure ())
          if ((← (is_running ())) : Bool)
          then (k_code_copy a destination source_word length)
          else (pure ()))
      else (pure ()))
  | .EXTCODEHASH () =>
    (do
      let a ← do (pure (word_to_address (← (pop ()))))
      let warm ← do (k_access_account a)
      (charge_constant (account_cost warm))
      if ((← (is_running ())) : Bool)
      then (push (hash_to_word (← (k_get_codehash a))))
      else (pure ()))
  | .RETURNDATASIZE () =>
    (do
      (charge_constant G_base)
      (push (← (word_of_byte_quantity (← (returndata_size ()))))))
  | .RETURNDATACOPY () =>
    (do
      (charge_constant G_verylow)
      let destination_word ← do (pop ())
      let source_word ← do (pop ())
      let length_word ← do (pop ())
      if ((← (is_running ())) : Bool)
      then
        (do
          let (destination, length) := (word_byte_range_or_oog destination_word length_word)
          if ((← (is_running ())) : Bool)
          then
            (do
              (charge_copy_gas length)
              (charge_memory_expansion destination_word length_word))
          else (pure ())
          if ((← (is_running ())) : Bool)
          then (returndata_copy_words destination source_word length_word)
          else (pure ()))
      else (pure ()))
  | _ => assert false "sail/evm/execute.sail:411.21-411.22"

/-- Executes block-context opcodes. -/
def execute_block (op : ast) : SailM Unit := do
  match op with
  | .BLOCKHASH () =>
    (do
      (charge_constant (GasConstant 20))
      (push (hash_to_word (← (k_blockhash (← (pop ())))))))
  | .COINBASE () =>
    (do
      (charge_constant G_base)
      (push (← (k_env F_Coinbase))))
  | .TIMESTAMP () =>
    (do
      (charge_constant G_base)
      (push (← (k_env F_Timestamp))))
  | .NUMBER () =>
    (do
      (charge_constant G_base)
      (push (← (k_env F_Number))))
  | .SLOTNUM () =>
    (do
      (charge_constant G_base)
      (push (← (k_env F_SlotNumber))))
  | .PREVRANDAO () =>
    (do
      (charge_constant G_base)
      (push (← (k_env F_PrevRandao))))
  | .GASLIMIT () =>
    (do
      (charge_constant G_base)
      (push (← (k_env F_GasLimit))))
  | .CHAINID () =>
    (do
      (charge_constant G_base)
      (push (← (k_env F_ChainId))))
  | .BASEFEE () =>
    (do
      (charge_constant G_base)
      (push (← (k_env F_BaseFee))))
  | .BLOBBASEFEE () =>
    (do
      (charge_constant G_base)
      (push (← (blob_base_fee ⟨((← readReg k_header).excess_blob_gas).value⟩))))
  | .BLOBHASH () =>
    (do
      (charge_constant G_verylow)
      (push (← (k_blobhash (← (pop ()))))))
  | _ => assert false "sail/evm/execute.sail:460.21-460.22"

/-- Executes stack and linear-memory opcodes. -/
def execute_memory (op : ast) : SailM Unit := do
  match op with
  | .POP () =>
    (do
      (charge_constant G_base)
      let _ ← do (pop ())
      (pure ()))
  | .MLOAD () =>
    (do
      (charge_constant G_verylow)
      let offset_word ← do (pop ())
      (charge_memory_expansion offset_word (← (word_of_protocol_quantity ⟨32⟩)))
      if ((← (is_running ())) : Bool)
      then
        (do
          let offset := (word_to_byte_quantity_or_oog offset_word)
          if ((← (is_running ())) : Bool)
          then (push (← (mem_load offset)))
          else (pure ()))
      else (pure ()))
  | .MSTORE () =>
    (do
      (charge_constant G_verylow)
      let offset_word ← do (pop ())
      let v ← do (pop ())
      (charge_memory_expansion offset_word (← (word_of_protocol_quantity ⟨32⟩)))
      if ((← (is_running ())) : Bool)
      then
        (do
          let offset := (word_to_byte_quantity_or_oog offset_word)
          if ((← (is_running ())) : Bool)
          then (mem_store offset v)
          else (pure ()))
      else (pure ()))
  | .MSTORE8 () =>
    (do
      (charge_constant G_verylow)
      let offset_word ← do (pop ())
      let v ← do (pop ())
      (charge_memory_expansion offset_word WORD_ONE)
      if ((← (is_running ())) : Bool)
      then
        (do
          let offset := (word_to_byte_quantity_or_oog offset_word)
          if ((← (is_running ())) : Bool)
          then (mem_store_byte offset v)
          else (pure ()))
      else (pure ()))
  | .MSIZE () =>
    (do
      (charge_constant G_base)
      (push (← (word_of_byte_quantity (← (evm_memory_size ()))))))
  | .MCOPY () =>
    (do
      (charge_constant G_verylow)
      let destination_word ← do (pop ())
      let source_word ← do (pop ())
      let length_word ← do (pop ())
      let required_size :=
        (maximum_memory_required_size destination_word length_word source_word length_word)
      if ((← (is_running ())) : Bool)
      then
        (do
          let (destination, length) := (word_byte_range_or_oog destination_word length_word)
          let (source, _) := (word_byte_range_or_oog source_word length_word)
          if ((← (is_running ())) : Bool)
          then
            (do
              (charge_copy_gas length)
              (charge_cost (← (memory_expansion_gas required_size))))
          else (pure ())
          if ((← (is_running ())) : Bool)
          then (expand_memory required_size)
          else (pure ())
          if ((← (is_running ())) : Bool)
          then (mem_mcopy destination source length)
          else (pure ()))
      else (pure ()))
  | _ => assert false "sail/evm/execute.sail:530.21-530.22"

/-- Executes persistent and transient storage opcodes. -/
def execute_storage (op : ast) : SailM Unit := do
  match op with
  | .SLOAD () =>
    (do
      let s ← do (pop ())
      let warm ← do (k_slot_is_warm (← (self_addr ())) s)
      (charge_constant (sload_cost warm))
      if ((← (is_running ())) : Bool)
      then (push (← (k_sload (← (self_addr ())) s)).curr)
      else (pure ()))
  | .SSTORE () =>
    (do
      if ((← (guard_static ())) : Bool)
      then (pure ())
      else
        (do
          if ((gas_le (← readReg gas_remaining) G_callstipend) : Bool)
          then (exc_halt OutOfGas)
          else
            (do
              let s ← do (pop ())
              let v ← do (pop ())
              if ((← (is_running ())) : Bool)
              then
                (do
                  let warm ← do (k_slot_is_warm (← (self_addr ())) s)
                  let entry ← do (k_sload (← (self_addr ())) s)
                  let (cost, refund) ← do (sstore_gas entry.orig entry.curr v (! warm))
                  (charge_cost cost)
                  if ((! (refund == GAS_REFUND_ZERO)) : Bool)
                  then (record_refund refund)
                  else (pure ())
                  if ((bne entry.curr v) : Bool)
                  then
                    (k_sstore (← (self_addr ())) s
                      { curr := v,
                        orig := entry.orig })
                  else (pure ()))
              else (pure ()))))
  | .TLOAD () =>
    (do
      (charge_constant G_warm_access)
      let s ← do (pop ())
      (push (← (k_tload (← (self_addr ())) s))))
  | .TSTORE () =>
    (do
      if ((← (guard_static ())) : Bool)
      then (pure ())
      else
        (do
          (charge_constant G_warm_access)
          let s ← do (pop ())
          let v ← do (pop ())
          (k_tstore (← (self_addr ())) s v)))
  | _ => assert false "sail/evm/execute.sail:585.21-585.22"

/-- Executes control-flow and stack-shuffling opcodes. -/
def execute_control (op : ast) : SailM Unit := do
  match op with
  | .JUMP () =>
    (do
      (charge_constant G_mid)
      let dest ← do (pop ())
      (do_jump dest))
  | .JUMPI () =>
    (do
      (charge_constant G_high)
      let dest ← do (pop ())
      let cond ← do (pop ())
      if ((word_is_zero cond) : Bool)
      then (pure ())
      else (do_jump dest))
  | .PC () =>
    (do
      (charge_constant G_base)
      (push (alu_sub (← (word_of_byte_quantity (← readReg pc))) WORD_ONE)))
  | .GAS () =>
    (do
      (charge_constant G_base)
      (push (← (word_of_gas (← readReg gas_remaining)))))
  | .JUMPDEST () => (charge_constant G_jumpdest)
  | .PUSH (n, v) =>
    let n := (n).value
    (do
      if ((n == 0) : Bool)
      then (charge_constant G_base)
      else (charge_constant G_verylow)
      (push v))
  | .DUP n =>
    let n := (n).value
    (do
      (charge_constant G_verylow)
      if ((((← (stack_height ()))).value <b n) : Bool)
      then (exc_halt StackUnderflow)
      else (push (← (peek ⟨(n -i 1)⟩))))
  | .SWAP n =>
    let n := (n).value
    (do
      (charge_constant G_verylow)
      if ((((← (stack_height ()))).value <b (n + 1)) : Bool)
      then (exc_halt StackUnderflow)
      else
        (do
          let top ← do (peek ⟨0⟩)
          let other ← do (peek ⟨n⟩)
          (stack_set ⟨0⟩ other)
          (stack_set ⟨n⟩ top)))
  | _ => assert false "sail/evm/execute.sail:641.21-641.22"

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
          let offset_word ← do (pop ())
          let length_word ← do (pop ())
          let topics ← do (pop_log_topics ⟨n⟩)
          if ((← (is_running ())) : Bool)
          then
            (do
              let (offset, length) := (word_byte_range_or_oog offset_word length_word)
              if ((← (is_running ())) : Bool)
              then
                (do
                  (charge_log_gas ⟨n⟩ length)
                  (charge_memory_expansion offset_word length_word))
              else (pure ())
              if ((← (is_running ())) : Bool)
              then
                (k_log (← (self_addr ())) topics
                  (BytesSlice (← (memory_byte_slice offset length))))
              else (pure ()))
          else (pure ())))
  | _ => assert false "sail/evm/execute.sail:665.21-665.22"

/-- Executes frame-halting opcodes other than call and create. -/
def execute_halt (op : ast) : SailM Unit := do
  match op with
  | .STOP () => writeReg frame_status (Halted (HaltStop ()))
  | .RETURN () =>
    (do
      let offset_word ← do (pop ())
      let length_word ← do (pop ())
      (charge_memory_expansion offset_word length_word)
      if ((← (is_running ())) : Bool)
      then
        (do
          let (offset, length) := (word_byte_range_or_oog offset_word length_word)
          if ((← (is_running ())) : Bool)
          then
            writeReg frame_status (Halted
              (HaltReturn (← (freeze_output (← (memory_byte_slice offset length))))))
          else (pure ()))
      else (pure ()))
  | .REVERT () =>
    (do
      let offset_word ← do (pop ())
      let length_word ← do (pop ())
      (charge_memory_expansion offset_word length_word)
      if ((← (is_running ())) : Bool)
      then
        (do
          let (offset, length) := (word_byte_range_or_oog offset_word length_word)
          if ((← (is_running ())) : Bool)
          then
            writeReg frame_status (Halted
              (HaltRevert (← (freeze_output (← (memory_byte_slice offset length))))))
          else (pure ()))
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
          let beneficiary ← do (pure (word_to_address (← (pop ()))))
          let bal ← do (k_get_balance (← (self_addr ())))
          let warm ← do (k_access_account beneficiary)
          (charge_constant G_selfdestruct)
          if ((! warm) : Bool)
          then (charge_constant G_cold_account)
          else (pure ())
          if (((← (is_running ())) && ((word_nonzero bal) && (← (k_account_is_empty beneficiary)))) : Bool)
          then (charge_constant G_newaccount)
          else (pure ())
          if ((← (is_running ())) : Bool)
          then
            (do
              let first_selfdestruct ← do
                (pure (! (← (k_is_selfdestructed (← (self_addr ()))))))
              if (((fork_lt (← readReg k_fork) London) && first_selfdestruct) : Bool)
              then (record_refund (gas_constant_to_refund R_selfdestruct_pre_london))
              else (pure ())
              (k_transfer (← (self_addr ())) beneficiary bal)
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
                      if ((beneficiary == (← (self_addr ()))) : Bool)
                      then (k_emit_burn_log (← (self_addr ())) bal)
                      else (pure ())
                      (k_zero_balance (← (self_addr ())))
                      (k_selfdestruct (← (self_addr ()))))
                  else (pure ()))
              writeReg frame_status (Halted (HaltSelfDestruct ())))
          else (pure ())))
  | _ => assert false "sail/evm/execute.sail:734.21-734.22"

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
  | .LOG _ => (execute_log op)
  | .CREATE () => assert false "sail/evm/execute.sail:827.28-827.29"
  | .CREATE2 () => assert false "sail/evm/execute.sail:828.29-828.30"
  | .CALL () => assert false "sail/evm/execute.sail:829.26-829.27"
  | .CALLCODE () => assert false "sail/evm/execute.sail:830.30-830.31"
  | .DELEGATECALL () => assert false "sail/evm/execute.sail:831.34-831.35"
  | .STATICCALL () => assert false "sail/evm/execute.sail:832.32-832.33"
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
/- Type quantifiers: k_ex161349_ : Bool -/
def executable_code (target : address) (dele : Bool) (dtgt : address) : SailM Code := do
  if (dele : Bool)
  then
    (do
      match (Option.map (fun semanticValue => (semanticValue).value) ((word_to_precompile_id
        (address_to_word dtgt)))) with
      | .some number =>
        (do
          if ((← (is_precompile ⟨number⟩)) : Bool)
          then (pure EMPTY_CODE)
          else (code_db_resolve (← (k_code_key dtgt))))
      | _ => (code_db_resolve (← (k_code_key dtgt))))
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
  let gas_request ← do (pop ())
  let target ← do (pure (word_to_address (← (pop ()))))
  let value ← do
    if ((call_takes_value kind) : Bool)
    then (pop ())
    else (pure WORD_ZERO)
  let args_off_word ← do (pop ())
  let args_len_word ← do (pop ())
  let ret_off_word ← do (pop ())
  let ret_len_word ← do (pop ())
  if ((! (← (is_running ()))) : Bool)
  then (pure ())
  else
    (do
      let required_memory_size :=
        (maximum_memory_required_size args_off_word args_len_word ret_off_word ret_len_word)
      let mem_gas ← do (memory_expansion_gas required_memory_size)
      if (((call_transfers_value kind) && ((word_nonzero value) && (← readReg message).is_static)) : Bool)
      then
        (do
          (exc_halt WriteProtection))
      else
        (do
          let warm ← do (k_access_account target)
          let target_cost : gas_constant := (account_cost warm)
          let transfer_cost : gas_constant :=
            if ((word_nonzero value) : Bool)
            then G_callvalue
            else GAS_CONSTANT_ZERO
          let static_cost := (gas_cost_add (gas_constant_add target_cost transfer_cost) mem_gas)
          if (((! (← (is_running ()))) || (gas_lt_cost (← readReg gas_remaining) static_cost)) : Bool)
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
                    (pure (account_cost dw)))
                else (pure GAS_CONSTANT_ZERO) ) : SailM gas_constant )
              let access_cost := (gas_constant_add target_cost delegation_cost)
              let create_cost ← (( do
                if (((word_nonzero value) && ((call_transfers_value kind) && (← (k_account_is_empty
                           target)))) : Bool)
                then (pure G_newaccount)
                else (pure GAS_CONSTANT_ZERO) ) : SailM gas_constant )
              let extra_gas :=
                (gas_cost_add_constant (gas_cost_add_constant access_cost transfer_cost) create_cost)
              let required := (gas_cost_add extra_gas mem_gas)
              let stipend : gas :=
                if ((word_nonzero value) : Bool)
                then G_callstipend
                else GAS_ZERO
              if ((! (← (is_running ()))) : Bool)
              then (pure ())
              else
                (do
                  let avail ← do readReg gas_remaining
                  let base_child ← (( do
                    if ((gas_lt_cost avail required) : Bool)
                    then (pure GAS_ZERO)
                    else
                      (do
                        let available_after_cost ← do (gas_sub_cost_or_oog avail required)
                        (call_gas_cap_word available_after_cost gas_request)) ) : SailM gas )
                  let total_charge := (gas_cost_add (gas_to_cost base_child) required)
                  (charge_cost total_charge)
                  if ((! (← (is_running ()))) : Bool)
                  then (pure ())
                  else
                    (do
                      if (tg_deleg : Bool)
                      then
                        (do
                          let _ ← do (code_db_resolve (← (k_code_key tg_target)))
                          let _ ← do (k_aload tg_target)
                          (pure ()))
                      else (pure ())
                      (expand_charged_memory required_memory_size)
                      if ((! (← (is_running ()))) : Bool)
                      then (pure ())
                      else
                        (do
                          let (args_offset, args_length) :=
                            (word_byte_range_or_oog args_off_word args_len_word)
                          if ((! (← (is_running ()))) : Bool)
                          then (pure ())
                          else
                            (do
                              let (ret_offset, ret_length) :=
                                (word_byte_range_or_oog ret_off_word ret_len_word)
                              if ((! (← (is_running ()))) : Bool)
                              then (pure ())
                              else
                                (do
                                  let child_gas ← (( do
                                    if ((gas_sum_supported base_child stipend) : Bool)
                                    then (gas_add base_child stipend)
                                    else sailThrow ((InvalidBlock ExecutionInvalid)) ) : SailM gas )
                                  if ((! (← (is_running ()))) : Bool)
                                  then (pure ())
                                  else
                                    (do
                                      let _ ← do (k_aload target)
                                      let depth_ok ← do
                                        (pure ((← readReg call_depth) <b (DEPTH_LIMIT).value))
                                      let balance_ok ← (( do
                                        if (((call_takes_value kind) && (word_nonzero value)) : Bool)
                                        then (pure (word_ule value (← (k_get_balance caller))))
                                        else (pure true) ) : SailM Bool )
                                      if (((! depth_ok) || (! balance_ok)) : Bool)
                                      then
                                        (do
                                          (returndata_clear ())
                                          (refund_gas child_gas)
                                          (push WORD_ZERO))
                                      else
                                        (do
                                          match (Option.map (fun semanticValue => (semanticValue).value) ((word_to_precompile_id
                                            (address_to_word target)))) with
                                          | .some precompile_number =>
                                            (do
                                              if ((← (is_precompile ⟨precompile_number⟩)) : Bool)
                                              then
                                                (do
                                                  let input ← do
                                                    (memory_byte_slice args_offset args_length)
                                                  match (← (precompile_gas ⟨precompile_number⟩
                                                      input)) with
                                                  | .some used =>
                                                    (do
                                                      if ((gas_cost_le_gas used child_gas) : Bool)
                                                      then
                                                        (do
                                                          let result ← do
                                                            (run_precompile_slice
                                                              ⟨precompile_number⟩ input)
                                                          if (result.success : Bool)
                                                          then
                                                            (do
                                                              writeReg returndata result.output
                                                              if (((call_transfers_value kind) && (word_nonzero
                                                                     value)) : Bool)
                                                              then (k_transfer caller target value)
                                                              else (pure ())
                                                              (returndata_copy_prefix ret_offset
                                                                ret_length)
                                                              let unused ← do
                                                                (gas_sub_cost_or_oog child_gas used)
                                                              if ((← (is_running ())) : Bool)
                                                              then
                                                                (do
                                                                  (refund_gas unused)
                                                                  (push WORD_ONE))
                                                              else (pure ()))
                                                          else
                                                            (do
                                                              (returndata_clear ())
                                                              (push WORD_ZERO)))
                                                      else
                                                        (do
                                                          (returndata_clear ())
                                                          (push WORD_ZERO)))
                                                  | _ =>
                                                    (do
                                                      (returndata_clear ())
                                                      (push WORD_ZERO)))
                                              else
                                                (do
                                                  let child_code ← do
                                                    (executable_code target tg_deleg tg_target)
                                                  let child_addr : address :=
                                                    if ((call_uses_target_address kind) : Bool)
                                                    then target
                                                    else caller
                                                  let child_caller ← (( do
                                                    if ((call_is_delegate kind) : Bool)
                                                    then (pure (← readReg message).caller)
                                                    else (pure caller) ) : SailM address )
                                                  let child_value ← (( do
                                                    if ((call_is_delegate kind) : Bool)
                                                    then (pure (← readReg message).value)
                                                    else (pure value) ) : SailM word )
                                                  let child_static ← (( do
                                                    if ((call_is_static kind) : Bool)
                                                    then (pure true)
                                                    else (pure (← readReg message).is_static) ) :
                                                    SailM Bool )
                                                  let child_calldata ← do
                                                    if ((← (is_running ())) : Bool)
                                                    then (memory_byte_slice args_offset args_length)
                                                    else (pure EMPTY_SLICE)
                                                  let checkpoint ← do (suspend_frame ())
                                                  (frame_stack_push
                                                    (ResumeCall
                                                      { checkpoint := checkpoint,
                                                        return_offset := ret_offset,
                                                        return_length := ret_length }))
                                                  if (((call_transfers_value kind) && (word_nonzero
                                                         value)) : Bool)
                                                  then (k_transfer caller target value)
                                                  else (pure ())
                                                  writeReg message { caller := child_caller,
                                                                     address := child_addr,
                                                                     code_address := target,
                                                                     value := child_value,
                                                                     is_static := child_static,
                                                                     depth := ← do
                                                                         let semanticField ← (do
                                                                             let semanticResult ← (frame_depth_increment
                                                                             ⟨(checkpoint.call_depth).value⟩)
                                                                             pure ((semanticResult).value))
                                                                         pure (⟨semanticField⟩) }
                                                  (calldata_install child_calldata)
                                                  writeReg pc BYTE_ZERO
                                                  writeReg gas_remaining child_gas
                                                  writeReg frame_status (Running ())
                                                  (returndata_clear ())
                                                  writeReg frame_code child_code
                                                  writeReg call_depth ((← (frame_depth_increment
                                                    ⟨(checkpoint.call_depth).value⟩))).value
                                                  writeReg frame_refund GAS_REFUND_ZERO))
                                          | _ =>
                                            (do
                                              let child_code ← do
                                                (executable_code target tg_deleg tg_target)
                                              let child_addr : address :=
                                                if ((call_uses_target_address kind) : Bool)
                                                then target
                                                else caller
                                              let child_caller ← (( do
                                                if ((call_is_delegate kind) : Bool)
                                                then (pure (← readReg message).caller)
                                                else (pure caller) ) : SailM address )
                                              let child_value ← (( do
                                                if ((call_is_delegate kind) : Bool)
                                                then (pure (← readReg message).value)
                                                else (pure value) ) : SailM word )
                                              let child_static ← (( do
                                                if ((call_is_static kind) : Bool)
                                                then (pure true)
                                                else (pure (← readReg message).is_static) ) :
                                                SailM Bool )
                                              let child_calldata ← do
                                                if ((← (is_running ())) : Bool)
                                                then (memory_byte_slice args_offset args_length)
                                                else (pure EMPTY_SLICE)
                                              let checkpoint ← do (suspend_frame ())
                                              (frame_stack_push
                                                (ResumeCall
                                                  { checkpoint := checkpoint,
                                                    return_offset := ret_offset,
                                                    return_length := ret_length }))
                                              if (((call_transfers_value kind) && (word_nonzero
                                                     value)) : Bool)
                                              then (k_transfer caller target value)
                                              else (pure ())
                                              writeReg message { caller := child_caller,
                                                                 address := child_addr,
                                                                 code_address := target,
                                                                 value := child_value,
                                                                 is_static := child_static,
                                                                 depth := ← do
                                                                     let semanticField ← (do
                                                                         let semanticResult ← (frame_depth_increment
                                                                         ⟨(checkpoint.call_depth).value⟩)
                                                                         pure ((semanticResult).value))
                                                                     pure (⟨semanticField⟩) }
                                              (calldata_install child_calldata)
                                              writeReg pc BYTE_ZERO
                                              writeReg gas_remaining child_gas
                                              writeReg frame_status (Running ())
                                              (returndata_clear ())
                                              writeReg frame_code child_code
                                              writeReg call_depth ((← (frame_depth_increment
                                                ⟨(checkpoint.call_depth).value⟩))).value
                                              writeReg frame_refund GAS_REFUND_ZERO)))))))))))

/-- `CREATE` (`is2 = false`) and `CREATE2` (`is2 = true`, EIP-1014).
Both deploy a new contract by running the initcode supplied in
memory; the new address derives from `(creator, nonce)` for `CREATE`
or `(creator, salt, keccak256(initcode))` for `CREATE2`. Operand
layout: `value`, `offset`, `length`, `salt` (for `CREATE2`). Pushes
the new address on success, 0 on failure. -/
/- Type quantifiers: k_ex161350_ : Bool -/
def run_create (is2 : Bool) : SailM Unit := do
  let creator ← do (self_addr ())
  let value ← do (pop ())
  let off_word ← do (pop ())
  let len_word ← do (pop ())
  let salt ← do
    if (is2 : Bool)
    then (pop ())
    else (pure WORD_ZERO)
  if ((! (← (is_running ()))) : Bool)
  then (pure ())
  else
    (do
      if ((← (guard_static ())) : Bool)
      then (pure ())
      else
        (do
          let initcode_size := (word_to_byte_quantity_or_oog len_word)
          if ((! (← (is_running ()))) : Bool)
          then (pure ())
          else
            (do
              (charge_memory_expansion off_word len_word)
              (charge_constant G_create)
              (charge_cost (← (initcode_gas initcode_size)))
              if (is2 : Bool)
              then
                (charge_cost
                  (gas_constant_scale_byte_quantity G_keccak_word
                    (← (memory_word_count_bytes initcode_size))))
              else (pure ())
              if ((! (← (is_running ()))) : Bool)
              then (pure ())
              else
                (do
                  let (initcode_offset, initcode_length) :=
                    (word_byte_range_or_oog off_word len_word)
                  if ((! (← (is_running ()))) : Bool)
                  then (pure ())
                  else
                    (do
                      if ((! (← (initcode_size_allowed initcode_size))) : Bool)
                      then (exc_halt InitCodeTooLarge)
                      else
                        (do
                          let nonce ← do
                            (do
                                let semanticResult ← (k_get_nonce creator)
                                pure ((semanticResult).value))
                          let new_addr ← (( do
                            if ((is2 && (← (is_running ()))) : Bool)
                            then
                              (k_create2_addr creator salt
                                (word_to_hash (← (mem_keccak initcode_offset initcode_length))))
                            else (k_create_addr creator ⟨nonce⟩) ) : SailM address )
                          let avail ← do readReg gas_remaining
                          let retained_gas ← do (gas_quotient avail ⟨64⟩)
                          let child_gas ← do (gas_sub_gas_or_oog avail retained_gas)
                          if ((! (← (is_running ()))) : Bool)
                          then (pure ())
                          else
                            (do
                              writeReg gas_remaining retained_gas
                              if ((← readReg message).is_static : Bool)
                              then
                                (do
                                  (exc_halt WriteProtection))
                              else
                                (do
                                  let depth_ok ← do
                                    (pure ((← readReg call_depth) <b (DEPTH_LIMIT).value))
                                  let balance_ok ← (( do
                                    (pure (word_ule value (← (k_get_balance creator)))) ) : SailM
                                    Bool )
                                  let nonce_ok : Bool := (nonce != (BYTE_QUANTITY_MAX).value)
                                  if (((! depth_ok) || ((! balance_ok) || (! nonce_ok))) : Bool)
                                  then
                                    (do
                                      (returndata_clear ())
                                      (refund_gas child_gas)
                                      (push WORD_ZERO))
                                  else
                                    (do
                                      let _ ← do (k_access_account new_addr)
                                      let occupied ← (( do (k_account_occupied new_addr) ) : SailM
                                        Bool )
                                      (returndata_clear ())
                                      (k_bump_nonce creator)
                                      if (occupied : Bool)
                                      then (push WORD_ZERO)
                                      else
                                        (do
                                          let initcode ← do
                                            if ((← (is_running ())) : Bool)
                                            then (memory_byte_slice initcode_offset initcode_length)
                                            else (pure EMPTY_SLICE)
                                          let child_code ← do
                                            (code_db_resolve (← (code_db_insert initcode)))
                                          let checkpoint ← do (suspend_frame ())
                                          (frame_stack_push
                                            (ResumeCreate
                                              { checkpoint := checkpoint,
                                                address := new_addr }))
                                          (k_mark_created new_addr)
                                          (k_clear_storage new_addr)
                                          (k_bump_nonce new_addr)
                                          (k_transfer creator new_addr value)
                                          writeReg message { caller := creator,
                                                             address := new_addr,
                                                             code_address := new_addr,
                                                             value := value,
                                                             is_static := checkpoint.message.is_static,
                                                             depth := ← do
                                                                 let semanticField ← (do
                                                                     let semanticResult ← (frame_depth_increment
                                                                     ⟨(checkpoint.call_depth).value⟩)
                                                                     pure ((semanticResult).value))
                                                                 pure (⟨semanticField⟩) }
                                          (calldata_install EMPTY_SLICE)
                                          writeReg pc BYTE_ZERO
                                          writeReg gas_remaining child_gas
                                          writeReg frame_status (Running ())
                                          (returndata_clear ())
                                          writeReg frame_code child_code
                                          writeReg call_depth ((← (frame_depth_increment
                                            ⟨(checkpoint.call_depth).value⟩))).value
                                          writeReg frame_refund GAS_REFUND_ZERO))))))))))

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

