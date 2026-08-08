#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint8_t NPR_BEACON_ROOT = UINT8_C(8);


const uint8_t PL_FEE_RECIPIENT = UINT8_C(32);


const uint8_t PL_STATE_ROOT = UINT8_C(52);


const uint8_t PL_RECEIPTS_ROOT = UINT8_C(84);


const uint8_t PL_LOGS_BLOOM = UINT8_C(116);


const uint16_t PL_PREV_RANDAO = UINT16_C(372);


const uint16_t PL_BLOCK_NUMBER = UINT16_C(404);


const uint16_t PL_GAS_LIMIT = UINT16_C(412);


const uint16_t PL_GAS_USED = UINT16_C(420);


const uint16_t PL_TIMESTAMP = UINT16_C(428);


const uint16_t PL_BASE_FEE = UINT16_C(440);


const uint16_t PL_BLOCK_HASH = UINT16_C(472);


const uint16_t PL_BLOB_GAS_USED = UINT16_C(512);


const uint16_t PL_EXCESS_BLOB_GAS = UINT16_C(520);


const uint16_t PL_SLOT_NUMBER = UINT16_C(532);


const uint8_t WD_SIZE = UINT8_C(44);


const uint8_t CC_CHAIN_ID = UINT8_C(0);


const uint8_t CC_ACTIVE_FORK_OFF = UINT8_C(8);


const uint8_t FC_ACTIVATION_OFF = UINT8_C(0);


const uint8_t FA_BLOCK_NUMBER_OFF = UINT8_C(0);


const uint8_t FA_TIMESTAMP_OFF = UINT8_C(4);


const uint8_t PUBLIC_KEY_LENGTH = UINT8_C(65);


const uint8_t CHAIN_CONFIG_HEADER_LENGTH = UINT8_C(12);


const uint8_t CHAIN_CONFIG_MIN_LENGTH = UINT8_C(24);


struct ParentHeaderFields EMPTY_PARENT_HEADER_FIELDS;


uint32_t ssz_offset_table_position(uint32_t index)
{
  return (index * UINT32_C(4));
}

struct BoundedSszListCursor ssz_list_cursor(struct BoundedSszListRef items)
{
  uint32_t current;
  if (items.count != UINT8_C(0)) {
    uint32_t first_offset = ssz_u32(items.bytes, UINT8_C(0));
    current = ssz_offset_to_source_pointer(first_offset);
  } else {
    current = items.bytes.len;
  }
  return ((struct BoundedSszListCursor){.current = current, .index = UINT32_C(0), .items = items});
}

bool ssz_list_cursor_empty(struct BoundedSszListCursor cursor)
{
  return (bool)(cursor.items.count <= cursor.index);
}

struct tuple_Bytes_BoundedSszListCursor ssz_list_pop(struct BoundedSszListCursor cursor)
{
  uint32_t next_index;
  if (cursor.index < cursor.items.count) {
    next_index = (cursor.index + UINT32_C(1));
  } else {
    fatal_error(InvalidConfig);
  }
  struct BoundedSszListRef items = cursor.items;
  Bytes bytes = items.bytes;
  uint32_t nat = bytes.len;
  uint32_t next;
  if (next_index < items.count) {
    uint32_t table_position = ssz_offset_table_position(next_index);
    uint32_t next_offset = ssz_u32_in_slice(bytes, table_position);
    next = ssz_offset_to_source_pointer(next_offset);
  } else {
    next = nat;
  }
  if ((cursor.current <= next) && (next <= nat)) {
    uint32_t item_length = (next - cursor.current);
    bool tmp_3_831 = (bool)((items.max_item_length != UINT8_C(0)) && (items.max_item_length < item_length));
    if (tmp_3_831) {
      fatal_error(InvalidConfig);
    }
    Bytes item = stateless_input_sub_slice(bytes, cursor.current, item_length);
    return ((struct tuple_Bytes_BoundedSszListCursor){.tup0 = item, .tup1 = ((struct BoundedSszListCursor){.current = next, .index = next_index, .items = items})});
  }
  fatal_error(InvalidConfig);
}

struct StatelessInputRef decode_stateless_input_ref_(Bytes input)
{
  return decode_stateless_input_ref(input);
}

void index_witness_nodes_(struct BoundedSszListRef nodes)
{
  index_witness_nodes(nodes);
}

void index_witness_codes_(struct BoundedSszListRef codes)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool result_2_408 = (bool)(execution_profile.protocol.fork >= Amsterdam);
  index_witness_codes(codes, result_2_408);
}

void create_letbind_273(void) {

  struct ParentHeaderFields let_value_3_483;
  struct ParentHeaderFields tmp_3_482;
  tmp_3_482.base_fee = ZERO_WORD;
  tmp_3_482.blob_gas_used = UINT32_C(0);
  tmp_3_482.excess_blob_gas = UINT32_C(0);
  tmp_3_482.have_base_fee = false;
  tmp_3_482.have_blob_gas = false;
  tmp_3_482.have_excess_blob_gas = false;
  tmp_3_482.have_parent = false;
  tmp_3_482.have_state = false;
  tmp_3_482.parent_hash = ZERO_HASH;
  tmp_3_482.state_root = ZERO_HASH;
  let_value_3_483 = tmp_3_482;
  EMPTY_PARENT_HEADER_FIELDS = let_value_3_483;
}
void kill_letbind_273(void) {
}

uint8_t next_parent_header_field(uint8_t index)
{
  if (index < UINT8_C(19)) {
    return ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)index));
  }
  return UINT8_C(19);
}

struct WitnessHeaderIndex index_witness_header_cursor(struct WitnessHeaderIndex state)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool cursor_empty = ssz_list_cursor_empty(state.cursor);
  if (cursor_empty) {
    return state;
  }
  struct tuple_Bytes_BoundedSszListCursor result_2_365 = ssz_list_pop(state.cursor);
  Bytes header = result_2_365.tup0;
  struct BoundedSszListCursor next = result_2_365.tup1;
  struct WitnessHeaderIndex result = state;
  result.cursor = next;
  if (((uint16_t)state.cursor.index != UINT8_C(0)) || ((uint16_t)next.index == (uint16_t)state.cursor.items.count)) {
    Bytes fields = rlp_node_cursor(header);
    struct ParentHeaderFields decoded = decode_parent_header_fields_Bytes_uint8_t_struct_ParentHeaderFields_to_struct_ParentHeaderFields_variant_2(fields, UINT8_C(0), EMPTY_PARENT_HEADER_FIELDS);
    bool tmp_3_803;
    if ((uint16_t)state.cursor.index != UINT8_C(0)) {
      bool tmp_3_802;
      if (decoded.have_parent) {
        tmp_3_802 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C32__(decoded.parent_hash, state.previous_hash);
      } else {
        tmp_3_802 = true;
      }
      tmp_3_803 = tmp_3_802;
    } else {
      tmp_3_803 = false;
    }
    if (tmp_3_803) {
      result.valid = false;
    }
    if ((uint16_t)next.index == (uint16_t)state.cursor.items.count) {
      result.parent_state_root = decoded.state_root;
      result.parent_base_fee_per_gas = decoded.base_fee;
      result.parent_blob_gas_used = decoded.blob_gas_used;
      result.parent_excess_blob_gas = decoded.excess_blob_gas;
      bool tmp_3_812;
      if (decoded.have_state) {
        bool result_2_373 = (bool)(execution_profile.protocol.fork < Cancun);
        bool tmp_3_811;
        if (result_2_373 || decoded.have_base_fee) {
          bool result_2_377 = (bool)(execution_profile.protocol.fork < Cancun);
          bool tmp_3_810 = (bool)(result_2_377 || (decoded.have_blob_gas == decoded.have_excess_blob_gas));
          tmp_3_811 = tmp_3_810;
        } else {
          tmp_3_811 = false;
        }
        tmp_3_812 = tmp_3_811;
      } else {
        tmp_3_812 = false;
      }
      result.parent_fields_valid = tmp_3_812;
    }
  }
  bytes32 current_hash = host_keccak_stateless_input(header);
  result.previous_hash = current_hash;
  uint16_t distance;
  if ((uint16_t)next.index <= (uint16_t)state.cursor.items.count) {
    distance = (uint16_t)((uint32_t)(uint16_t)state.cursor.items.count - (uint32_t)(uint16_t)next.index);
  } else {
    fatal_error(WitnessDeficient);
  }
  if (distance < UINT16_C(256)) {
    ancestor_hash_write((uint8_t)distance, current_hash);
  }
  return index_witness_header_cursor(result);
}

struct WitnessContext index_witness_headers(struct BoundedSszListRef headers)
{
  struct BoundedSszListCursor cursor = ssz_list_cursor(headers);
  struct WitnessHeaderIndex initial = ((struct WitnessHeaderIndex){.cursor = cursor, .parent_base_fee_per_gas = ZERO_WORD, .parent_blob_gas_used = UINT32_C(0), .parent_excess_blob_gas = UINT32_C(0), .parent_fields_valid = false, .parent_state_root = ZERO_HASH, .previous_hash = ZERO_HASH, .valid = (bool)((uint16_t)headers.count != UINT8_C(0))});
  struct WitnessHeaderIndex indexed = index_witness_header_cursor(initial);
  k_n_headers = (uint16_t)headers.count;
  if (!indexed.valid) {
    fatal_error(WitnessDeficient);
  }
  if (!indexed.parent_fields_valid) {
    fatal_error(RlpDecode);
  }
  return ((struct WitnessContext){.parent_base_fee_per_gas = indexed.parent_base_fee_per_gas, .parent_blob_gas_used = indexed.parent_blob_gas_used, .parent_excess_blob_gas = indexed.parent_excess_blob_gas, .parent_hash = indexed.previous_hash, .parent_state_root = indexed.parent_state_root});
}

uint32_t decode_payload_blob_gas_used(Bytes payload, struct ProtocolProfileFields profile)
{
  uint32_t result_8_610;
  uint64_t value = decode_ssz_uint_Bytes_uint16_t_to_uint64_t(payload, PL_BLOB_GAS_USED);
  uint64_t count = (value >> 17);
  bool tmp_3_793 = (bool)((count <= profile.blob_schedule.max) && (value == (count * UINT64_C(131072))));
  if (tmp_3_793) {
    result_8_610 = (uint32_t)(count * UINT64_C(131072));
  } else {
    fatal_error(InvalidBlobGasUsed);
  }
  return result_8_610;
}

uint32_t decode_payload_excess_blob_gas(Bytes payload, struct ProtocolProfileFields profile)
{
  uint64_t value = decode_ssz_uint_Bytes_uint16_t_to_uint64_t(payload, PL_EXCESS_BLOB_GAS);
  if (value <= profile.excess_blob_gas_limit) {
    return (uint32_t)value;
  }
  fatal_error(InvalidExcessBlobGas);
}

struct BlockHeader decode_block_header_ssz(struct StatelessInputRef input_ref)
{
  Bytes payload = input_ref.execution_payload;
  uint64_t gas_limit_value = decode_ssz_uint_Bytes_uint16_t_to_uint64_t(payload, PL_GAS_LIMIT);
  uint64_t gas_used_value = decode_ssz_uint_Bytes_uint16_t_to_uint64_t(payload, PL_GAS_USED);
  bytes32 prev_randao_hash = ssz_bytes32_Bytes_uint16_t_to_bytes32(payload, PL_PREV_RANDAO);
  u256 prev_randao = hash_to_word(prev_randao_hash);
  uint64_t decode_ssz_uint_result_2_333 = decode_ssz_uint_Bytes_uint16_t_to_uint64_t(payload, PL_BLOCK_NUMBER);
  uint64_t decode_ssz_uint_result_2_334 = decode_ssz_uint_Bytes_uint16_t_to_uint64_t(payload, PL_TIMESTAMP);
  u256 ssz_u256_result_2_335 = ssz_u256_(payload, PL_BASE_FEE);
  uint32_t result_2_337 = decode_payload_blob_gas_used(payload, input_ref.protocol);
  uint32_t result_2_339 = decode_payload_excess_blob_gas(payload, input_ref.protocol);
  bytes32 ssz_bytes32_result_2_340 = ssz_bytes32_Bytes_uint8_t_to_bytes32(payload, PL_STATE_ROOT);
  bytes32 ssz_bytes32_result_2_341 = ssz_bytes32_Bytes_uint8_t_to_bytes32(payload, PL_RECEIPTS_ROOT);
  Bytes stateless_input_sub_slice_result_2_342 = stateless_input_sub_slice_Bytes_uint8_t_uint16_t_to_Bytes(payload, PL_LOGS_BLOOM, UINT16_C(256));
  bytes20 ssz_addr_result_2_343 = ssz_addr(payload, PL_FEE_RECIPIENT);
  bytes32 ssz_bytes32_result_2_344 = ssz_bytes32_Bytes_uint8_t_to_bytes32(payload, UINT8_C(0));
  bytes32 result_2_346 = ssz_bytes32_Bytes_uint8_t_to_bytes32(input_ref.new_payload_request, NPR_BEACON_ROOT);
  uint64_t decode_ssz_uint_result_2_347 = decode_ssz_uint_Bytes_uint16_t_to_uint64_t(payload, PL_SLOT_NUMBER);
  return ((struct BlockHeader){.base_fee = ssz_u256_result_2_335, .blob_gas_used = result_2_337, .excess_blob_gas = result_2_339, .extra_data = input_ref.extra_data, .fee_recipient = ssz_addr_result_2_343, .gas_limit = gas_limit_value, .gas_used = gas_used_value, .logs_bloom = stateless_input_sub_slice_result_2_342, .number = decode_ssz_uint_result_2_333, .parent_beacon_block_root = result_2_346, .parent_hash = ssz_bytes32_result_2_344, .prev_randao = prev_randao, .receipts_root = ssz_bytes32_result_2_341, .slot_number = decode_ssz_uint_result_2_347, .state_root = ssz_bytes32_result_2_340, .timestamp = decode_ssz_uint_result_2_334});
}

struct Withdrawal decode_withdrawal_(Bytes withdrawal)
{
  return decode_withdrawal(withdrawal);
}

struct ChainConfig decode_chain_config(Bytes cc, uint64_t number, uint64_t timestamp)
{
  uint32_t cc_length = cc.len;
  uint8_t header_length = CHAIN_CONFIG_HEADER_LENGTH;
  uint8_t minimum_length = CHAIN_CONFIG_MIN_LENGTH;
  if (cc_length < header_length) {
    fatal_error(InvalidConfig);
  }
  uint32_t f_offset = ssz_u32(cc, CC_ACTIVE_FORK_OFF);
  if ((f_offset != UINT8_C(12)) || (cc_length < minimum_length)) {
    fatal_error(InvalidConfig);
  }
  uint8_t activation_position = ssz_field_offset_uint8_t_uint8_t_to_uint8_t(UINT8_C(12), FC_ACTIVATION_OFF);
  uint32_t activation_offset = ssz_u32(cc, activation_position);
  if (activation_offset != UINT8_C(4)) {
    fatal_error(InvalidConfig);
  }
  uint8_t block_number_position = ssz_field_offset_uint8_t_uint8_t_to_uint8_t(UINT8_C(16), FA_BLOCK_NUMBER_OFF);
  uint32_t block_number_offset = ssz_u32(cc, block_number_position);
  uint8_t timestamp_position = ssz_field_offset_uint8_t_uint8_t_to_uint8_t(UINT8_C(16), FA_TIMESTAMP_OFF);
  uint32_t timestamp_offset = ssz_u32(cc, timestamp_position);
  uint32_t block_number_start;
  if (block_number_offset <= ((int64_t)cc_length - (int64_t)UINT8_C(16))) {
    block_number_start = (block_number_offset + UINT32_C(16));
  } else {
    fatal_error(InvalidConfig);
  }
  uint32_t timestamp_start;
  if (timestamp_offset <= ((int64_t)cc_length - (int64_t)UINT8_C(16))) {
    timestamp_start = (timestamp_offset + UINT32_C(16));
  } else {
    fatal_error(InvalidConfig);
  }
  if (UINT8_C(24) != block_number_start) {
    fatal_error(InvalidConfig);
  }
  if (timestamp_start < block_number_start) {
    fatal_error(InvalidConfig);
  }
  if (cc_length < timestamp_start) {
    fatal_error(InvalidConfig);
  }
  int64_t bn_length = ((int64_t)timestamp_start - (int64_t)block_number_start);
  int64_t ts_length = ((int64_t)cc_length - (int64_t)timestamp_start);
  bool result_2_320 = (bool)((bn_length != UINT8_C(0)) && (bn_length != SSZ_UINT_BYTES));
  bool tmp_3_785 = (bool)(result_2_320 || ((ts_length != UINT8_C(0)) && (ts_length != SSZ_UINT_BYTES)));
  if (tmp_3_785) {
    fatal_error(InvalidConfig);
  }
  if ((bn_length == UINT8_C(0)) && (ts_length == UINT8_C(0))) {
    fatal_error(InvalidConfig);
  }
  bool eq_int_result_2_324 = (bool)(bn_length == SSZ_UINT_BYTES);
  if (eq_int_result_2_324) {
    uint64_t activation_block = decode_ssz_uint(cc, block_number_start);
    if (number < activation_block) {
      fatal_error(InvalidConfig);
    }
  }
  bool eq_int_result_2_326 = (bool)(ts_length == SSZ_UINT_BYTES);
  if (eq_int_result_2_326) {
    uint64_t activation_timestamp = decode_ssz_uint(cc, timestamp_start);
    if (timestamp < activation_timestamp) {
      fatal_error(InvalidConfig);
    }
  }
  uint64_t decode_ssz_uint_result_2_328 = decode_ssz_uint_Bytes_uint8_t_to_uint64_t(cc, CC_CHAIN_ID);
  return ((struct ChainConfig){.chain_id = decode_ssz_uint_result_2_328});
}

struct StatelessInput decode_stateless_input(struct StatelessInputRef input_ref)
{
  struct BlockHeader header = decode_block_header_ssz(input_ref);
  struct ChainConfig chain_config = decode_chain_config(input_ref.chain_config, header.number, header.timestamp);
  k_set_header(header);
  k_chain_id = chain_config.chain_id;
  k_execution_profile = execution_profile_for(input_ref.protocol, header.gas_limit);
  bytes32 result_2_301 = ssz_bytes32_Bytes_uint16_t_to_bytes32(input_ref.execution_payload, PL_BLOCK_HASH);
  return ((struct StatelessInput){.chain_config = chain_config, .payload = ((struct ExecutionPayload){.block = ((struct Block){.body = ((struct BlockBody){.block_access_list = input_ref.block_access_list, .transactions = input_ref.transactions, .withdrawals = input_ref.withdrawals}), .header = header}), .expected_block_hash = result_2_301})});
}

struct WitnessContext index_execution_witness(struct StatelessInputRef input_ref)
{
  nodedb_reset();
  index_witness_nodes_(input_ref.witness_state);
  index_witness_codes_(input_ref.witness_codes);
  struct WitnessContext witness = index_witness_headers(input_ref.witness_headers);
  k_parent_state_root = witness.parent_state_root;
  return witness;
}

struct TransactionFields decode_transaction_(Bytes transaction, Bytes public_key)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  if (transaction.len <= UINT32_C(1073741824)) {
    return rlp_decode_tx(transaction, public_key, execution_profile.protocol.transaction_blob_limit);
  }
  fatal_error(InvalidConfig);
}

struct ParentHeaderFields decode_parent_header_fields_Bytes_uint8_t_struct_ParentHeaderFields_to_struct_ParentHeaderFields(Bytes cursor, uint8_t field_index, struct ParentHeaderFields fields)
{
  if (cursor.len == UINT8_C(0)) {
    return fields;
  }
  struct RlpFieldRef field = rlp_decode_item(cursor);
  Bytes next = rlp_cursor_advance(cursor, field.source.len);
  struct ParentHeaderFields decoded = fields;
  if (field_index == UINT8_C(0)) {
    u256 parent_hash_word = rlp_decode_word(field);
    decoded.parent_hash = word_to_hash(parent_hash_word);
    decoded.have_parent = true;
  } else if (field_index == UINT8_C(3)) {
    u256 state_root_word = rlp_decode_word(field);
    decoded.state_root = word_to_hash(state_root_word);
    decoded.have_state = true;
  } else if (field_index == UINT8_C(15)) {
    decoded.base_fee = rlp_decode_u256(field);
    decoded.have_base_fee = true;
  } else if (field_index == UINT8_C(17)) {
    uint64_t value = rlp_decode_uint64(field);
    uint64_t count = (value >> 17);
    struct ExecutionProfileFields execution_profile = k_execution_profile;
    bool tmp_3_819 = (bool)((count <= execution_profile.protocol.blob_schedule.max) && (value == (count * UINT64_C(131072))));
    if (tmp_3_819) {
      decoded.blob_gas_used = (uint32_t)(count * UINT64_C(131072));
      decoded.have_blob_gas = true;
    } else {
      fatal_error(RlpDecode);
    }
  } else if (field_index == UINT8_C(18)) {
    uint64_t value_3_824 = rlp_decode_uint64(field);
    struct ExecutionProfileFields execution_profile_3_825 = k_execution_profile;
    if (value_3_824 <= execution_profile_3_825.protocol.excess_blob_gas_limit) {
      decoded.excess_blob_gas = (uint32_t)value_3_824;
      decoded.have_excess_blob_gas = true;
    } else {
      fatal_error(RlpDecode);
    }
  } else {

  }
  uint8_t next_field = next_parent_header_field(field_index);
  return decode_parent_header_fields_Bytes_uint8_t_struct_ParentHeaderFields_to_struct_ParentHeaderFields(next, next_field, decoded);
}

struct ParentHeaderFields decode_parent_header_fields_Bytes_uint8_t_struct_ParentHeaderFields_to_struct_ParentHeaderFields_variant_2(Bytes cursor, uint8_t field_index, struct ParentHeaderFields fields)
{
  if (cursor.len == UINT8_C(0)) {
    return fields;
  }
  struct RlpFieldRef field = rlp_decode_item(cursor);
  Bytes next = rlp_cursor_advance(cursor, field.source.len);
  struct ParentHeaderFields decoded = fields;
  u256 parent_hash_word = rlp_decode_word(field);
  decoded.parent_hash = word_to_hash(parent_hash_word);
  decoded.have_parent = true;
  uint8_t next_field = next_parent_header_field_uint8_t_to_uint8_t(field_index);
  return decode_parent_header_fields_Bytes_uint8_t_struct_ParentHeaderFields_to_struct_ParentHeaderFields(next, next_field, decoded);
}

uint8_t next_parent_header_field_uint8_t_to_uint8_t(uint8_t index)
{
  return ((uint8_t)((uint32_t)UINT8_C(1) + (uint32_t)index));
}

struct tuple_Bytes_BoundedSszListRef ssz_fixed_list_pop(struct BoundedSszListRef items, uint8_t item_size)
{
  if ((UINT8_C(0) < items.count) && ((uint32_t)item_size <= items.bytes.len)) {
    Bytes item = stateless_input_sub_slice_Bytes_uint8_t_uint8_t_to_Bytes(items.bytes, UINT8_C(0), item_size);
    Bytes stateless_input_slice_suffix_result_2_411 = stateless_input_slice_suffix(items.bytes, (uint32_t)item_size);
    return ((struct tuple_Bytes_BoundedSszListRef){.tup0 = item, .tup1 = ((struct BoundedSszListRef){.bytes = stateless_input_slice_suffix_result_2_411, .count = (items.count - UINT32_C(1)), .max_item_length = items.max_item_length})});
  }
  fatal_error(InvalidConfig);
}

