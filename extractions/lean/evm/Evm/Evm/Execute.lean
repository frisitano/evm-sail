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
import Evm.Host.Kernel.Lifecycle
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
open StateCheckpoint
open Register
open NodeRef
open MerkleSlot
open HaltKind
open FrameStatus
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

def self_addr (_ : Unit) : SailM address := do
  (pure (← readReg message).address)

def guard_static (_ : Unit) : SailM Bool := do
  if ((← readReg message).is_static : Bool)
  then
    (do
      (exc_halt WriteProtection)
      (pure true))
  else (pure false)

def do_jump (dest_word : word) : SailM Unit := do
  if ((word_ult dest_word (← (word_of_byte_quantity (← (frame_code_len ()))))) : Bool)
  then
    (do
      let destination := (ByteQuantity (BitVec.toNatInt dest_word))
      if ((← (frame_jumpdest_valid destination)) : Bool)
      then writeReg pc destination
      else (exc_halt InvalidJump))
  else (exc_halt InvalidJump)

/-- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ 4 -/
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

def call_is_delegate (kind : CallKind) : Bool :=
  match kind with
  | .DelegateCall => true
  | _ => false

def call_is_static (kind : CallKind) : Bool :=
  match kind with
  | .StaticCall => true
  | _ => false

def call_takes_value (kind : CallKind) : Bool :=
  match kind with
  | .Call => true
  | .CallCode => true
  | _ => false

def call_transfers_value (kind : CallKind) : Bool :=
  match kind with
  | .Call => true
  | _ => false

def call_uses_target_address (kind : CallKind) : Bool :=
  match kind with
  | .Call => true
  | .StaticCall => true
  | _ => false

/-- Type quantifiers: k_ex160965_ : Bool -/
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

def frame_succeeded (_ : Unit) : SailM Bool := do
  match (← readReg frame_status) with
  | .Halted (.HaltRevert _) => (pure false)
  | .Halted _ => (pure true)
  | .Running () => (pure true)
  | .Exceptional _ => (pure false)

def advance_pc (delta : code_length) : SailM Unit := do
  writeReg pc (← (byte_quantity_add (← readReg pc) delta))

/-- Type quantifiers: opcode : Nat, 0 ≤ opcode ∧ opcode ≤ 255 -/
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

def read_push (n : code_length) : SailM word := do
  (slice_load_n (← readReg frame_code).bytes (← readReg pc) n)

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


mutual
/-- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_execute (op : ast) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      match op with
      | .STOP () => writeReg frame_status (Halted (HaltStop ()))
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
      | .KECCAK256 () =>
        (do
          let offset_word ← do (pop ())
          let length_word ← do (pop ())
          (charge_keccak_gas (BitVec.toNatInt length_word))
          (charge_memory_expansion offset_word length_word)
          if ((← (is_running ())) : Bool)
          then
            (push
              (← (mem_keccak (ByteQuantity (BitVec.toNatInt offset_word))
                  (ByteQuantity (BitVec.toNatInt length_word)))))
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
          (charge_copy_gas (BitVec.toNatInt length_word))
          (charge_memory_expansion destination_word length_word)
          if ((← (is_running ())) : Bool)
          then
            (slice_copy_word_offset (← readReg calldata)
              (ByteQuantity (BitVec.toNatInt destination_word)) source_word
              (ByteQuantity (BitVec.toNatInt length_word)))
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
          (charge_copy_gas (BitVec.toNatInt length_word))
          (charge_memory_expansion destination_word length_word)
          if ((← (is_running ())) : Bool)
          then
            (slice_copy_word_offset (← readReg frame_code).bytes
              (ByteQuantity (BitVec.toNatInt destination_word)) source_word
              (ByteQuantity (BitVec.toNatInt length_word)))
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
          (charge_copy_gas (BitVec.toNatInt length_word))
          (charge_memory_expansion destination_word length_word)
          if ((← (is_running ())) : Bool)
          then
            (k_code_copy a (ByteQuantity (BitVec.toNatInt destination_word)) source_word
              (ByteQuantity (BitVec.toNatInt length_word)))
          else (pure ()))
      | .EXTCODEHASH () =>
        (do
          let a ← do (pure (word_to_address (← (pop ()))))
          let warm ← do (k_access_account a)
          (charge_constant (account_cost warm))
          if ((← (is_running ())) : Bool)
          then (push (← (k_get_codehash a)))
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
          (charge_copy_gas (BitVec.toNatInt length_word))
          (charge_memory_expansion destination_word length_word)
          if ((← (is_running ())) : Bool)
          then
            (returndata_copy_words (ByteQuantity (BitVec.toNatInt destination_word)) source_word
              length_word)
          else (pure ()))
      | .BLOCKHASH () =>
        (do
          (charge_constant (GasConstant 20))
          (push (← (k_blockhash (← (pop ()))))))
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
      | .POP () =>
        (do
          (charge_constant G_base)
          let _ ← do (pop ())
          (pure ()))
      | .MLOAD () =>
        (do
          (charge_constant G_verylow)
          let offset_word ← do (pop ())
          (charge_memory_expansion offset_word (← (word_of_nat 32)))
          if ((← (is_running ())) : Bool)
          then
            (do
              let offset := (ByteQuantity (BitVec.toNatInt offset_word))
              (push (← (mem_load offset))))
          else (pure ()))
      | .MSTORE () =>
        (do
          (charge_constant G_verylow)
          let offset_word ← do (pop ())
          let v ← do (pop ())
          (charge_memory_expansion offset_word (← (word_of_nat 32)))
          if ((← (is_running ())) : Bool)
          then
            (do
              let offset := (ByteQuantity (BitVec.toNatInt offset_word))
              (mem_store offset v))
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
              let offset := (ByteQuantity (BitVec.toNatInt offset_word))
              (mem_store_byte offset v))
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
          (charge_copy_gas (BitVec.toNatInt length_word))
          (charge_cost (← (memory_expansion_gas required_size)))
          if ((← (is_running ())) : Bool)
          then (expand_memory required_size)
          else (pure ())
          if ((← (is_running ())) : Bool)
          then
            (mem_mcopy (ByteQuantity (BitVec.toNatInt destination_word))
              (ByteQuantity (BitVec.toNatInt source_word))
              (ByteQuantity (BitVec.toNatInt length_word)))
          else (pure ()))
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
                      if ((! (gas_refund_equal refund GAS_REFUND_ZERO)) : Bool)
                      then (record_refund refund)
                      else (pure ())
                      if ((entry.curr != v) : Bool)
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
              (charge_log_gas ⟨n⟩ (BitVec.toNatInt length_word))
              (charge_memory_expansion offset_word length_word)
              if ((← (is_running ())) : Bool)
              then
                (k_log (← (self_addr ())) topics
                  (BytesSlice
                    (← (memory_byte_slice (ByteQuantity (BitVec.toNatInt offset_word))
                        (ByteQuantity (BitVec.toNatInt length_word))))))
              else (pure ())))
      | .CREATE () => (_rec_run_create false _reclimit_pred)
      | .CREATE2 () => (_rec_run_create true _reclimit_pred)
      | .CALL () => (_rec_run_call Call _reclimit_pred)
      | .CALLCODE () => (_rec_run_call CallCode _reclimit_pred)
      | .DELEGATECALL () => (_rec_run_call DelegateCall _reclimit_pred)
      | .STATICCALL () => (_rec_run_call StaticCall _reclimit_pred)
      | .RETURN () =>
        (do
          let offset_word ← do (pop ())
          let length_word ← do (pop ())
          (charge_memory_expansion offset_word length_word)
          if ((← (is_running ())) : Bool)
          then
            writeReg frame_status (Halted
              (HaltReturn
                (← (freeze_output
                    (← (memory_byte_slice (ByteQuantity (BitVec.toNatInt offset_word))
                        (ByteQuantity (BitVec.toNatInt length_word))))))))
          else (pure ()))
      | .REVERT () =>
        (do
          let offset_word ← do (pop ())
          let length_word ← do (pop ())
          (charge_memory_expansion offset_word length_word)
          if ((← (is_running ())) : Bool)
          then
            writeReg frame_status (Halted
              (HaltRevert
                (← (freeze_output
                    (← (memory_byte_slice (ByteQuantity (BitVec.toNatInt offset_word))
                        (ByteQuantity (BitVec.toNatInt length_word))))))))
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
              if (((← (is_running ())) && ((word_nonzero bal) && (← (k_account_is_empty
                         beneficiary)))) : Bool)
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
              else (pure ()))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _
/-- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_interpret (_ : Unit) (_reclimit : Nat) : SailM EvmByteSlice := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      let loop_vars ← whileFuelM (fuel :=(GAS_MAX_VALUE + 1)) (fun () => (is_running ())) ()
        fun () => do
          assert true "loop dummy assert"
          let op ← do (fetch ())
          (_rec_execute op _reclimit_pred)
      (pure loop_vars)
      match (← readReg frame_status) with
      | .Halted (.HaltReturn output) => (pure output)
      | .Halted (.HaltRevert output) => (pure output)
      | _ => (pure EMPTY_SLICE))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _
/-- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_run_call (kind : CallKind) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
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
                    (gas_cost_add_constant (gas_cost_add_constant access_cost transfer_cost)
                      create_cost)
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
                              let .Gas base_child_amount := base_child
                              let .Gas stipend_amount := stipend
                              let child_gas ← (( do
                                if ((stipend_amount ≤b (((2 ^i 63) -i 1) -i base_child_amount)) : Bool)
                                then (pure (Gas (base_child_amount + stipend_amount)))
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
                                                (memory_byte_slice
                                                  (ByteQuantity (BitVec.toNatInt args_off_word))
                                                  (ByteQuantity (BitVec.toNatInt args_len_word)))
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
                                                          (returndata_copy_prefix
                                                            (ByteQuantity
                                                              (BitVec.toNatInt ret_off_word))
                                                            (ByteQuantity
                                                              (BitVec.toNatInt ret_len_word)))
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
                                              let child_addr : (BitVec 160) :=
                                                if ((call_uses_target_address kind) : Bool)
                                                then target
                                                else caller
                                              let child_caller ← (( do
                                                if ((call_is_delegate kind) : Bool)
                                                then (pure (← readReg message).caller)
                                                else (pure caller) ) : SailM (BitVec 160) )
                                              let child_value ← (( do
                                                if ((call_is_delegate kind) : Bool)
                                                then (pure (← readReg message).value)
                                                else (pure value) ) : SailM (BitVec 256) )
                                              let child_static ← (( do
                                                if ((call_is_static kind) : Bool)
                                                then (pure true)
                                                else (pure (← readReg message).is_static) ) :
                                                SailM Bool )
                                              let child_calldata ← do
                                                if ((← (is_running ())) : Bool)
                                                then
                                                  (memory_byte_slice
                                                    (ByteQuantity (BitVec.toNatInt args_off_word))
                                                    (ByteQuantity (BitVec.toNatInt args_len_word)))
                                                else (pure EMPTY_SLICE)
                                              let checkpoint ← do (suspend_frame ())
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
                                              writeReg frame_refund GAS_REFUND_ZERO
                                              writeReg returndata (← (_rec_interpret ()
                                                  _reclimit_pred))
                                              let succeeded ← do (frame_succeeded ())
                                              let child_left ← do readReg gas_remaining
                                              let child_refund ← do readReg frame_refund
                                              (restore_frame checkpoint)
                                              (refund_gas child_left)
                                              (returndata_copy_prefix
                                                (ByteQuantity (BitVec.toNatInt ret_off_word))
                                                (ByteQuantity (BitVec.toNatInt ret_len_word)))
                                              if (succeeded : Bool)
                                              then
                                                (do
                                                  (record_refund child_refund)
                                                  (push WORD_ONE))
                                              else
                                                (do
                                                  (k_revert checkpoint.state)
                                                  (push WORD_ZERO))))
                                      | _ =>
                                        (do
                                          let child_code ← do
                                            (executable_code target tg_deleg tg_target)
                                          let child_addr : (BitVec 160) :=
                                            if ((call_uses_target_address kind) : Bool)
                                            then target
                                            else caller
                                          let child_caller ← (( do
                                            if ((call_is_delegate kind) : Bool)
                                            then (pure (← readReg message).caller)
                                            else (pure caller) ) : SailM (BitVec 160) )
                                          let child_value ← (( do
                                            if ((call_is_delegate kind) : Bool)
                                            then (pure (← readReg message).value)
                                            else (pure value) ) : SailM (BitVec 256) )
                                          let child_static ← (( do
                                            if ((call_is_static kind) : Bool)
                                            then (pure true)
                                            else (pure (← readReg message).is_static) ) : SailM
                                            Bool )
                                          let child_calldata ← do
                                            if ((← (is_running ())) : Bool)
                                            then
                                              (memory_byte_slice
                                                (ByteQuantity (BitVec.toNatInt args_off_word))
                                                (ByteQuantity (BitVec.toNatInt args_len_word)))
                                            else (pure EMPTY_SLICE)
                                          let checkpoint ← do (suspend_frame ())
                                          if (((call_transfers_value kind) && (word_nonzero value)) : Bool)
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
                                          writeReg frame_refund GAS_REFUND_ZERO
                                          writeReg returndata (← (_rec_interpret () _reclimit_pred))
                                          let succeeded ← do (frame_succeeded ())
                                          let child_left ← do readReg gas_remaining
                                          let child_refund ← do readReg frame_refund
                                          (restore_frame checkpoint)
                                          (refund_gas child_left)
                                          (returndata_copy_prefix
                                            (ByteQuantity (BitVec.toNatInt ret_off_word))
                                            (ByteQuantity (BitVec.toNatInt ret_len_word)))
                                          if (succeeded : Bool)
                                          then
                                            (do
                                              (record_refund child_refund)
                                              (push WORD_ONE))
                                          else
                                            (do
                                              (k_revert checkpoint.state)
                                              (push WORD_ZERO))))))))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _
/-- Type quantifiers: _reclimit : Nat, k_ex160967_ : Bool, 0 ≤ _reclimit -/
def _rec_run_create (is2 : Bool) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
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
          let initcode_size : Nat := (BitVec.toNatInt len_word)
          if ((← (guard_static ())) : Bool)
          then (pure ())
          else
            (do
              (charge_memory_expansion off_word len_word)
              (charge_constant G_create)
              (charge_cost (← (initcode_gas initcode_size)))
              if (is2 : Bool)
              then
                (charge_cost
                  (gas_constant_scale G_keccak_word (← (memory_word_count initcode_size))))
              else (pure ())
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
                            (← (mem_keccak (ByteQuantity (BitVec.toNatInt off_word))
                                (ByteQuantity initcode_size))))
                        else (k_create_addr creator ⟨nonce⟩) ) : SailM (BitVec 160) )
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
                                (pure (word_ule value (← (k_get_balance creator)))) ) : SailM Bool
                                )
                              let nonce_ok : Bool := (nonce != ((2 ^i 64) -i 1))
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
                                        then
                                          (memory_byte_slice
                                            (ByteQuantity (BitVec.toNatInt off_word))
                                            (ByteQuantity initcode_size))
                                        else (pure EMPTY_SLICE)
                                      let child_code ← do
                                        (code_db_resolve (← (code_db_insert initcode)))
                                      let checkpoint ← do (suspend_frame ())
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
                                      writeReg frame_refund GAS_REFUND_ZERO
                                      writeReg returndata (← (_rec_interpret () _reclimit_pred))
                                      let succeeded ← do (frame_succeeded ())
                                      let child_left ← do readReg gas_remaining
                                      let child_refund ← do readReg frame_refund
                                      let dep_len ← do (returndata_size ())
                                      let code_ok ← do
                                        (pure ((byte_quantity_le dep_len (← (max_code_size ()))) && ((fork_lt
                                                (← readReg k_fork) London) || ((byte_quantity_equal
                                                  dep_len BYTE_ZERO) || ((← (slice_byte
                                                      (← readReg returndata) BYTE_ZERO)) != 0xEF#8)))))
                                      let deposit :=
                                        (gas_constant_scale_byte_quantity G_codedeposit dep_len)
                                      (restore_frame checkpoint)
                                      (refund_gas child_left)
                                      if ((succeeded && (code_ok && (gas_cost_le_gas deposit
                                               child_left))) : Bool)
                                      then
                                        (do
                                          writeReg gas_remaining (← (gas_sub_cost_or_oog
                                              (← readReg gas_remaining) deposit))
                                          if ((← (is_running ())) : Bool)
                                          then
                                            (do
                                              (record_refund child_refund)
                                              (k_deploy_code new_addr (← readReg returndata))
                                              (push (address_to_word new_addr)))
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
                                      else (pure ()))))))))))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _
end

def execute (_arg0 : ast) : SailM Unit := do
  let _measure ← do (pure ((3 *i ((DEPTH_LIMIT).value -i (← readReg call_depth))) +i 1))
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_execute _arg0 (_measure + 1))

def interpret (_arg0 : Unit) : SailM EvmByteSlice := do
  let _measure ← do (pure ((3 *i ((DEPTH_LIMIT).value -i (← readReg call_depth))) +i 2))
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_interpret _arg0 (_measure + 1))

def run_call (_arg0 : CallKind) : SailM Unit := do
  let _measure ← do (pure (3 *i ((DEPTH_LIMIT).value -i (← readReg call_depth))))
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_run_call _arg0 (_measure + 1))

/-- Type quantifiers: k_ex160969_ : Bool -/
def run_create (_arg0 : Bool) : SailM Unit := do
  let _measure ← do (pure (3 *i ((DEPTH_LIMIT).value -i (← readReg call_depth))))
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_run_create _arg0 (_measure + 1))

