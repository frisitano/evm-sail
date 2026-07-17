import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Quantities
import Evm.Gas
import Evm.Bytes
import Evm.Code
import Evm.ByteSlice
import Evm.Code0
import Evm.Output
import Evm.Environment
import Evm.Storage
import Evm.Logs
import Evm.Accounts
import Evm.Code0
import Evm.Selfdestruct
import Evm.Lifecycle
import Evm.Machine
import Evm.Gas0
import Evm.Precompiles

set_option maxHeartbeats 1_000_000_000
set_option maxRecDepth 1_000_000
set_option linter.unusedVariables false
set_option match.ignoreUnusedAlts true

open Sail
open ConcurrencyInterfaceV1

namespace Evm.Functions

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
open Register
open NodeRef
open MerkleSlot
open JEntry
open HaltKind
open FrameStatus
open Fork
open ExceptionKind
open EnvField
open CallKind
open Bytes
open ByteSource
open BlockError

def self_addr (_ : Unit) : SailM (BitVec 160) := do
  (pure (← readReg message).address)

def guard_static (_ : Unit) : SailM Bool := do
  if ((← readReg message).is_static : Bool)
  then
    (do
      (exc_halt WriteProtection)
      (pure true))
  else (pure false)

def do_jump (dest_word : (BitVec 256)) : SailM Unit := do
  if ((word_ult dest_word (← (word_of_byte_quantity (← (frame_code_len ()))))) : Bool)
  then
    (do
      let destination := (ByteQuantity (BitVec.toNatInt dest_word))
      if ((← (frame_jumpdest_valid destination)) : Bool)
      then writeReg pc destination
      else (exc_halt InvalidJump))
  else (exc_halt InvalidJump)

/-- Type quantifiers: count : Nat, 0 ≤ count ∧ count ≤ 4 -/
def pop_log_topics (count : Nat) : SailM (List (BitVec 256)) := do
  match count with
  | 0 => []
  | 1 => [(pop ())]
  | 2 =>
    (do
      let t0 ← do (pop ())
      let t1 ← do (pop ())
      [(pure t0), (pure t1)])
  | 3 =>
    (do
      let t0 ← do (pop ())
      let t1 ← do (pop ())
      let t2 ← do (pop ())
      [(pure t0), (pure t1), (pure t2)])
  | 4 =>
    (do
      let t0 ← do (pop ())
      let t1 ← do (pop ())
      let t2 ← do (pop ())
      let t3 ← do (pop ())
      [(pure t0), (pure t1), (pure t2), (pure t3)])
  | _ => []

def call_is_delegate (kind : CallKind) : Bool :=
  match kind with
  | DelegateCall => true
  | _ => false

def call_is_static (kind : CallKind) : Bool :=
  match kind with
  | StaticCall => true
  | _ => false

def call_takes_value (kind : CallKind) : Bool :=
  match kind with
  | Call => true
  | CallCode => true
  | _ => false

def call_transfers_value (kind : CallKind) : Bool :=
  match kind with
  | Call => true
  | _ => false

def call_uses_target_address (kind : CallKind) : Bool :=
  match kind with
  | Call => true
  | StaticCall => true
  | _ => false

/-- Type quantifiers: k_ex162194_ : Bool -/
def executable_code (target : (BitVec 160)) (dele : Bool) (dtgt : (BitVec 160)) : SailM Code := do
  if (dele : Bool)
  then
    (do
      match (word_to_precompile_id (address_to_word dtgt)) with
      | .some number =>
        (do
          if ((← (is_precompile number)) : Bool)
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

def advance_pc (delta : byte_quantity) : SailM Unit := do
  writeReg pc (← (byte_quantity_add (← readReg pc) delta))

/-- Type quantifiers: opcode : Nat, 0 ≤ opcode ∧ opcode ≤ 255 -/
def decode_simple (opcode : Nat) : SailM ast := do
  if (((128 ≤b opcode) && (opcode ≤b 143)) : Bool)
  then (pure (DUP (opcode -i 127)))
  else
    (do
      if (((144 ≤b opcode) && (opcode ≤b 159)) : Bool)
      then (pure (SWAP (opcode -i 143)))
      else
        (do
          if (((160 ≤b opcode) && (opcode ≤b 164)) : Bool)
          then (pure (LOG (opcode -i 160)))
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

def read_push (n : byte_quantity) : SailM (BitVec 256) := do
  (slice_load_n (← readReg frame_code).bytes (← readReg pc) n)

def fetch (_ : Unit) : SailM ast := do
  if ((! (byte_quantity_lt (← readReg pc) (← (frame_code_len ())))) : Bool)
  then (pure (STOP ()))
  else
    (do
      let opcode ← (( do
        (pure (BitVec.toNatInt (← (slice_byte (← readReg frame_code).bytes (← readReg pc)))))
        ) : SailM opcode )
      (advance_pc BYTE_ONE)
      if (((opcode == 95) && (fork_lt (← readReg k_fork) Shanghai)) : Bool)
      then (pure (INVALID ()))
      else
        (do
          if (((95 ≤b opcode) && (opcode ≤b 127)) : Bool)
          then
            (do
              let encoded_size : push_width := (opcode -i 95)
              let size : code_length := (ByteQuantity encoded_size)
              let value ← do (read_push size)
              (advance_pc size)
              (pure (PUSH (encoded_size, value))))
          else (decode_simple opcode)))

def DEPTH_LIMIT : frame_depth := 1024


mutual
def execute (op : ast) : SailM Unit := do
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
        (do
          let size := (ByteQuantity (BitVec.toNatInt length_word))
          let start :=
            if ((byte_quantity_equal size BYTE_ZERO) : Bool)
            then BYTE_ZERO
            else (ByteQuantity (BitVec.toNatInt offset_word))
          (push (← (mem_keccak start size))))
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
        (do
          let size := (ByteQuantity (BitVec.toNatInt length_word))
          let destination :=
            if ((byte_quantity_equal size BYTE_ZERO) : Bool)
            then BYTE_ZERO
            else (ByteQuantity (BitVec.toNatInt destination_word))
          (slice_copy_word_offset (← readReg calldata) destination source_word size))
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
        (do
          let size := (ByteQuantity (BitVec.toNatInt length_word))
          let destination :=
            if ((byte_quantity_equal size BYTE_ZERO) : Bool)
            then BYTE_ZERO
            else (ByteQuantity (BitVec.toNatInt destination_word))
          (slice_copy_word_offset (← readReg frame_code).bytes destination source_word size))
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
        (do
          let size := (ByteQuantity (BitVec.toNatInt length_word))
          let destination :=
            if ((byte_quantity_equal size BYTE_ZERO) : Bool)
            then BYTE_ZERO
            else (ByteQuantity (BitVec.toNatInt destination_word))
          (k_code_copy a destination source_word size))
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
        (do
          let size := (ByteQuantity (BitVec.toNatInt length_word))
          let destination :=
            if ((byte_quantity_equal size BYTE_ZERO) : Bool)
            then BYTE_ZERO
            else (ByteQuantity (BitVec.toNatInt destination_word))
          (returndata_copy_words destination source_word length_word))
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
      (push (← (blob_base_fee (← readReg k_header).excess_blob_gas))))
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
        (do
          let size := (ByteQuantity (BitVec.toNatInt length_word))
          let destination :=
            if ((byte_quantity_equal size BYTE_ZERO) : Bool)
            then BYTE_ZERO
            else (ByteQuantity (BitVec.toNatInt destination_word))
          let source :=
            if ((byte_quantity_equal size BYTE_ZERO) : Bool)
            then BYTE_ZERO
            else (ByteQuantity (BitVec.toNatInt source_word))
          (mem_mcopy destination source size))
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
                      { curr := v
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
    (do
      if ((n == 0) : Bool)
      then (charge_constant G_base)
      else (charge_constant G_verylow)
      (push v))
  | .DUP n =>
    (do
      (charge_constant G_verylow)
      if (((← (stack_height ())) <b n) : Bool)
      then (exc_halt StackUnderflow)
      else (push (← (peek (n -i 1)))))
  | .SWAP n =>
    (do
      (charge_constant G_verylow)
      if (((← (stack_height ())) <b (n +i 1)) : Bool)
      then (exc_halt StackUnderflow)
      else
        (do
          let top ← do (peek 0)
          let other ← do (peek n)
          (stack_set 0 other)
          (stack_set n top)))
  | .LOG n =>
    (do
      if ((← (guard_static ())) : Bool)
      then (pure ())
      else
        (do
          let offset_word ← do (pop ())
          let length_word ← do (pop ())
          let topics ← do (pop_log_topics n)
          (charge_log_gas n (BitVec.toNatInt length_word))
          (charge_memory_expansion offset_word length_word)
          if ((← (is_running ())) : Bool)
          then
            (do
              let size := (ByteQuantity (BitVec.toNatInt length_word))
              let start :=
                if ((byte_quantity_equal size BYTE_ZERO) : Bool)
                then BYTE_ZERO
                else (ByteQuantity (BitVec.toNatInt offset_word))
              (k_log (← (self_addr ())) topics (BytesSlice (← (memory_byte_slice start size)))))
          else (pure ())))
  | .CREATE () => (run_create false)
  | .CREATE2 () => (run_create true)
  | .CALL () => (run_call Call)
  | .CALLCODE () => (run_call CallCode)
  | .DELEGATECALL () => (run_call DelegateCall)
  | .STATICCALL () => (run_call StaticCall)
  | .RETURN () =>
    (do
      let offset_word ← do (pop ())
      let length_word ← do (pop ())
      (charge_memory_expansion offset_word length_word)
      if ((← (is_running ())) : Bool)
      then
        (do
          let size := (ByteQuantity (BitVec.toNatInt length_word))
          let start :=
            if ((byte_quantity_equal size BYTE_ZERO) : Bool)
            then BYTE_ZERO
            else (ByteQuantity (BitVec.toNatInt offset_word))
          writeReg frame_status (Halted
            (HaltReturn (← (freeze_output (← (memory_byte_slice start size)))))))
      else (pure ()))
  | .REVERT () =>
    (do
      let offset_word ← do (pop ())
      let length_word ← do (pop ())
      (charge_memory_expansion offset_word length_word)
      if ((← (is_running ())) : Bool)
      then
        (do
          let size := (ByteQuantity (BitVec.toNatInt length_word))
          let start :=
            if ((byte_quantity_equal size BYTE_ZERO) : Bool)
            then BYTE_ZERO
            else (ByteQuantity (BitVec.toNatInt offset_word))
          writeReg frame_status (Halted
            (HaltRevert (← (freeze_output (← (memory_byte_slice start size)))))))
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
def interpret (_ : Unit) : SailM ByteSlice := do
  let mut loop_vars := ()
  while (← (λ _ => do (is_running ())) loop_vars) do
    let () := loop_vars
    loop_vars ← do
      let op ← do (fetch ())
      (execute op)
  (pure loop_vars)
  match (← readReg frame_status) with
  | .Halted (.HaltReturn output) => (pure output)
  | .Halted (.HaltRevert output) => (pure output)
  | _ => (pure EMPTY_SLICE)
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
              let access_cost : gas_cost := (gas_constant_to_cost target_cost)
              let (tg_deleg, tg_target) ← do (k_deleg_target target)
              let access_cost ← (( do
                if (tg_deleg : Bool)
                then
                  (do
                    let dw ← do (k_access_account tg_target)
                    (pure (gas_cost_add_constant access_cost (account_cost dw))))
                else (pure access_cost) ) : SailM gas_cost )
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
                          let args_size := (ByteQuantity (BitVec.toNatInt args_len_word))
                          let args_start :=
                            if ((byte_quantity_equal args_size BYTE_ZERO) : Bool)
                            then BYTE_ZERO
                            else (ByteQuantity (BitVec.toNatInt args_off_word))
                          let ret_size := (ByteQuantity (BitVec.toNatInt ret_len_word))
                          let ret_start :=
                            if ((byte_quantity_equal ret_size BYTE_ZERO) : Bool)
                            then BYTE_ZERO
                            else (ByteQuantity (BitVec.toNatInt ret_off_word))
                          let .Gas base_child_amount := base_child
                          let .Gas stipend_amount := stipend
                          let child_gas ← (( do
                            if ((stipend_amount ≤b (((2 ^i 63) -i 1) -i base_child_amount)) : Bool)
                            then (pure (Gas (base_child_amount +i stipend_amount)))
                            else sailThrow ((InvalidBlock ExecutionInvalid)) ) : SailM gas )
                          if ((! (← (is_running ()))) : Bool)
                          then (pure ())
                          else
                            (do
                              let _ ← do (k_aload target)
                              let depth_ok ← do (pure ((← readReg call_depth) <b DEPTH_LIMIT))
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
                                  match (word_to_precompile_id (address_to_word target)) with
                                  | .some precompile_number =>
                                    (do
                                      if ((← (is_precompile precompile_number)) : Bool)
                                      then
                                        (do
                                          let input ← do (memory_byte_slice args_start args_size)
                                          match (← (precompile_gas precompile_number input)) with
                                          | .some used =>
                                            (do
                                              if ((gas_cost_le_gas used child_gas) : Bool)
                                              then
                                                (do
                                                  let result ← do
                                                    (run_precompile_slice precompile_number input)
                                                  if (result.success : Bool)
                                                  then
                                                    (do
                                                      writeReg returndata result.output
                                                      if (((call_transfers_value kind) && (word_nonzero
                                                             value)) : Bool)
                                                      then (k_transfer caller target value)
                                                      else (pure ())
                                                      (returndata_copy_prefix ret_start ret_size)
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
                                            else (pure (← readReg message).is_static) ) : SailM
                                            Bool )
                                          let child_calldata ← do
                                            if ((← (is_running ())) : Bool)
                                            then (memory_byte_slice args_start args_size)
                                            else (pure EMPTY_SLICE)
                                          let checkpoint ← do (suspend_frame ())
                                          if (((call_transfers_value kind) && (word_nonzero value)) : Bool)
                                          then (k_transfer caller target value)
                                          else (pure ())
                                          writeReg message { caller := child_caller
                                                             address := child_addr
                                                             code_address := target
                                                             value := child_value
                                                             is_static := child_static
                                                             depth := ← (frame_depth_increment
                                                                 checkpoint.call_depth) }
                                          (calldata_install child_calldata)
                                          writeReg pc BYTE_ZERO
                                          writeReg gas_remaining child_gas
                                          writeReg frame_status (Running ())
                                          (returndata_clear ())
                                          writeReg frame_code child_code
                                          writeReg call_depth (← (frame_depth_increment
                                              checkpoint.call_depth))
                                          writeReg frame_refund GAS_REFUND_ZERO
                                          writeReg returndata (← (interpret ()))
                                          let succeeded ← do (frame_succeeded ())
                                          let child_left ← do readReg gas_remaining
                                          let child_refund ← do readReg frame_refund
                                          (restore_frame checkpoint)
                                          (refund_gas child_left)
                                          (returndata_copy_prefix ret_start ret_size)
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
                                        else (pure (← readReg message).is_static) ) : SailM Bool )
                                      let child_calldata ← do
                                        if ((← (is_running ())) : Bool)
                                        then (memory_byte_slice args_start args_size)
                                        else (pure EMPTY_SLICE)
                                      let checkpoint ← do (suspend_frame ())
                                      if (((call_transfers_value kind) && (word_nonzero value)) : Bool)
                                      then (k_transfer caller target value)
                                      else (pure ())
                                      writeReg message { caller := child_caller
                                                         address := child_addr
                                                         code_address := target
                                                         value := child_value
                                                         is_static := child_static
                                                         depth := ← (frame_depth_increment
                                                             checkpoint.call_depth) }
                                      (calldata_install child_calldata)
                                      writeReg pc BYTE_ZERO
                                      writeReg gas_remaining child_gas
                                      writeReg frame_status (Running ())
                                      (returndata_clear ())
                                      writeReg frame_code child_code
                                      writeReg call_depth (← (frame_depth_increment
                                          checkpoint.call_depth))
                                      writeReg frame_refund GAS_REFUND_ZERO
                                      writeReg returndata (← (interpret ()))
                                      let succeeded ← do (frame_succeeded ())
                                      let child_left ← do readReg gas_remaining
                                      let child_refund ← do readReg frame_refund
                                      (restore_frame checkpoint)
                                      (refund_gas child_left)
                                      (returndata_copy_prefix ret_start ret_size)
                                      if (succeeded : Bool)
                                      then
                                        (do
                                          (record_refund child_refund)
                                          (push WORD_ONE))
                                      else
                                        (do
                                          (k_revert checkpoint.state)
                                          (push WORD_ZERO)))))))))))
/-- Type quantifiers: k_ex162224_ : Bool -/
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
            (charge_cost (gas_constant_scale G_keccak_word (← (memory_word_count initcode_size))))
          else (pure ())
          if ((! (← (is_running ()))) : Bool)
          then (pure ())
          else
            (do
              if ((! (← (initcode_size_allowed initcode_size))) : Bool)
              then (exc_halt InitCodeTooLarge)
              else
                (do
                  let initcode_length := (ByteQuantity initcode_size)
                  let initcode_start :=
                    if ((byte_quantity_equal initcode_length BYTE_ZERO) : Bool)
                    then BYTE_ZERO
                    else (ByteQuantity (BitVec.toNatInt off_word))
                  let nonce ← do (k_get_nonce creator)
                  let new_addr ← (( do
                    if ((is2 && (← (is_running ()))) : Bool)
                    then
                      (k_create2_addr creator salt (← (mem_keccak initcode_start initcode_length)))
                    else (k_create_addr creator nonce) ) : SailM address )
                  let avail ← do readReg gas_remaining
                  let retained_gas ← do (gas_quotient avail 64)
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
                          let depth_ok ← do (pure ((← readReg call_depth) <b DEPTH_LIMIT))
                          let balance_ok ← (( do
                            (pure (word_ule value (← (k_get_balance creator)))) ) : SailM Bool )
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
                              let occupied ← (( do (k_account_occupied new_addr) ) : SailM Bool )
                              (returndata_clear ())
                              (k_bump_nonce creator)
                              if (occupied : Bool)
                              then (push WORD_ZERO)
                              else
                                (do
                                  let initcode ← do
                                    if ((← (is_running ())) : Bool)
                                    then (memory_byte_slice initcode_start initcode_length)
                                    else (pure EMPTY_SLICE)
                                  let child_code ← do
                                    (code_db_resolve (← (code_db_insert initcode)))
                                  let checkpoint ← do (suspend_frame ())
                                  (k_mark_created new_addr)
                                  (k_clear_storage new_addr)
                                  (k_bump_nonce new_addr)
                                  (k_transfer creator new_addr value)
                                  writeReg message { caller := creator
                                                     address := new_addr
                                                     code_address := new_addr
                                                     value := value
                                                     is_static := checkpoint.message.is_static
                                                     depth := ← (frame_depth_increment
                                                         checkpoint.call_depth) }
                                  (calldata_install EMPTY_SLICE)
                                  writeReg pc BYTE_ZERO
                                  writeReg gas_remaining child_gas
                                  writeReg frame_status (Running ())
                                  (returndata_clear ())
                                  writeReg frame_code child_code
                                  writeReg call_depth (← (frame_depth_increment
                                      checkpoint.call_depth))
                                  writeReg frame_refund GAS_REFUND_ZERO
                                  writeReg returndata (← (interpret ()))
                                  let succeeded ← do (frame_succeeded ())
                                  let child_left ← do readReg gas_remaining
                                  let child_refund ← do readReg frame_refund
                                  let dep_len ← do (returndata_size ())
                                  let code_ok ← do
                                    (pure ((byte_quantity_le dep_len (← (max_code_size ()))) && ((fork_lt
                                            (← readReg k_fork) London) || ((byte_quantity_equal
                                              dep_len BYTE_ZERO) || ((← (slice_byte
                                                  (← readReg returndata) BYTE_ZERO)) != 0xEF#8)))))
                                  let .ByteQuantity deployed_size := dep_len
                                  let deposit := (gas_constant_scale G_codedeposit deployed_size)
                                  (restore_frame checkpoint)
                                  (refund_gas child_left)
                                  if ((succeeded && (code_ok && (gas_cost_le_gas deposit child_left))) : Bool)
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
                                  else (pure ())))))))))
end

