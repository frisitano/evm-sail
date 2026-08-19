import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Host.RegionAccess
import Evm.Kernel.Scratch
import Evm.Primitives.Crypto
import Evm.Primitives.Fork
import Evm.Primitives.System
import Evm.Primitives.Tx
import Evm.Host.Code
import Evm.Host.Stack
import Evm.Kernel.Environment
import Evm.Kernel.Logs
import Evm.Kernel.Code
import Evm.Kernel.Lifecycle
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

abbrev SYSTEM_CALL_INPUT_LENGTH : Nat := 32

abbrev DEPOSIT_EVENT_DATA_LENGTH : Nat := 576

abbrev DEPOSIT_PUBKEY_HEAD : Nat := 0

abbrev DEPOSIT_WITHDRAWAL_CREDENTIALS_HEAD : Nat := 32

abbrev DEPOSIT_AMOUNT_HEAD : Nat := 64

abbrev DEPOSIT_SIGNATURE_HEAD : Nat := 96

abbrev DEPOSIT_INDEX_HEAD : Nat := 128

abbrev DEPOSIT_PUBKEY_LENGTH_WORD : Nat := 160

abbrev DEPOSIT_PUBKEY_DATA : Nat := 192

abbrev DEPOSIT_PUBKEY_LENGTH : Nat := 48

abbrev DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH_WORD : Nat := 256

abbrev DEPOSIT_WITHDRAWAL_CREDENTIALS_DATA : Nat := 288

abbrev DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH : Nat := 32

abbrev DEPOSIT_AMOUNT_LENGTH_WORD : Nat := 320

abbrev DEPOSIT_AMOUNT_DATA : Nat := 352

abbrev DEPOSIT_AMOUNT_LENGTH : Nat := 8

abbrev DEPOSIT_SIGNATURE_LENGTH_WORD : Nat := 384

abbrev DEPOSIT_SIGNATURE_DATA : Nat := 416

abbrev DEPOSIT_SIGNATURE_LENGTH : Nat := 96

abbrev DEPOSIT_INDEX_LENGTH_WORD : Nat := 512

abbrev DEPOSIT_INDEX_DATA : Nat := 544

abbrev DEPOSIT_INDEX_LENGTH : Nat := 8

abbrev DEPOSIT_REQUEST_LENGTH : Nat := 192

abbrev DEPOSIT_REQUEST_PUBKEY : Nat := 0

abbrev DEPOSIT_REQUEST_WITHDRAWAL_CREDENTIALS : Nat := 48

abbrev DEPOSIT_REQUEST_AMOUNT : Nat := 80

abbrev DEPOSIT_REQUEST_SIGNATURE : Nat := 88

abbrev DEPOSIT_REQUEST_INDEX : Nat := 184

/-- Runs a top-level protocol system-call frame. The caller has already
resolved `code`, made a fresh memory frame and, for a word input, frozen
the parent-memory span that `input` references. -/
/- Type quantifiers: code_dependentWitness1 : Nat, code_dependentWitness0 : Nat, 0 ≤
  code_dependentWitness0 ∧
  0 ≤ code_dependentWitness1 ∧
  (code_dependentWitness0 + code_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ code_dependentWitness1 ∧ (code_dependentWitness1 + 32) ≤ (2 ^ 32 - 1) -/
def run_system_call_frame (tgt : (Vector (BitVec 8) 20)) (code : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (CodeFields k_off k_len)))) (input : CalldataSlice) : SailM (Nat × Nat × Nat × Int × FrameStatus × (Sigma
  fun (code_dependentWitness0 : Nat) =>
  (Sigma fun (code_dependentWitness1 : Nat) =>
  (OutputSliceFields code_dependentWitness0 code_dependentWitness1)))) := do
  let code_dependentWitness0 := (code).1
  let code_dependentWitness1 := ((code).2).1
  let code := ((code).2).2
  (k_journal_checkpoint ())
  (interpret SYSTEM_CALL_GAS_LIMIT STATE_GAS_ZERO STATE_GAS_SPILL_ZERO GAS_REFUND_ZERO
    (← (stack_reset ())) ⟨_, ⟨_, EMPTY_EVM_MEMORY_SLICE⟩⟩ SYSTEM_ADDRESS tgt tgt ZERO_WORD
    STATE_GAS_ZERO false 0 ⟨_, ⟨_, code⟩⟩ input)

/-- Issues one unchecked block-start system call: a 30M-gas frame from
`SYSTEM_ADDRESS` with a 32-byte input; skipped when the target has no
code, and its output is discarded. -/
def system_call (tgt : (Vector (BitVec 8) 20)) (input : (Vector (BitVec 8) 32)) : SailM Unit := do
  let code_hash ← do (k_code_key tgt)
  if ((code_hash == KECCAK_EMPTY) : Bool)
  then (pure ())
  else
    (do
      let ⟨_, ⟨_, code⟩⟩ ← do (code_db_resolve code_hash)
      let ⟨_, ⟨_, initial_memory⟩⟩ ← do (memory_reset ())
      let input_range : (Sigma fun (k_off : Nat) =>
        (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len))) :=
        ((⟨_, ⟨_, (memory_range 0 SYSTEM_CALL_INPUT_LENGTH)⟩⟩ : (Sigma fun (k_off : Nat) =>
        (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
        (Sigma fun (k_len : Nat) => (MemoryRangeFields k_off k_len))))
      let (_, expanded_memory) ← do
        (memory_expand_to ⟨_, ⟨_, initial_memory⟩⟩ ((input_range).2).2.len)
      let input_word := (hash_to_word input)
      (mem_store_word ((input_range).2).2.off input_word)
      let (input_slice, accessed_memory) ← do
        (active_memory_slice expanded_memory ((input_range).2).2.off ((input_range).2).2.len)
      let ⟨_, ⟨_, parent_memory⟩⟩ ← do (memory_frame_enter ())
      let memory_input := (evm_memory_slice ((input_slice).2).2.bytes ((input_slice).2).2.len)
      let frame_input := (MemoryCalldata ⟨_, ⟨_, memory_input⟩⟩)
      let (_, _, _, _, status, _) ← do
        (run_system_call_frame tgt ⟨_, ⟨_, code⟩⟩ frame_input)
      let ⟨_, ⟨_, _⟩⟩ ← do (memory_frame_leave ⟨_, ⟨_, parent_memory⟩⟩)
      let succeeded := (frame_succeeded status)
      let failed := (! succeeded)
      if (failed : Bool)
      then (k_journal_revert ())
      else (k_journal_commit ())
      (k_tx_merge ()))

/-- Issues one checked block-end system call (EIP-7002/EIP-7251/EIP-8282): the
target must exist and the call must succeed. Missing code or frame failure
throws immediately; a successful call returns the dequeued requests. -/
def system_call_checked (tgt : (Vector (BitVec 8) 20)) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))) := do
  let code_hash ← do (k_code_key tgt)
  if _sailIf0 : ((code_hash == KECCAK_EMPTY) : Bool) = true
  then
    (do
      (fatal_error ExecutionInvalid))
  else
    (do
      let ⟨_, ⟨_, code⟩⟩ ← do (code_db_resolve code_hash)
      let ⟨_, ⟨_, _⟩⟩ ← do (memory_reset ())
      let ⟨_, ⟨_, parent_memory⟩⟩ ← do (memory_frame_enter ())
      let (_, _, _, _, status, output) ← do
        (run_system_call_frame tgt ⟨_, ⟨_, code⟩⟩ EMPTY_CALLDATA)
      let succeeded := (frame_succeeded status)
      if _sailIf1 : (succeeded : Bool) = true
      then
        (do
          let start ← do (scratch_reserve ((output).2).2.len)
          (output_scratch_push_slice output)
          let ⟨_, ⟨_, result⟩⟩ ← do (scratch_finish start)
          let ⟨_, ⟨_, _⟩⟩ ← do (memory_frame_leave ⟨_, ⟨_, parent_memory⟩⟩)
          (k_journal_commit ())
          (k_tx_merge ())
          (pure ((⟨_, ⟨_, result⟩⟩ : (Sigma fun (k_off : Nat) =>
            (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len)))) : (Sigma fun
            (k_off : Nat) => (Sigma fun (k_len : Nat) => (ScratchSliceFields k_off k_len))))))
      else
        (do
          let ⟨_, ⟨_, _⟩⟩ ← do (memory_frame_leave ⟨_, ⟨_, parent_memory⟩⟩)
          (k_journal_revert ())
          (k_tx_merge ())
          (fatal_error ExecutionInvalid)))

/-- Whether a retained log is a `DepositEvent` from the deposit contract
(EIP-6110). -/
/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def deposit_log_matches (index : Nat) : SailM Bool := do
  let address ← do (log_address index)
  if ((bne address DEPOSIT_CONTRACT_ADDR) : Bool)
  then (pure false)
  else
    (do
      let topic_count ← do (log_topics_count index)
      if ((topic_count == 0) : Bool)
      then (pure false)
      else
        (do
          let first_topic ← do (log_topic index 0)
          (pure (first_topic == DEPOSIT_EVENT_TOPIC))))

/-- Validates one `DepositEvent` ABI payload against the next authenticated
consensus-layer deposit request and returns the unconsumed request
suffix (EIP-6110). -/
/- Type quantifiers: expected_dependentWitness1 : Nat, expected_dependentWitness0 : Nat, data_dependentWitness1
  : Nat, data_dependentWitness0 : Nat, 0 ≤ data_dependentWitness0 ∧
  0 ≤ data_dependentWitness1 ∧
  (data_dependentWitness0 + data_dependentWitness1) ≤ (2 ^ 32 - 1), 0 ≤
  expected_dependentWitness0 ∧
  0 ≤ expected_dependentWitness1 ∧
  (expected_dependentWitness0 + expected_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def authenticate_deposit_request (data : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (LogDataSliceFields k_off k_len)))) (expected : (Sigma fun
  (data_dependentWitness0 : Nat) =>
  (Sigma fun (data_dependentWitness1 : Nat) =>
  (StatelessInputSliceFields data_dependentWitness0 data_dependentWitness1)))) : SailM (Sigma fun
  (expected_dependentWitness0 : Nat) =>
  (Sigma fun (expected_dependentWitness1 : Nat) =>
  (StatelessInputSliceFields expected_dependentWitness0 expected_dependentWitness1))) := do
  let data_dependentWitness0 := (data).1
  let data_dependentWitness1 := ((data).2).1
  let data := ((data).2).2
  let expected_dependentWitness0 := (expected).1
  let expected_dependentWitness1 := ((expected).2).1
  let expected := ((expected).2).2
  let ⟨_, data⟩ ← (( do
    if _sailIf0 : ((data.len == DEPOSIT_EVENT_DATA_LENGTH) : Bool) = true
    then
      (pure ((⟨_, (cast (by first | rfl | omega | (congr 1 <;> simp_all) | (congr 1 <;> omega) | (simp_all <;> omega) | (simp_all <;> rfl) | simp_all) (data))⟩ : (Sigma
        fun (expected_dependentWitness0 : Nat) =>
        (LogDataSliceFields expected_dependentWitness0 576))) : (Sigma fun
        (expected_dependentWitness0 : Nat) => (LogDataSliceFields expected_dependentWitness0 576))))
    else
      (do
        (fatal_error InvalidExecutionRequests)) ) : SailM
    (Sigma fun (expected_dependentWitness0 : Nat) =>
    (LogDataSliceFields expected_dependentWitness0 576)) )
  let pubkey_head ← do (log_data_slice_load ⟨_, ⟨_, data⟩⟩ DEPOSIT_PUBKEY_HEAD)
  let withdrawal_credentials_head ← do
    (log_data_slice_load ⟨_, ⟨_, data⟩⟩ DEPOSIT_WITHDRAWAL_CREDENTIALS_HEAD)
  let amount_head ← do (log_data_slice_load ⟨_, ⟨_, data⟩⟩ DEPOSIT_AMOUNT_HEAD)
  let signature_head ← do (log_data_slice_load ⟨_, ⟨_, data⟩⟩ DEPOSIT_SIGNATURE_HEAD)
  let index_head ← do (log_data_slice_load ⟨_, ⟨_, data⟩⟩ DEPOSIT_INDEX_HEAD)
  let pubkey_length ← do (log_data_slice_load ⟨_, ⟨_, data⟩⟩ DEPOSIT_PUBKEY_LENGTH_WORD)
  let withdrawal_credentials_length ← do
    (log_data_slice_load ⟨_, ⟨_, data⟩⟩ DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH_WORD)
  let amount_length ← do (log_data_slice_load ⟨_, ⟨_, data⟩⟩ DEPOSIT_AMOUNT_LENGTH_WORD)
  let signature_length ← do
    (log_data_slice_load ⟨_, ⟨_, data⟩⟩ DEPOSIT_SIGNATURE_LENGTH_WORD)
  let index_length ← do (log_data_slice_load ⟨_, ⟨_, data⟩⟩ DEPOSIT_INDEX_LENGTH_WORD)
  let expected_pubkey_head : Nat := (u256 160)
  let expected_withdrawal_credentials_head : Nat := (u256 256)
  let expected_amount_head : Nat := (u256 320)
  let expected_signature_head : Nat := (u256 384)
  let expected_index_head : Nat := (u256 512)
  let expected_pubkey_length : Nat := (u256 48)
  let expected_withdrawal_credentials_length : Nat := (u256 32)
  let expected_amount_length : Nat := (u256 8)
  let expected_signature_length : Nat := (u256 96)
  let expected_index_length : Nat := (u256 8)
  if (((pubkey_head != expected_pubkey_head) || ((withdrawal_credentials_head != expected_withdrawal_credentials_head) || ((amount_head != expected_amount_head) || ((signature_head != expected_signature_head) || ((index_head != expected_index_head) || ((pubkey_length != expected_pubkey_length) || ((withdrawal_credentials_length != expected_withdrawal_credentials_length) || ((amount_length != expected_amount_length) || ((signature_length != expected_signature_length) || (index_length != expected_index_length)))))))))) : Bool)
  then (fatal_error InvalidExecutionRequests)
  else (pure ())
  if _sailIf0 : ((DEPOSIT_REQUEST_LENGTH ≤b expected.len) : Bool) = true
  then
    (do
      let log_pubkey := (log_data_sub_slice data DEPOSIT_PUBKEY_DATA DEPOSIT_PUBKEY_LENGTH)
      let expected_pubkey :=
        (stateless_input_sub_slice expected DEPOSIT_REQUEST_PUBKEY DEPOSIT_PUBKEY_LENGTH)
      let pubkey_matches ← do
        (log_input_slices_equal ⟨_, ⟨_, log_pubkey⟩⟩ ⟨_, ⟨_, expected_pubkey⟩⟩)
      let pubkey_mismatch := (! pubkey_matches)
      let log_withdrawal_credentials :=
        (log_data_sub_slice data DEPOSIT_WITHDRAWAL_CREDENTIALS_DATA
          DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH)
      let expected_withdrawal_credentials :=
        (stateless_input_sub_slice expected DEPOSIT_REQUEST_WITHDRAWAL_CREDENTIALS
          DEPOSIT_WITHDRAWAL_CREDENTIALS_LENGTH)
      let withdrawal_credentials_match ← do
        (log_input_slices_equal ⟨_, ⟨_, log_withdrawal_credentials⟩⟩
          ⟨_, ⟨_, expected_withdrawal_credentials⟩⟩)
      let withdrawal_credentials_mismatch := (! withdrawal_credentials_match)
      let log_amount := (log_data_sub_slice data DEPOSIT_AMOUNT_DATA DEPOSIT_AMOUNT_LENGTH)
      let expected_amount :=
        (stateless_input_sub_slice expected DEPOSIT_REQUEST_AMOUNT DEPOSIT_AMOUNT_LENGTH)
      let amount_matches ← do
        (log_input_slices_equal ⟨_, ⟨_, log_amount⟩⟩ ⟨_, ⟨_, expected_amount⟩⟩)
      let amount_mismatch := (! amount_matches)
      let log_signature := (log_data_sub_slice data DEPOSIT_SIGNATURE_DATA DEPOSIT_SIGNATURE_LENGTH)
      let expected_signature :=
        (stateless_input_sub_slice expected DEPOSIT_REQUEST_SIGNATURE DEPOSIT_SIGNATURE_LENGTH)
      let signature_matches ← do
        (log_input_slices_equal ⟨_, ⟨_, log_signature⟩⟩ ⟨_, ⟨_, expected_signature⟩⟩)
      let signature_mismatch := (! signature_matches)
      let log_index := (log_data_sub_slice data DEPOSIT_INDEX_DATA DEPOSIT_INDEX_LENGTH)
      let expected_index :=
        (stateless_input_sub_slice expected DEPOSIT_REQUEST_INDEX DEPOSIT_INDEX_LENGTH)
      let index_matches ← do
        (log_input_slices_equal ⟨_, ⟨_, log_index⟩⟩ ⟨_, ⟨_, expected_index⟩⟩)
      let index_mismatch := (! index_matches)
      if ((pubkey_mismatch || (withdrawal_credentials_mismatch || (amount_mismatch || (signature_mismatch || index_mismatch)))) : Bool)
      then (fatal_error InvalidExecutionRequests)
      else (pure ())
      (pure ((⟨_, ⟨_, (stateless_input_slice_suffix expected DEPOSIT_REQUEST_LENGTH)⟩⟩ : (Sigma
        fun (expected_dependentWitness0 : Nat) =>
        (Sigma fun (expected_dependentWitness1 : Nat) =>
        (StatelessInputSliceFields expected_dependentWitness0 expected_dependentWitness1)))) : (Sigma
        fun (expected_dependentWitness0 : Nat) =>
        (Sigma fun (expected_dependentWitness1 : Nat) =>
        (StatelessInputSliceFields expected_dependentWitness0 expected_dependentWitness1))))))
  else
    (do
      (fatal_error InvalidExecutionRequests))

/-- Authenticates matching deposit logs in emission order and returns the
unconsumed suffix of the expected request bytes. -/
/- Type quantifiers: expected_dependentWitness1 : Nat, expected_dependentWitness0 : Nat, 0 ≤
  expected_dependentWitness0 ∧
  0 ≤ expected_dependentWitness1 ∧
  (expected_dependentWitness0 + expected_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def authenticate_deposit_logs (logs : LogSeriesRef) (expected : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Sigma fun
  (expected_dependentWitness0 : Nat) =>
  (Sigma fun (expected_dependentWitness1 : Nat) =>
  (StatelessInputSliceFields expected_dependentWitness0 expected_dependentWitness1))) := do
  let expected_dependentWitness0 := (expected).1
  let expected_dependentWitness1 := ((expected).2).1
  let expected := ((expected).2).2
  let remaining : (Sigma fun (expected_dependentWitness0 : Nat) =>
    (Sigma fun (expected_dependentWitness1 : Nat) =>
    (StatelessInputSliceFields expected_dependentWitness0 expected_dependentWitness1))) :=
    ((⟨_, ⟨_, expected⟩⟩ : (Sigma fun (expected_dependentWitness0 : Nat) =>
    (Sigma fun (expected_dependentWitness1 : Nat) =>
    (StatelessInputSliceFields expected_dependentWitness0 expected_dependentWitness1)))) : (Sigma
    fun (expected_dependentWitness0 : Nat) =>
    (Sigma fun (expected_dependentWitness1 : Nat) =>
    (StatelessInputSliceFields expected_dependentWitness0 expected_dependentWitness1))))
  let offset : Nat := 0
  let (offset, remaining) ← (( do
    let loop_vars ← whileFuelM (fuel :=(logs.count -i offset)) (fun (offset, remaining) => (pure (offset <b logs.count))) (offset, remaining)
      fun (offset, remaining) => do
        assert true "loop dummy assert"
        let index ← do (log_store_index_add logs.start offset)
        let matches' ← do (deposit_log_matches index)
        let ⟨_, ⟨_, remaining⟩⟩ ← (( do
          if _sailIf0 : (matches' : Bool) = true
          then
            (do
              let ⟨_, ⟨_, data⟩⟩ ← do (read_log_data index)
              (authenticate_deposit_request ⟨_, ⟨_, data⟩⟩ remaining))
          else
            (pure (remaining : (Sigma fun (expected_dependentWitness0 : Nat) =>
              (Sigma fun (expected_dependentWitness1 : Nat) =>
              (StatelessInputSliceFields expected_dependentWitness0 expected_dependentWitness1)))))
          ) : SailM
          (Sigma fun (expected_dependentWitness0 : Nat) =>
          (Sigma fun (expected_dependentWitness1 : Nat) =>
          (StatelessInputSliceFields expected_dependentWitness0 expected_dependentWitness1))) )
        let offset ← (log_store_index_increment offset)
        (pure ((offset : Nat), ((⟨_, ⟨_, remaining⟩⟩ : (Sigma fun
          (expected_dependentWitness0 : Nat) =>
          (Sigma fun (expected_dependentWitness1 : Nat) =>
          (StatelessInputSliceFields expected_dependentWitness0 expected_dependentWitness1)))) : (Sigma
          fun (expected_dependentWitness0 : Nat) =>
          (Sigma fun (expected_dependentWitness1 : Nat) =>
          (StatelessInputSliceFields expected_dependentWitness0 expected_dependentWitness1))))))
    (pure loop_vars) ) : SailM
    (Nat × (Sigma fun (expected_dependentWitness0 : Nat) =>
    (Sigma fun (expected_dependentWitness1 : Nat) =>
    (StatelessInputSliceFields expected_dependentWitness0 expected_dependentWitness1)))) )
  (pure (remaining : (Sigma fun (expected_dependentWitness0 : Nat) =>
    (Sigma fun (expected_dependentWitness1 : Nat) =>
    (StatelessInputSliceFields expected_dependentWitness0 expected_dependentWitness1)))))

/-- Dequeues one block-end request stream and validates it byte for byte
against the input's committed request bytes. -/
/- Type quantifiers: expected_dependentWitness1 : Nat, expected_dependentWitness0 : Nat, 0 ≤
  expected_dependentWitness0 ∧
  0 ≤ expected_dependentWitness1 ∧
  (expected_dependentWitness0 + expected_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def validate_request_stream (tgt : (Vector (BitVec 8) 20)) (expected : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM Unit := do
  let expected_dependentWitness0 := (expected).1
  let expected_dependentWitness1 := ((expected).2).1
  let expected := ((expected).2).2
  let ⟨_, ⟨_, dequeued⟩⟩ ← do (system_call_checked tgt)
  let matches' ← do
    (scratch_input_slices_equal ⟨_, ⟨_, dequeued⟩⟩ ⟨_, ⟨_, expected⟩⟩)
  let mismatch := (! matches')
  if (mismatch : Bool)
  then (fatal_error InvalidExecutionRequests)
  else (pure ())

/-- Validates the EIP-7685 execution requests at block end in request-type
order against the input's committed request bytes: withdrawal
(EIP-7002), consolidation (EIP-7251), and, from Amsterdam, builder
deposit and builder exit (EIP-8282). Before Amsterdam the input must
commit to empty builder request streams. Deposits (EIP-6110) are
authenticated inline against the transaction receipt logs. -/
def validate_execution_requests (input_ref : StatelessInputRef) : SailM Unit := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  (validate_request_stream WITHDRAWAL_REQUEST_ADDR input_ref.withdrawal_requests)
  (validate_request_stream CONSOLIDATION_REQUEST_ADDR input_ref.consolidation_requests)
  if ((profile.fork ≥b Amsterdam) : Bool)
  then
    (do
      (validate_request_stream BUILDER_DEPOSIT_REQUEST_ADDR input_ref.builder_deposit_requests)
      (validate_request_stream BUILDER_EXIT_REQUEST_ADDR input_ref.builder_exit_requests))
  else
    (do
      let builder_deposit_length :=
        (stateless_input_slice_length input_ref.builder_deposit_requests)
      let builder_exit_length := (stateless_input_slice_length input_ref.builder_exit_requests)
      if (((builder_deposit_length != 0) || (builder_exit_length != 0)) : Bool)
      then (fatal_error InvalidExecutionRequests)
      else (pure ()))

