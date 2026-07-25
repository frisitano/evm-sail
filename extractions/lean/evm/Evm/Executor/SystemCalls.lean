import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Primitives.Crypto
import Evm.Primitives.System
import Evm.Host.EvmByteSlice
import Evm.Host.Kernel.Scratch
import Evm.Host.Code
import Evm.Host.Kernel.Environment
import Evm.Host.Kernel.Code
import Evm.Host.Kernel.Lifecycle
import Evm.Evm.Machine
import Evm.Evm.Interpreter

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

/-! # Protocol system calls

The block-level system calls the protocol issues around the transactions:
the EIP-4788 / EIP-2935 block-start writes (unchecked — skipped if the
contract is absent) and the EIP-7002 / EIP-7251 / EIP-8282 block-end
request dequeues (checked — absence or failure invalidates the block). Each is a
30M-gas frame from `SYSTEM_ADDRESS` whose committed storage writes shape
the post-state root.

## Constants

The deposit offsets describe the ABI-encoded `DepositEvent` payload, while
`SYSTEM_CALL_INPUT_LENGTH` is the fixed input length of each block-start
system call. -/

def SYSTEM_CALL_INPUT_LENGTH : Nat := WORD_BYTE_LENGTH

def DEPOSIT_EVENT_DATA_LENGTH : Nat := 576

def DEPOSIT_PUBKEY_HEAD : Nat := 0

def DEPOSIT_WITHDRAWAL_CREDENTIALS_HEAD : Nat := 32

def DEPOSIT_AMOUNT_HEAD : Nat := 64

def DEPOSIT_SIGNATURE_HEAD : Nat := 96

def DEPOSIT_INDEX_HEAD : Nat := 128

def DEPOSIT_PUBKEY_LENGTH_WORD : Nat := 160

def DEPOSIT_PUBKEY_DATA : Nat := 192

def DEPOSIT_PUBKEY_LENGTH : Nat := 48

def DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH_WORD : Nat := 256

def DEPOSIT_WITHDRAWAL_CREDENTIALS_DATA : Nat := 288

def DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH : Nat := WORD_BYTE_LENGTH

def DEPOSIT_AMOUNT_LENGTH_WORD : Nat := 320

def DEPOSIT_AMOUNT_DATA : Nat := 352

def DEPOSIT_AMOUNT_LENGTH : Nat := EIGHT_BYTE_LENGTH

def DEPOSIT_SIGNATURE_LENGTH_WORD : Nat := 384

def DEPOSIT_SIGNATURE_DATA : Nat := 416

def DEPOSIT_SIGNATURE_LENGTH : Nat := 96

def DEPOSIT_INDEX_LENGTH_WORD : Nat := 512

def DEPOSIT_INDEX_DATA : Nat := 544

def DEPOSIT_INDEX_LENGTH : Nat := EIGHT_BYTE_LENGTH

/-- Enters a top-level protocol system-call frame. The caller has already
made a fresh memory frame and, for a word input, frozen the
parent-memory span that `input` references. -/
/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, 0 ≤
  input_dependentWitness0 ∧ 0 ≤ input_dependentWitness1 -/
def enter_system_call_frame (tgt : (Vector (BitVec 8) 20)) (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM Nat := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  let checkpoint ← do (k_state_checkpoint ())
  writeReg pc 0
  writeReg call_depth 0
  writeReg gas_remaining SYSTEM_CALL_GAS_LIMIT
  writeReg state_gas_remaining GAS_ZERO
  writeReg state_gas_spilled STATE_GAS_SPILL_ZERO
  writeReg frame_refund GAS_REFUND_ZERO
  (stack_reset ())
  (returndata_clear ())
  writeReg frame_status (Running ())
  writeReg calldata ⟨_, ⟨_, input⟩⟩
  writeReg message { caller := SYSTEM_ADDRESS,
                     address := tgt,
                     code_address := tgt,
                     value := ZERO_WORD,
                     state_gas_reservoir := GAS_ZERO,
                     is_static := false,
                     depth := 0 }
  writeReg frame_code (← (code_db_resolve (← (k_code_key tgt))))
  (pure checkpoint)

/-- Issues one unchecked block-start system call: a 30M-gas frame from
`SYSTEM_ADDRESS` with a 32-byte input; skipped when the target has no
code, and its output is discarded. -/
def system_call (tgt : (Vector (BitVec 8) 20)) (input : (Vector (BitVec 8) 32)) : SailM Unit := do
  if (((← (k_code_key tgt)) == KECCAK_EMPTY) : Bool)
  then (pure ())
  else
    (do
      (memory_reset ())
      let input_range : (Sigma fun (k_off : Nat) =>
        (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len))) :=
        ((⟨_, ⟨_, (memory_range 0 SYSTEM_CALL_INPUT_LENGTH)⟩⟩ : (Sigma fun (k_off : Nat) =>
        (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
        (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len))))
      let ⟨_, ⟨_, _⟩⟩ ← do (memory_expand_to ((input_range).2).2.len)
      (mem_store_word ((input_range).2).2.off (hash_to_word input))
      let ⟨_, ⟨_, input_slice⟩⟩ ← do
        (memory_byte_slice ((input_range).2).2.off ((input_range).2).2.len)
      let ⟨_, ⟨_, parent_memory⟩⟩ ← do (memory_frame_enter ())
      let checkpoint ← do (enter_system_call_frame tgt ⟨_, ⟨_, input_slice⟩⟩)
      let ⟨_, ⟨_, _⟩⟩ ← do (interpret ())
      (memory_frame_leave ⟨_, ⟨_, parent_memory⟩⟩)
      if ((! (← (frame_succeeded ()))) : Bool)
      then (k_revert checkpoint)
      else (pure ())
      (k_tx_merge ()))

/-- Issues one checked block-end system call (EIP-7002/EIP-7251/EIP-8282): the
target must exist and the call must succeed. Missing code or frame failure
throws immediately; a successful call returns the dequeued requests. -/
def system_call_checked (tgt : (Vector (BitVec 8) 20)) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) := do
  if _sailIf0 : (((← (k_code_key tgt)) == KECCAK_EMPTY) : Bool) = true
  then
    (do
      sailThrow ((InvalidBlock ExecutionInvalid)))
  else
    (do
      (memory_reset ())
      let ⟨_, ⟨_, parent_memory⟩⟩ ← do (memory_frame_enter ())
      let checkpoint ← do (enter_system_call_frame tgt ⟨_, ⟨_, EMPTY_SLICE⟩⟩)
      let ⟨_, ⟨_, output⟩⟩ ← do (interpret ())
      if _sailIf1 : ((← (frame_succeeded ())) : Bool) = true
      then
        (do
          let start ← do (scratch_begin ())
          (scratch_push_slice ⟨_, ⟨_, output⟩⟩)
          let ⟨_, ⟨_, result⟩⟩ ← do (scratch_finish start)
          (memory_frame_leave ⟨_, ⟨_, parent_memory⟩⟩)
          (k_tx_merge ())
          (pure ((⟨_, ⟨_, result⟩⟩ : (Sigma fun (k_off : Nat) =>
            (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat)
            => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))))
      else
        (do
          (memory_frame_leave ⟨_, ⟨_, parent_memory⟩⟩)
          (k_revert checkpoint)
          (k_tx_merge ())
          sailThrow ((InvalidBlock ExecutionInvalid))))

/-- Whether a log is a `DepositEvent` from the deposit contract
(EIP-6110). -/
def deposit_log_matches (log : LogEntry) : Bool :=
  if ((bne log.address DEPOSIT_CONTRACT_ADDR) : Bool)
  then false
  else
    (match log.topics with
    | (topic :: _) => (topic == DEPOSIT_EVENT_TOPIC)
    | [] => false)

/-- Strips the ABI framing from one `DepositEvent` and appends the
consensus-layer deposit record's fields directly to the requests
scratch (EIP-6110). -/
/- Type quantifiers: data_dependentWitness1 : Nat, data_dependentWitness0 : Nat, 0 ≤
  data_dependentWitness0 ∧ 0 ≤ data_dependentWitness1 -/
def append_deposit_request (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM Unit := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  if ((data.len != DEPOSIT_EVENT_DATA_LENGTH) : Bool)
  then sailThrow ((InvalidBlock InvalidExecutionRequests))
  else
    (do
      if ((← if (((← (slice_load ⟨_, ⟨_, data⟩⟩ DEPOSIT_PUBKEY_HEAD)) != (U256 160)) : Bool)
           then (pure true)
           else
             (do
               if (((← (slice_load ⟨_, ⟨_, data⟩⟩ DEPOSIT_WITHDRAWAL_CREDENTIALS_HEAD)) != (U256
                      256)) : Bool)
               then (pure true)
               else
                 (do
                   if (((← (slice_load ⟨_, ⟨_, data⟩⟩ DEPOSIT_AMOUNT_HEAD)) != (U256 320)) : Bool)
                   then (pure true)
                   else
                     (do
                       if (((← (slice_load ⟨_, ⟨_, data⟩⟩ DEPOSIT_SIGNATURE_HEAD)) != (U256
                              384)) : Bool)
                       then (pure true)
                       else
                         (do
                           if (((← (slice_load ⟨_, ⟨_, data⟩⟩ DEPOSIT_INDEX_HEAD)) != (U256
                                  512)) : Bool)
                           then (pure true)
                           else
                             (do
                               if (((← (slice_load ⟨_, ⟨_, data⟩⟩
                                        DEPOSIT_PUBKEY_LENGTH_WORD)) != (U256 48)) : Bool)
                               then (pure true)
                               else
                                 (do
                                   if (((← (slice_load ⟨_, ⟨_, data⟩⟩
                                            DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH_WORD)) != (U256 32)) : Bool)
                                   then (pure true)
                                   else
                                     (do
                                       if (((← (slice_load ⟨_, ⟨_, data⟩⟩
                                                DEPOSIT_AMOUNT_LENGTH_WORD)) != (U256 8)) : Bool)
                                       then (pure true)
                                       else
                                         (do
                                           if (((← (slice_load ⟨_, ⟨_, data⟩⟩
                                                    DEPOSIT_SIGNATURE_LENGTH_WORD)) != (U256 96)) : Bool)
                                           then (pure true)
                                           else
                                             (do
                                               (pure ((← (slice_load ⟨_, ⟨_, data⟩⟩
                                                       DEPOSIT_INDEX_LENGTH_WORD)) != (U256 8))))))))))))) : Bool)
      then sailThrow ((InvalidBlock InvalidExecutionRequests))
      else
        (do
          (scratch_push_slice
            ⟨_, ⟨_, (sub_slice data DEPOSIT_PUBKEY_DATA DEPOSIT_PUBKEY_LENGTH)⟩⟩)
          (scratch_push_slice
            ⟨_, ⟨_, (sub_slice data DEPOSIT_WITHDRAWAL_CREDENTIALS_DATA
              DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH)⟩⟩)
          (scratch_push_slice
            ⟨_, ⟨_, (sub_slice data DEPOSIT_AMOUNT_DATA DEPOSIT_AMOUNT_LENGTH)⟩⟩)
          (scratch_push_slice
            ⟨_, ⟨_, (sub_slice data DEPOSIT_SIGNATURE_DATA DEPOSIT_SIGNATURE_LENGTH)⟩⟩)
          (scratch_push_slice
            ⟨_, ⟨_, (sub_slice data DEPOSIT_INDEX_DATA DEPOSIT_INDEX_LENGTH)⟩⟩)))

/-- Appends every matching deposit log in emission order. -/
def append_deposit_logs (logs : (List LogEntry)) : SailM Unit := do
  match logs with
  | [] => (pure ())
  | (log :: rest) =>
    (do
      if ((deposit_log_matches log) : Bool)
      then (append_deposit_request log.data)
      else (pure ())
      (append_deposit_logs rest))

/-- Collects the EIP-7685 execution requests at block end in request-type
order: deposits from logs (EIP-6110), withdrawal (EIP-7002),
consolidation (EIP-7251), and, from Amsterdam, builder deposit and
builder exit (EIP-8282). -/
/- Type quantifiers: deposits_dependentWitness1 : Nat, deposits_dependentWitness0 : Nat, 0 ≤
  deposits_dependentWitness0 ∧ 0 ≤ deposits_dependentWitness1 -/
def collect_execution_requests (deposits : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : SailM ExecutionRequests := do
  let deposits_dependentWitness0 := (deposits).1
  let deposits_dependentWitness1 := ((deposits).2).1
  let deposits := ((deposits).2).2
  let ⟨_, ⟨_, withdrawals⟩⟩ ← do (system_call_checked WITHDRAWAL_REQUEST_ADDR)
  let ⟨_, ⟨_, consolidations⟩⟩ ← do (system_call_checked CONSOLIDATION_REQUEST_ADDR)
  let ⟨_, ⟨_, builder_deposits⟩⟩ ← (( do
    if _sailIf0 : ((fork_gteq (← readReg k_fork) Amsterdam) : Bool) = true
    then
      (do
        (system_call_checked BUILDER_DEPOSIT_REQUEST_ADDR))
    else
      (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (deposits_dependentWitness0 : Nat) =>
        (Sigma fun (deposits_dependentWitness1 : Nat) =>
        (EvmByteSliceFields deposits_dependentWitness0 deposits_dependentWitness1)))) : (Sigma fun
        (deposits_dependentWitness0 : Nat) =>
        (Sigma fun (deposits_dependentWitness1 : Nat) =>
        (EvmByteSliceFields deposits_dependentWitness0 deposits_dependentWitness1))))) ) : SailM
    (Sigma fun (deposits_dependentWitness0 : Nat) =>
    (Sigma fun (deposits_dependentWitness1 : Nat) =>
    (EvmByteSliceFields deposits_dependentWitness0 deposits_dependentWitness1))) )
  let ⟨_, ⟨_, builder_exits⟩⟩ ← (( do
    if _sailIf0 : ((fork_gteq (← readReg k_fork) Amsterdam) : Bool) = true
    then
      (do
        (system_call_checked BUILDER_EXIT_REQUEST_ADDR))
    else
      (pure ((⟨_, ⟨_, EMPTY_SLICE⟩⟩ : (Sigma fun (deposits_dependentWitness0 : Nat) =>
        (Sigma fun (deposits_dependentWitness1 : Nat) =>
        (EvmByteSliceFields deposits_dependentWitness0 deposits_dependentWitness1)))) : (Sigma fun
        (deposits_dependentWitness0 : Nat) =>
        (Sigma fun (deposits_dependentWitness1 : Nat) =>
        (EvmByteSliceFields deposits_dependentWitness0 deposits_dependentWitness1))))) ) : SailM
    (Sigma fun (deposits_dependentWitness0 : Nat) =>
    (Sigma fun (deposits_dependentWitness1 : Nat) =>
    (EvmByteSliceFields deposits_dependentWitness0 deposits_dependentWitness1))) )
  (pure { deposits := ⟨_, ⟨_, deposits⟩⟩,
          withdrawals := ⟨_, ⟨_, withdrawals⟩⟩,
          consolidations := ⟨_, ⟨_, consolidations⟩⟩,
          builder_deposits := ⟨_, ⟨_, builder_deposits⟩⟩,
          builder_exits := ⟨_, ⟨_, builder_exits⟩⟩ })

