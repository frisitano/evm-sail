import Evm.Flow
import Evm.Prelude
import Evm.Primitives.Bytes
import Evm.Exceptions
import Evm.Host.RegionAccess
import Evm.Kernel.Scratch
import Evm.Primitives.Fork
import Evm.Primitives.ChainConfig
import Evm.Host.Code
import Evm.Lib.Ssz.Ssz
import Evm.Lib.Rlp.Decoding
import Evm.Lib.Rlp.Codecs.Transactions
import Evm.Kernel.Environment
import Evm.Kernel.Lifecycle

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
open ScratchTrieNode
open RlpResult
open Register
open PrecompileId
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

/-! # The stateless input decoder

Materialized references into the concrete `SszStatelessInput` wire format
and the stateless guest decoder. The layout constants transcribe the SSZ
schema; the decoder resolves every variable region once and hands
consumers explicit source spans. -/

/-- The input starts with a 2-byte big-endian schema id; the SSZ body
follows. -/
abbrev SSZ_BODY : Nat := 2

abbrev IN_NPR_OFF : Nat := 0

abbrev IN_WITNESS_OFF : Nat := 4

abbrev IN_CHAIN_CONFIG_OFF : Nat := 8

abbrev IN_PUBLIC_KEYS_OFF : Nat := 12

abbrev NPR_PAYLOAD_OFF : Nat := 0

abbrev NPR_VHASHES_OFF : Nat := 4

abbrev NPR_BEACON_ROOT : Nat := 8

abbrev NPR_REQUESTS_OFF : Nat := 40

abbrev REQ_DEPOSITS_OFF : Nat := 0

abbrev REQ_WITHDRAWALS_OFF : Nat := 4

abbrev REQ_CONSOLIDATIONS_OFF : Nat := 8

abbrev REQ_BUILDER_DEPOSITS_OFF : Nat := 12

abbrev REQ_BUILDER_EXITS_OFF : Nat := 16

abbrev PL_FEE_RECIPIENT : Nat := 32

abbrev PL_STATE_ROOT : Nat := 52

abbrev PL_RECEIPTS_ROOT : Nat := 84

abbrev PL_LOGS_BLOOM : Nat := 116

abbrev PL_PREV_RANDAO : Nat := 372

abbrev PL_BLOCK_NUMBER : Nat := 404

abbrev PL_GAS_LIMIT : Nat := 412

abbrev PL_GAS_USED : Nat := 420

abbrev PL_TIMESTAMP : Nat := 428

abbrev PL_EXTRA_OFF : Nat := 436

abbrev PL_BASE_FEE : Nat := 440

abbrev PL_BLOCK_HASH : Nat := 472

abbrev PL_TXS_OFF : Nat := 504

abbrev PL_WDS_OFF : Nat := 508

abbrev PL_BLOB_GAS_USED : Nat := 512

abbrev PL_EXCESS_BLOB_GAS : Nat := 520

abbrev PL_BAL_OFF : Nat := 528

abbrev PL_SLOT_NUMBER : Nat := 532

abbrev WD_SIZE : Nat := 44

abbrev WD_INDEX : Nat := 0

abbrev WD_VALIDATOR_INDEX : Nat := 8

abbrev WD_ADDRESS : Nat := 16

abbrev WD_AMOUNT : Nat := 36

abbrev CC_CHAIN_ID : Nat := 0

abbrev CC_ACTIVE_FORK_OFF : Nat := 8

abbrev FC_ACTIVATION_OFF : Nat := 0

abbrev FA_BLOCK_NUMBER_OFF : Nat := 0

abbrev FA_TIMESTAMP_OFF : Nat := 4

abbrev WIT_STATE_OFF : Nat := 0

abbrev WIT_CODES_OFF : Nat := 4

abbrev WIT_HEADERS_OFF : Nat := 8

abbrev STATELESS_INPUT_FIXED_LENGTH : Nat := 18

abbrev STATELESS_INPUT_BODY_FIXED_LENGTH : Nat := 16

abbrev NEW_PAYLOAD_REQUEST_FIXED_LENGTH : Nat := 44

abbrev EXECUTION_PAYLOAD_FIXED_LENGTH : Nat := 540

abbrev EXECUTION_REQUESTS_FIXED_LENGTH : Nat := 20

abbrev EXECUTION_WITNESS_FIXED_LENGTH : Nat := 12

abbrev PUBLIC_KEY_LENGTH : Nat := 65

abbrev CHAIN_CONFIG_HEADER_LENGTH : Nat := 12

abbrev CHAIN_CONFIG_MIN_LENGTH : Nat := 24

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
def ssz_offset_table_position (index : Nat) : Nat :=
  (index *i 4)

/-- A cursor at the first element of a variable-element SSZ list. -/
/- Type quantifiers: k_maximum : Nat, (source_valid_length k_maximum) -/
def ssz_list_cursor (items : (BoundedSszListRef k_maximum)) : SailM (BoundedSszListCursor k_maximum) := do
  let ⟨_, ⟨_, bytes⟩⟩ := items.bytes
  let current ← do
    if ((items.count != 0) : Bool)
    then
      (do
        let first_offset ← do (ssz_u32 ⟨_, ⟨_, bytes⟩⟩ 0)
        (pure (ssz_offset_to_source_pointer first_offset)))
    else (pure bytes.len)
  (pure { items := items,
          index := 0,
          current := current })

/-- Whether the cursor has consumed all elements. -/
/- Type quantifiers: k_maximum : Nat, (source_valid_length k_maximum) -/
def ssz_list_cursor_empty (cursor : (BoundedSszListCursor k_maximum)) : Bool :=
  (cursor.items.count ≤b cursor.index)

/-- The next element's span, and the advanced cursor. -/
/- Type quantifiers: k_maximum : Nat, (source_valid_length k_maximum) ∧ k_maximum ≤ (2 ^ 30 - 1) -/
def ssz_list_pop (cursor : (BoundedSszListCursor k_maximum)) : SailM ((Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) × (BoundedSszListCursor k_maximum)) := do
  let count := cursor.items.count
  let index := cursor.index
  let next_index ← (( do
    if ((index <b count) : Bool)
    then (pure (index + 1))
    else (fatal_error InvalidConfig) ) : SailM Nat )
  let items := cursor.items
  let ⟨_, ⟨_, bytes⟩⟩ := items.bytes
  let nat := bytes.len
  let next ← do
    if ((next_index <b items.count) : Bool)
    then
      (do
        let table_position := (ssz_offset_table_position next_index)
        let next_offset ← do (ssz_u32_in_slice ⟨_, ⟨_, bytes⟩⟩ table_position)
        (pure (ssz_offset_to_source_pointer next_offset)))
    else (pure nat)
  let current_value := cursor.current
  let next_value := next
  if _sailIf0 : (((current_value ≤b next_value) && (next_value ≤b nat)) : Bool) = true
  then
    (do
      let item_length := (next_value - current_value)
      if (((items.max_item_length != 0) && ((items.max_item_length <b item_length) : Bool)) : Bool)
      then (fatal_error InvalidConfig)
      else (pure ())
      let item := (stateless_input_sub_slice bytes current_value item_length)
      (pure (((⟨_, ⟨_, item⟩⟩ : (Sigma fun (k_off : Nat) =>
        (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma fun
        (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))), ({ items := items,
                                                                                                    index := next_index,
                                                                                                    current := next } : (BoundedSszListCursor k_maximum)))))
  else
    (do
      (fatal_error InvalidConfig))

/-- Returns a variable-width list item by resolving its adjacent offsets. -/
/- Type quantifiers: k_maximum : Nat, index : Nat, (source_valid_length k_maximum) ∧
  k_maximum ≤ (2 ^ 30 - 1) ∧ 0 ≤ index -/
def ssz_list_at (items : (BoundedSszListRef k_maximum)) (index : Nat) : SailM (Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) := do
  let ⟨_, ⟨_, bytes⟩⟩ := items.bytes
  let count := items.count
  let item_index ← (( do
    if ((index <b count) : Bool)
    then (pure index)
    else (fatal_error InvalidConfig) ) : SailM Nat )
  let next_index : Nat := (item_index + 1)
  let start_position := (ssz_offset_table_position item_index)
  let start_offset ← do (ssz_u32_in_slice ⟨_, ⟨_, bytes⟩⟩ start_position)
  let start := (ssz_offset_to_source_pointer start_offset)
  let stop ← (( do
    if ((next_index <b items.count) : Bool)
    then
      (do
        let stop_position := (ssz_offset_table_position next_index)
        let stop_offset ← do (ssz_u32_in_slice ⟨_, ⟨_, bytes⟩⟩ stop_position)
        (pure (ssz_offset_to_source_pointer stop_offset)))
    else (pure bytes.len) ) : SailM Nat )
  let start_value := start
  let stop_value := stop
  let items_length := bytes.len
  if _sailIf0 : (((start_value ≤b stop_value) && (stop_value ≤b items_length)) : Bool) = true
  then
    (do
      let item_length := (stop_value - start_value)
      if (((items.max_item_length != 0) && ((items.max_item_length <b item_length) : Bool)) : Bool)
      then (fatal_error InvalidConfig)
      else (pure ())
      (pure ((⟨_, ⟨_, (stateless_input_sub_slice bytes start item_length)⟩⟩ : (Sigma fun
        (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma
        fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))))))
  else
    (do
      (fatal_error InvalidConfig))

/- Type quantifiers: k_maximum : Nat, item_size : Nat, index : Nat, (source_valid_length k_maximum)
  ∧ (source_valid_length item_size) ∧ 0 ≤ index -/
def ssz_fixed_list_at (items : (BoundedSszListRef k_maximum)) (index : Nat) (item_size : Nat) : SailM (Sigma
  fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) := do
  let ⟨_, ⟨_, bytes⟩⟩ := items.bytes
  let width_value := item_size
  let offset_value := (index *i width_value)
  let items_length := bytes.len
  if _sailIf0 : (((index <b items.count) && ((offset_value + width_value) ≤b items_length)) : Bool) = true
  then
    (pure ((⟨_, ⟨_, (stateless_input_sub_slice bytes offset_value width_value)⟩⟩ : (Sigma
      fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma
      fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))))
  else
    (do
      (fatal_error InvalidConfig))

/- Type quantifiers: k_maximum : Nat, item_size : Nat, (source_valid_length k_maximum) ∧
  (source_valid_length item_size) -/
def ssz_fixed_list_pop (items : (BoundedSszListRef k_maximum)) (item_size : Nat) : SailM ((Sigma fun
  (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) × (BoundedSszListRef k_maximum)) := do
  let ⟨_, ⟨_, bytes⟩⟩ := items.bytes
  let items_length := bytes.len
  let width := item_size
  let count := items.count
  if _sailIf0 : (((0 <b count) && (width ≤b items_length)) : Bool) = true
  then
    (let item := (stateless_input_sub_slice bytes 0 item_size)
    let rest : (BoundedSszListRef k_maximum) :=
      { bytes := ⟨_, ⟨_, (stateless_input_slice_suffix bytes width)⟩⟩,
        count := (count - 1),
        max_item_length := items.max_item_length }
    (pure (((⟨_, ⟨_, item⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma fun
      (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))), (rest : (BoundedSszListRef k_maximum)))))
  else
    (do
      (fatal_error InvalidConfig))

/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, minimum : Nat, 0
  ≤ minimum, 0 ≤ bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def ssz_container_bytes (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (minimum : Nat) : SailM (Sigma
  fun (bytes_dependentWitness0 : Nat) =>
  (Sigma fun (bytes_dependentWitness1 : Nat) =>
  (StatelessInputSliceFields bytes_dependentWitness0 bytes_dependentWitness1))) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let fields := bytes
  if _sailIf0 : ((minimum ≤b fields.len) : Bool) = true
  then
    (pure ((⟨_, ⟨_, fields⟩⟩ : (Sigma fun (bytes_dependentWitness0 : Nat) =>
      (Sigma fun (bytes_dependentWitness1 : Nat) =>
      (StatelessInputSliceFields bytes_dependentWitness0 bytes_dependentWitness1)))) : (Sigma fun
      (bytes_dependentWitness0 : Nat) =>
      (Sigma fun (bytes_dependentWitness1 : Nat) =>
      (StatelessInputSliceFields bytes_dependentWitness0 bytes_dependentWitness1)))))
  else
    (do
      (fatal_error InvalidConfig))

/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, fixed_length :
  Nat, 0 ≤ fixed_length, 0 ≤ bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) ∧
  0 ≤ fixed_length ∧ fixed_length ≤ bytes_dependentWitness1 -/
def ssz_container_cursor (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (fixed_length : Nat) : SszContainerCursor :=
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  { bytes := ⟨_, ⟨_, bytes⟩⟩,
    current := fixed_length }

/-- Takes the next variable field, ending at its container-relative SSZ
offset, and returns the advanced cursor. -/
/- Type quantifiers: k_ex554016_ : Nat, 0 ≤ k_ex554016_ ∧ k_ex554016_ ≤ (2 ^ 32 - 1) -/
def ssz_take (cursor : SszContainerCursor) (stop : Nat) : SailM ((Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) × SszContainerCursor) := do
  let current_value := cursor.current
  let ⟨_, ⟨_, bytes⟩⟩ := cursor.bytes
  let container_length := bytes.len
  if _sailIf0 : (((current_value ≤b stop) && (stop ≤b container_length)) : Bool) = true
  then
    (let stop_pointer : Nat := stop
    let span_length := (stop - current_value)
    let span := (stateless_input_sub_slice bytes current_value span_length)
    (pure (((⟨_, ⟨_, span⟩⟩ : (Sigma fun (k_off : Nat) =>
      (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma fun
      (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))), ({ bytes := ⟨_, ⟨_, bytes⟩⟩,
                                                                                                  current := stop_pointer } : SszContainerCursor))))
  else
    (do
      (fatal_error InvalidConfig))

/-- Takes the remainder of a container after its last offset-delimited
field. -/
def ssz_finish (cursor : SszContainerCursor) : SailM (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))) := do
  let current_value := cursor.current
  let ⟨_, ⟨_, bytes⟩⟩ := cursor.bytes
  let container_length := bytes.len
  if _sailIf0 : ((current_value ≤b container_length) : Bool) = true
  then
    (let remaining := (container_length - current_value)
    (pure ((⟨_, ⟨_, (stateless_input_sub_slice bytes current_value remaining)⟩⟩ : (Sigma fun
      (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : (Sigma
      fun (k_off : Nat) => (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len))))))
  else
    (do
      (fatal_error InvalidConfig))

/-- Constructs a variable-item list with its SSZ count and per-item byte
limits attached. Item lengths are checked lazily when reached, before a
consumer can construct any narrower optimized value. -/
/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, maximum_count :
  Nat, maximum_item_length : Nat, (source_valid_length maximum_count) ∧
  (source_valid_length maximum_item_length), 0 ≤ bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def ssz_bounded_variable_list_ref (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (maximum_count : Nat) (maximum_item_length : Nat) : SailM (BoundedSszListRef maximum_count) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let span := bytes.len
  let raw_count ← (( do
    if ((span == 0) : Bool)
    then (pure 0)
    else
      (do
        if ((span <b SSZ_OFF_BYTES) : Bool)
        then (fatal_error InvalidConfig)
        else (pure ())
        let first_offset ← do (ssz_u32 ⟨_, ⟨_, bytes⟩⟩ 0)
        let count : Nat := (Nat.div first_offset 4)
        let offset_remainder := (Nat.mod first_offset 4)
        if (((offset_remainder != 0) || ((count == 0) || (first_offset >b span))) : Bool)
        then (fatal_error InvalidConfig)
        else (pure ())
        (pure count)) ) : SailM Nat )
  let count ← (( do
    if ((raw_count ≤b maximum_count) : Bool)
    then (pure raw_count)
    else (fatal_error InvalidConfig) ) : SailM Nat )
  (pure { bytes := ⟨_, ⟨_, bytes⟩⟩,
          count := count,
          max_item_length := maximum_item_length })

/-- Constructs a schema-bounded fixed-item SSZ list reference. -/
/- Type quantifiers: bytes_dependentWitness1 : Nat, bytes_dependentWitness0 : Nat, maximum_count :
  Nat, item_size : Nat, (source_valid_length maximum_count) ∧ (source_valid_length item_size), 0
  ≤ bytes_dependentWitness0 ∧
  0 ≤ bytes_dependentWitness1 ∧
  (bytes_dependentWitness0 + bytes_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def ssz_bounded_fixed_list_ref (bytes : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (item_size : Nat) (maximum_count : Nat) : SailM (BoundedSszListRef maximum_count) := do
  let bytes_dependentWitness0 := (bytes).1
  let bytes_dependentWitness1 := ((bytes).2).1
  let bytes := ((bytes).2).2
  let width ← (( do
    if ((0 <b item_size) : Bool)
    then (pure item_size)
    else (fatal_error InvalidConfig) ) : SailM Nat )
  let span := bytes.len
  let raw_count := (span / width)
  if ((span != (raw_count *i width)) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  let count ← (( do
    if ((raw_count ≤b maximum_count) : Bool)
    then (pure raw_count)
    else (fatal_error InvalidConfig) ) : SailM Nat )
  (pure { bytes := ⟨_, ⟨_, bytes⟩⟩,
          count := count,
          max_item_length := item_size })

/-- Resolves the input's offset tables into a
[StatelessInputRef][type-StatelessInputRef], validating the schema id
and every region bound; a malformed frame is `InvalidConfig`. -/
/- Type quantifiers: input_dependentWitness1 : Nat, input_dependentWitness0 : Nat, 0 ≤
  input_dependentWitness0 ∧
  0 ≤ input_dependentWitness1 ∧
  (input_dependentWitness0 + input_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def decode_stateless_input_ref (input : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM StatelessInputRef := do
  let input_dependentWitness0 := (input).1
  let input_dependentWitness1 := ((input).2).1
  let input := ((input).2).2
  let fixed_length := STATELESS_INPUT_FIXED_LENGTH
  let body_offset_value := SSZ_BODY
  let ⟨_, ⟨_, input_fields⟩⟩ ← (( do
    if _sailIf0 : ((fixed_length ≤b input.len) : Bool) = true
    then
      (pure ((⟨_, ⟨_, input⟩⟩ : (Sigma fun (input_dependentWitness0 : Nat) =>
        (Sigma fun (input_dependentWitness1 : Nat) =>
        (StatelessInputSliceFields input_dependentWitness0 input_dependentWitness1)))) : (Sigma fun
        (input_dependentWitness0 : Nat) =>
        (Sigma fun (input_dependentWitness1 : Nat) =>
        (StatelessInputSliceFields input_dependentWitness0 input_dependentWitness1)))))
    else
      (do
        (fatal_error InvalidConfig)) ) : SailM
    (Sigma fun (input_dependentWitness0 : Nat) =>
    (Sigma fun (input_dependentWitness1 : Nat) =>
    (StatelessInputSliceFields input_dependentWitness0 input_dependentWitness1))) )
  let schema_version ← do (stateless_input_slice_byte ⟨_, ⟨_, input_fields⟩⟩ 1)
  if ((schema_version != 0x01#8) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  let schema_fork ← do (stateless_input_slice_byte ⟨_, ⟨_, input_fields⟩⟩ 0)
  let schema_matches := (schema_protocol_profile_forwards_matches schema_fork)
  let schema_mismatch := (! schema_matches)
  if (schema_mismatch : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, protocol⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ : (Sigma
    fun (k_fork : Nat) =>
    (Sigma fun (k_target : Nat) =>
    (Sigma fun (k_maximum : Nat) =>
    (Sigma fun (k_denominator : Nat) =>
    (Sigma fun (k_code_limit : Nat) =>
    (Sigma fun (k_initcode_limit : Nat) =>
    (Sigma fun (k_transaction_total_gas_limit : Nat) =>
    (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
    (Sigma fun (k_transaction_blob_limit : Nat) =>
    (Sigma fun (k_refund_divisor : Nat) =>
    (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor))))))))))) :=
    ((schema_protocol_profile schema_fork) : (Sigma fun (k_fork : Nat) =>
    (Sigma fun (k_target : Nat) =>
    (Sigma fun (k_maximum : Nat) =>
    (Sigma fun (k_denominator : Nat) =>
    (Sigma fun (k_code_limit : Nat) =>
    (Sigma fun (k_initcode_limit : Nat) =>
    (Sigma fun (k_transaction_total_gas_limit : Nat) =>
    (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
    (Sigma fun (k_transaction_blob_limit : Nat) =>
    (Sigma fun (k_refund_divisor : Nat) =>
    (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor))))))))))))
  let body_bytes := (stateless_input_slice_suffix input_fields body_offset_value)
  let ⟨_, ⟨_, body⟩⟩ ← do
    (ssz_container_bytes ⟨_, ⟨_, body_bytes⟩⟩ STATELESS_INPUT_BODY_FIXED_LENGTH)
  let container_start : Nat := 0
  let new_payload_request_position := (ssz_field_offset container_start IN_NPR_OFF)
  let new_payload_request_offset ← do
    (ssz_u32 ⟨_, ⟨_, body⟩⟩ new_payload_request_position)
  let witness_position := (ssz_field_offset container_start IN_WITNESS_OFF)
  let witness_offset ← do (ssz_u32 ⟨_, ⟨_, body⟩⟩ witness_position)
  let chain_config_position := (ssz_field_offset container_start IN_CHAIN_CONFIG_OFF)
  let chain_config_offset ← do (ssz_u32 ⟨_, ⟨_, body⟩⟩ chain_config_position)
  let public_keys_position := (ssz_field_offset container_start IN_PUBLIC_KEYS_OFF)
  let public_keys_offset ← do (ssz_u32 ⟨_, ⟨_, body⟩⟩ public_keys_position)
  let body_fixed_length := STATELESS_INPUT_BODY_FIXED_LENGTH
  if ((new_payload_request_offset != body_fixed_length) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  let body_cursor := (ssz_container_cursor ⟨_, ⟨_, body⟩⟩ STATELESS_INPUT_BODY_FIXED_LENGTH)
  let (new_payload_request_bytes, body_after_payload_request) ← do
    (ssz_take body_cursor witness_offset)
  let ⟨_, ⟨_, new_payload_request⟩⟩ ← do
    (ssz_container_bytes new_payload_request_bytes NEW_PAYLOAD_REQUEST_FIXED_LENGTH)
  let (execution_witness_bytes, body_after_witness) ← do
    (ssz_take body_after_payload_request chain_config_offset)
  let ⟨_, ⟨_, execution_witness⟩⟩ ← do
    (ssz_container_bytes execution_witness_bytes EXECUTION_WITNESS_FIXED_LENGTH)
  let (chain_config, body_after_chain_config) ← do
    (ssz_take body_after_witness public_keys_offset)
  let ⟨_, ⟨_, public_keys⟩⟩ ← do (ssz_finish body_after_chain_config)
  let npr_start : Nat := 0
  let payload_position := (ssz_field_offset npr_start NPR_PAYLOAD_OFF)
  let payload_offset ← do (ssz_u32 ⟨_, ⟨_, new_payload_request⟩⟩ payload_position)
  let versioned_hashes_position := (ssz_field_offset npr_start NPR_VHASHES_OFF)
  let versioned_hashes_offset ← do
    (ssz_u32 ⟨_, ⟨_, new_payload_request⟩⟩ versioned_hashes_position)
  let requests_position := (ssz_field_offset npr_start NPR_REQUESTS_OFF)
  let requests_offset ← do (ssz_u32 ⟨_, ⟨_, new_payload_request⟩⟩ requests_position)
  let npr_fixed_length := NEW_PAYLOAD_REQUEST_FIXED_LENGTH
  if ((payload_offset != npr_fixed_length) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  let npr_cursor :=
    (ssz_container_cursor ⟨_, ⟨_, new_payload_request⟩⟩ NEW_PAYLOAD_REQUEST_FIXED_LENGTH)
  let (execution_payload_bytes, npr_after_payload) ← do
    (ssz_take npr_cursor versioned_hashes_offset)
  let (versioned_hashes, npr_after_versioned_hashes) ← do
    (ssz_take npr_after_payload requests_offset)
  let ⟨_, ⟨_, execution_requests_bytes⟩⟩ ← do (ssz_finish npr_after_versioned_hashes)
  let ⟨_, ⟨_, execution_requests⟩⟩ ← do
    (ssz_container_bytes ⟨_, ⟨_, execution_requests_bytes⟩⟩ EXECUTION_REQUESTS_FIXED_LENGTH)
  let ⟨_, ⟨_, execution_payload⟩⟩ ← do
    (ssz_container_bytes execution_payload_bytes EXECUTION_PAYLOAD_FIXED_LENGTH)
  let payload_start : Nat := 0
  let extra_data_position := (ssz_field_offset payload_start PL_EXTRA_OFF)
  let extra_data_offset ← do (ssz_u32 ⟨_, ⟨_, execution_payload⟩⟩ extra_data_position)
  let transactions_position := (ssz_field_offset payload_start PL_TXS_OFF)
  let transactions_offset ← do (ssz_u32 ⟨_, ⟨_, execution_payload⟩⟩ transactions_position)
  let withdrawals_position := (ssz_field_offset payload_start PL_WDS_OFF)
  let withdrawals_offset ← do (ssz_u32 ⟨_, ⟨_, execution_payload⟩⟩ withdrawals_position)
  let block_access_list_position := (ssz_field_offset payload_start PL_BAL_OFF)
  let block_access_list_offset ← do
    (ssz_u32 ⟨_, ⟨_, execution_payload⟩⟩ block_access_list_position)
  let payload_fixed_length := EXECUTION_PAYLOAD_FIXED_LENGTH
  if ((extra_data_offset != payload_fixed_length) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  let payload_cursor :=
    (ssz_container_cursor ⟨_, ⟨_, execution_payload⟩⟩ EXECUTION_PAYLOAD_FIXED_LENGTH)
  let (extra_data_bytes, payload_after_extra_data) ← do
    (ssz_take payload_cursor transactions_offset)
  let (transaction_bytes, payload_after_transactions) ← do
    (ssz_take payload_after_extra_data withdrawals_offset)
  let transactions ← do
    (ssz_bounded_variable_list_ref transaction_bytes MAX_TRANSACTIONS_PER_PAYLOAD
      MAX_TRANSACTION_LENGTH)
  let (withdrawal_bytes, payload_after_withdrawals) ← do
    (ssz_take payload_after_transactions block_access_list_offset)
  let withdrawals ← do
    (ssz_bounded_fixed_list_ref withdrawal_bytes WD_SIZE MAX_WITHDRAWALS_PER_PAYLOAD)
  let ⟨_, ⟨_, block_access_list_bytes⟩⟩ ← do (ssz_finish payload_after_withdrawals)
  let ⟨_, ⟨_, extra_data⟩⟩ ← (( do
    if _sailIf0 : ((((extra_data_bytes).2).2.len ≤b MAX_EXTRA_DATA_LENGTH) : Bool) = true
    then
      (pure ((⟨_, ⟨_, ((extra_data_bytes).2).2⟩⟩ : (Sigma fun
        (input_dependentWitness0 : Nat) =>
        (Sigma fun (input_dependentWitness1 : Nat) =>
        (StatelessInputSliceFields input_dependentWitness0 input_dependentWitness1)))) : (Sigma fun
        (input_dependentWitness0 : Nat) =>
        (Sigma fun (input_dependentWitness1 : Nat) =>
        (StatelessInputSliceFields input_dependentWitness0 input_dependentWitness1)))))
    else
      (do
        (fatal_error InvalidConfig)) ) : SailM
    (Sigma fun (input_dependentWitness0 : Nat) =>
    (Sigma fun (input_dependentWitness1 : Nat) =>
    (StatelessInputSliceFields input_dependentWitness0 input_dependentWitness1))) )
  let ⟨_, ⟨_, block_access_list⟩⟩ ← (( do
    if _sailIf0 : ((block_access_list_bytes.len ≤b MAX_BLOCK_ACCESS_LIST_LENGTH) : Bool) = true
    then
      (pure ((⟨_, ⟨_, block_access_list_bytes⟩⟩ : (Sigma fun (input_dependentWitness0 : Nat)
        =>
        (Sigma fun (input_dependentWitness1 : Nat) =>
        (StatelessInputSliceFields input_dependentWitness0 input_dependentWitness1)))) : (Sigma fun
        (input_dependentWitness0 : Nat) =>
        (Sigma fun (input_dependentWitness1 : Nat) =>
        (StatelessInputSliceFields input_dependentWitness0 input_dependentWitness1)))))
    else
      (do
        (fatal_error InvalidConfig)) ) : SailM
    (Sigma fun (input_dependentWitness0 : Nat) =>
    (Sigma fun (input_dependentWitness1 : Nat) =>
    (StatelessInputSliceFields input_dependentWitness0 input_dependentWitness1))) )
  let requests_start : Nat := 0
  let deposits_position := (ssz_field_offset requests_start REQ_DEPOSITS_OFF)
  let deposits_offset ← do (ssz_u32 ⟨_, ⟨_, execution_requests⟩⟩ deposits_position)
  let withdrawal_requests_position := (ssz_field_offset requests_start REQ_WITHDRAWALS_OFF)
  let withdrawal_requests_offset ← do
    (ssz_u32 ⟨_, ⟨_, execution_requests⟩⟩ withdrawal_requests_position)
  let consolidation_requests_position := (ssz_field_offset requests_start REQ_CONSOLIDATIONS_OFF)
  let consolidation_requests_offset ← do
    (ssz_u32 ⟨_, ⟨_, execution_requests⟩⟩ consolidation_requests_position)
  let builder_deposit_requests_position :=
    (ssz_field_offset requests_start REQ_BUILDER_DEPOSITS_OFF)
  let builder_deposit_requests_offset ← do
    (ssz_u32 ⟨_, ⟨_, execution_requests⟩⟩ builder_deposit_requests_position)
  let builder_exit_requests_position := (ssz_field_offset requests_start REQ_BUILDER_EXITS_OFF)
  let builder_exit_requests_offset ← do
    (ssz_u32 ⟨_, ⟨_, execution_requests⟩⟩ builder_exit_requests_position)
  let requests_fixed_length := EXECUTION_REQUESTS_FIXED_LENGTH
  if ((deposits_offset != requests_fixed_length) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  let requests_cursor :=
    (ssz_container_cursor ⟨_, ⟨_, execution_requests⟩⟩ EXECUTION_REQUESTS_FIXED_LENGTH)
  let (deposits, requests_after_deposits) ← do
    (ssz_take requests_cursor withdrawal_requests_offset)
  let (withdrawal_requests, requests_after_withdrawals) ← do
    (ssz_take requests_after_deposits consolidation_requests_offset)
  let (consolidation_requests, requests_after_consolidations) ← do
    (ssz_take requests_after_withdrawals builder_deposit_requests_offset)
  let (builder_deposit_requests, requests_after_builder_deposits) ← do
    (ssz_take requests_after_consolidations builder_exit_requests_offset)
  let ⟨_, ⟨_, builder_exit_requests⟩⟩ ← do (ssz_finish requests_after_builder_deposits)
  let witness_start : Nat := 0
  let witness_state_position := (ssz_field_offset witness_start WIT_STATE_OFF)
  let witness_state_offset ← do
    (ssz_u32 ⟨_, ⟨_, execution_witness⟩⟩ witness_state_position)
  let witness_codes_position := (ssz_field_offset witness_start WIT_CODES_OFF)
  let witness_codes_offset ← do
    (ssz_u32 ⟨_, ⟨_, execution_witness⟩⟩ witness_codes_position)
  let witness_headers_position := (ssz_field_offset witness_start WIT_HEADERS_OFF)
  let witness_headers_offset ← do
    (ssz_u32 ⟨_, ⟨_, execution_witness⟩⟩ witness_headers_position)
  let witness_fixed_length := EXECUTION_WITNESS_FIXED_LENGTH
  if ((witness_state_offset != witness_fixed_length) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  let witness_cursor :=
    (ssz_container_cursor ⟨_, ⟨_, execution_witness⟩⟩ EXECUTION_WITNESS_FIXED_LENGTH)
  let (witness_state_bytes, witness_after_state) ← do
    (ssz_take witness_cursor witness_codes_offset)
  let witness_state ← do
    (ssz_bounded_variable_list_ref witness_state_bytes MAX_WITNESS_NODES MAX_WITNESS_NODE_LENGTH)
  let (witness_code_bytes, witness_after_codes) ← do
    (ssz_take witness_after_state witness_headers_offset)
  let witness_codes ← do
    (ssz_bounded_variable_list_ref witness_code_bytes MAX_WITNESS_CODES MAX_WITNESS_CODE_LENGTH)
  let ⟨_, ⟨_, witness_header_bytes⟩⟩ ← do (ssz_finish witness_after_codes)
  let witness_headers ← do
    (ssz_bounded_variable_list_ref ⟨_, ⟨_, witness_header_bytes⟩⟩ MAX_WITNESS_HEADERS
      MAX_WITNESS_HEADER_LENGTH)
  let public_key_bytes := public_keys.len
  let public_key_length := PUBLIC_KEY_LENGTH
  let public_key_count := (public_key_bytes / public_key_length)
  if ((public_key_bytes != (public_key_count *i public_key_length)) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  if ((MAX_PUBLIC_KEYS <b public_key_count) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  let _ ← do
    (ssz_bounded_fixed_list_ref versioned_hashes WORD_BYTE_LENGTH MAX_BLOB_COMMITMENTS_PER_BLOCK)
  (pure { protocol := ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, protocol⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩,
          new_payload_request := ⟨_, ⟨_, new_payload_request⟩⟩,
          execution_payload := ⟨_, ⟨_, execution_payload⟩⟩,
          versioned_hashes := versioned_hashes,
          deposits := deposits,
          withdrawal_requests := withdrawal_requests,
          consolidation_requests := consolidation_requests,
          builder_deposit_requests := builder_deposit_requests,
          builder_exit_requests := ⟨_, ⟨_, builder_exit_requests⟩⟩,
          extra_data := ⟨_, ⟨_, extra_data⟩⟩,
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
/- Type quantifiers: s_dependentWitness1 : Nat, s_dependentWitness0 : Nat, 0 ≤ s_dependentWitness0
  ∧ 0 ≤ s_dependentWitness1 ∧ (s_dependentWitness0 + s_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def sha256_request_digest (request_type : (BitVec 8)) (s : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM (Vector (BitVec 8) 32) := do
  let s_dependentWitness0 := (s).1
  let s_dependentWitness1 := ((s).2).1
  let s := ((s).2).2
  let digest_length ← do (scratch_length_add 1 s.len)
  let mark ← do (scratch_reserve digest_length)
  (scratch_push_byte request_type)
  (stateless_input_scratch_push_slice ⟨_, ⟨_, s⟩⟩)
  let ⟨_, ⟨_, preimage⟩⟩ ← do (scratch_finish mark)
  let digest ← do (scratch_sha256 ⟨_, ⟨_, preimage⟩⟩)
  (scratch_rewind mark)
  (pure digest)

/-- Hashes and indexes every remaining witness trie node from its source
slice. -/
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_index_witness_nodes_cursor (cursor : (BoundedSszListCursor (2 ^ 22))) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      let cursor_empty := (ssz_list_cursor_empty cursor)
      if (cursor_empty : Bool)
      then (pure ())
      else
        (do
          let (node, next) ← do (ssz_list_pop cursor)
          let node_hash ← do (stateless_input_keccak256 node)
          (nodedb_insert node_hash ((node).2).2.bytes ((node).2).2.len)
          (_rec_index_witness_nodes_cursor next _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Hashes and indexes every remaining witness trie node from its source
slice. -/
def index_witness_nodes_cursor (cursor : (BoundedSszListCursor (2 ^ 22))) : SailM Unit := do
  let _measure := ((cursor.items.count -i cursor.index) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_index_witness_nodes_cursor cursor (_measure + 1))

/-- Indexes every witness trie node into the node-db, keyed by its
KECCAK-256 digest, directly from the SSZ list reference. -/
def index_witness_nodes (nodes : (BoundedSszListRef (2 ^ 22))) : SailM Unit := do
  let cursor ← do (ssz_list_cursor nodes)
  (index_witness_nodes_cursor cursor)

/-- Analyzes and indexes every remaining witness code body from its source
slice. -/
/- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_index_witness_codes_cursor (cursor : (BoundedSszListCursor (2 ^ 18))) (_reclimit : Nat) : SailM Unit := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
        readReg k_execution_profile
      let profile := execution_profile.protocol
      let cursor_empty := (ssz_list_cursor_empty cursor)
      if (cursor_empty : Bool)
      then (pure ())
      else
        (do
          let (code, next) ← do (ssz_list_pop cursor)
          let code_length := ((code).2).2.len
          if ((MAX_WITNESS_CODE_LENGTH <b code_length) : Bool)
          then (fatal_error InvalidConfig)
          else (pure ())
          let ⟨_, ⟨_, executable⟩⟩ ← do (code_db_intern_input code)
          let _ ← do (code_db_insert ⟨_, ⟨_, executable⟩⟩ profile.fork)
          (_rec_index_witness_codes_cursor next _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Analyzes and indexes every remaining witness code body from its source
slice. -/
def index_witness_codes_cursor (cursor : (BoundedSszListCursor (2 ^ 18))) : SailM Unit := do
  let _measure := ((cursor.items.count -i cursor.index) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_index_witness_codes_cursor cursor (_measure + 1))

/-- Indexes every witness code blob into the content-addressed code
store. -/
def index_witness_codes (codes : (BoundedSszListRef (2 ^ 18))) : SailM Unit := do
  let cursor ← do (ssz_list_cursor codes)
  (index_witness_codes_cursor cursor)

def EMPTY_PARENT_HEADER_FIELDS : ParentHeaderFields :=
  { parent_hash := ZERO_HASH,
    state_root := ZERO_HASH,
    base_fee := ZERO_WORD,
    blob_gas_used := 0,
    excess_blob_gas := 0,
    have_parent := false,
    have_state := false,
    have_base_fee := false,
    have_blob_gas := false,
    have_excess_blob_gas := false }

/-- Advances the parent-header field cursor, saturating at its unused-field
sentinel. -/
/- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ 19 -/
def next_parent_header_field (index : Nat) : Nat :=
  if ((index <b 19) : Bool)
  then (index + 1)
  else 19

/-- Extracts the execution-relevant fields while walking one parent header. -/
/- Type quantifiers: _reclimit : Nat, k_ex554087_ : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ k_ex554087_ ∧ k_ex554087_ ≤ 19, 0 ≤ _reclimit -/
def _rec_decode_parent_header_fields (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (field_index : Nat) (fields : ParentHeaderFields) (_reclimit : Nat) : SailM ParentHeaderFields := do
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((k_source_len == 0) : Bool)
      then (pure fields)
      else
        (do
          let ⟨_, ⟨_, field⟩⟩ ← do (rlp_decode_item cursor)
          let next := (rlp_cursor_advance cursor field.source.len)
          let decoded := fields
          let decoded ← (( do
            if ((field_index == 0) : Bool)
            then
              (do
                let parent_hash_word ← do (rlp_decode_word field)
                let decoded : ParentHeaderFields :=
                  { decoded with parent_hash := (word_to_hash parent_hash_word) }
                (pure { decoded with have_parent := true }))
            else
              (do
                if ((field_index == 3) : Bool)
                then
                  (do
                    let state_root_word ← do (rlp_decode_word field)
                    let decoded : ParentHeaderFields :=
                      { decoded with state_root := (word_to_hash state_root_word) }
                    (pure { decoded with have_state := true }))
                else
                  (do
                    if ((field_index == 15) : Bool)
                    then
                      (do
                        let decoded ←
                          (pure { decoded with base_fee := ← (rlp_decode_u256 field) })
                        (pure { decoded with have_base_fee := true }))
                    else
                      (do
                        if ((field_index == 17) : Bool)
                        then
                          (do
                            let value ← do (rlp_decode_uint64 field)
                            let count := (value / (2 ^i 17))
                            let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
                              readReg k_execution_profile
                            let profile := execution_profile.protocol
                            if (((count ≤b profile.blob_schedule.max) && (value == ((2 ^i 17) *i count))) : Bool)
                            then
                              (let decoded : ParentHeaderFields :=
                                { decoded with blob_gas_used := ((2 ^i 17) *i count) }
                              (pure { decoded with have_blob_gas := true }))
                            else
                              (do
                                (fatal_error RlpDecode)
                                (pure decoded)))
                        else
                          (do
                            if ((field_index == 18) : Bool)
                            then
                              (do
                                let value ← do (rlp_decode_uint64 field)
                                let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
                                  readReg k_execution_profile
                                let profile := execution_profile.protocol
                                let limit := profile.excess_blob_gas_limit
                                if ((value ≤b limit) : Bool)
                                then
                                  (let decoded : ParentHeaderFields :=
                                    { decoded with excess_blob_gas := value }
                                  (pure { decoded with have_excess_blob_gas := true }))
                                else
                                  (do
                                    (fatal_error RlpDecode)
                                    (pure decoded)))
                            else (pure decoded))))) ) : SailM ParentHeaderFields )
          let next_field := (next_parent_header_field field_index)
          (_rec_decode_parent_header_fields next next_field decoded _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Extracts the execution-relevant fields while walking one parent header. -/
/- Type quantifiers: field_index : Nat, k_source_off : Nat, k_source_len : Nat, (source_valid_range k_source_off k_source_len), 0
  ≤ field_index ∧ field_index ≤ 19 -/
def decode_parent_header_fields (cursor : (StatelessInputSliceFields k_source_off k_source_len)) (field_index : Nat) (fields : ParentHeaderFields) : SailM ParentHeaderFields := do
  let _measure := (k_source_len : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_decode_parent_header_fields cursor field_index fields (_measure + 1))

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
      let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
        readReg k_execution_profile
      let profile := execution_profile.protocol
      let cursor_empty := (ssz_list_cursor_empty state.cursor)
      if (cursor_empty : Bool)
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
                let ⟨_, ⟨_, fields⟩⟩ ← do (rlp_node_cursor header)
                let decoded ← do (decode_parent_header_fields fields 0 EMPTY_PARENT_HEADER_FIELDS)
                let parent_missing := (! decoded.have_parent)
                let result : WitnessHeaderIndex :=
                  if (((index != 0) && (parent_missing || (bne decoded.parent_hash
                           state.previous_hash))) : Bool)
                  then { result with valid := false }
                  else result
                if (is_last : Bool)
                then
                  (let result : WitnessHeaderIndex :=
                    { result with parent_state_root := decoded.state_root }
                  let result : WitnessHeaderIndex :=
                    { result with parent_base_fee_per_gas := decoded.base_fee }
                  let result : WitnessHeaderIndex :=
                    { result with parent_blob_gas_used := decoded.blob_gas_used }
                  let result : WitnessHeaderIndex :=
                    { result with parent_excess_blob_gas := decoded.excess_blob_gas }
                  (pure { result with parent_fields_valid := (decoded.have_state && (((profile.fork <b Cancun) || decoded.have_base_fee) && ((profile.fork <b Cancun) || (decoded.have_blob_gas == decoded.have_excess_blob_gas)))) }))
                else (pure result))
            else (pure result) ) : SailM WitnessHeaderIndex )
          let current_hash ← do (stateless_input_keccak256 header)
          let result : WitnessHeaderIndex := { result with previous_hash := current_hash }
          let header_count := state.cursor.items.count
          let next_index := next.index
          let distance ← (( do
            if ((next_index ≤b header_count) : Bool)
            then (pure (header_count - next_index))
            else (fatal_error WitnessDeficient) ) : SailM Nat )
          if ((distance <b 256) : Bool)
          then
            (do
              let ancestor : Nat := distance
              (ancestor_hash_write ancestor current_hash))
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
def index_witness_headers (headers : (BoundedSszListRef (2 ^ 8))) : SailM WitnessContext := do
  let cursor ← do (ssz_list_cursor headers)
  let initial : WitnessHeaderIndex :=
    { cursor := cursor,
      previous_hash := ZERO_HASH,
      valid := (headers.count != 0),
      parent_state_root := ZERO_HASH,
      parent_base_fee_per_gas := ZERO_WORD,
      parent_blob_gas_used := 0,
      parent_excess_blob_gas := 0,
      parent_fields_valid := false }
  let indexed ← do (index_witness_header_cursor initial)
  writeReg k_n_headers headers.count
  let invalid := (! indexed.valid)
  if (invalid : Bool)
  then (fatal_error WitnessDeficient)
  else (pure ())
  let parent_fields_invalid := (! indexed.parent_fields_valid)
  if (parent_fields_invalid : Bool)
  then (fatal_error RlpDecode)
  else (pure ())
  (pure { parent_hash := indexed.previous_hash,
          parent_state_root := indexed.parent_state_root,
          parent_base_fee_per_gas := indexed.parent_base_fee_per_gas,
          parent_blob_gas_used := indexed.parent_blob_gas_used,
          parent_excess_blob_gas := indexed.parent_excess_blob_gas })

/-- Decodes the execution-payload header fields from their fixed SSZ
offsets. -/
/- Type quantifiers: profile_dependentWitness9 : Nat, profile_dependentWitness8 : Nat, profile_dependentWitness7
  : Nat, profile_dependentWitness6 : Nat, profile_dependentWitness5 : Nat, profile_dependentWitness4
  : Nat, profile_dependentWitness3 : Nat, profile_dependentWitness2 : Nat, profile_dependentWitness1
  : Nat, profile_dependentWitness0 : Nat, payload_dependentWitness1 : Nat, payload_dependentWitness0
  : Nat, 0 ≤ payload_dependentWitness0 ∧
  0 ≤ payload_dependentWitness1 ∧
  (payload_dependentWitness0 + payload_dependentWitness1) ≤ (2 ^ 32 - 1), profile_dependentWitness0
  = 5 ∧
  profile_dependentWitness1 = 0 ∧
  profile_dependentWitness2 = 0 ∧
  profile_dependentWitness3 = 1 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 0 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 0 ∧ profile_dependentWitness9 = 2 ∨
  6 ≤ profile_dependentWitness0 ∧ profile_dependentWitness0 ≤ 9 ∧
  profile_dependentWitness1 = 0 ∧
  profile_dependentWitness2 = 0 ∧
  profile_dependentWitness3 = 1 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 0 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 0 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 10 ∧
  profile_dependentWitness1 = 0 ∧
  profile_dependentWitness2 = 0 ∧
  profile_dependentWitness3 = 1 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 0 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 11 ∧
  profile_dependentWitness1 = 3 ∧
  profile_dependentWitness2 = 6 ∧
  profile_dependentWitness3 = 3338477 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 12 ∧
  profile_dependentWitness1 = 6 ∧
  profile_dependentWitness2 = 9 ∧
  profile_dependentWitness3 = 5007716 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 9 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 13 ∧
  profile_dependentWitness1 = 6 ∧
  profile_dependentWitness2 = 9 ∧
  profile_dependentWitness3 = 5007716 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 24) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 14 ∧
  profile_dependentWitness1 = 10 ∧
  profile_dependentWitness2 = 15 ∧
  profile_dependentWitness3 = 8346193 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 24) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 15 ∧
  profile_dependentWitness1 = 14 ∧
  profile_dependentWitness2 = 21 ∧
  profile_dependentWitness3 = 11684671 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 24) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 16 ∧
  profile_dependentWitness1 = 14 ∧
  profile_dependentWitness2 = 21 ∧
  profile_dependentWitness3 = 11684671 ∧
  profile_dependentWitness4 = 65536 ∧
  profile_dependentWitness5 = 131072 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 -/
def decode_payload_blob_gas_used (payload : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (profile : (Sigma fun
  (k_fork : Nat) =>
  (Sigma fun (k_target : Nat) =>
  (Sigma fun (k_maximum : Nat) =>
  (Sigma fun (k_denominator : Nat) =>
  (Sigma fun (k_code_limit : Nat) =>
  (Sigma fun (k_initcode_limit : Nat) =>
  (Sigma fun (k_transaction_total_gas_limit : Nat) =>
  (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
  (Sigma fun (k_transaction_blob_limit : Nat) =>
  (Sigma fun (k_refund_divisor : Nat) =>
  (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))) : SailM Nat := do
  let payload_dependentWitness0 := (payload).1
  let payload_dependentWitness1 := ((payload).2).1
  let payload := ((payload).2).2
  let profile_dependentWitness0 := (profile).1
  let profile_dependentWitness1 := ((profile).2).1
  let profile_dependentWitness2 := (((profile).2).2).1
  let profile_dependentWitness3 := ((((profile).2).2).2).1
  let profile_dependentWitness4 := (((((profile).2).2).2).2).1
  let profile_dependentWitness5 := ((((((profile).2).2).2).2).2).1
  let profile_dependentWitness6 := (((((((profile).2).2).2).2).2).2).1
  let profile_dependentWitness7 := ((((((((profile).2).2).2).2).2).2).2).1
  let profile_dependentWitness8 := (((((((((profile).2).2).2).2).2).2).2).2).1
  let profile_dependentWitness9 := ((((((((((profile).2).2).2).2).2).2).2).2).2).1
  let profile := ((((((((((profile).2).2).2).2).2).2).2).2).2).2
  let value ← do (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_BLOB_GAS_USED)
  let count := (value / (2 ^i 17))
  if (((count ≤b profile.blob_schedule.max) && (value == ((2 ^i 17) *i count))) : Bool)
  then (pure ((2 ^i 17) *i count))
  else (fatal_error InvalidBlobGasUsed)

/-- Narrows the SSZ `uint64` excess field to the supported-fork
reachable-chain invariant. The wider wire value remains explicit in
`excess_blob_gas_wire_bound`; this check relies on an authenticated,
previously valid parent chain, not on SSZ alone. -/
/- Type quantifiers: profile_dependentWitness9 : Nat, profile_dependentWitness8 : Nat, profile_dependentWitness7
  : Nat, profile_dependentWitness6 : Nat, profile_dependentWitness5 : Nat, profile_dependentWitness4
  : Nat, profile_dependentWitness3 : Nat, profile_dependentWitness2 : Nat, profile_dependentWitness1
  : Nat, profile_dependentWitness0 : Nat, payload_dependentWitness1 : Nat, payload_dependentWitness0
  : Nat, 0 ≤ payload_dependentWitness0 ∧
  0 ≤ payload_dependentWitness1 ∧
  (payload_dependentWitness0 + payload_dependentWitness1) ≤ (2 ^ 32 - 1), profile_dependentWitness0
  = 5 ∧
  profile_dependentWitness1 = 0 ∧
  profile_dependentWitness2 = 0 ∧
  profile_dependentWitness3 = 1 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 0 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 0 ∧ profile_dependentWitness9 = 2 ∨
  6 ≤ profile_dependentWitness0 ∧ profile_dependentWitness0 ≤ 9 ∧
  profile_dependentWitness1 = 0 ∧
  profile_dependentWitness2 = 0 ∧
  profile_dependentWitness3 = 1 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 0 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 0 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 10 ∧
  profile_dependentWitness1 = 0 ∧
  profile_dependentWitness2 = 0 ∧
  profile_dependentWitness3 = 1 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 0 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 11 ∧
  profile_dependentWitness1 = 3 ∧
  profile_dependentWitness2 = 6 ∧
  profile_dependentWitness3 = 3338477 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 12 ∧
  profile_dependentWitness1 = 6 ∧
  profile_dependentWitness2 = 9 ∧
  profile_dependentWitness3 = 5007716 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 64 - 1) ∧
  profile_dependentWitness8 = 9 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 13 ∧
  profile_dependentWitness1 = 6 ∧
  profile_dependentWitness2 = 9 ∧
  profile_dependentWitness3 = 5007716 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 24) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 14 ∧
  profile_dependentWitness1 = 10 ∧
  profile_dependentWitness2 = 15 ∧
  profile_dependentWitness3 = 8346193 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 24) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 15 ∧
  profile_dependentWitness1 = 14 ∧
  profile_dependentWitness2 = 21 ∧
  profile_dependentWitness3 = 11684671 ∧
  profile_dependentWitness4 = 24576 ∧
  profile_dependentWitness5 = 49152 ∧
  profile_dependentWitness6 = (2 ^ 24) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 ∨
  profile_dependentWitness0 = 16 ∧
  profile_dependentWitness1 = 14 ∧
  profile_dependentWitness2 = 21 ∧
  profile_dependentWitness3 = 11684671 ∧
  profile_dependentWitness4 = 65536 ∧
  profile_dependentWitness5 = 131072 ∧
  profile_dependentWitness6 = (2 ^ 64 - 1) ∧
  profile_dependentWitness7 = (2 ^ 24) ∧
  profile_dependentWitness8 = 6 ∧ profile_dependentWitness9 = 5 -/
def decode_payload_excess_blob_gas (payload : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (profile : (Sigma fun
  (k_fork : Nat) =>
  (Sigma fun (k_target : Nat) =>
  (Sigma fun (k_maximum : Nat) =>
  (Sigma fun (k_denominator : Nat) =>
  (Sigma fun (k_code_limit : Nat) =>
  (Sigma fun (k_initcode_limit : Nat) =>
  (Sigma fun (k_transaction_total_gas_limit : Nat) =>
  (Sigma fun (k_transaction_regular_gas_limit : Nat) =>
  (Sigma fun (k_transaction_blob_limit : Nat) =>
  (Sigma fun (k_refund_divisor : Nat) =>
  (ProtocolProfileFields k_fork k_target k_maximum k_denominator k_code_limit k_initcode_limit k_transaction_total_gas_limit k_transaction_regular_gas_limit k_transaction_blob_limit k_refund_divisor)))))))))))) : SailM Nat := do
  let payload_dependentWitness0 := (payload).1
  let payload_dependentWitness1 := ((payload).2).1
  let payload := ((payload).2).2
  let profile_dependentWitness0 := (profile).1
  let profile_dependentWitness1 := ((profile).2).1
  let profile_dependentWitness2 := (((profile).2).2).1
  let profile_dependentWitness3 := ((((profile).2).2).2).1
  let profile_dependentWitness4 := (((((profile).2).2).2).2).1
  let profile_dependentWitness5 := ((((((profile).2).2).2).2).2).1
  let profile_dependentWitness6 := (((((((profile).2).2).2).2).2).2).1
  let profile_dependentWitness7 := ((((((((profile).2).2).2).2).2).2).2).1
  let profile_dependentWitness8 := (((((((((profile).2).2).2).2).2).2).2).2).1
  let profile_dependentWitness9 := ((((((((((profile).2).2).2).2).2).2).2).2).2).1
  let profile := ((((((((((profile).2).2).2).2).2).2).2).2).2).2
  let value ← do (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_EXCESS_BLOB_GAS)
  let limit := profile.excess_blob_gas_limit
  if ((value ≤b limit) : Bool)
  then (pure value)
  else (fatal_error InvalidExcessBlobGas)

/-- Decodes the execution-payload header fields from their fixed SSZ
offsets. -/
def decode_block_header_ssz (input_ref : StatelessInputRef) : SailM BlockHeader := do
  let ⟨_, ⟨_, payload⟩⟩ := input_ref.execution_payload
  let gas_limit_value ← do (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_GAS_LIMIT)
  let gas_used_value ← do (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_GAS_USED)
  let prev_randao_hash ← do (ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ PL_PREV_RANDAO)
  let prev_randao := (hash_to_word prev_randao_hash)
  (pure { number := ← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_BLOCK_NUMBER),
          timestamp := ← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_TIMESTAMP),
          gas_limit := gas_limit_value,
          gas_used := gas_used_value,
          prev_randao := prev_randao,
          base_fee := ← (ssz_u256 ⟨_, ⟨_, payload⟩⟩ PL_BASE_FEE),
          blob_gas_used := ← (decode_payload_blob_gas_used ⟨_, ⟨_, payload⟩⟩
              input_ref.protocol),
          excess_blob_gas := ← (decode_payload_excess_blob_gas ⟨_, ⟨_, payload⟩⟩
              input_ref.protocol),
          state_root := ← (ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ PL_STATE_ROOT),
          receipts_root := ← (ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ PL_RECEIPTS_ROOT),
          logs_bloom := ⟨_, ⟨_, (stateless_input_sub_slice payload PL_LOGS_BLOOM 256)⟩⟩,
          fee_recipient := ← (ssz_addr ⟨_, ⟨_, payload⟩⟩ PL_FEE_RECIPIENT),
          parent_hash := ← (ssz_bytes32 ⟨_, ⟨_, payload⟩⟩ 0),
          parent_beacon_block_root := ← (ssz_bytes32 input_ref.new_payload_request NPR_BEACON_ROOT),
          slot_number := ← (decode_ssz_uint ⟨_, ⟨_, payload⟩⟩ PL_SLOT_NUMBER),
          extra_data := input_ref.extra_data })

/-- Decodes one fixed-layout SSZ withdrawal element. -/
/- Type quantifiers: withdrawal_dependentWitness1 : Nat, withdrawal_dependentWitness0 : Nat, 0 ≤
  withdrawal_dependentWitness0 ∧
  0 ≤ withdrawal_dependentWitness1 ∧
  (withdrawal_dependentWitness0 + withdrawal_dependentWitness1) ≤ (2 ^ 32 - 1) -/
def decode_withdrawal (withdrawal : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) : SailM Withdrawal := do
  let withdrawal_dependentWitness0 := (withdrawal).1
  let withdrawal_dependentWitness1 := ((withdrawal).2).1
  let withdrawal := ((withdrawal).2).2
  (pure { index := ← (decode_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_INDEX),
          validator_index := ← (decode_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_VALIDATOR_INDEX),
          address := ← (ssz_addr ⟨_, ⟨_, withdrawal⟩⟩ WD_ADDRESS),
          amount := ← (decode_ssz_uint ⟨_, ⟨_, withdrawal⟩⟩ WD_AMOUNT) })

/-- Decodes the `SszChainConfig` at `cc`: the chain id (`u64` at 0) and
the active Amsterdam `SszForkConfig` activation.
The activation point (optional block number / timestamp, `List[u64,1]`
each) must be reached by this payload: at least one bound set, none
exceeding the payload's — a future activation invalidates the block. -/
/- Type quantifiers: k_ex554195_ : Nat, k_ex554194_ : Nat, cc_dependentWitness1 : Nat, cc_dependentWitness0
  : Nat, 0 ≤ cc_dependentWitness0 ∧
  0 ≤ cc_dependentWitness1 ∧ (cc_dependentWitness0 + cc_dependentWitness1) ≤ (2 ^ 32 - 1), 0
  ≤ k_ex554194_ ∧ k_ex554194_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex554195_ ∧
  k_ex554195_ ≤ (2 ^ 64 - 1) -/
def decode_chain_config (cc : (Sigma fun (k_off : Nat) =>
  (Sigma fun (k_len : Nat) => (StatelessInputSliceFields k_off k_len)))) (number : Nat) (timestamp : Nat) : SailM ChainConfig := do
  let cc_dependentWitness0 := (cc).1
  let cc_dependentWitness1 := ((cc).2).1
  let cc := ((cc).2).2
  let cc_length := cc.len
  let header_length := CHAIN_CONFIG_HEADER_LENGTH
  let minimum_length := CHAIN_CONFIG_MIN_LENGTH
  if ((cc_length <b header_length) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  let f_offset ← do (ssz_u32 ⟨_, ⟨_, cc⟩⟩ CC_ACTIVE_FORK_OFF)
  if (((f_offset != 12) || (cc_length <b minimum_length)) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  let f : Nat := 12
  let activation_position := (ssz_field_offset f FC_ACTIVATION_OFF)
  let activation_offset ← do (ssz_u32 ⟨_, ⟨_, cc⟩⟩ activation_position)
  if ((activation_offset != 4) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  let activation_start : Nat := 16
  let activation_fixed_end : Nat := 24
  let a : Nat := 16
  let block_number_position := (ssz_field_offset a FA_BLOCK_NUMBER_OFF)
  let block_number_offset ← do (ssz_u32 ⟨_, ⟨_, cc⟩⟩ block_number_position)
  let timestamp_position := (ssz_field_offset a FA_TIMESTAMP_OFF)
  let timestamp_offset ← do (ssz_u32 ⟨_, ⟨_, cc⟩⟩ timestamp_position)
  let block_number_start ← (( do
    if ((block_number_offset ≤b (cc_length -i activation_start)) : Bool)
    then (pure (activation_start + block_number_offset))
    else (fatal_error InvalidConfig) ) : SailM Nat )
  let timestamp_start ← (( do
    if ((timestamp_offset ≤b (cc_length -i activation_start)) : Bool)
    then (pure (activation_start + timestamp_offset))
    else (fatal_error InvalidConfig) ) : SailM Nat )
  if ((activation_fixed_end != block_number_start) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  if ((timestamp_start <b block_number_start) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  if ((cc_length <b timestamp_start) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  let bn_length := (timestamp_start -i block_number_start)
  let ts_length := (cc_length -i timestamp_start)
  if ((((bn_length != 0) && (bn_length != SSZ_UINT_BYTES)) || ((ts_length != 0) && (ts_length != SSZ_UINT_BYTES))) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  if (((bn_length == 0) && (ts_length == 0)) : Bool)
  then (fatal_error InvalidConfig)
  else (pure ())
  if ((bn_length == SSZ_UINT_BYTES) : Bool)
  then
    (do
      let activation_block ← (( do (decode_ssz_uint ⟨_, ⟨_, cc⟩⟩ block_number_start) ) :
        SailM Nat )
      if ((number <b activation_block) : Bool)
      then (fatal_error InvalidConfig)
      else (pure ()))
  else (pure ())
  if ((ts_length == SSZ_UINT_BYTES) : Bool)
  then
    (do
      let activation_timestamp ← (( do (decode_ssz_uint ⟨_, ⟨_, cc⟩⟩ timestamp_start) ) :
        SailM Nat )
      if ((timestamp <b activation_timestamp) : Bool)
      then (fatal_error InvalidConfig)
      else (pure ()))
  else (pure ())
  (pure { chain_id := ← (decode_ssz_uint ⟨_, ⟨_, cc⟩⟩ CC_CHAIN_ID) })

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
  writeReg k_execution_profile ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ((((((((((((⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, (execution_profile_for
    ((((((((((input_ref.protocol).2).2).2).2).2).2).2).2).2).2 header.gas_limit)⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ : (Sigma
    fun (k_ex612300_ : Nat) =>
    (Sigma fun (k_ex612301_ : Nat) =>
    (Sigma fun (k_ex612302_ : Nat) =>
    (Sigma fun (k_ex612303_ : Nat) =>
    (Sigma fun (k_ex612304_ : Nat) =>
    (Sigma fun (k_ex612305_ : Nat) =>
    (Sigma fun (k_ex612306_ : Nat) =>
    (Sigma fun (k_ex612307_ : Nat) =>
    (Sigma fun (k_ex612308_ : Nat) =>
    (Sigma fun (k_ex612309_ : Nat) =>
    (Sigma fun (k_ex612310_ : Nat) =>
    (ExecutionProfileFields k_ex612300_ k_ex612301_ k_ex612302_ k_ex612303_ k_ex612304_ k_ex612305_ k_ex612306_ k_ex612307_ k_ex612308_ k_ex612309_ k_ex612310_ (if ( k_ex612310_
    < k_ex612306_  : Bool) then k_ex612310_ else k_ex612306_) (if ( (if ( k_ex612310_ < k_ex612306_  : Bool) then k_ex612310_ else k_ex612306_)
    < k_ex612307_  : Bool) then (if ( k_ex612310_ < k_ex612306_  : Bool) then k_ex612310_ else k_ex612306_) else k_ex612307_))))))))))))))).2).2).2).2).2).2).2).2).2).2).2⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩
  (pure { payload := ← (pure { expected_block_hash := ← (ssz_bytes32 input_ref.execution_payload
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

/- Type quantifiers: k_transaction_off : Nat, k_transaction_len : Nat, k_public_key_off : Nat, (source_valid_range k_transaction_off k_transaction_len)
  ∧ (source_valid_range k_public_key_off 65) -/
def decode_transaction (transaction : (StatelessInputSliceFields k_transaction_off k_transaction_len)) (public_key : (StatelessInputSliceFields k_public_key_off 65)) : SailM (Sigma
  fun (k_blob_limit : Nat) => (TransactionFields k_blob_limit)) := do
  let ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, ⟨_, execution_profile⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩⟩ ← do
    readReg k_execution_profile
  let profile := execution_profile.protocol
  if _sailIf0 : ((k_transaction_len ≤b (2 ^i 30)) : Bool) = true
  then
    (do
      (rlp_decode_tx transaction public_key profile.transaction_blob_limit))
  else
    (do
      (fatal_error InvalidConfig))

