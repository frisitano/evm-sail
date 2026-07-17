import Evm.Flow
import Evm.Arith
import Evm.Prelude
import Evm.Primitives.Quantities
import Evm.Primitives.Gas
import Evm.Primitives.Bytes
import Evm.Primitives.Crypto
import Evm.Primitives.Fork
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

def SSZ_BODY : source_pointer := (ByteQuantity 2)

def IN_NPR_OFF : source_pointer := BYTE_ZERO

def IN_WITNESS_OFF : source_pointer := (ByteQuantity 4)

def IN_CHAIN_CONFIG_OFF : source_pointer := (ByteQuantity 8)

def IN_PUBLIC_KEYS_OFF : source_pointer := (ByteQuantity 12)

def NPR_PAYLOAD_OFF : source_pointer := BYTE_ZERO

def NPR_VHASHES_OFF : source_pointer := (ByteQuantity 4)

def NPR_BEACON_ROOT : source_pointer := (ByteQuantity 8)

def NPR_REQUESTS_OFF : source_pointer := (ByteQuantity 40)

def REQ_DEPOSITS_OFF : source_pointer := BYTE_ZERO

def REQ_WITHDRAWALS_OFF : source_pointer := (ByteQuantity 4)

def REQ_CONSOLIDATIONS_OFF : source_pointer := (ByteQuantity 8)

def PL_FEE_RECIPIENT : source_pointer := (ByteQuantity 32)

def PL_STATE_ROOT : source_pointer := (ByteQuantity 52)

def PL_RECEIPTS_ROOT : source_pointer := (ByteQuantity 84)

def PL_LOGS_BLOOM : source_pointer := (ByteQuantity 116)

def PL_PREV_RANDAO : source_pointer := (ByteQuantity 372)

def PL_BLOCK_NUMBER : source_pointer := (ByteQuantity 404)

def PL_GAS_LIMIT : source_pointer := (ByteQuantity 412)

def PL_GAS_USED : source_pointer := (ByteQuantity 420)

def PL_TIMESTAMP : source_pointer := (ByteQuantity 428)

def PL_EXTRA_OFF : source_pointer := (ByteQuantity 436)

def PL_BASE_FEE : source_pointer := (ByteQuantity 440)

def PL_BLOCK_HASH : source_pointer := (ByteQuantity 472)

def PL_TXS_OFF : source_pointer := (ByteQuantity 504)

def PL_WDS_OFF : source_pointer := (ByteQuantity 508)

def PL_BLOB_GAS_USED : source_pointer := (ByteQuantity 512)

def PL_EXCESS_BLOB_GAS : source_pointer := (ByteQuantity 520)

def PL_BAL_OFF : source_pointer := (ByteQuantity 528)

def PL_SLOT_NUMBER : source_pointer := (ByteQuantity 532)

def WD_SIZE : byte_length := (ByteQuantity 44)

def WD_INDEX : source_pointer := BYTE_ZERO

def WD_VALIDATOR_INDEX : source_pointer := (ByteQuantity 8)

def WD_ADDRESS : source_pointer := (ByteQuantity 16)

def WD_AMOUNT : source_pointer := (ByteQuantity 36)

def CC_CHAIN_ID : source_pointer := BYTE_ZERO

def CC_ACTIVE_FORK_OFF : source_pointer := (ByteQuantity 8)

def FC_FORK : source_pointer := BYTE_ZERO

def FC_ACTIVATION_OFF : source_pointer := (ByteQuantity 8)

def FC_BLOBSCHED_OFF : source_pointer := (ByteQuantity 12)

def FA_BLOCK_NUMBER_OFF : source_pointer := BYTE_ZERO

def FA_TIMESTAMP_OFF : source_pointer := (ByteQuantity 4)

def WIT_STATE_OFF : source_pointer := BYTE_ZERO

def WIT_CODES_OFF : source_pointer := (ByteQuantity 4)

def WIT_HEADERS_OFF : source_pointer := (ByteQuantity 8)

def STATELESS_INPUT_FIXED_LENGTH : byte_length := (ByteQuantity 18)

def PUBLIC_KEY_LENGTH : byte_length := (ByteQuantity 65)

def CHAIN_CONFIG_HEADER_LENGTH : byte_length := (ByteQuantity 12)

def CHAIN_CONFIG_MIN_LENGTH : byte_length := (ByteQuantity 28)

def BLOB_SCHEDULE_LENGTH : byte_length := (ByteQuantity 24)

/-- Type quantifiers: index : Nat, 0 ≤ index ∧ index ≤ (2 ^ 64 - 1) -/
def ssz_offset_table_position (index : item_index) : SailM source_pointer := do
  let index := (index).value
  let position := (index *i 4)
  if ((position ≤b BYTE_QUANTITY_MAX) : Bool)
  then (pure (ByteQuantity position))
  else sailThrow ((InvalidBlock InvalidConfig))

def ssz_list_cursor (items : SszListRef) : SailM SszListCursor := do
  let current ← do
    if (((items.count).value != 0) : Bool)
    then (pure (ByteQuantity ((← (ssz_u32 items.bytes BYTE_ZERO))).value))
    else (pure items.bytes.len)
  (pure { items := items,
          index := ⟨0⟩,
          current := current })

def ssz_list_cursor_empty (cursor : SszListCursor) : Bool :=
  ((cursor.items.count).value ≤b (cursor.index).value)

def ssz_list_pop (cursor : SszListCursor) : SailM (EvmByteSlice × SszListCursor) := do
  if ((ssz_list_cursor_empty cursor) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let next_index ← do
    (do
        let semanticResult ← (protocol_quantity_increment ⟨(cursor.index).value⟩)
        pure ((semanticResult).value))
  let stop := cursor.items.bytes.len
  let next ← do
    if ((next_index <b (cursor.items.count).value) : Bool)
    then
      (pure (ByteQuantity
          ((← (ssz_u32 cursor.items.bytes (← (ssz_offset_table_position ⟨next_index⟩))))).value))
    else (pure stop)
  if (((byte_quantity_le cursor.current next) && (byte_quantity_le next stop)) : Bool)
  then
    (do
      let item ← do
        (sub_slice cursor.items.bytes cursor.current (← (byte_quantity_sub next cursor.current)))
      (pure (item, { items := cursor.items,
                     index := ⟨next_index⟩,
                     current := next })))
  else sailThrow ((InvalidBlock InvalidConfig))

/-- Type quantifiers: k_ex161648_ : Nat, 0 ≤ k_ex161648_ ∧ k_ex161648_ ≤ (2 ^ 64 - 1) -/
def ssz_list_at (items : SszListRef) (index : item_index) : SailM EvmByteSlice := do
  let index := (index).value
  if (((items.count).value ≤b index) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let next_index ← do
    (do
        let semanticResult ← (protocol_quantity_increment ⟨index⟩)
        pure ((semanticResult).value))
  let start ← do
    (pure (ByteQuantity
        ((← (ssz_u32_at items.bytes (← (ssz_offset_table_position ⟨index⟩))))).value))
  let stop ← do
    if ((next_index <b (items.count).value) : Bool)
    then
      (pure (ByteQuantity
          ((← (ssz_u32_at items.bytes (← (ssz_offset_table_position ⟨next_index⟩))))).value))
    else (pure items.bytes.len)
  if (((byte_quantity_le start stop) && (byte_quantity_le stop items.bytes.len)) : Bool)
  then (sub_slice items.bytes start (← (byte_quantity_sub stop start)))
  else sailThrow ((InvalidBlock InvalidConfig))

/-- Type quantifiers: k_ex161649_ : Nat, 0 ≤ k_ex161649_ ∧ k_ex161649_ ≤ (2 ^ 64 - 1) -/
def ssz_fixed_list_at (items : SszListRef) (index : item_index) (item_size : byte_length) : SailM EvmByteSlice := do
  let index := (index).value
  if (((items.count).value ≤b index) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let width := item_size
  let .ByteQuantity width_value := width
  let offset_value := (index *i width_value)
  if ((BYTE_QUANTITY_MAX <b offset_value) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let offset : byte_quantity := (ByteQuantity offset_value)
  if ((byte_quantity_lt items.bytes.len offset) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  if ((byte_quantity_lt (← (byte_quantity_sub items.bytes.len offset)) width) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  (sub_slice items.bytes offset width)

def ssz_fixed_list_pop (items : SszListRef) (item_size : byte_length) : SailM (EvmByteSlice × SszListRef) := do
  let width := item_size
  if ((((items.count).value == 0) || (byte_quantity_lt items.bytes.len width)) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let remaining ← do (byte_quantity_sub items.bytes.len width)
  let item ← do (sub_slice items.bytes BYTE_ZERO width)
  let rest ← (( do
    (pure { bytes := ← (sub_slice items.bytes width remaining),
            count := ← do
                let semanticField ← (do
                    let semanticResult ← (protocol_quantity_decrement ⟨(items.count).value⟩)
                    pure ((semanticResult).value))
                pure (⟨semanticField⟩) }) ) : SailM SszListRef )
  (pure (item, rest))

/-- Type quantifiers: k_ex161650_ : Nat, 0 ≤ k_ex161650_ ∧ k_ex161650_ ≤ (2 ^ 64 - 1) -/
def ssz_checked_offset (base : source_pointer) (relative : protocol_quantity) (limit : source_pointer) : SailM source_pointer := do
  let relative := (relative).value
  let .ByteQuantity base_value := base
  let resolved_value := (base_value + relative)
  if ((BYTE_QUANTITY_MAX <b resolved_value) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let resolved : byte_quantity := (ByteQuantity resolved_value)
  if ((byte_quantity_gt resolved limit) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  (pure resolved)

def ssz_span (input : EvmByteSlice) (start : source_pointer) (stop : source_pointer) : SailM EvmByteSlice := do
  if (((byte_quantity_gt start stop) || (byte_quantity_gt stop input.len)) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  (sub_slice input start (← (byte_quantity_sub stop start)))

def ssz_variable_list_ref (input : EvmByteSlice) (start : source_pointer) (stop : source_pointer) : SailM SszListRef := do
  let bytes ← do (ssz_span input start stop)
  if ((byte_quantity_equal start stop) : Bool)
  then
    (pure { bytes := bytes,
            count := ⟨0⟩ })
  else
    (do
      let span ← do (byte_quantity_sub stop start)
      if ((byte_quantity_lt span SSZ_OFF_BYTES) : Bool)
      then sailThrow ((InvalidBlock InvalidConfig))
      else (pure ())
      let first_offset ← do
        (do
            let semanticResult ← (ssz_u32 input start)
            pure ((semanticResult).value))
      let count ← do
        (do
            let semanticResult ← (protocol_quantity_quotient ⟨first_offset⟩ ⟨4⟩)
            pure ((semanticResult).value))
      let first := (ByteQuantity first_offset)
      if ((((Int.tmod first_offset 4) != 0) || ((count == 0) || (byte_quantity_gt first span))) : Bool)
      then sailThrow ((InvalidBlock InvalidConfig))
      else (pure ())
      (pure { bytes := bytes,
              count := ⟨count⟩ }))

def ssz_fixed_list_ref (input : EvmByteSlice) (start : source_pointer) (stop : source_pointer) (item_size : byte_length) : SailM SszListRef := do
  let bytes ← do (ssz_span input start stop)
  if ((byte_quantity_equal item_size BYTE_ZERO) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let span ← do (byte_quantity_sub stop start)
  let count ← do (byte_quantity_quotient span item_size)
  if ((byte_quantity_not_equal span (← (byte_quantity_mul count item_size))) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let .ByteQuantity count_value := count
  if ((((2 ^i 64) -i 1) <b count_value) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  (pure { bytes := bytes,
          count := ⟨count_value⟩ })

def decode_stateless_input_ref (input : EvmByteSlice) : SailM StatelessInputRef := do
  let input_end := input.len
  if ((byte_quantity_lt input_end STATELESS_INPUT_FIXED_LENGTH) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  if ((((← (slice_byte input BYTE_ZERO)) != 0x00#8) || ((← (slice_byte input BYTE_ONE)) != 0x01#8)) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let npr_off ← do
    (do
        let semanticResult ← (ssz_u32 input (← (byte_quantity_add SSZ_BODY IN_NPR_OFF)))
        pure ((semanticResult).value))
  let witness_off ← do
    (do
        let semanticResult ← (ssz_u32 input (← (byte_quantity_add SSZ_BODY IN_WITNESS_OFF)))
        pure ((semanticResult).value))
  let cc_off ← do
    (do
        let semanticResult ← (ssz_u32 input (← (byte_quantity_add SSZ_BODY IN_CHAIN_CONFIG_OFF)))
        pure ((semanticResult).value))
  let public_keys_off ← do
    (do
        let semanticResult ← (ssz_u32 input (← (byte_quantity_add SSZ_BODY IN_PUBLIC_KEYS_OFF)))
        pure ((semanticResult).value))
  if (((npr_off != 16) || ((witness_off <b npr_off) || ((cc_off <b witness_off) || (public_keys_off <b cc_off)))) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let npr ← do (ssz_checked_offset SSZ_BODY ⟨npr_off⟩ input_end)
  let witness ← do (ssz_checked_offset SSZ_BODY ⟨witness_off⟩ input_end)
  let cc ← do (ssz_checked_offset SSZ_BODY ⟨cc_off⟩ input_end)
  let public_keys ← do (ssz_checked_offset SSZ_BODY ⟨public_keys_off⟩ input_end)
  let public_key_bytes ← do (byte_quantity_sub input_end public_keys)
  let public_key_count ← do (byte_quantity_quotient public_key_bytes PUBLIC_KEY_LENGTH)
  if ((byte_quantity_not_equal public_key_bytes
       (← (byte_quantity_mul public_key_count PUBLIC_KEY_LENGTH))) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let npr_fixed_end ← do (ssz_checked_offset npr ⟨44⟩ witness)
  let payload ← do
    (ssz_checked_offset npr
      ⟨((← (ssz_u32 input (← (byte_quantity_add npr NPR_PAYLOAD_OFF))))).value⟩ witness)
  let versioned_hashes ← do
    (ssz_checked_offset npr
      ⟨((← (ssz_u32 input (← (byte_quantity_add npr NPR_VHASHES_OFF))))).value⟩ witness)
  let requests ← do
    (ssz_checked_offset npr
      ⟨((← (ssz_u32 input (← (byte_quantity_add npr NPR_REQUESTS_OFF))))).value⟩ witness)
  if (((byte_quantity_not_equal npr_fixed_end payload) || ((byte_quantity_gt payload
           versioned_hashes) || (byte_quantity_gt versioned_hashes requests))) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let requests_fixed_end ← do (ssz_checked_offset requests ⟨12⟩ witness)
  let deposits ← do
    (ssz_checked_offset requests
      ⟨((← (ssz_u32 input (← (byte_quantity_add requests REQ_DEPOSITS_OFF))))).value⟩
      witness)
  let withdrawal_requests ← do
    (ssz_checked_offset requests
      ⟨((← (ssz_u32 input (← (byte_quantity_add requests REQ_WITHDRAWALS_OFF))))).value⟩
      witness)
  let consolidation_requests ← do
    (ssz_checked_offset requests
      ⟨((← (ssz_u32 input (← (byte_quantity_add requests REQ_CONSOLIDATIONS_OFF))))).value⟩
      witness)
  if (((byte_quantity_not_equal requests_fixed_end deposits) || ((byte_quantity_gt deposits
           withdrawal_requests) || (byte_quantity_gt withdrawal_requests consolidation_requests))) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let payload_fixed_end ← do (ssz_checked_offset payload ⟨540⟩ versioned_hashes)
  let extra_data ← do
    (ssz_checked_offset payload
      ⟨((← (ssz_u32 input (← (byte_quantity_add payload PL_EXTRA_OFF))))).value⟩
      versioned_hashes)
  let transactions ← do
    (ssz_checked_offset payload
      ⟨((← (ssz_u32 input (← (byte_quantity_add payload PL_TXS_OFF))))).value⟩
      versioned_hashes)
  let withdrawals ← do
    (ssz_checked_offset payload
      ⟨((← (ssz_u32 input (← (byte_quantity_add payload PL_WDS_OFF))))).value⟩
      versioned_hashes)
  let block_access_list ← do
    (ssz_checked_offset payload
      ⟨((← (ssz_u32 input (← (byte_quantity_add payload PL_BAL_OFF))))).value⟩
      versioned_hashes)
  if (((byte_quantity_not_equal payload_fixed_end extra_data) || ((byte_quantity_gt extra_data
           transactions) || ((byte_quantity_gt transactions withdrawals) || (byte_quantity_gt
             withdrawals block_access_list)))) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let witness_fixed_end ← do (ssz_checked_offset witness ⟨12⟩ cc)
  let witness_state ← do
    (ssz_checked_offset witness
      ⟨((← (ssz_u32 input (← (byte_quantity_add witness WIT_STATE_OFF))))).value⟩ cc)
  let witness_codes ← do
    (ssz_checked_offset witness
      ⟨((← (ssz_u32 input (← (byte_quantity_add witness WIT_CODES_OFF))))).value⟩ cc)
  let witness_headers ← do
    (ssz_checked_offset witness
      ⟨((← (ssz_u32 input (← (byte_quantity_add witness WIT_HEADERS_OFF))))).value⟩ cc)
  if (((byte_quantity_not_equal witness_fixed_end witness_state) || ((byte_quantity_gt witness_state
           witness_codes) || (byte_quantity_gt witness_codes witness_headers))) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  (pure { new_payload_request := ← (ssz_span input npr witness),
          execution_payload := ← (ssz_span input payload versioned_hashes),
          versioned_hashes := ← (ssz_span input versioned_hashes requests),
          deposits := ← (ssz_span input deposits withdrawal_requests),
          withdrawal_requests := ← (ssz_span input withdrawal_requests consolidation_requests),
          consolidation_requests := ← (ssz_span input consolidation_requests witness),
          extra_data := ← (ssz_span input extra_data transactions),
          transactions := ← (ssz_variable_list_ref input transactions withdrawals),
          withdrawals := ← (ssz_fixed_list_ref input withdrawals block_access_list WD_SIZE),
          block_access_list := ← (ssz_span input block_access_list versioned_hashes),
          witness_state := ← (ssz_variable_list_ref input witness_state witness_codes),
          witness_codes := ← (ssz_variable_list_ref input witness_codes witness_headers),
          witness_headers := ← (ssz_variable_list_ref input witness_headers cc),
          chain_config := ← (ssz_span input cc public_keys),
          public_keys := ← (ssz_span input public_keys input_end) })

def sha256_request_digest (request_type : byte) (s : EvmByteSlice) : SailM (BitVec 256) := do
  (sha256_segments [(bytes_list [request_type] BYTE_ONE), (BytesSlice s)])

/-- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_index_witness_nodes_cursor (cursor : SszListCursor) (_reclimit : Nat) : SailM Unit := do
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
          (nodedb_insert (← (keccak256_slice node)) node.off node.len)
          (_rec_index_witness_nodes_cursor next _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

def index_witness_nodes_cursor (cursor : SszListCursor) : SailM Unit := do
  let _measure := (((cursor.items.count).value -i (cursor.index).value) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_index_witness_nodes_cursor cursor (_measure + 1))

def index_witness_nodes (nodes : SszListRef) : SailM Unit := do
  (index_witness_nodes_cursor (← (ssz_list_cursor nodes)))

/-- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
def _rec_index_witness_codes_cursor (cursor : SszListCursor) (_reclimit : Nat) : SailM Unit := do
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
          let _ ← do (code_db_insert code)
          (_rec_index_witness_codes_cursor next _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

def index_witness_codes_cursor (cursor : SszListCursor) : SailM Unit := do
  let _measure := (((cursor.items.count).value -i (cursor.index).value) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_index_witness_codes_cursor cursor (_measure + 1))

def index_witness_codes (codes : SszListRef) : SailM Unit := do
  (index_witness_codes_cursor (← (ssz_list_cursor codes)))

def undefined_ParentHeaderFields (_ : Unit) : SailM ParentHeaderFields := do
  (pure { parent_hash := ← (undefined_bitvector 256),
          state_root := ← (undefined_bitvector 256),
          base_fee := ← (undefined_bitvector 256),
          blob_gas_used := ← do
              let semanticField ← (undefined_range 0 ((2 ^i 64) -i 1))
              pure (⟨semanticField⟩),
          excess_blob_gas := ← do
              let semanticField ← (undefined_range 0 ((2 ^i 64) -i 1))
              pure (⟨semanticField⟩),
          have_parent := ← (undefined_bool ()),
          have_state := ← (undefined_bool ()),
          have_base_fee := ← (undefined_bool ()),
          have_blob_gas := ← (undefined_bool ()),
          have_excess_blob_gas := ← (undefined_bool ()) })

def EMPTY_PARENT_HEADER_FIELDS : ParentHeaderFields :=
  { parent_hash := ZERO_WORD,
    state_root := ZERO_WORD,
    base_fee := ZERO_WORD,
    blob_gas_used := ⟨0⟩,
    excess_blob_gas := ⟨0⟩,
    have_parent := false,
    have_state := false,
    have_base_fee := false,
    have_blob_gas := false,
    have_excess_blob_gas := false }

/-- Type quantifiers: _reclimit : Nat, k_ex161658_ : Nat, 0 ≤ k_ex161658_ ∧
  k_ex161658_ ≤ (2 ^ 64 - 1), 0 ≤ _reclimit -/
def _rec_decode_parent_header_fields (cursor : RlpCursor) (field_index : item_index) (fields : ParentHeaderFields) (_reclimit : Nat) : SailM ParentHeaderFields := do
  let field_index := (field_index).value
  match _reclimit with
  | 0 =>
    (do
      assert false "recursion limit reached"
      throw Error.Exit)
  | _reclimit_pred + 1 =>
    (do
      if ((rlp_cursor_empty cursor) : Bool)
      then (pure fields)
      else
        (do
          let (field, next) ← do (rlp_cursor_pop cursor)
          let decoded := fields
          let decoded ← (( do
            if ((field_index == 0) : Bool)
            then
              (do
                let decoded ← (pure { decoded with parent_hash := ← (rlp_ref_word field) })
                (pure { decoded with have_parent := true }))
            else
              (do
                if ((field_index == 3) : Bool)
                then
                  (do
                    let decoded ← (pure { decoded with state_root := ← (rlp_ref_word field) })
                    (pure { decoded with have_state := true }))
                else
                  (do
                    if ((field_index == 15) : Bool)
                    then
                      (do
                        let decoded ←
                          (pure { decoded with base_fee := ← (rlp_ref_uint_word field) })
                        (pure { decoded with have_base_fee := true }))
                    else
                      (do
                        if ((field_index == 17) : Bool)
                        then
                          (do
                            let decoded ←
                              (pure { decoded with blob_gas_used := ← do
                                    let semanticField ← (do
                                        let semanticResult ← (rlp_ref_uint field)
                                        pure ((semanticResult).value))
                                    pure (⟨semanticField⟩) })
                            (pure { decoded with have_blob_gas := true }))
                        else
                          (do
                            if ((field_index == 18) : Bool)
                            then
                              (do
                                let decoded ←
                                  (pure { decoded with excess_blob_gas := ← do
                                        let semanticField ← (do
                                            let semanticResult ← (rlp_ref_uint field)
                                            pure ((semanticResult).value))
                                        pure (⟨semanticField⟩) })
                                (pure { decoded with have_excess_blob_gas := true }))
                            else (pure decoded))))) ) : SailM ParentHeaderFields )
          (_rec_decode_parent_header_fields next
            ⟨((← (protocol_quantity_increment ⟨field_index⟩))).value⟩ decoded
            _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

/-- Type quantifiers: field_index : Nat, 0 ≤ field_index ∧ field_index ≤ (2 ^ 64 - 1) -/
def decode_parent_header_fields (cursor : RlpCursor) (field_index : item_index) (fields : ParentHeaderFields) : SailM ParentHeaderFields := do
  let field_index := (field_index).value
  let _measure :=
    (let .ByteQuantity stop := cursor.stop
    let .ByteQuantity current := cursor.current
    (stop -i current) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_decode_parent_header_fields cursor ⟨field_index⟩ fields (_measure + 1))

/-- Type quantifiers: _reclimit : Nat, 0 ≤ _reclimit -/
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
          let index := (state.cursor.index).value
          let (header, next) ← do (ssz_list_pop state.cursor)
          let is_last := ((next.index).value == (state.cursor.items.count).value)
          let result := state
          let result : WitnessHeaderIndex := { result with cursor := next }
          let result ← (( do
            if (((index != 0) || is_last) : Bool)
            then
              (do
                let fields ← do (rlp_node_cursor header)
                if (fields.valid : Bool)
                then
                  (do
                    let decoded ← do
                      (decode_parent_header_fields fields ⟨0⟩ EMPTY_PARENT_HEADER_FIELDS)
                    let result : WitnessHeaderIndex :=
                      if (((index != 0) && ((! decoded.have_parent) || (decoded.parent_hash != state.previous_hash))) : Bool)
                      then { result with valid := false }
                      else result
                    if (is_last : Bool)
                    then
                      (do
                        let result : WitnessHeaderIndex :=
                          { result with parent_state_root := decoded.state_root }
                        let result : WitnessHeaderIndex :=
                          { result with parent_base_fee_per_gas := decoded.base_fee }
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
          let distance := ((state.cursor.items.count).value - (next.index).value)
          if ((distance <b 256) : Bool)
          then
            (do
              let ancestor : Nat := distance
              (ancestor_hash_write ⟨ancestor⟩ current_hash))
          else (pure ())
          (_rec_index_witness_header_cursor result _reclimit_pred)))
termination_by _reclimit
decreasing_by all_goals exact Nat.lt_succ_self _

def index_witness_header_cursor (state : WitnessHeaderIndex) : SailM WitnessHeaderIndex := do
  let _measure := (((state.cursor.items.count).value -i (state.cursor.index).value) : Int)
  if ((_measure <b 0) : Bool)
  then throw Error.Exit
  else (_rec_index_witness_header_cursor state (_measure + 1))

def index_witness_headers (headers : SszListRef) : SailM WitnessContext := do
  let indexed ← do
    (index_witness_header_cursor
      { cursor := ← (ssz_list_cursor headers),
        previous_hash := ZERO_WORD,
        valid := ((headers.count).value != 0),
        parent_state_root := ZERO_WORD,
        parent_base_fee_per_gas := ZERO_WORD,
        parent_blob_gas_used := ⟨0⟩,
        parent_excess_blob_gas := ⟨0⟩,
        parent_fields_valid := false })
  writeReg k_n_headers (headers.count).value
  if ((! indexed.valid) : Bool)
  then sailThrow ((InvalidBlock WitnessDeficient))
  else (pure ())
  if ((! indexed.parent_fields_valid) : Bool)
  then sailThrow ((InvalidBlock RlpDecode))
  else (pure ())
  (pure { parent_hash := indexed.previous_hash,
          parent_state_root := indexed.parent_state_root,
          parent_base_fee_per_gas := indexed.parent_base_fee_per_gas,
          parent_blob_gas_used := ⟨(indexed.parent_blob_gas_used).value⟩,
          parent_excess_blob_gas := ⟨(indexed.parent_excess_blob_gas).value⟩ })

def decode_block_header_ssz (input_ref : StatelessInputRef) : SailM BlockHeader := do
  let payload := input_ref.execution_payload
  let gas_limit_value ← do
    (do
        let semanticResult ← (ssz_uint payload PL_GAS_LIMIT)
        pure ((semanticResult).value))
  let gas_used_value ← do
    (do
        let semanticResult ← (ssz_uint payload PL_GAS_USED)
        pure ((semanticResult).value))
  if ((GAS_MAX_VALUE <b gas_limit_value) : Bool)
  then sailThrow ((InvalidBlock InvalidGasLimit))
  else (pure ())
  if ((GAS_MAX_VALUE <b gas_used_value) : Bool)
  then sailThrow ((InvalidBlock InvalidGasUsed))
  else (pure ())
  (pure { number := ← do
              let semanticField ← (do
                  let semanticResult ← (ssz_uint payload PL_BLOCK_NUMBER)
                  pure ((semanticResult).value))
              pure (⟨semanticField⟩),
          timestamp := ← do
              let semanticField ← (do
                  let semanticResult ← (ssz_uint payload PL_TIMESTAMP)
                  pure ((semanticResult).value))
              pure (⟨semanticField⟩),
          gas_limit := ← (nat_to_gas gas_limit_value),
          gas_used := ← (nat_to_gas gas_used_value),
          prev_randao := ← (ssz_bytes32 payload PL_PREV_RANDAO),
          base_fee := ← (ssz_u256 payload PL_BASE_FEE),
          blob_gas_used := ← do
              let semanticField ← (do
                  let semanticResult ← (ssz_uint payload PL_BLOB_GAS_USED)
                  pure ((semanticResult).value))
              pure (⟨semanticField⟩),
          excess_blob_gas := ← do
              let semanticField ← (do
                  let semanticResult ← (ssz_uint payload PL_EXCESS_BLOB_GAS)
                  pure ((semanticResult).value))
              pure (⟨semanticField⟩),
          state_root := ← (ssz_bytes32 payload PL_STATE_ROOT),
          receipts_root := ← (ssz_bytes32 payload PL_RECEIPTS_ROOT),
          logs_bloom := ← (ssz_logs_bloom payload PL_LOGS_BLOOM),
          fee_recipient := ← (ssz_addr payload PL_FEE_RECIPIENT),
          parent_hash := ← (ssz_bytes32 payload BYTE_ZERO),
          parent_beacon_block_root := ← (ssz_bytes32 input_ref.new_payload_request NPR_BEACON_ROOT),
          slot_number := ← do
              let semanticField ← (do
                  let semanticResult ← (ssz_uint payload PL_SLOT_NUMBER)
                  pure ((semanticResult).value))
              pure (⟨semanticField⟩),
          extra_data := input_ref.extra_data })

def decode_withdrawal (withdrawal : EvmByteSlice) : SailM Withdrawal := do
  (pure { index := ← do
              let semanticField ← (do
                  let semanticResult ← (ssz_uint withdrawal WD_INDEX)
                  pure ((semanticResult).value))
              pure (⟨semanticField⟩),
          validator_index := ← do
              let semanticField ← (do
                  let semanticResult ← (ssz_uint withdrawal WD_VALIDATOR_INDEX)
                  pure ((semanticResult).value))
              pure (⟨semanticField⟩),
          address := ← (ssz_addr withdrawal WD_ADDRESS),
          amount := ← do
              let semanticField ← (do
                  let semanticResult ← (ssz_uint withdrawal WD_AMOUNT)
                  pure ((semanticResult).value))
              pure (⟨semanticField⟩) })

/-- Type quantifiers: k_ex161665_ : Nat, k_ex161664_ : Nat, 0 ≤ k_ex161664_ ∧
  k_ex161664_ ≤ (2 ^ 64 - 1), 0 ≤ k_ex161665_ ∧ k_ex161665_ ≤ (2 ^ 64 - 1) -/
def decode_chain_config (cc : EvmByteSlice) (number : block_number) (timestamp : block_timestamp) : SailM ChainConfig := do
  let number := (number).value
  let timestamp := (timestamp).value
  let cc_end := cc.len
  if ((byte_quantity_lt cc_end CHAIN_CONFIG_HEADER_LENGTH) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let f_offset ← do
    (do
        let semanticResult ← (ssz_u32 cc CC_ACTIVE_FORK_OFF)
        pure ((semanticResult).value))
  if (((f_offset != 12) || (byte_quantity_lt cc_end CHAIN_CONFIG_MIN_LENGTH)) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let f : byte_quantity := (ByteQuantity f_offset)
  let fidx ← do
    (do
        let semanticResult ← (ssz_uint cc (← (byte_quantity_add f FC_FORK)))
        pure ((semanticResult).value))
  let a ← do
    (ssz_checked_offset f
      ⟨((← (ssz_u32 cc (← (byte_quantity_add f FC_ACTIVATION_OFF))))).value⟩ cc_end)
  let bs_start ← do
    (ssz_checked_offset f
      ⟨((← (ssz_u32 cc (← (byte_quantity_add f FC_BLOBSCHED_OFF))))).value⟩ cc_end)
  if ((byte_quantity_gt a bs_start) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let activation_fixed_end ← do (ssz_checked_offset a ⟨8⟩ bs_start)
  let bn_start ← do
    (ssz_checked_offset a
      ⟨((← (ssz_u32 cc (← (byte_quantity_add a FA_BLOCK_NUMBER_OFF))))).value⟩ bs_start)
  let ts_start ← do
    (ssz_checked_offset a
      ⟨((← (ssz_u32 cc (← (byte_quantity_add a FA_TIMESTAMP_OFF))))).value⟩ bs_start)
  if (((byte_quantity_not_equal activation_fixed_end bn_start) || (byte_quantity_gt bn_start
         ts_start)) : Bool)
  then sailThrow ((InvalidBlock InvalidConfig))
  else (pure ())
  let bn_present ← do
    (pure (byte_quantity_le SSZ_UINT_BYTES (← (byte_quantity_sub ts_start bn_start))))
  let ts_present ← do
    (pure (byte_quantity_le SSZ_UINT_BYTES (← (byte_quantity_sub bs_start ts_start))))
  let bn_ok ← do
    (pure ((! bn_present) || (← do
          (pure (((← (ssz_uint cc bn_start))).value ≤b number)))))
  let ts_ok ← do
    (pure ((! ts_present) || (← do
          (pure (((← (ssz_uint cc ts_start))).value ≤b timestamp)))))
  let have_schedule ← do
    (pure (byte_quantity_equal (← (byte_quantity_add bs_start BLOB_SCHEDULE_LENGTH)) cc_end))
  let blob_schedule_shape_valid := ((byte_quantity_equal bs_start cc_end) || have_schedule)
  let blob_schedule ← (( do
    if (have_schedule : Bool)
    then
      (pure (some
          { target := ← do
                let semanticField ← (do
                    let semanticResult ← (ssz_uint cc bs_start)
                    pure ((semanticResult).value))
                pure (⟨semanticField⟩),
            max := ← do
                let semanticField ← (do
                    let semanticResult ← (ssz_uint cc
                    (← (byte_quantity_add bs_start SSZ_UINT_BYTES)))
                    pure ((semanticResult).value))
                pure (⟨semanticField⟩),
            base_fee_update_fraction := ← do
                let semanticField ← (do
                    let semanticResult ← (ssz_uint cc
                    (← (byte_quantity_add (← (byte_quantity_add bs_start SSZ_UINT_BYTES))
                        SSZ_UINT_BYTES)))
                    pure ((semanticResult).value))
                pure (⟨semanticField⟩) }))
    else (pure none) ) : SailM (Option BlobSchedule) )
  (pure { chain_id := ← do
              let semanticField ← (do
                  let semanticResult ← (ssz_uint cc CC_CHAIN_ID)
                  pure ((semanticResult).value))
              pure (⟨semanticField⟩),
          fork_index := ⟨fidx⟩,
          fork := (fork_of_protocol_index ⟨fidx⟩),
          activation_active := ((bn_present || ts_present) && (bn_ok && ts_ok)),
          blob_schedule := blob_schedule,
          blob_schedule_shape_valid := blob_schedule_shape_valid })

def decode_stateless_input (input_ref : StatelessInputRef) : SailM StatelessInput := do
  let payload := input_ref.execution_payload
  let header ← do (decode_block_header_ssz input_ref)
  let chain_config ← do
    (decode_chain_config input_ref.chain_config ⟨(header.number).value⟩
      ⟨(header.timestamp).value⟩)
  (k_set_header header)
  writeReg k_chain_id (chain_config.chain_id).value
  writeReg k_blob_schedule (match chain_config.blob_schedule with
  | .some schedule => schedule
  | none =>
    { target := ⟨0⟩,
      max := ⟨0⟩,
      base_fee_update_fraction := ⟨1⟩ })
  (pure { payload := ← (pure { expected_block_hash := ← (ssz_bytes32 input_ref.execution_payload
                                     PL_BLOCK_HASH),
                                 block' := { header := header,
                                             body := { transactions := input_ref.transactions,
                                                       withdrawals := input_ref.withdrawals,
                                                       block_access_list := input_ref.block_access_list } } }),
          chain_config := chain_config })

def index_execution_witness (input_ref : StatelessInputRef) : SailM WitnessContext := do
  (nodedb_reset ())
  (index_witness_nodes input_ref.witness_state)
  (index_witness_codes input_ref.witness_codes)
  let witness ← do (index_witness_headers input_ref.witness_headers)
  writeReg k_parent_state_root witness.parent_state_root
  (pure witness)

def decode_transaction (transaction : EvmByteSlice) (public_key : EvmByteSlice) : SailM Transaction := do
  (rlp_decode_tx transaction public_key (← readReg k_fork))

