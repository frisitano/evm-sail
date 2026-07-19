import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Primitives.Crypto
import Evm.Primitives.System
import Evm.Host.EvmByteSlice
import Evm.Host.Kernel.Scratch
import Evm.Host.Code
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

/-! # Protocol system calls

The block-level system calls the protocol issues around the transactions:
the EIP-4788 / EIP-2935 block-start writes (unchecked — skipped if the
contract is absent) and the EIP-7002 / EIP-7251 block-end request
dequeues (checked — absence or failure invalidates the block). Each is a
30M-gas frame from `SYSTEM_ADDRESS` whose committed storage writes shape
the post-state root.

## Constants

The deposit offsets describe the ABI-encoded `DepositEvent` payload, while
`SYSTEM_CALL_INPUT_LENGTH` is the fixed input length of each block-start
system call. -/

def SYSTEM_CALL_INPUT_LENGTH : byte_length := WORD_BYTE_LENGTH

def DEPOSIT_EVENT_DATA_LENGTH : byte_length := (ByteQuantity 576)

def DEPOSIT_PUBKEY_HEAD : source_pointer := BYTE_ZERO

def DEPOSIT_WITHDRAWAL_CREDENTIALS_HEAD : source_pointer := (ByteQuantity 32)

def DEPOSIT_AMOUNT_HEAD : source_pointer := (ByteQuantity 64)

def DEPOSIT_SIGNATURE_HEAD : source_pointer := (ByteQuantity 96)

def DEPOSIT_INDEX_HEAD : source_pointer := (ByteQuantity 128)

def DEPOSIT_PUBKEY_LENGTH_WORD : source_pointer := (ByteQuantity 160)

def DEPOSIT_PUBKEY_DATA : source_pointer := (ByteQuantity 192)

def DEPOSIT_PUBKEY_LENGTH : byte_length := (ByteQuantity 48)

def DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH_WORD : source_pointer := (ByteQuantity 256)

def DEPOSIT_WITHDRAWAL_CREDENTIALS_DATA : source_pointer := (ByteQuantity 288)

def DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH : byte_length := WORD_BYTE_LENGTH

def DEPOSIT_AMOUNT_LENGTH_WORD : source_pointer := (ByteQuantity 320)

def DEPOSIT_AMOUNT_DATA : source_pointer := (ByteQuantity 352)

def DEPOSIT_AMOUNT_LENGTH : byte_length := EIGHT_BYTE_LENGTH

def DEPOSIT_SIGNATURE_LENGTH_WORD : source_pointer := (ByteQuantity 384)

def DEPOSIT_SIGNATURE_DATA : source_pointer := (ByteQuantity 416)

def DEPOSIT_SIGNATURE_LENGTH : byte_length := (ByteQuantity 96)

def DEPOSIT_INDEX_LENGTH_WORD : source_pointer := (ByteQuantity 512)

def DEPOSIT_INDEX_DATA : source_pointer := (ByteQuantity 544)

def DEPOSIT_INDEX_LENGTH : byte_length := EIGHT_BYTE_LENGTH

/-- Enters a top-level protocol system-call frame. The caller has already
made a fresh memory frame and, for a word input, frozen the
parent-memory span that `input` references. -/
def enter_system_call_frame (tgt : address) (input : EvmByteSlice) : SailM StateCheckpoint := do
  let checkpoint ← do (k_state_checkpoint ())
  writeReg pc BYTE_ZERO
  writeReg call_depth 0
  writeReg gas_remaining (Gas 30000000)
  writeReg frame_refund GAS_REFUND_ZERO
  (stack_reset ())
  (returndata_clear ())
  writeReg frame_status (Running ())
  writeReg calldata input
  writeReg message { caller := SYSTEM_ADDRESS,
                     address := tgt,
                     code_address := tgt,
                     value := ZERO_WORD,
                     is_static := false,
                     depth := ⟨0⟩ }
  writeReg frame_code (← (code_db_resolve (← (k_code_key tgt))))
  (pure checkpoint)

/-- Issues one unchecked block-start system call: a 30M-gas frame from
`SYSTEM_ADDRESS` with a 32-byte input; skipped when the target has no
code, and its output is discarded. -/
def system_call (tgt : address) (input : hash) : SailM Unit := do
  if (((← (k_code_key tgt)) == KECCAK_EMPTY) : Bool)
  then (pure ())
  else
    (do
      (memory_reset ())
      (memory_expand_to SYSTEM_CALL_INPUT_LENGTH)
      (mem_store_word BYTE_ZERO (hash_to_word input))
      let input_slice ← do (memory_byte_slice BYTE_ZERO SYSTEM_CALL_INPUT_LENGTH)
      let parent_memory ← do (memory_frame_enter ())
      let checkpoint ← do (enter_system_call_frame tgt input_slice)
      let _ ← do (interpret ())
      (memory_frame_leave parent_memory)
      if ((! (← (frame_succeeded ()))) : Bool)
      then (k_revert checkpoint)
      else (pure ())
      (k_tx_merge ()))

/-- Issues one checked block-end system call (EIP-7002/EIP-7251): the
target must exist and the call must succeed — otherwise the block is
invalid — and the returned dequeued requests are the call's output. -/
def system_call_checked (tgt : address) : SailM (Option EvmByteSlice) := do
  if (((← (k_code_key tgt)) == KECCAK_EMPTY) : Bool)
  then (pure none)
  else
    (do
      (memory_reset ())
      let parent_memory ← do (memory_frame_enter ())
      let checkpoint ← do (enter_system_call_frame tgt EMPTY_SLICE)
      let output ← do (interpret ())
      let ok ← do (frame_succeeded ())
      let result ← (( do
        if (ok : Bool)
        then
          (do
            let start ← do (scratch_begin ())
            (scratch_push_slice output)
            (pure (some (← (scratch_finish start)))))
        else (pure none) ) : SailM (Option EvmByteSlice) )
      (memory_frame_leave parent_memory)
      if ((! ok) : Bool)
      then (k_revert checkpoint)
      else (pure ())
      (k_tx_merge ())
      (pure result))

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
def append_deposit_request (data : EvmByteSlice) : SailM Unit := do
  if (((bne data.len DEPOSIT_EVENT_DATA_LENGTH) || ((bne (← (slice_load data DEPOSIT_PUBKEY_HEAD))
           (← (word_of_protocol_quantity ⟨160⟩))) || ((bne
             (← (slice_load data DEPOSIT_WITHDRAWAL_CREDENTIALS_HEAD))
             (← (word_of_protocol_quantity ⟨256⟩))) || ((bne
               (← (slice_load data DEPOSIT_AMOUNT_HEAD))
               (← (word_of_protocol_quantity ⟨320⟩))) || ((bne
                 (← (slice_load data DEPOSIT_SIGNATURE_HEAD))
                 (← (word_of_protocol_quantity ⟨384⟩))) || ((bne
                   (← (slice_load data DEPOSIT_INDEX_HEAD))
                   (← (word_of_protocol_quantity ⟨512⟩))) || ((bne
                     (← (slice_load data DEPOSIT_PUBKEY_LENGTH_WORD))
                     (← (word_of_protocol_quantity ⟨48⟩))) || ((bne
                       (← (slice_load data DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH_WORD))
                       (← (word_of_protocol_quantity ⟨32⟩))) || ((bne
                         (← (slice_load data DEPOSIT_AMOUNT_LENGTH_WORD))
                         (← (word_of_protocol_quantity ⟨8⟩))) || ((bne
                           (← (slice_load data DEPOSIT_SIGNATURE_LENGTH_WORD))
                           (← (word_of_protocol_quantity ⟨96⟩))) || (bne
                           (← (slice_load data DEPOSIT_INDEX_LENGTH_WORD))
                           (← (word_of_protocol_quantity ⟨8⟩))))))))))))) : Bool)
  then sailThrow ((InvalidBlock InvalidExecutionRequests))
  else (pure ())
  (scratch_push_slice (← (sub_slice data DEPOSIT_PUBKEY_DATA DEPOSIT_PUBKEY_LENGTH)))
  (scratch_push_slice
    (← (sub_slice data DEPOSIT_WITHDRAWAL_CREDENTIALS_DATA DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH)))
  (scratch_push_slice (← (sub_slice data DEPOSIT_AMOUNT_DATA DEPOSIT_AMOUNT_LENGTH)))
  (scratch_push_slice (← (sub_slice data DEPOSIT_SIGNATURE_DATA DEPOSIT_SIGNATURE_LENGTH)))
  (scratch_push_slice (← (sub_slice data DEPOSIT_INDEX_DATA DEPOSIT_INDEX_LENGTH)))

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

/-- Collects the EIP-7685 execution requests at block end: deposits from
logs (EIP-6110), then the withdrawal (EIP-7002) and consolidation
(EIP-7251) queue dequeues. -/
def collect_execution_requests (deposits : EvmByteSlice) : SailM (Bool × ExecutionRequests) := do
  match (← (system_call_checked WITHDRAWAL_REQUEST_ADDR)) with
  | none =>
    (pure (false, { deposits := deposits,
                    withdrawals := EMPTY_SLICE,
                    consolidations := EMPTY_SLICE }))
  | .some withdrawals =>
    (do
      match (← (system_call_checked CONSOLIDATION_REQUEST_ADDR)) with
      | none =>
        (pure (false, { deposits := deposits,
                        withdrawals := withdrawals,
                        consolidations := EMPTY_SLICE }))
      | .some consolidations =>
        (pure (true, { deposits := deposits,
                       withdrawals := withdrawals,
                       consolidations := consolidations })))

