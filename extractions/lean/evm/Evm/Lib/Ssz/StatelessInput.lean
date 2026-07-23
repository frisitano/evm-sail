import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Primitives.Code
import Evm.Primitives.Crypto
import Evm.Primitives.ChainConfig
import Evm.Host.EvmByteSlice
import Evm.Host.Code
import Evm.Lib.Ssz.Ssz
import Evm.Lib.Rlp.Rlp
import Evm.Lib.Rlp.Tx
import Evm.Host.Kernel.Environment
import Evm.Host.Kernel.Lifecycle

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

/-! # The stateless input decoder

Materialized references into the concrete `SszStatelessInput` wire format
and the stateless guest decoder. The layout constants transcribe the SSZ
schema; the decoder resolves every variable region once and hands
consumers explicit source spans. -/

/-- The input starts with a 2-byte big-endian schema id; the SSZ body
follows. -/
def SSZ_BODY : Nat := 2

def IN_NPR_OFF : Nat := 0

def IN_WITNESS_OFF : Nat := 4

def IN_CHAIN_CONFIG_OFF : Nat := 8

def IN_PUBLIC_KEYS_OFF : Nat := 12

def NPR_PAYLOAD_OFF : Nat := 0

def NPR_VHASHES_OFF : Nat := 4

def NPR_BEACON_ROOT : Nat := 8

def NPR_REQUESTS_OFF : Nat := 40

def REQ_DEPOSITS_OFF : Nat := 0

def REQ_WITHDRAWALS_OFF : Nat := 4

def REQ_CONSOLIDATIONS_OFF : Nat := 8

def REQ_BUILDER_DEPOSITS_OFF : Nat := 12

def REQ_BUILDER_EXITS_OFF : Nat := 16

def PL_FEE_RECIPIENT : Nat := 32

def PL_STATE_ROOT : Nat := 52

def PL_RECEIPTS_ROOT : Nat := 84

def PL_LOGS_BLOOM : Nat := 116

def PL_PREV_RANDAO : Nat := 372

def PL_BLOCK_NUMBER : Nat := 404

def PL_GAS_LIMIT : Nat := 412

def PL_GAS_USED : Nat := 420

def PL_TIMESTAMP : Nat := 428

def PL_EXTRA_OFF : Nat := 436

def PL_BASE_FEE : Nat := 440

def PL_BLOCK_HASH : Nat := 472

def PL_TXS_OFF : Nat := 504

def PL_WDS_OFF : Nat := 508

def PL_BLOB_GAS_USED : Nat := 512

def PL_EXCESS_BLOB_GAS : Nat := 520

def PL_BAL_OFF : Nat := 528

def PL_SLOT_NUMBER : Nat := 532

def WD_SIZE : Nat := 44

def WD_INDEX : Nat := 0

def WD_VALIDATOR_INDEX : Nat := 8

def WD_ADDRESS : Nat := 16

def WD_AMOUNT : Nat := 36

def CC_CHAIN_ID : Nat := 0

def CC_ACTIVE_FORK_OFF : Nat := 8

def FC_ACTIVATION_OFF : Nat := 0

def FA_BLOCK_NUMBER_OFF : Nat := 0

def FA_TIMESTAMP_OFF : Nat := 4

def WIT_STATE_OFF : Nat := 0

def WIT_CODES_OFF : Nat := 4

def WIT_HEADERS_OFF : Nat := 8

def STATELESS_INPUT_FIXED_LENGTH : Nat := 18

def STATELESS_INPUT_BODY_FIXED_LENGTH : Nat := 16

def NEW_PAYLOAD_REQUEST_FIXED_LENGTH : Nat := 44

def EXECUTION_PAYLOAD_FIXED_LENGTH : Nat := 540

def EXECUTION_REQUESTS_FIXED_LENGTH : Nat := 20

def EXECUTION_WITNESS_FIXED_LENGTH : Nat := 12

def PUBLIC_KEY_LENGTH : Nat := 65

def CHAIN_CONFIG_HEADER_LENGTH : Nat := 12

def CHAIN_CONFIG_MIN_LENGTH : Nat := 24

def MAX_EXTRA_DATA_LENGTH : Nat := (2 ^i 5)

def MAX_TRANSACTION_LENGTH : Nat := (2 ^i 30)

def MAX_TRANSACTIONS_PER_PAYLOAD : Nat := (2 ^i 20)

def MAX_WITHDRAWALS_PER_PAYLOAD : Nat := (2 ^i 4)

def MAX_BLOCK_ACCESS_LIST_LENGTH : Nat := (2 ^i 30)

def MAX_BLOB_COMMITMENTS_PER_BLOCK : Nat := (2 ^i 12)

def MAX_WITNESS_NODES : Nat := (2 ^i 22)

def MAX_WITNESS_NODE_LENGTH : Nat := (2 ^i 10)

def MAX_WITNESS_CODES : Nat := (2 ^i 18)

def MAX_WITNESS_CODE_LENGTH : Nat := (2 ^i 16)

def MAX_WITNESS_HEADERS : Nat := (2 ^i 8)

def MAX_WITNESS_HEADER_LENGTH : Nat := (2 ^i 10)

def MAX_PUBLIC_KEYS : Nat := (2 ^i 15)

/-- Returns the byte position of an entry in a `uint32` SSZ offset table. -/
/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 30 - 1) -/
def ssz_offset_table_position (index : ssz_offset_index) : ssz_offset :=
  let index := (index).value
  ⟨(index *i 4)⟩

/-- A cursor at the first element of a variable-element SSZ list. -/
/- Type quantifiers: k_maximum : Nat, source_valid_length(k_maximum) -/
def ssz_list_cursor (items : (BoundedSszListRef k_maximum)) : SailM (BoundedSszListCursor k_maximum) := do
  let ⟨_, ⟨_, bytes⟩⟩ := items.bytes
  let current ← do
    if ((items.count != 0) : Bool)
    then
      (pure (ssz_offset_to_source_pointer ⟨((← (ssz_u32 ⟨_, ⟨_, bytes⟩⟩ 0))).value⟩))
    else (pure bytes.len)
  (pure { items := items,
          index := 0,
          current := current })

/-- Whether the cursor has consumed all elements. -/
/- Type quantifiers: k_maximum : Nat, source_valid_length(k_maximum) -/
def ssz_list_cursor_empty (cursor : (BoundedSszListCursor k_maximum)) : Bool :=
  (cursor.items.count ≤b cursor.index)

/-- The next element's span, and the advanced cursor. -/
/- Type quantifiers: k_maximum : Nat, source_valid_length(k_maximum) ∧ k_maximum ≤ (2 ^ 30 - 1) -/
def ssz_list_pop (cursor : (BoundedSszListCursor k_maximum)) : SailM (EvmByteSlice × (BoundedSszListCursor k_maximum)) := do
  let count := cursor.items.count
  let index := cursor.index
  if ((count ≤b index) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let next_index : Nat := (index + 1)
  let items := cursor.items
  let ⟨_, ⟨_, bytes⟩⟩ := items.bytes
  let nat := bytes.len
  let next ← do
    if ((next_index <b items.count) : Bool)
    then
      (pure (ssz_offset_to_source_pointer
          ⟨((← (ssz_u32 ⟨_, ⟨_, bytes⟩⟩
            ((ssz_offset_table_position ⟨next_index⟩)).value))).value⟩))
    else (pure nat)
  let current_value := cursor.current
  let next_value := next
  if ((current_value ≤b next_value) : Bool)
  then
    (do
      if ((next_value ≤b nat) : Bool)
      then
        (do
          let item_length := (next_value - current_value)
          if (((items.max_item_length != 0) && ((items.max_item_length <b item_length) : Bool)) : Bool)
          then sailThrow ((InvalidBlock InvalidConfig))
          else (pure ())
          let item := (sub_slice bytes current_value item_length)
          (pure ((((fun (dependentValue0, dependentValue1) => (⟨_, ⟨_, dependentValue0⟩⟩, dependentValue1)) ((item, { items := items,
                                                                                                                              index := next_index,
                                                                                                                              current := next }))) : ((Sigma
            fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × (BoundedSszListCursor k_maximum))) : ((Sigma
            fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × (BoundedSszListCursor k_maximum)))))
      else
        (do
          sailThrow ((InvalidBlock InvalidConfig))))
  else
    (do
      sailThrow ((InvalidBlock InvalidConfig)))

/-- Returns a variable-width list item by resolving its adjacent offsets. -/
/- Type quantifiers: k_maximum : Nat, index : Nat, source_valid_length(k_maximum) ∧
  k_maximum ≤ (2 ^ 30 - 1) ∧ 0 ≤ index -/
def ssz_list_at (items : (BoundedSszListRef k_maximum)) (index : Nat) : SailM EvmByteSlice := do
  let ⟨_, ⟨_, bytes⟩⟩ := items.bytes
  let count := items.count
  if ((count ≤b index) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let next_index : Nat := (index + 1)
  let start ← do
    (pure (ssz_offset_to_source_pointer
        ⟨((← (ssz_u32_at ⟨_, ⟨_, bytes⟩⟩ ((ssz_offset_table_position ⟨index⟩)).value))).value⟩))
  let stop ← (( do
    if ((next_index <b items.count) : Bool)
    then
      (pure (ssz_offset_to_source_pointer
          ⟨((← (ssz_u32_at ⟨_, ⟨_, bytes⟩⟩
            ((ssz_offset_table_position ⟨next_index⟩)).value))).value⟩))
    else (pure bytes.len) ) : SailM Nat )
  let start_value := start
  let stop_value := stop
  let items_length := bytes.len
  if ((start_value ≤b stop_value) : Bool)
  then
    (do
      if ((stop_value ≤b items_length) : Bool)
      then
        (do
          let item_length := (stop_value - start_value)
          if (((items.max_item_length != 0) && ((items.max_item_length <b item_length) : Bool)) : Bool)
          then sailThrow ((InvalidBlock InvalidConfig))
          else (pure ())
          (pure ((⟨_, ⟨_, (sub_slice bytes start item_length)⟩⟩ : (Sigma fun (k_off : Nat)
            => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun
            (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))))
      else
        (do
          sailThrow ((InvalidBlock InvalidConfig))))
  else
    (do
      sailThrow ((InvalidBlock InvalidConfig)))

/- Type quantifiers: k_maximum : Nat, item_size : Nat, index : Nat, source_valid_length(k_maximum)
  ∧ source_valid_length(item_size) ∧ 0 ≤ index -/
def ssz_fixed_list_at (items : (BoundedSszListRef k_maximum)) (index : Nat) (item_size : Nat) : SailM (EvmByteSliceLength item_size) := do
  let ⟨_, ⟨_, bytes⟩⟩ := items.bytes
  if ((items.count ≤b index) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let width_value := item_size
  let offset_value := (index *i width_value)
  let items_length := bytes.len
  if (((offset_value + width_value) ≤b items_length) : Bool)
  then
    (pure ((⟨_, ⟨_, (sub_slice bytes offset_value width_value)⟩⟩ : (Sigma fun (k_off : Nat)
      => (Sigma fun (item_size : Nat) => (EvmByteSliceFields k_off item_size)))) : (Sigma fun
      (k_off : Nat) => (Sigma fun (item_size : Nat) => (EvmByteSliceFields k_off item_size)))))
  else
    (do
      sailThrow ((InvalidBlock InvalidConfig)))

/- Type quantifiers: k_maximum : Nat, item_size : Nat, source_valid_length(k_maximum) ∧
  source_valid_length(item_size) -/
def ssz_fixed_list_pop (items : (BoundedSszListRef k_maximum)) (item_size : Nat) : SailM ((EvmByteSliceLength item_size) × (BoundedSszListRef k_maximum)) := do
  let ⟨_, ⟨_, bytes⟩⟩ := items.bytes
  let items_length := bytes.len
  let width := item_size
  let count := items.count
  if ((count == 0) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  if ((items_length <b width) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let item := (sub_slice bytes 0 item_size)
  let rest : (BoundedSszListRef k_maximum) :=
    { bytes := ⟨_, ⟨_, (slice_suffix bytes width)⟩⟩,
      count := (count - 1),
      max_item_length := items.max_item_length }
  (pure ((((fun (dependentValue0, dependentValue1) => (⟨_, ⟨_, dependentValue0⟩⟩, dependentValue1)) ((item, rest))) : ((Sigma
    fun (k_off : Nat) => (Sigma fun (item_size : Nat) => (EvmByteSliceFields k_off item_size))) × (BoundedSszListRef k_maximum))) : ((Sigma
    fun (k_off : Nat) => (Sigma fun (item_size : Nat) => (EvmByteSliceFields k_off item_size))) × (BoundedSszListRef k_maximum))))

/- Type quantifiers: k_ex411288_ : Nat, k_ex411287_ : Nat, minimum : Nat, 0 ≤ minimum, 0 ≤
  k_ex411287_ ∧ 0 ≤ k_ex411288_ -/
def ssz_container_bytes (bytes : EvmByteSlice) (minimum : Nat) : SailM (EvmByteSliceAtLeast minimum) := do
  let bytes := ((bytes).2).2
  let fields := bytes
  if ((minimum ≤b fields.len) : Bool)
  then
    (pure ((⟨_, ⟨_, fields⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))))
  else
    (do
      sailThrow ((InvalidBlock InvalidConfig)))

/- Type quantifiers: k_ex411293_ : Nat, k_ex411292_ : Nat, fixed_length : Nat, 0 ≤ fixed_length, 0
  ≤ k_ex411292_ ∧ 0 ≤ k_ex411293_ ∧ 0 ≤ fixed_length ∧ fixed_length ≤ k_ex411293_ -/
def ssz_container_cursor (bytes : (EvmByteSliceAtLeast k_minimum)) (fixed_length : Nat) : SszContainerCursor :=
  let bytes := ((bytes).2).2
  { bytes := ⟨_, ⟨_, bytes⟩⟩,
    current := fixed_length }

/-- Takes the next variable field, ending at its container-relative SSZ
offset, and returns the advanced cursor. -/
/- Type quantifiers: k_ex411294_ : Nat, 0 ≤ k_ex411294_ ∧ k_ex411294_ ≤ (2 ^ 32 - 1) -/
def ssz_take (cursor : SszContainerCursor) (stop : ssz_offset) : SailM (EvmByteSlice × SszContainerCursor) := do
  let stop := (stop).value
  let current_value := cursor.current
  let ⟨_, ⟨_, bytes⟩⟩ := cursor.bytes
  let container_length := bytes.len
  if (((current_value ≤b stop) && (stop ≤b container_length)) : Bool)
  then
    (let stop_pointer : Nat := stop
    let span_length := (stop - current_value)
    let span := (sub_slice bytes current_value span_length)
    (pure ((((fun (dependentValue0, dependentValue1) => (⟨_, ⟨_, dependentValue0⟩⟩, dependentValue1)) ((span, { bytes := ⟨_, ⟨_, bytes⟩⟩,
                                                                                                                        current := stop_pointer }))) : ((Sigma
      fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × SszContainerCursor)) : ((Sigma
      fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))) × SszContainerCursor))))
  else
    (do
      sailThrow ((InvalidBlock InvalidConfig)))

/-- Takes the remainder of a container after its last offset-delimited
field. -/
def ssz_finish (cursor : SszContainerCursor) : SailM EvmByteSlice := do
  let current_value := cursor.current
  let ⟨_, ⟨_, bytes⟩⟩ := cursor.bytes
  let container_length := bytes.len
  if ((current_value ≤b container_length) : Bool)
  then
    (let remaining := (container_length - current_value)
    (pure ((⟨_, ⟨_, (sub_slice bytes current_value remaining)⟩⟩ : (Sigma fun (k_off : Nat)
      => (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len)))) : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))))
  else
    (do
      sailThrow ((InvalidBlock InvalidConfig)))

/-- Constructs a variable-item list with its SSZ count and per-item byte
limits attached. Item lengths are checked lazily when reached, before a
consumer can construct any narrower optimized value. -/
/- Type quantifiers: k_ex411305_ : Nat, k_ex411304_ : Nat, maximum_count : Nat, maximum_item_length
  : Nat, source_valid_length(maximum_count) ∧ source_valid_length(maximum_item_length), 0 ≤
  k_ex411304_ ∧ 0 ≤ k_ex411305_ -/
def ssz_bounded_variable_list_ref (bytes : EvmByteSlice) (maximum_count : Nat) (maximum_item_length : Nat) : SailM (BoundedSszListRef maximum_count) := do
  let bytes := ((bytes).2).2
  let span := bytes.len
  let raw_count ← (( do
    if ((span == 0) : Bool)
    then (pure 0)
    else
      (do
        if ((span <b SSZ_OFF_BYTES) : Bool)
        then sailThrow ((InvalidBlock InvalidConfig))
        else (pure ())
        let first_offset ← do
          (do
              let publicResult ← (ssz_u32 ⟨_, ⟨_, bytes⟩⟩ 0)
              pure ((publicResult).value))
        let count : Nat := (Nat.div first_offset 4)
        if ((((Nat.mod first_offset 4) != 0) || ((count == 0) || (first_offset >b span))) : Bool)
        then sailThrow ((InvalidBlock InvalidConfig))
        else (pure ())
        (pure count)) ) : SailM Nat )
  if ((maximum_count <b raw_count) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let count : Nat := raw_count
  (pure { bytes := ⟨_, ⟨_, bytes⟩⟩,
          count := count,
          max_item_length := maximum_item_length })

/-- Constructs a schema-bounded fixed-item SSZ list reference. -/
/- Type quantifiers: k_ex411320_ : Nat, k_ex411319_ : Nat, maximum_count : Nat, item_size : Nat, source_valid_length(maximum_count)
  ∧ source_valid_length(item_size), 0 ≤ k_ex411319_ ∧ 0 ≤ k_ex411320_ -/
def ssz_bounded_fixed_list_ref (bytes : EvmByteSlice) (item_size : Nat) (maximum_count : Nat) : SailM (BoundedSszListRef maximum_count) := do
  let bytes := ((bytes).2).2
  let width := item_size
  if ((width == 0) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let span := bytes.len
  let raw_count := (Int.ediv span width)
  if ((span != (raw_count *i width)) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  if ((maximum_count <b raw_count) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let count : Nat := raw_count
  (pure { bytes := ⟨_, ⟨_, bytes⟩⟩,
          count := count,
          max_item_length := item_size })

/-- Resolves the input's offset tables into a
[StatelessInputRef][type-StatelessInputRef], validating the schema id
and every region bound; a malformed frame is `InvalidConfig`. -/
/- Type quantifiers: k_ex411328_ : Nat, k_ex411327_ : Nat, 0 ≤ k_ex411327_ ∧ 0 ≤ k_ex411328_ -/
def decode_stateless_input_ref (input : EvmByteSlice) : SailM StatelessInputRef := do
  let input := ((input).2).2
  let input_fields := input
  let input_length := input_fields.len
  let fixed_length := STATELESS_INPUT_FIXED_LENGTH
  let body_offset_value := SSZ_BODY
  if ((input_length <b fixed_length) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  if (((← (slice_byte ⟨_, ⟨_, input_fields⟩⟩ 1)) != 0x01#8) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let protocol ← do (protocol_profile (← (slice_byte ⟨_, ⟨_, input_fields⟩⟩ 0)))
  let body_bytes := (slice_suffix input_fields body_offset_value)
  let ⟨_, ⟨_, body⟩⟩ ← do
    (ssz_container_bytes ⟨_, ⟨_, body_bytes⟩⟩ STATELESS_INPUT_BODY_FIXED_LENGTH)
  let container_start : Nat := 0
  let new_payload_request_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, body⟩⟩
        (ssz_field_offset container_start IN_NPR_OFF))
        pure ((publicResult).value))
  let witness_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, body⟩⟩
        (ssz_field_offset container_start IN_WITNESS_OFF))
        pure ((publicResult).value))
  let chain_config_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, body⟩⟩
        (ssz_field_offset container_start IN_CHAIN_CONFIG_OFF))
        pure ((publicResult).value))
  let public_keys_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, body⟩⟩
        (ssz_field_offset container_start IN_PUBLIC_KEYS_OFF))
        pure ((publicResult).value))
  let body_fixed_length := STATELESS_INPUT_BODY_FIXED_LENGTH
  if ((new_payload_request_offset != body_fixed_length) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let body_cursor :=
    (ssz_container_cursor (k_minimum := 16) ⟨_, ⟨_, body⟩⟩ STATELESS_INPUT_BODY_FIXED_LENGTH)
  let (new_payload_request_bytes, body_after_payload_request) ← do
    (ssz_take body_cursor ⟨witness_offset⟩)
  let ⟨_, ⟨_, new_payload_request⟩⟩ ← do
    (ssz_container_bytes new_payload_request_bytes NEW_PAYLOAD_REQUEST_FIXED_LENGTH)
  let (execution_witness_bytes, body_after_witness) ← do
    (ssz_take body_after_payload_request ⟨chain_config_offset⟩)
  let ⟨_, ⟨_, execution_witness⟩⟩ ← do
    (ssz_container_bytes execution_witness_bytes EXECUTION_WITNESS_FIXED_LENGTH)
  let (chain_config, body_after_chain_config) ← do
    (ssz_take body_after_witness ⟨public_keys_offset⟩)
  let ⟨_, ⟨_, public_keys⟩⟩ ← do (ssz_finish body_after_chain_config)
  let npr_start : Nat := 0
  let payload_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, new_payload_request⟩⟩
        (ssz_field_offset npr_start NPR_PAYLOAD_OFF))
        pure ((publicResult).value))
  let versioned_hashes_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, new_payload_request⟩⟩
        (ssz_field_offset npr_start NPR_VHASHES_OFF))
        pure ((publicResult).value))
  let requests_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, new_payload_request⟩⟩
        (ssz_field_offset npr_start NPR_REQUESTS_OFF))
        pure ((publicResult).value))
  let npr_fixed_length := NEW_PAYLOAD_REQUEST_FIXED_LENGTH
  if ((payload_offset != npr_fixed_length) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let npr_cursor :=
    (ssz_container_cursor (k_minimum := 44) ⟨_, ⟨_, new_payload_request⟩⟩
      NEW_PAYLOAD_REQUEST_FIXED_LENGTH)
  let (execution_payload_bytes, npr_after_payload) ← do
    (ssz_take npr_cursor ⟨versioned_hashes_offset⟩)
  let (versioned_hashes, npr_after_versioned_hashes) ← do
    (ssz_take npr_after_payload ⟨requests_offset⟩)
  let ⟨_, ⟨_, execution_requests_bytes⟩⟩ ← do (ssz_finish npr_after_versioned_hashes)
  let ⟨_, ⟨_, execution_requests⟩⟩ ← do
    (ssz_container_bytes ⟨_, ⟨_, execution_requests_bytes⟩⟩ EXECUTION_REQUESTS_FIXED_LENGTH)
  let ⟨_, ⟨_, execution_payload⟩⟩ ← do
    (ssz_container_bytes execution_payload_bytes EXECUTION_PAYLOAD_FIXED_LENGTH)
  let payload_start : Nat := 0
  let extra_data_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, execution_payload⟩⟩
        (ssz_field_offset payload_start PL_EXTRA_OFF))
        pure ((publicResult).value))
  let transactions_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, execution_payload⟩⟩
        (ssz_field_offset payload_start PL_TXS_OFF))
        pure ((publicResult).value))
  let withdrawals_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, execution_payload⟩⟩
        (ssz_field_offset payload_start PL_WDS_OFF))
        pure ((publicResult).value))
  let block_access_list_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, execution_payload⟩⟩
        (ssz_field_offset payload_start PL_BAL_OFF))
        pure ((publicResult).value))
  let payload_fixed_length := EXECUTION_PAYLOAD_FIXED_LENGTH
  if ((extra_data_offset != payload_fixed_length) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let payload_cursor :=
    (ssz_container_cursor (k_minimum := 540) ⟨_, ⟨_, execution_payload⟩⟩
      EXECUTION_PAYLOAD_FIXED_LENGTH)
  let (extra_data, payload_after_extra_data) ← do
    (ssz_take payload_cursor ⟨transactions_offset⟩)
  let (transaction_bytes, payload_after_transactions) ← do
    (ssz_take payload_after_extra_data ⟨withdrawals_offset⟩)
  let transactions ← do
    (ssz_bounded_variable_list_ref transaction_bytes MAX_TRANSACTIONS_PER_PAYLOAD
      MAX_TRANSACTION_LENGTH)
  let (withdrawal_bytes, payload_after_withdrawals) ← do
    (ssz_take payload_after_transactions ⟨block_access_list_offset⟩)
  let withdrawals ← do
    (ssz_bounded_fixed_list_ref withdrawal_bytes WD_SIZE MAX_WITHDRAWALS_PER_PAYLOAD)
  let ⟨_, ⟨_, block_access_list⟩⟩ ← do (ssz_finish payload_after_withdrawals)
  if ((MAX_EXTRA_DATA_LENGTH <b ((extra_data).2).2.len) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  if ((MAX_BLOCK_ACCESS_LIST_LENGTH <b block_access_list.len) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let requests_start : Nat := 0
  let deposits_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, execution_requests⟩⟩
        (ssz_field_offset requests_start REQ_DEPOSITS_OFF))
        pure ((publicResult).value))
  let withdrawal_requests_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, execution_requests⟩⟩
        (ssz_field_offset requests_start REQ_WITHDRAWALS_OFF))
        pure ((publicResult).value))
  let consolidation_requests_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, execution_requests⟩⟩
        (ssz_field_offset requests_start REQ_CONSOLIDATIONS_OFF))
        pure ((publicResult).value))
  let builder_deposit_requests_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, execution_requests⟩⟩
        (ssz_field_offset requests_start REQ_BUILDER_DEPOSITS_OFF))
        pure ((publicResult).value))
  let builder_exit_requests_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, execution_requests⟩⟩
        (ssz_field_offset requests_start REQ_BUILDER_EXITS_OFF))
        pure ((publicResult).value))
  let requests_fixed_length := EXECUTION_REQUESTS_FIXED_LENGTH
  if ((deposits_offset != requests_fixed_length) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let requests_cursor :=
    (ssz_container_cursor (k_minimum := 20) ⟨_, ⟨_, execution_requests⟩⟩
      EXECUTION_REQUESTS_FIXED_LENGTH)
  let (deposits, requests_after_deposits) ← do
    (ssz_take requests_cursor ⟨withdrawal_requests_offset⟩)
  let (withdrawal_requests, requests_after_withdrawals) ← do
    (ssz_take requests_after_deposits ⟨consolidation_requests_offset⟩)
  let (consolidation_requests, requests_after_consolidations) ← do
    (ssz_take requests_after_withdrawals ⟨builder_deposit_requests_offset⟩)
  let (builder_deposit_requests, requests_after_builder_deposits) ← do
    (ssz_take requests_after_consolidations ⟨builder_exit_requests_offset⟩)
  let ⟨_, ⟨_, builder_exit_requests⟩⟩ ← do (ssz_finish requests_after_builder_deposits)
  let witness_start : Nat := 0
  let witness_state_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, execution_witness⟩⟩
        (ssz_field_offset witness_start WIT_STATE_OFF))
        pure ((publicResult).value))
  let witness_codes_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, execution_witness⟩⟩
        (ssz_field_offset witness_start WIT_CODES_OFF))
        pure ((publicResult).value))
  let witness_headers_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, execution_witness⟩⟩
        (ssz_field_offset witness_start WIT_HEADERS_OFF))
        pure ((publicResult).value))
  let witness_fixed_length := EXECUTION_WITNESS_FIXED_LENGTH
  if ((witness_state_offset != witness_fixed_length) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let witness_cursor :=
    (ssz_container_cursor (k_minimum := 12) ⟨_, ⟨_, execution_witness⟩⟩
      EXECUTION_WITNESS_FIXED_LENGTH)
  let (witness_state_bytes, witness_after_state) ← do
    (ssz_take witness_cursor ⟨witness_codes_offset⟩)
  let witness_state ← do
    (ssz_bounded_variable_list_ref witness_state_bytes MAX_WITNESS_NODES MAX_WITNESS_NODE_LENGTH)
  let (witness_code_bytes, witness_after_codes) ← do
    (ssz_take witness_after_state ⟨witness_headers_offset⟩)
  let witness_codes ← do
    (ssz_bounded_variable_list_ref witness_code_bytes MAX_WITNESS_CODES MAX_WITNESS_CODE_LENGTH)
  let witness_headers ← do
    (ssz_bounded_variable_list_ref (← (ssz_finish witness_after_codes)) MAX_WITNESS_HEADERS
      MAX_WITNESS_HEADER_LENGTH)
  let public_key_bytes := public_keys.len
  let public_key_length := PUBLIC_KEY_LENGTH
  let public_key_count := (Int.ediv public_key_bytes public_key_length)
  if ((public_key_bytes != (public_key_count *i public_key_length)) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  if ((MAX_PUBLIC_KEYS <b public_key_count) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let _ ← do
    (ssz_bounded_fixed_list_ref versioned_hashes WORD_BYTE_LENGTH MAX_BLOB_COMMITMENTS_PER_BLOCK)
  (pure { protocol := protocol,
          new_payload_request := ⟨_, ⟨_, new_payload_request⟩⟩,
          execution_payload := ⟨_, ⟨_, execution_payload⟩⟩,
          versioned_hashes := versioned_hashes,
          deposits := deposits,
          withdrawal_requests := withdrawal_requests,
          consolidation_requests := consolidation_requests,
          builder_deposit_requests := builder_deposit_requests,
          builder_exit_requests := ⟨_, ⟨_, builder_exit_requests⟩⟩,
          extra_data := extra_data,
          transactions := transactions,
          withdrawals := withdrawals,
          block_access_list := ⟨_, ⟨_, block_access_list⟩⟩,
          witness_state := witness_state,
          witness_codes := witness_codes,
          witness_headers := witness_headers,
          chain_config := chain_config,
          public_keys := ⟨_, ⟨_, public_keys⟩⟩ })

/-- The EIP-7685 per-type request digest:
`sha256(request_type ‖ request_data)`. -/
/- Type quantifiers: k_ex411332_ : Nat, k_ex411331_ : Nat, 0 ≤ k_ex411331_ ∧ 0 ≤ k_ex411332_ -/
def sha256_request_digest (request_type : byte) (s : EvmByteSlice) : SailM hash := do
  let s := ((s).2).2
  (sha256_segments [(bytes_list [request_type] 1), (BytesSlice ⟨_, ⟨_, s⟩⟩)])

/-- Hashes and indexes every remaining witness trie node from its source
slice. -/
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_index_witness_nodes_cursor (cursor : WitnessNodeListCursor) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((ssz_list_cursor_empty cursor) : Bool)
      then (pure ())
      else
        (do
          let (node, next) ← do (ssz_list_pop cursor)
          (nodedb_insert (← (keccak256_slice node)) ((node).2).2.off ((node).2).2.len)
          (_rec_index_witness_nodes_cursor next _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Hashes and indexes every remaining witness trie node from its source
slice. -/
def index_witness_nodes_cursor (cursor : WitnessNodeListCursor) : SailM Unit := do
  let _measure := ((cursor.items.count -i cursor.index) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_index_witness_nodes_cursor cursor (_measure + 1))

/-- Indexes every witness trie node into the node-db, keyed by its
KECCAK-256 digest, directly from the SSZ list reference. -/
def index_witness_nodes (nodes : WitnessNodeListRef) : SailM Unit := do
  (index_witness_nodes_cursor (← (ssz_list_cursor nodes)))

/-- Analyzes and indexes every remaining witness code body from its source
slice. -/
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_index_witness_codes_cursor (cursor : WitnessCodeListCursor) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((ssz_list_cursor_empty cursor) : Bool)
      then (pure ())
      else
        (do
          let (code, next) ← do (ssz_list_pop cursor)
          let code_length := ((code).2).2.len
          if ((MAX_WITNESS_CODE_LENGTH <b code_length) : Bool)
          then sailThrow ((InvalidBlock InvalidConfig))
          else (pure ())
          let executable := (code_slice ((code).2).2)
          let _ ← do (code_db_insert ⟨_, ⟨_, executable⟩⟩ (← readReg k_fork))
          (_rec_index_witness_codes_cursor next _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Analyzes and indexes every remaining witness code body from its source
slice. -/
def index_witness_codes_cursor (cursor : WitnessCodeListCursor) : SailM Unit := do
  let _measure := ((cursor.items.count -i cursor.index) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_index_witness_codes_cursor cursor (_measure + 1))

/-- Indexes every witness code blob into the content-addressed code
store. -/
def index_witness_codes (codes : WitnessCodeListRef) : SailM Unit := do
  (index_witness_codes_cursor (← (ssz_list_cursor codes)))

def undefined_ParentHeaderFields (_ : Unit) : SailM ParentHeaderFields := do
  (pure { parent_hash := ← (undefined_vector 32 (← (undefined_bitvector 8))),
          state_root := ← (undefined_vector 32 (← (undefined_bitvector 8))),
          base_fee := ← do
              let publicField ← (undefined_range 0 ((2 ^i 256) - 1))
              pure (⟨publicField⟩),
          blob_gas_used := ← do
              let publicField ← (undefined_range 0 (21 *i (2 ^i 17)))
              pure (⟨publicField⟩),
          excess_blob_gas := ← do
              let publicField ← (undefined_range 0 ((2 ^i 64) - 1))
              pure (⟨publicField⟩),
          have_parent := ← (undefined_bool ()),
          have_state := ← (undefined_bool ()),
          have_base_fee := ← (undefined_bool ()),
          have_blob_gas := ← (undefined_bool ()),
          have_excess_blob_gas := ← (undefined_bool ()) })

def EMPTY_PARENT_HEADER_FIELDS : ParentHeaderFields :=
  { parent_hash := ZERO_HASH,
    state_root := ZERO_HASH,
    base_fee := ⟨(ZERO_WORD).value⟩,
    blob_gas_used := ⟨0⟩,
    excess_blob_gas := ⟨0⟩,
    have_parent := false,
    have_state := false,
    have_base_fee := false,
    have_blob_gas := false,
    have_excess_blob_gas := false }

/-- Advances the parent-header field cursor, saturating at its unused-field
sentinel. -/
/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ 19 -/
def next_parent_header_field (index : parent_header_field_cursor) : parent_header_field_cursor :=
  let index := (index).value
  ⟨if ((index <b 19) : Bool)
  then (index + 1)
  else 19⟩

/-- Extracts the execution-relevant fields while walking one parent header. -/
/- Type quantifiers: _reclimit : Nat, k_ex411349_ : Nat, k_ex411348_ : Nat, k_ex411347_ : Nat, k_ex411346_
  : Nat, k_ex411345_ : Nat, 0 ≤ k_ex411345_ ∧ 0 ≤ k_ex411346_ ∧
  0 ≤ k_ex411347_ ∧ k_ex411347_ ≤ k_ex411348_ ∧ k_ex411348_ ≤ k_ex411346_, 0 ≤
  k_ex411349_ ∧ k_ex411349_ ≤ 19, 0 ≤ _reclimit -/
def _rec_decode_parent_header_fields (cursor : RlpCursor) (field_index : parent_header_field_cursor) (fields : ParentHeaderFields) (_reclimit : Nat) : SailM ParentHeaderFields := do
  let cursor := ((((cursor).2).2).2).2
  let field_index := (field_index).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((rlp_cursor_empty ⟨_, ⟨_, ⟨_, ⟨_, cursor⟩⟩⟩⟩) : Bool)
      then (pure fields)
      else
        (do
          let ⟨k_full_len, ⟨k_next, ⟨k_content_len, ⟨k_content, (field, next)⟩⟩⟩⟩ ← do
            (rlp_cursor_pop cursor)
          let decoded := fields
          let decoded ← (( do
            if ((field_index == 0) : Bool)
            then
              (do
                let decoded ←
                  (pure { decoded with parent_hash := ← (pure (word_to_hash
                          ⟨((← (rlp_ref_word
                            ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, field⟩⟩⟩⟩⟩⟩))).value⟩)) })
                (pure { decoded with have_parent := true }))
            else
              (do
                if ((field_index == 3) : Bool)
                then
                  (do
                    let decoded ←
                      (pure { decoded with state_root := ← (pure (word_to_hash
                              ⟨((← (rlp_ref_word
                                ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, field⟩⟩⟩⟩⟩⟩))).value⟩)) })
                    (pure { decoded with have_state := true }))
                else
                  (do
                    if ((field_index == 15) : Bool)
                    then
                      (do
                        let decoded ←
                          (pure { decoded with base_fee := ← do
                                let publicField ← (do
                                    let publicResult ← (rlp_ref_uint_word
                                    ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, field⟩⟩⟩⟩⟩⟩)
                                    pure ((publicResult).value))
                                pure (⟨publicField⟩) })
                        (pure { decoded with have_base_fee := true }))
                    else
                      (do
                        if ((field_index == 17) : Bool)
                        then
                          (do
                            let decoded ←
                              (pure { decoded with blob_gas_used := ← do
                                    let publicField ← (do
                                        let publicResult ← (rlp_ref_blob_gas_used
                                        ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, field⟩⟩⟩⟩⟩⟩)
                                        pure ((publicResult).value))
                                    pure (⟨publicField⟩) })
                            (pure { decoded with have_blob_gas := true }))
                        else
                          (do
                            if ((field_index == 18) : Bool)
                            then
                              (do
                                let decoded ←
                                  (pure { decoded with excess_blob_gas := ← do
                                        let publicField ← (do
                                            let publicResult ← (rlp_ref_excess_blob_gas
                                            ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, field⟩⟩⟩⟩⟩⟩)
                                            pure ((publicResult).value))
                                        pure (⟨publicField⟩) })
                                (pure { decoded with have_excess_blob_gas := true }))
                            else (pure decoded))))) ) : SailM ParentHeaderFields )
          (_rec_decode_parent_header_fields ⟨_, ⟨_, ⟨_, ⟨_, next⟩⟩⟩⟩
            ⟨((next_parent_header_field ⟨field_index⟩)).value⟩ decoded _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Extracts the execution-relevant fields while walking one parent header. -/
/- Type quantifiers: field_index : Nat, k_ex411358_ : Nat, k_ex411357_ : Nat, k_ex411356_ : Nat, k_ex411355_
  : Nat, 0 ≤ k_ex411355_ ∧ 0 ≤ k_ex411356_ ∧
  0 ≤ k_ex411357_ ∧ k_ex411357_ ≤ k_ex411358_ ∧ k_ex411358_ ≤ k_ex411356_, 0 ≤
  field_index ∧ field_index ≤ 19 -/
def decode_parent_header_fields (cursor : RlpCursor) (field_index : parent_header_field_cursor) (fields : ParentHeaderFields) : SailM ParentHeaderFields := do
  let cursor := ((((cursor).2).2).2).2
  let field_index := (field_index).value
  let _measure :=
    (let stop := cursor.stop
    let current := cursor.current
    (stop - current) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else
    (_rec_decode_parent_header_fields ⟨_, ⟨_, ⟨_, ⟨_, cursor⟩⟩⟩⟩ ⟨field_index⟩
      fields (_measure + 1))

/-- Authenticates and indexes each remaining ancestor header in chain order. -/
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_index_witness_header_cursor (state : WitnessHeaderIndex) (_reclimit : Nat) : SailM WitnessHeaderIndex := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((ssz_list_cursor_empty state.cursor) : Bool)
      then (pure state)
      else
        (do
          let index := state.cursor.index
          let (header, next) ← do (ssz_list_pop state.cursor)
          let is_last := (next.index == state.cursor.items.count)
          let result := state
          let result : WitnessHeaderIndex := { result with cursor := next }
          let result ← (( do
            if (((index != 0) || is_last) : Bool)
            then
              (do
                let ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩ ← do (rlp_node_cursor header)
                if (fields.valid : Bool)
                then
                  (do
                    let decoded ← do
                      (decode_parent_header_fields ⟨_, ⟨_, ⟨_, ⟨_, fields⟩⟩⟩⟩
                        ⟨0⟩ EMPTY_PARENT_HEADER_FIELDS)
                    let result : WitnessHeaderIndex :=
                      if (((index != 0) && ((! decoded.have_parent) || (bne decoded.parent_hash
                               state.previous_hash))) : Bool)
                      then { result with valid := false }
                      else result
                    if (is_last : Bool)
                    then
                      (do
                        let result : WitnessHeaderIndex :=
                          { result with parent_state_root := decoded.state_root }
                        let result : WitnessHeaderIndex :=
                          { result with parent_base_fee_per_gas := ⟨(decoded.base_fee).value⟩ }
                        let result : WitnessHeaderIndex :=
                          { result with parent_blob_gas_used := ⟨(decoded.blob_gas_used).value⟩ }
                        let result : WitnessHeaderIndex :=
                          { result with parent_excess_blob_gas := ⟨(decoded.excess_blob_gas).value⟩ }
                        (pure { result with parent_fields_valid := ← (pure (decoded.have_state && (((fork_lt
                                      (← readReg k_fork) Cancun) || decoded.have_base_fee) && ((fork_lt
                                      (← readReg k_fork) Cancun) || (decoded.have_blob_gas == decoded.have_excess_blob_gas))))) }))
                    else (pure result))
                else
                  (if ((index != 0) : Bool)
                  then (pure { result with valid := false })
                  else (pure result)))
            else (pure result) ) : SailM WitnessHeaderIndex )
          let current_hash ← do (keccak256_slice header)
          let result : WitnessHeaderIndex := { result with previous_hash := current_hash }
          let header_count := state.cursor.items.count
          let next_index := next.index
          let distance ← (( do
            if ((next_index ≤b header_count) : Bool)
            then (pure (header_count - next_index))
            else sailThrow ((InvalidBlock WitnessDeficient)) ) : SailM Nat )
          if ((distance <b 256) : Bool)
          then
            (do
              let ancestor : Nat := distance
              (ancestor_hash_write ⟨ancestor⟩ current_hash))
          else (pure ())
          (_rec_index_witness_header_cursor result _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Authenticates and indexes each remaining ancestor header in chain order. -/
def index_witness_header_cursor (state : WitnessHeaderIndex) : SailM WitnessHeaderIndex := do
  let _measure := ((state.cursor.items.count -i state.cursor.index) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_index_witness_header_cursor state (_measure + 1))

/-- Parses the witness header chain once. Each header is hashed into the
distance-indexed ancestor table (the `BLOCKHASH` source), adjacent
parent hashes are checked (a break is `HeaderChainBroken`), and the
newest header's execution context — parent state root, base fee, blob
gas — is decoded while its fields are consumed head-to-tail. -/
def index_witness_headers (headers : WitnessHeaderListRef) : SailM WitnessContext := do
  let indexed ← do
    (index_witness_header_cursor
      { cursor := ← (ssz_list_cursor headers),
        previous_hash := ZERO_HASH,
        valid := (headers.count != 0),
        parent_state_root := ZERO_HASH,
        parent_base_fee_per_gas := ⟨(ZERO_WORD).value⟩,
        parent_blob_gas_used := ⟨0⟩,
        parent_excess_blob_gas := ⟨0⟩,
        parent_fields_valid := false })
  writeReg k_n_headers ⟨headers.count⟩
  if ((! indexed.valid) : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else (pure ())
  if ((! indexed.parent_fields_valid) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (pure ())
  (pure { parent_hash := indexed.previous_hash,
          parent_state_root := indexed.parent_state_root,
          parent_base_fee_per_gas := ⟨(indexed.parent_base_fee_per_gas).value⟩,
          parent_blob_gas_used := ⟨(indexed.parent_blob_gas_used).value⟩,
          parent_excess_blob_gas := ⟨(indexed.parent_excess_blob_gas).value⟩ })

/-- Decodes the execution-payload header fields from their fixed SSZ
offsets. -/
/- Type quantifiers: k_ex411366_ : Nat, k_ex411365_ : Nat, 0 ≤ k_ex411365_ ∧ 0 ≤ k_ex411366_ -/
def decode_payload_blob_gas_used (payload : EvmByteSlice) : SailM blob_gas_used := do
  let payload := ((payload).2).2
  let publicResult ← do
    let value ← do
      (do
          let publicResult ← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_BLOB_GAS_USED)
          pure ((publicResult).value))
    if ((value ≤b (21 *i (2 ^i 17))) : Bool)
    then (pure value)
    else sailThrow ((InvalidBlock InvalidBlobGasUsed))
  pure (⟨publicResult⟩)

/-- Decodes the execution-payload header fields from their fixed SSZ
offsets. -/
def decode_block_header_ssz (input_ref : StatelessInputRef) : SailM BlockHeader := do
  let ⟨_, ⟨_, payload⟩⟩ := input_ref.execution_payload
  let gas_limit_value ← do
    (do
        let publicResult ← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_GAS_LIMIT)
        pure ((publicResult).value))
  let gas_used_value ← do
    (do
        let publicResult ← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_GAS_USED)
        pure ((publicResult).value))
  (pure { number := ← (do
                let publicResult ← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_BLOCK_NUMBER)
                pure ((publicResult).value)),
          timestamp := ← (do
                let publicResult ← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_TIMESTAMP)
                pure ((publicResult).value)),
          gas_limit := ⟨gas_limit_value⟩,
          gas_used := gas_used_value,
          prev_randao := ← do
              let publicField ← (pure ((hash_to_word
                  (← (ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ PL_PREV_RANDAO)))).value)
              pure (⟨publicField⟩),
          base_fee := ← do
              let publicField ← (do
                  let publicResult ← (ssz_u256 ⟨_, ⟨_, payload⟩⟩ PL_BASE_FEE)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          blob_gas_used := ← do
              let publicField ← (do
                  let publicResult ← (decode_payload_blob_gas_used ⟨_, ⟨_, payload⟩⟩)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          excess_blob_gas := ← do
              let publicField ← (do
                  let publicResult ← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_EXCESS_BLOB_GAS)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          state_root := ← (ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ PL_STATE_ROOT),
          receipts_root := ← (ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ PL_RECEIPTS_ROOT),
          logs_bloom := ← (ssz_logs_bloom ⟨_, ⟨_, payload⟩⟩ PL_LOGS_BLOOM),
          fee_recipient := ← (ssz_addr ⟨_, ⟨_, payload⟩⟩ PL_FEE_RECIPIENT),
          parent_hash := ← (ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ 0),
          parent_beacon_block_root := ← (ssz_bytes32 input_ref.new_payload_request NPR_BEACON_ROOT),
          slot_number := ← do
              let publicField ← (do
                  let publicResult ← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_SLOT_NUMBER)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          extra_data := input_ref.extra_data })

/-- Decodes one fixed-layout SSZ withdrawal element. -/
/- Type quantifiers: k_ex411370_ : Nat, k_ex411369_ : Nat, 0 ≤ k_ex411369_ ∧ 0 ≤ k_ex411370_ -/
def decode_withdrawal (withdrawal : EvmByteSlice) : SailM Withdrawal := do
  let withdrawal := ((withdrawal).2).2
  (pure { index := ← do
              let publicField ← (do
                  let publicResult ← (decode_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_INDEX)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          validator_index := ← do
              let publicField ← (do
                  let publicResult ← (decode_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩
                  WD_VALIDATOR_INDEX)
                  pure ((publicResult).value))
              pure (⟨publicField⟩),
          address := ← (ssz_addr ⟨_, ⟨_, withdrawal⟩⟩ WD_ADDRESS),
          amount := ← do
              let publicField ← (do
                  let publicResult ← (decode_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_AMOUNT)
                  pure ((publicResult).value))
              pure (⟨publicField⟩) })

/-- Decodes the `SszChainConfig` at `cc`: the chain id (`u64` at 0) and
the active Amsterdam `SszForkConfig` activation.
The activation point (optional block number / timestamp, `List[u64,1]`
each) must be reached by this payload: at least one bound set, none
exceeding the payload's — a future activation invalidates the block. -/
/- Type quantifiers: k_ex411376_ : Nat, k_ex411375_ : Nat, k_ex411374_ : Nat, k_ex411373_ : Nat, 0
  ≤ k_ex411373_ ∧ 0 ≤ k_ex411374_, 0 ≤ k_ex411375_, 0 ≤ k_ex411376_ -/
def decode_chain_config (cc : EvmByteSlice) (number : block_number) (timestamp : block_timestamp) : SailM ChainConfig := do
  let cc := ((cc).2).2
  let cc_length := cc.len
  let header_length := CHAIN_CONFIG_HEADER_LENGTH
  let minimum_length := CHAIN_CONFIG_MIN_LENGTH
  if ((cc_length <b header_length) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let f_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, cc⟩⟩ CC_ACTIVE_FORK_OFF)
        pure ((publicResult).value))
  if (((f_offset != 12) || (cc_length <b minimum_length)) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let f : Nat := 12
  let activation_offset ← do
    (do
        let publicResult ← (ssz_u32 ⟨_, ⟨_, cc⟩⟩ (ssz_field_offset f FC_ACTIVATION_OFF))
        pure ((publicResult).value))
  if ((activation_offset != 4) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let activation_start : Nat := 16
  let activation_fixed_end : Nat := 24
  let a : Nat := 16
  let block_number_start ← do
    (pure (activation_start + ((← (ssz_u32 ⟨_, ⟨_, cc⟩⟩
          (ssz_field_offset a FA_BLOCK_NUMBER_OFF)))).value))
  let timestamp_start ← do
    (pure (activation_start + ((← (ssz_u32 ⟨_, ⟨_, cc⟩⟩
          (ssz_field_offset a FA_TIMESTAMP_OFF)))).value))
  if ((activation_fixed_end != block_number_start) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  if ((timestamp_start <b block_number_start) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  if ((cc_length <b timestamp_start) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let bn_length := (timestamp_start - block_number_start)
  let ts_length := (cc_length - timestamp_start)
  if ((((bn_length != 0) && (bn_length != SSZ_UINT_BYTES)) || ((ts_length != 0) && (ts_length != SSZ_UINT_BYTES))) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  if (((bn_length == 0) && (ts_length == 0)) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  if ((bn_length == SSZ_UINT_BYTES) : Bool)
  then
    (do
      let activation_block ← (( do
        (do
            let publicResult ← (decode_ssz_uint ⟨_, ⟨_, cc⟩⟩ block_number_start)
            pure ((publicResult).value)) ) : SailM Nat )
      if ((number <b activation_block) : Bool)
      then sailThrow ((InvalidBlock InvalidConfig))
      else (pure ()))
  else (pure ())
  if ((ts_length == SSZ_UINT_BYTES) : Bool)
  then
    (do
      let activation_timestamp ← (( do
        (do
            let publicResult ← (decode_ssz_uint ⟨_, ⟨_, cc⟩⟩ timestamp_start)
            pure ((publicResult).value)) ) : SailM Nat )
      if ((timestamp <b activation_timestamp) : Bool)
      then sailThrow ((InvalidBlock InvalidConfig))
      else (pure ()))
  else (pure ())
  (pure { chain_id := ← (do
                let publicResult ← (decode_ssz_uint ⟨_, ⟨_, cc⟩⟩ CC_CHAIN_ID)
                pure ((publicResult).value)) })

/-- Decodes the semantic payload structure — header, chain config, body
references — without touching an encoded transaction or withdrawal
element, and installs the header and chain parameters in the kernel. -/
def decode_stateless_input (input_ref : StatelessInputRef) : SailM StatelessInput := do
  let ⟨_, ⟨_, payload⟩⟩ := input_ref.execution_payload
  let header ← do (decode_block_header_ssz input_ref)
  let chain_config ← do
    (decode_chain_config input_ref.chain_config header.number header.timestamp)
  (k_set_header header)
  writeReg k_chain_id chain_config.chain_id
  writeReg k_blob_schedule input_ref.protocol.blob_schedule
  (pure { payload := ← (pure { expected_block_hash := ← (ssz_bytes32
                                     (⟨_, ⟨_, ((input_ref.execution_payload).2).2⟩⟩ : (Sigma
                                     fun (k_off : Nat) =>
                                     (Sigma fun (k_len : Nat) => (EvmByteSliceFields k_off k_len))))
                                     PL_BLOCK_HASH),
                                 block' := { header := header,
                                             body := { transactions := input_ref.transactions,
                                                       withdrawals := input_ref.withdrawals,
                                                       block_access_list := input_ref.block_access_list } } }),
          chain_config := chain_config })

/-- Indexes the source-backed witness (nodes, codes, headers) and
installs its authenticated parent-state anchor; no witness list is
materialized. -/
def index_execution_witness (input_ref : StatelessInputRef) : SailM WitnessContext := do
  (nodedb_reset ())
  (index_witness_nodes input_ref.witness_state)
  (index_witness_codes input_ref.witness_codes)
  let witness ← do (index_witness_headers input_ref.witness_headers)
  writeReg k_parent_state_root witness.parent_state_root
  (pure witness)

/- Type quantifiers: k_transaction_off : Nat, k_transaction_len : Nat, k_public_key_off : Nat, source_valid_range(k_transaction_off, k_transaction_len)
  ∧ source_valid_range(k_public_key_off, 65) -/
def decode_transaction (transaction : (EvmByteSliceFields k_transaction_off k_transaction_len)) (public_key : (EvmByteSliceFields k_public_key_off 65)) : SailM Transaction := do
  if ((k_transaction_len ≤b (2 ^i 30)) : Bool)
  then (rlp_decode_tx transaction public_key (← readReg k_fork))
  else sailThrow ((InvalidBlock InvalidConfig))

