#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

const uint16_t G_transaction = UINT16_C(21000);


const uint16_t G_txcreate = UINT16_C(32000);


const uint8_t G_txdatazero = UINT8_C(4);


const uint8_t G_txdatanonzero = UINT8_C(16);


const uint16_t G_access_list_address = UINT16_C(2400);


const uint16_t G_access_list_storage_key = UINT16_C(1900);


const uint16_t PER_AUTH_BASE = UINT16_C(12500);


const uint16_t PER_EMPTY_ACCOUNT = UINT16_C(25000);


const uint16_t AMSTERDAM_TX_BASE = UINT16_C(12000);


const uint16_t AMSTERDAM_CREATE_ACCESS = UINT16_C(11000);


const uint16_t AMSTERDAM_COLD_ACCOUNT_ACCESS = UINT16_C(3000);


const uint16_t AMSTERDAM_TX_VALUE_COST = UINT16_C(4244);


const uint16_t AMSTERDAM_TRANSFER_LOG_COST = UINT16_C(1756);


const uint16_t AMSTERDAM_ACCESS_LIST_ADDRESS = UINT16_C(3000);


const uint16_t AMSTERDAM_ACCESS_LIST_SLOT = UINT16_C(3000);


const uint16_t AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR = UINT16_C(1280);


const uint16_t AMSTERDAM_ACCESS_LIST_SLOT_FLOOR = UINT16_C(2048);


const uint16_t AMSTERDAM_AUTH_BASE = UINT16_C(7816);


const uint8_t AMSTERDAM_CALLDATA_FLOOR_BYTE = UINT8_C(64);


Bytes transaction_initcode_slice(Bytes input)
{
  Bytes input_slice = stateless_input_slice(input.bytes, input.len);
  return code_db_intern_input(input_slice);
}

uint64_t calldata_cost(Bytes input)
{
  uint64_t result_8_878;
  uint32_t nonzeroes = slice_count_nonzero(input);
  if (nonzeroes <= input.len) {
    int64_t integer_result_3_3614 = ((int64_t)G_txdatazero * ((int64_t)input.len - (int64_t)nonzeroes));
    uint64_t mult_atom_result_2_967 = ((uint64_t)G_txdatanonzero * (uint64_t)nonzeroes);
    result_8_878 = (uint64_t)((int64_t)(uint64_t)integer_result_3_3614 + (int64_t)mult_atom_result_2_967);
  } else {
    fatal_error(ExecutionInvalid);
  }
  return result_8_878;
}

uint64_t legacy_intrinsic_gas(struct TransactionFields tx)
{
  uint64_t result_8_879;
  uint64_t data_cost = calldata_cost(tx.input_src);
  uint64_t address_cost = ((uint64_t)G_access_list_address * (uint64_t)tx.access_list.address_count);
  uint64_t slot_cost = ((uint64_t)G_access_list_storage_key * (uint64_t)tx.access_list.slot_count);
  uint64_t auth_cost = ((uint64_t)PER_EMPTY_ACCOUNT * (uint64_t)tx.authorizations.count);
  __int128 integer_result_3_3616 = ((__int128)data_cost + (__int128)G_transaction);
  if (tx.is_create) {
    __int128 integer_result_3_3620 = ((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)integer_result_3_3616 + (__int128)address_cost) + (__int128)slot_cost) + (__int128)auth_cost) + (__int128)G_txcreate);
    uint32_t transaction_initcode_gas_result_2_955 = transaction_initcode_gas(tx.input_src.len);
    result_8_879 = (uint64_t)((__int128)(uint64_t)integer_result_3_3620 + (__int128)transaction_initcode_gas_result_2_955);
  } else {
    result_8_879 = (uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)integer_result_3_3616 + (__int128)address_cost) + (__int128)slot_cost) + (__int128)auth_cost);
  }
  return result_8_879;
}

uint64_t legacy_calldata_floor(Bytes input)
{
  uint64_t result_8_880;
  uint32_t nonzeroes = slice_count_nonzero(input);
  if (nonzeroes <= input.len) {
    int64_t integer_result_3_3624 = ((int64_t)(uint64_t)((int64_t)(uint64_t)((int64_t)UINT8_C(10) * ((int64_t)input.len - (int64_t)nonzeroes)) + (int64_t)(UINT64_C(40) * (uint64_t)nonzeroes)) + (int64_t)G_transaction);
    result_8_880 = (uint64_t)integer_result_3_3624;
  } else {
    fatal_error(ExecutionInvalid);
  }
  return result_8_880;
}

uint16_t amsterdam_recipient_execution_cost(struct TransactionFields tx)
{
  bool transfers_value = word_nonzero(tx.value);
  if (tx.is_create) {
    if (transfers_value) {
      return ((uint16_t)((uint32_t)AMSTERDAM_TRANSFER_LOG_COST + (uint32_t)AMSTERDAM_CREATE_ACCESS));
    }
    return AMSTERDAM_CREATE_ACCESS;
  }
  bool result_2_946 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C20__(tx.recipient, tx.sender);
  if (result_2_946) {
    if (transfers_value) {
      uint16_t add_atom_result_2_947 = ((uint16_t)((uint32_t)AMSTERDAM_TX_VALUE_COST + (uint32_t)AMSTERDAM_COLD_ACCOUNT_ACCESS));
      return ((uint16_t)((uint32_t)AMSTERDAM_TRANSFER_LOG_COST + (uint32_t)add_atom_result_2_947));
    }
    return AMSTERDAM_COLD_ACCOUNT_ACCESS;
  }
  return UINT16_C(0);
}

struct IntrinsicGasCost intrinsic_gas(struct TransactionFields tx)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  if (execution_profile.protocol.fork < Amsterdam) {
    uint64_t legacy_intrinsic_gas_result_2_917 = legacy_intrinsic_gas(tx);
    uint64_t result_2_919 = legacy_calldata_floor(tx.input_src);
    return ((struct IntrinsicGasCost){.calldata_floor = result_2_919, .execution = legacy_intrinsic_gas_result_2_917, .state = UINT64_C(0)});
  }
  Bytes input = tx.input_src;
  uint16_t recipient = amsterdam_recipient_execution_cost(tx);
  uint32_t address_count = tx.access_list.address_count;
  uint32_t slot_count = tx.access_list.slot_count;
  uint64_t mult_atom_result_2_935 = ((uint64_t)AMSTERDAM_ACCESS_LIST_ADDRESS * (uint64_t)address_count);
  uint64_t mult_atom_result_2_936 = ((uint64_t)AMSTERDAM_ACCESS_LIST_SLOT * (uint64_t)slot_count);
  uint64_t mult_atom_result_2_938 = ((uint64_t)AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR * (uint64_t)address_count);
  uint64_t mult_atom_result_2_940 = ((uint64_t)AMSTERDAM_ACCESS_LIST_SLOT_FLOOR * (uint64_t)slot_count);
  uint64_t authorization_execution = ((uint64_t)AMSTERDAM_AUTH_BASE * (uint64_t)tx.authorizations.count);
  uint32_t create_execution;
  if (tx.is_create) {
    create_execution = transaction_initcode_gas(input.len);
  } else {
    create_execution = UINT32_C(0);
  }
  uint64_t result_2_927 = calldata_cost(tx.input_src);
  __int128 integer_result_3_3625 = ((__int128)result_2_927 + (__int128)AMSTERDAM_TX_BASE);
  uint64_t result_2_921;
  uint64_t mult_atom_result_2_920 = ((uint64_t)AMSTERDAM_CALLDATA_FLOOR_BYTE * (uint64_t)input.len);
  result_2_921 = (mult_atom_result_2_920 + (uint64_t)AMSTERDAM_TX_BASE);
  uint64_t mult_atom_result_2_923 = ((uint64_t)AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR * (uint64_t)address_count);
  uint64_t mult_atom_result_2_925 = ((uint64_t)AMSTERDAM_ACCESS_LIST_SLOT_FLOOR * (uint64_t)slot_count);
  return ((struct IntrinsicGasCost){.calldata_floor = (mult_atom_result_2_925 + (mult_atom_result_2_923 + (result_2_921 + (uint64_t)recipient))), .execution = (uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)integer_result_3_3625 + (__int128)recipient) + ((__int128)mult_atom_result_2_940 + ((__int128)mult_atom_result_2_938 + ((__int128)mult_atom_result_2_936 + (__int128)mult_atom_result_2_935)))) + (__int128)authorization_execution) + (__int128)create_execution), .state = UINT64_C(0)});
}

struct TransactionCosts transaction_costs(struct ProtocolProfileFields profile, struct TransactionFields tx, uint64_t gas_limit, uint32_t excess_blob_gas)
{
  struct IntrinsicGasCost intrinsic = intrinsic_gas(tx);
  uint32_t blob_gas = (UINT32_C(131072) * (uint32_t)tx.blob_hashes.count);
  u256 blob_fee;
  if (blob_gas == UINT8_C(0)) {
    blob_fee = WORD_ZERO;
  } else {
    u256 blob_price = blob_base_fee(profile.fork, profile.blob_schedule, profile.excess_blob_gas_limit, excess_blob_gas);
    bool result_2_905 = word_ule(blob_price, tx.max_blob_fee);
    if (result_2_905) {
      blob_fee = validated_word_product_u256_uint32_t_to_u256_variant_2(blob_price, blob_gas);
    } else {
      fatal_error(ExecutionInvalid);
    }
  }
  u256 execution_cap = validated_word_product_u256_uint64_t_to_u256(tx.max_fee, gas_limit);
  u256 blob_cap = validated_word_product_u256_uint32_t_to_u256(tx.max_blob_fee, blob_gas);
  u256 execution_and_value;
  bool result_2_899;
  u256 word_sub_word_result_2_898;
  if (!u256_lt(WORD_ALL_ONES, execution_cap)) {
    word_sub_word_result_2_898 = u256_sub(WORD_ALL_ONES, execution_cap);
  } else {
    u256 sub_atom_result_2_2638 = u256_sub(execution_cap, WORD_ALL_ONES);
    word_sub_word_result_2_898 = u256_of_u320_unchecked(u320_add(u320_of_u256(u256_sub((u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}}, sub_atom_result_2_2638)), u320_of_u64(UINT8_C(1))));
  }
  result_2_899 = word_ule(tx.value, word_sub_word_result_2_898);
  if (result_2_899) {
    execution_and_value = word_add_word(execution_cap, tx.value);
  } else {
    fatal_error(ExecutionInvalid);
  }
  u256 upfront;
  bool result_2_896;
  u256 word_sub_word_result_2_895;
  if (!u256_lt(WORD_ALL_ONES, execution_and_value)) {
    word_sub_word_result_2_895 = u256_sub(WORD_ALL_ONES, execution_and_value);
  } else {
    u256 sub_atom_result_8_174 = u256_sub(execution_and_value, WORD_ALL_ONES);
    word_sub_word_result_2_895 = u256_of_u320_unchecked(u320_add(u320_of_u256(u256_sub((u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}}, sub_atom_result_8_174)), u320_of_u64(UINT8_C(1))));
  }
  result_2_896 = word_ule(blob_cap, word_sub_word_result_2_895);
  if (result_2_896) {
    upfront = word_add_word(execution_and_value, blob_cap);
  } else {
    fatal_error(ExecutionInvalid);
  }
  return ((struct TransactionCosts){.blob_fee = blob_fee, .blob_gas = blob_gas, .calldata_floor = intrinsic.calldata_floor, .intrinsic_execution = intrinsic.execution, .intrinsic_state = intrinsic.state, .upfront = upfront});
}

struct TxFrameGasSnapshotFields tx_frame_gas_snapshot(struct TransactionInitialGasFields initial, uint64_t execution, uint64_t state, __int128 state_delta)
{
  if (execution <= initial.admitted_limit) {
    if (state <= (initial.admitted_limit - execution)) {
      if (!(UINT8_C(0) < (state_delta + (__int128)initial.intrinsic_state))) {
        if ((initial.admitted_limit - (state + execution)) <= initial.regular_limit) {
          return tx_frame_gas_snapshot_fields_uint64_t_uint64_t_uint64_t_uint64_t_uint8_t_to_struct_TxFrameGasSnapshotFields(initial.admitted_limit, initial.regular_limit, initial.calldata_floor, (state + execution), UINT8_C(0));
        }
        fatal_error(ExecutionInvalid);
      }
      if (!u64_lt_u128((initial.admitted_limit - (state + execution)), ((u128){{(uint64_t)((state_delta + (__int128)initial.intrinsic_state)), (uint64_t)(((unsigned __int128)((state_delta + (__int128)initial.intrinsic_state))) >> 64)}}))) {
        if (((initial.admitted_limit - (state + execution)) - (uint64_t)u128_to_u64_unchecked(((u128){{(uint64_t)((state_delta + (__int128)initial.intrinsic_state)), (uint64_t)(((unsigned __int128)((state_delta + (__int128)initial.intrinsic_state))) >> 64)}}))) <= initial.regular_limit) {
          return tx_frame_gas_snapshot_fields(initial.admitted_limit, initial.regular_limit, initial.calldata_floor, (state + execution), (uint64_t)u128_to_u64_unchecked(((u128){{(uint64_t)((state_delta + (__int128)initial.intrinsic_state)), (uint64_t)(((unsigned __int128)((state_delta + (__int128)initial.intrinsic_state))) >> 64)}})));
        }
        fatal_error(ExecutionInvalid);
      }
      fatal_error(ExecutionInvalid);
    }
    fatal_error(ExecutionInvalid);
  }
  fatal_error(ExecutionInvalid);
}

struct TransactionGasAllowanceFields transaction_gas_allowance_fields(uint64_t value, uint64_t regular_limit)
{
  uint64_t regular = value < regular_limit ? value : regular_limit;
  return ((struct TransactionGasAllowanceFields){.regular = regular, .total = value});
}

uint16_t process_auth(struct Authorization au)
{
  uint16_t refund = UINT16_C(0);
  bool chain_id_is_zero = eq_u256(au.chain_id, WORD_ZERO);
  uint64_t expected_chain_id = word_of_chain_identifier(k_chain_id);
  if (au.valid_sig && (chain_id_is_zero || u256_eq_u64(au.chain_id, expected_chain_id))) {
    k_account_mark_warm(au.authority);
    struct tuple_bool_bytes20 k_deleg_target_result_2_854 = k_deleg_target(au.authority);
    bytes32 code_key = k_code_key(au.authority);
    uint64_t nonce = k_get_nonce(au.authority);
    bool eq_anything_result_2_855 = eq_bytes32(code_key, KECCAK_EMPTY);
    if ((eq_anything_result_2_855 || k_deleg_target_result_2_854.tup0) && (nonce == au.nonce)) {
      bool existed = k_account_exists(au.authority);
      if (eq_bytes20(au.address, ZERO_ADDRESS)) {
        k_clear_code(au.authority);
      } else {
        k_set_delegation(au.authority, au.address);
      }
      k_bump_nonce(au.authority);
      if (existed) {
        refund = ((uint16_t)((uint32_t)PER_EMPTY_ACCOUNT - (uint32_t)PER_AUTH_BASE));
      }
    }
  }
  return refund;
}

uint64_t authorization_refund_add(uint16_t item, uint64_t accumulated)
{
  if (accumulated <= (UINT64_C(13421772800000) - (uint64_t)item)) {
    return (accumulated + (uint64_t)item);
  }
  fatal_error(ExecutionInvalid);
}

uint64_t process_auth_cursor(PreparedAuthorizationList authorizations, uint16_t count)
{
  if (count == UINT8_C(0)) {
    return UINT64_C(0);
  }
  struct Authorization authorization = prepared_authorization_head_(authorizations);
  PreparedAuthorizationList remaining = prepared_authorization_tail_(authorizations, count);
  uint16_t item_refund = process_auth(authorization);
  uint64_t remaining_refund = process_auth_cursor(remaining, ((uint16_t)((uint32_t)count - (uint32_t)UINT16_C(1))));
  return authorization_refund_add(item_refund, remaining_refund);
}

uint64_t process_auth_list(PreparedAuthorizationList authorizations)
{
  return process_auth_cursor(authorizations, authorizations.count);
}

struct tuple_bool_uint_64_uint_64_uint_32 process_amsterdam_auth(struct Authorization au, bytes20 sender, bytes20 current_target, bool transfers_value, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill)
{
  struct tuple_bool_uint_64_uint_64_uint_32 result_8_890;
  uint64_t gas_after = carried_gas;
  uint64_t state_gas_after = carried_state_gas;
  uint32_t state_spill_after = carried_state_spill;
  bool chain_id_is_zero = eq_u256(au.chain_id, WORD_ZERO);
  uint64_t expected_chain_id = word_of_chain_identifier(k_chain_id);
  if (au.valid_sig && (chain_id_is_zero || u256_eq_u64(au.chain_id, expected_chain_id))) {
    k_account_mark_warm(au.authority);
    struct tuple_bool_bytes20 k_deleg_target_result_2_828 = k_deleg_target(au.authority);
    bool currently_delegated = k_deleg_target_result_2_828.tup0;
    bytes32 code_key = k_code_key(au.authority);
    uint64_t nonce = k_get_nonce(au.authority);
    bool eq_anything_result_2_829 = eq_bytes32(code_key, KECCAK_EMPTY);
    if ((eq_anything_result_2_829 || currently_delegated) && (nonce == au.nonce)) {
      bool seen = authorization_tracker_seen(au.authority);
      bool delegated_before_tx;
      if (seen) {
        delegated_before_tx = authorization_tracker_originally_delegated(au.authority);
      } else {
        delegated_before_tx = currently_delegated;
      }
      bool tmp_3_1276 = (bool)(seen || (eq_bytes20(au.authority, sender) || (transfers_value && eq_bytes20(au.authority, current_target))));
      bool account_exists = k_account_exists(au.authority);
      if (!account_exists) {
        struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_result_2_833 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after, state_spill_after, G_amsterdam_state_new_account);
        gas_after = charge_state_gas_result_2_833.tup1;
        state_gas_after = charge_state_gas_result_2_833.tup2;
        state_spill_after = charge_state_gas_result_2_833.tup3;
        if (charge_state_gas_result_2_833.tup0) {
          return ((struct tuple_bool_uint_64_uint_64_uint_32){.tup0 = false, .tup1 = gas_after, .tup2 = state_gas_after, .tup3 = state_spill_after});
        }
      }
      if (!tmp_3_1276) {
        if (gas_after < G_amsterdam_account_write) {
          struct tuple_bool_uint_8_uint_64_uint_32 tmp_3_1286 = ((struct tuple_bool_uint_8_uint_64_uint_32){.tup0 = false, .tup1 = GAS_ZERO, .tup2 = state_gas_after, .tup3 = state_spill_after});
          /* conversions */
          result_8_890.tup0 = tmp_3_1286.tup0;
          result_8_890.tup1 = (uint64_t)tmp_3_1286.tup1;
          result_8_890.tup2 = tmp_3_1286.tup2;
          result_8_890.tup3 = tmp_3_1286.tup3;
          /* end conversions */
          goto end_function_1622;
        }
        gas_after = gas_sub_uint64_t_uint16_t_to_uint64_t(gas_after, G_amsterdam_account_write);
      }
      bool delegation_set = authorization_tracker_delegation_set(au.authority);
      bool result_2_836 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C20__(au.address, ZERO_ADDRESS);
      if (result_2_836 && (!delegated_before_tx && !delegation_set)) {
        struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_result_2_838 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after, state_spill_after, G_amsterdam_state_auth_base);
        gas_after = charge_state_gas_result_2_838.tup1;
        state_gas_after = charge_state_gas_result_2_838.tup2;
        state_spill_after = charge_state_gas_result_2_838.tup3;
        if (charge_state_gas_result_2_838.tup0) {
          return ((struct tuple_bool_uint_64_uint_64_uint_32){.tup0 = false, .tup1 = gas_after, .tup2 = state_gas_after, .tup3 = state_spill_after});
        }
      }
      if (eq_bytes20(au.address, ZERO_ADDRESS)) {
        k_clear_code(au.authority);
      } else {
        k_set_delegation(au.authority, au.address);
      }
      k_bump_nonce(au.authority);
      bool result_2_843 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C20__(au.address, ZERO_ADDRESS);
      authorization_tracker_commit(au.authority, (bool)(!seen && currently_delegated), result_2_843);
    }
  }
  result_8_890 = ((struct tuple_bool_uint_64_uint_64_uint_32){.tup0 = true, .tup1 = gas_after, .tup2 = state_gas_after, .tup3 = state_spill_after});
end_function_1622: ;
  return result_8_890;
}

struct tuple_bool_uint_64_uint_64_uint_32 process_amsterdam_auth_cursor(PreparedAuthorizationList authorizations, uint16_t count, bytes20 sender, bytes20 current_target, bool transfers_value, uint64_t gas, uint64_t state_gas, uint32_t state_spill)
{
  if (count == UINT8_C(0)) {
    return ((struct tuple_bool_uint_64_uint_64_uint_32){.tup0 = true, .tup1 = gas, .tup2 = state_gas, .tup3 = state_spill});
  }
  struct Authorization authorization = prepared_authorization_head_(authorizations);
  PreparedAuthorizationList remaining = prepared_authorization_tail_(authorizations, count);
  struct tuple_bool_uint_64_uint_64_uint_32 process_amsterdam_auth_result_2_824 = process_amsterdam_auth(authorization, sender, current_target, transfers_value, gas, state_gas, state_spill);
  if (process_amsterdam_auth_result_2_824.tup0) {
    return process_amsterdam_auth_cursor(remaining, ((uint16_t)((uint32_t)count - (uint32_t)UINT16_C(1))), sender, current_target, transfers_value, process_amsterdam_auth_result_2_824.tup1, process_amsterdam_auth_result_2_824.tup2, process_amsterdam_auth_result_2_824.tup3);
  }
  return ((struct tuple_bool_uint_64_uint_64_uint_32){.tup0 = false, .tup1 = process_amsterdam_auth_result_2_824.tup1, .tup2 = process_amsterdam_auth_result_2_824.tup2, .tup3 = process_amsterdam_auth_result_2_824.tup3});
}

void warm_access_list_keys(Bytes cursor, bytes20 addr)
{
  if (cursor.len == UINT8_C(0)) {
    return;
  }
  struct RlpFieldRef key = rlp_decode_item(cursor);
  Bytes next = rlp_cursor_advance(cursor, key.source.len);
  u256 slot = rlp_decode_word(key);
  k_prewarm_slot(addr, slot);
  warm_access_list_keys(next, addr);
}

void warm_access_list(Bytes cursor)
{
  if (cursor.len == UINT8_C(0)) {
    return;
  }
  struct RlpFieldRef entry = rlp_decode_item(cursor);
  Bytes next = rlp_cursor_advance(cursor, entry.source.len);
  Bytes fields = rlp_decode_list(entry);
  struct RlpFieldRef addr_f = rlp_decode_item(fields);
  Bytes fields_1_6 = rlp_cursor_advance(fields, addr_f.source.len);
  struct RlpFieldRef keys_f = rlp_decode_item(fields_1_6);
  Bytes fields_1_7 = rlp_cursor_advance(fields_1_6, keys_f.source.len);
  rlp_cursor_expect_end(fields_1_7);
  u256 addr_word = rlp_decode_word(addr_f);
  bytes20 addr = word_to_address(addr_word);
  k_account_mark_warm(addr);
  Bytes keys = rlp_decode_list(keys_f);
  warm_access_list_keys(keys, addr);
  warm_access_list(next);
}

void prewarm(struct TransactionFields tx)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  k_account_mark_warm(tx.sender);
  if (!tx.is_create) {
    k_account_mark_warm(tx.recipient);
  }
  if (execution_profile.protocol.fork >= Shanghai) {
    bytes20 coinbase = k_coinbase();
    k_account_mark_warm(coinbase);
  }
  warm_access_list(tx.access_list.encoded);
}

struct tuple_u256_u256 eff_gas_price_for(u256 base_fee, u256 max_fee, u256 max_priority_fee)
{
  bool max_fee_below_base = word_ule(max_fee, base_fee);
  u256 price;
  if (max_fee_below_base) {
    price = max_fee;
  } else {
    u256 available_priority = !u256_lt(max_fee, base_fee) ? u256_sub(max_fee, base_fee) : u256_of_u320(u320_add(u320_of_u256(u256_sub((u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}}, u256_sub(base_fee, max_fee))), u320_of_u64(UINT8_C(1))));
    bool priority_within_cap = word_ule(max_priority_fee, available_priority);
    if (priority_within_cap) {
      price = word_add_word(base_fee, max_priority_fee);
    } else {
      price = max_fee;
    }
  }
  bool base_fee_covered = word_ule(base_fee, price);
  u256 priority;
  if (base_fee_covered) {
    priority = !u256_lt(price, base_fee) ? u256_sub(price, base_fee) : u256_of_u320(u320_add(u320_of_u256(u256_sub((u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}}, u256_sub(base_fee, price))), u320_of_u64(UINT8_C(1))));
  } else {
    priority = ZERO_WORD;
  }
  return ((struct tuple_u256_u256){.tup0 = price, .tup1 = priority});
}

struct TxValidityFields check_transaction_validity(struct TransactionFields tx, struct TransactionGasAllowanceFields allowance)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  struct TxTypeSemantics tx_semantics = tx_type_semantics(tx.tx_type);
  uint8_t parity = tx_signature_parity(k_chain_id, tx_semantics.signature, tx.sig_v);
  bool authenticated = tx_auth_valid(tx.sender, tx.signing_hash, parity, tx.sig_r, tx.sig_s);
  if (!authenticated) {
    fatal_error(InvalidSignature);
  }
  uint64_t gas_limit = allowance.total;
  struct tuple_u256_u256 result_2_754;
  u256 result_2_751 = k_header.base_fee;
  result_2_754 = eff_gas_price_for(result_2_751, tx.max_fee, tx.max_priority_fee);
  bytes20 sender = tx.sender;
  uint64_t nonce_before = k_get_nonce(sender);
  struct TransactionCosts costs;
  uint32_t result_2_797 = k_header.excess_blob_gas;
  costs = transaction_costs(profile, tx, gas_limit, result_2_797);
  uint64_t expected_nonce = word_of_account_nonce(nonce_before);
  if (!u256_eq_u64(tx.nonce, expected_nonce)) {
    fatal_error(ExecutionInvalid);
  }
  struct tuple_bool_bytes20 k_deleg_target_result_2_757 = k_deleg_target(sender);
  if (tx_semantics.blob) {
    bool result_2_763 = (bool)(profile.fork < Cancun);
    if (result_2_763 || ((tx.blob_hashes.count == UINT8_C(0)) || tx.is_create)) {
      fatal_error(ExecutionInvalid);
    }
  }
  bool result_2_767 = (bool)(profile.fork >= Prague);
  if (result_2_767 && (gas_limit < costs.calldata_floor)) {
    fatal_error(ExecutionInvalid);
  }
  u256 sender_balance = k_get_balance(sender);
  bool upfront_affordable = word_ule(costs.upfront, sender_balance);
  if (!upfront_affordable) {
    fatal_error(ExecutionInvalid);
  }
  bytes32 sender_code_key = k_code_key(sender);
  bool eq_anything_result_2_795 = eq_bytes32(sender_code_key, KECCAK_EMPTY);
  if (!eq_anything_result_2_795 && !k_deleg_target_result_2_757.tup0) {
    fatal_error(ExecutionInvalid);
  }
  if (allowance.regular < costs.calldata_floor) {
    fatal_error(ExecutionInvalid);
  }
  bool base_fee_affordable;
  u256 result_2_793 = k_header.base_fee;
  base_fee_affordable = word_ule(result_2_793, tx.max_fee);
  if (!base_fee_affordable) {
    fatal_error(ExecutionInvalid);
  }
  bool valid_initcode_size = initcode_size_allowed(tx.input_src.len);
  if (tx.is_create && !valid_initcode_size) {
    fatal_error(ExecutionInvalid);
  }
  bool valid_priority_fee = word_ule(tx.max_priority_fee, tx.max_fee);
  if (!valid_priority_fee) {
    fatal_error(ExecutionInvalid);
  }
  if (profile.fork < tx_semantics.minimum_fork) {
    fatal_error(ExecutionInvalid);
  }
  if (tx_semantics.set_code && tx.is_create) {
    fatal_error(ExecutionInvalid);
  }
  if (tx_semantics.set_code && (tx.authorizations.count == UINT8_C(0))) {
    fatal_error(ExecutionInvalid);
  }
  bool result_2_784 = eq_TxSignatureScheme(tx_semantics.signature, TypedSignature);
  if (result_2_784 && (tx.chain_id != k_chain_id)) {
    fatal_error(ExecutionInvalid);
  }
  if (nonce_before == UINT64_C(18446744073709551615)) {
    fatal_error(ExecutionInvalid);
  }
  struct TransactionInitialGasFields initial_gas = transaction_initial_gas(allowance, costs.intrinsic_execution, costs.intrinsic_state, costs.calldata_floor);
  return tx_validity_fields(sender, nonce_before, initial_gas, costs.blob_fee, result_2_754.tup0, result_2_754.tup1);
}

struct TxUpfrontResult apply_transaction_upfront_effects(struct TransactionFields tx, struct TxValidityFields v, PreparedAuthorizationList authorizations)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  bool create_target_prestate_empty;
  bool result_2_747 = (bool)(profile.fork >= Amsterdam);
  if (result_2_747 && tx.is_create) {
    bytes20 create_target = k_create_addr(v.sender, v.nonce_before);
    bool target_exists = k_account_exists(create_target);
    create_target_prestate_empty = (bool)(!target_exists);
  } else {
    create_target_prestate_empty = false;
  }
  u256 gas_debit = validated_word_product_u256_uint64_t_to_u256(v.gas_price, v.gas.admitted_limit);
  k_sub_balance(v.sender, gas_debit);
  bool has_blob_fee = word_nonzero(v.blob_fee);
  if (has_blob_fee) {
    k_sub_balance(v.sender, v.blob_fee);
  }
  k_bump_nonce(v.sender);
  prewarm(tx);
  uint64_t authorization_refund;
  if (profile.fork < Amsterdam) {
    authorization_refund = process_auth_list(authorizations);
  } else {
    authorization_refund = UINT64_C(0);
  }
  return ((struct TxUpfrontResult){.authorization_refund = authorization_refund, .create_target_prestate_empty = create_target_prestate_empty});
}

struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_uint_32_uint_32 enter_transaction_frame(struct TxValidityFields v)
{
  struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_uint_32_uint_32 result_8_898;
  StackPointer stack = stack_reset();
  struct tuple_uint_64_uint_64_uint_8_int_128_StackPointer_uint_32_uint_32 tmp_3_1218 = ((struct tuple_uint_64_uint_64_uint_8_int_128_StackPointer_uint_32_uint_32){.tup0 = v.gas.execution_remaining, .tup1 = v.gas.state_remaining, .tup2 = STATE_GAS_SPILL_ZERO, .tup3 = GAS_REFUND_ZERO, .tup4 = stack, .tup5 = MEMORY_BASE_ZERO, .tup6 = MEMORY_HEIGHT_ZERO});
  /* conversions */
  result_8_898.tup0 = tmp_3_1218.tup0;
  result_8_898.tup1 = tmp_3_1218.tup1;
  result_8_898.tup2 = (uint32_t)tmp_3_1218.tup2;
  result_8_898.tup3 = tmp_3_1218.tup3;
  result_8_898.tup4 = tmp_3_1218.tup4;
  result_8_898.tup5 = tmp_3_1218.tup5;
  result_8_898.tup6 = tmp_3_1218.tup6;
  /* end conversions */
  return result_8_898;
}

struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes run_create_transaction_frame(struct TransactionFields tx, bytes20 sender, uint64_t nonce_before, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_stack, uint32_t carried_memory_base, uint32_t carried_memory_height, struct CodeFields carried_code, struct CalldataSlice carried_calldata, uint64_t state_gas_reservoir)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bytes20 new_addr = k_create_addr(sender, nonce_before);
  uint64_t gas_after = carried_gas;
  uint64_t state_gas_after = carried_state_gas;
  uint32_t state_spill_after = carried_state_spill;
  __int128 refund_after = carried_refund;
  struct FrameStatus status_after = Running(UNIT);
  Bytes output_after = EMPTY_OUTPUT_SLICE;
  k_account_mark_warm(new_addr);
  bool occupied = k_account_occupied(new_addr);
  if (occupied) {
    gas_after = (uint64_t)GAS_ZERO;
    struct ExceptionalStateTransition exceptional;
    struct ExecutionProfileFields execution_profile_8_181 = k_execution_profile;
    if (execution_profile_8_181.protocol.fork >= Amsterdam) {
      struct FrameStatus Exceptional_result_2_2021 = Exceptional(AddressCollision);
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_2_2021});
    } else {
      struct FrameStatus Exceptional_result_2_2022 = Exceptional(AddressCollision);
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after, .state_gas_spilled = state_spill_after, .status = Exceptional_result_2_2022});
    }
    state_gas_after = exceptional.state_gas_remaining;
    state_spill_after = exceptional.state_gas_spilled;
    status_after = exceptional.status;
  } else {
    k_mark_created(new_addr);
    k_clear_storage(new_addr);
    k_bump_nonce(new_addr);
    bool transfers_value = word_nonzero(tx.value);
    if (transfers_value) {
      k_transfer(sender, new_addr, tx.value);
    }
    struct CodeFields frame_code = carried_code;
    struct CalldataSlice frame_calldata = carried_calldata;
    if (execution_profile.protocol.fork < Amsterdam) {
      Bytes initcode = transaction_initcode_slice(tx.input_src);
      bytes32 code_id = code_db_insert(initcode, execution_profile.protocol.fork);
      frame_code = code_db_resolve(code_id);
      frame_calldata = EMPTY_CALLDATA;
    }
    struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes result_2_708 = interpret_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint8_t_struct_CodeFields_struct_CalldataSlice_to_struct_tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes(gas_after, state_gas_after, state_spill_after, refund_after, carried_stack, carried_memory_base, carried_memory_height, sender, new_addr, new_addr, tx.value, state_gas_reservoir, false, UINT8_C(0), frame_code, frame_calldata);
    gas_after = result_2_708.tup0;
    state_gas_after = result_2_708.tup1;
    state_spill_after = result_2_708.tup2;
    refund_after = result_2_708.tup3;
    status_after = result_2_708.tup4;
    output_after = result_2_708.tup5;
    bool initcode_succeeded = frame_succeeded(status_after);
    if (initcode_succeeded) {
      uint32_t dep_len = output_after.len;
      bool valid_deployed_size = deployed_code_size_allowed(dep_len);
      bool valid_prefix;
      bool result_2_715 = (bool)(execution_profile.protocol.fork < London);
      if (result_2_715 || (dep_len == UINT8_C(0))) {
        valid_prefix = true;
      } else {
        uint64_t first_byte = output_byte(output_after, UINT8_C(0));
        valid_prefix = (bool)(first_byte != UINT64_C(0xEF));
      }
      if (valid_deployed_size && valid_prefix) {
        struct GasCharge deployment_charge = code_deployment_execution_cost(dep_len, gas_after);
        if (deployment_charge.affordable) {
          gas_after = gas_sub(gas_after, deployment_charge.cost);
          uint64_t state_deposit = code_deployment_state_cost(dep_len);
          bool deployment_halt = false;
          struct tuple_bool_uint_64_uint_64_uint_32 result_2_711 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint64_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after, state_spill_after, state_deposit);
          deployment_halt = result_2_711.tup0;
          gas_after = result_2_711.tup1;
          state_gas_after = result_2_711.tup2;
          state_spill_after = result_2_711.tup3;
          if (deployment_halt) {
            gas_after = (uint64_t)GAS_ZERO;
            struct ExceptionalStateTransition exceptional_3_1181;
            struct ExecutionProfileFields execution_profile_8_183 = k_execution_profile;
            if (execution_profile_8_183.protocol.fork >= Amsterdam) {
              struct FrameStatus Exceptional_result_8_187 = Exceptional(OutOfGas);
              exceptional_3_1181 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_187});
            } else {
              struct FrameStatus Exceptional_result_8_189 = Exceptional(OutOfGas);
              exceptional_3_1181 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after, .state_gas_spilled = state_spill_after, .status = Exceptional_result_8_189});
            }
            state_gas_after = exceptional_3_1181.state_gas_remaining;
            state_spill_after = exceptional_3_1181.state_gas_spilled;
            status_after = exceptional_3_1181.status;
          }
          bool deployment_succeeded = frame_succeeded(status_after);
          if (deployment_succeeded) {
            Bytes stored_code = code_db_intern_output(output_after);
            k_deploy_code(new_addr, stored_code);
          }
        } else {
          gas_after = (uint64_t)GAS_ZERO;
          struct ExceptionalStateTransition exceptional_3_1182;
          struct ExecutionProfileFields execution_profile_8_191 = k_execution_profile;
          if (execution_profile_8_191.protocol.fork >= Amsterdam) {
            struct FrameStatus Exceptional_result_8_195 = Exceptional(OutOfGas);
            exceptional_3_1182 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_195});
          } else {
            struct FrameStatus Exceptional_result_8_197 = Exceptional(OutOfGas);
            exceptional_3_1182 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after, .state_gas_spilled = state_spill_after, .status = Exceptional_result_8_197});
          }
          state_gas_after = exceptional_3_1182.state_gas_remaining;
          state_spill_after = exceptional_3_1182.state_gas_spilled;
          status_after = exceptional_3_1182.status;
        }
      } else {
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1183;
        struct ExecutionProfileFields execution_profile_8_199 = k_execution_profile;
        if (execution_profile_8_199.protocol.fork >= Amsterdam) {
          struct FrameStatus Exceptional_result_8_203 = Exceptional(OutOfGas);
          exceptional_3_1183 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_203});
        } else {
          struct FrameStatus Exceptional_result_8_205 = Exceptional(OutOfGas);
          exceptional_3_1183 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after, .state_gas_spilled = state_spill_after, .status = Exceptional_result_8_205});
        }
        state_gas_after = exceptional_3_1183.state_gas_remaining;
        state_spill_after = exceptional_3_1183.state_gas_spilled;
        status_after = exceptional_3_1183.status;
      }
    }
  }
  return ((struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes){.tup0 = gas_after, .tup1 = state_gas_after, .tup2 = state_spill_after, .tup3 = refund_after, .tup4 = status_after, .tup5 = output_after});
}

struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes run_call_transaction_frame(struct TransactionFields tx, bytes20 sender, bool delegated, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_stack, uint32_t carried_memory_base, uint32_t carried_memory_height, bytes20 carried_code_address, struct CodeFields carried_code, struct CalldataSlice carried_calldata, uint64_t state_gas_reservoir)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  uint64_t gas_after = carried_gas;
  uint64_t state_gas_after = carried_state_gas;
  uint32_t state_spill_after = carried_state_spill;
  __int128 refund_after = carried_refund;
  struct FrameStatus status_after = Running(UNIT);
  Bytes output_after = EMPTY_OUTPUT_SLICE;
  bytes20 code_address = carried_code_address;
  struct CodeFields frame_code = carried_code;
  struct CalldataSlice frame_calldata = carried_calldata;
  k_aload_(tx.recipient);
  bool transfers_value = word_nonzero(tx.value);
  if (transfers_value) {
    k_transfer(sender, tx.recipient, tx.value);
  }
  enum PrecompileId selected_precompile = precompile_id_for_address(tx.recipient);
  bool tmp_3_1108;
  if (delegated) {
    tmp_3_1108 = false;
  } else {
    tmp_3_1108 = neq_anything_EPrecompileId__(selected_precompile, NotPrecompile);
  }
  if (tmp_3_1108) {
    struct CalldataSlice precompile_input = InputCalldata(tx.input_src);
    struct GasCharge precompile_charge = precompile_gas(selected_precompile, precompile_input, gas_after);
    if (precompile_charge.affordable) {
      struct PrecompileResult result = run_precompile_slice(selected_precompile, precompile_input);
      if (result.success) {
        gas_after = gas_sub(gas_after, precompile_charge.cost);
        output_after = result.output;
        struct HaltKind halt_reason = HaltReturn(result.output);
        status_after = Halted(halt_reason);
      } else {
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional;
        struct ExecutionProfileFields execution_profile_8_207 = k_execution_profile;
        if (execution_profile_8_207.protocol.fork >= Amsterdam) {
          struct FrameStatus Exceptional_result_2_2021 = Exceptional(OutOfGas);
          exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_2_2021});
        } else {
          struct FrameStatus Exceptional_result_2_2022 = Exceptional(OutOfGas);
          exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after, .state_gas_spilled = state_spill_after, .status = Exceptional_result_2_2022});
        }
        state_gas_after = exceptional.state_gas_remaining;
        state_spill_after = exceptional.state_gas_spilled;
        status_after = exceptional.status;
      }
    } else {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1137;
      struct ExecutionProfileFields execution_profile_8_209 = k_execution_profile;
      if (execution_profile_8_209.protocol.fork >= Amsterdam) {
        struct FrameStatus Exceptional_result_8_213 = Exceptional(OutOfGas);
        exceptional_3_1137 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = Exceptional_result_8_213});
      } else {
        struct FrameStatus Exceptional_result_8_215 = Exceptional(OutOfGas);
        exceptional_3_1137 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after, .state_gas_spilled = state_spill_after, .status = Exceptional_result_8_215});
      }
      state_gas_after = exceptional_3_1137.state_gas_remaining;
      state_spill_after = exceptional_3_1137.state_gas_spilled;
      status_after = exceptional_3_1137.status;
    }
  } else {
    if (execution_profile.protocol.fork < Amsterdam) {
      frame_calldata = InputCalldata(tx.input_src);
      code_address = tx.recipient;
      struct tuple_bool_bytes20 result_2_694 = k_deleg_target(tx.recipient);
      bool tx_deleg = result_2_694.tup0;
      bytes20 tx_dtgt = result_2_694.tup1;
      if (tx_deleg) {
        k_account_mark_warm(tx_dtgt);
        k_aload_(tx_dtgt);
      }
      if (tx_deleg) {
        code_address = tx_dtgt;
      }
      frame_code = executable_code(tx.recipient, tx_deleg, tx_dtgt);
    }
    struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes result_2_699 = interpret_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint8_t_struct_CodeFields_struct_CalldataSlice_to_struct_tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes(gas_after, state_gas_after, state_spill_after, refund_after, carried_stack, carried_memory_base, carried_memory_height, sender, tx.recipient, code_address, tx.value, state_gas_reservoir, false, UINT8_C(0), frame_code, frame_calldata);
    gas_after = result_2_699.tup0;
    state_gas_after = result_2_699.tup1;
    state_spill_after = result_2_699.tup2;
    refund_after = result_2_699.tup3;
    status_after = result_2_699.tup4;
    output_after = result_2_699.tup5;
  }
  return ((struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes){.tup0 = gas_after, .tup1 = state_gas_after, .tup2 = state_spill_after, .tup3 = refund_after, .tup4 = status_after, .tup5 = output_after});
}

struct TxFrameResultFields run_legacy_transaction_frame(struct TransactionFields tx, struct TxValidityFields v)
{
  k_journal_checkpoint();
  struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_uint_32_uint_32 enter_transaction_frame_result_2_673 = enter_transaction_frame(v);
  uint64_t initial_state_gas = enter_transaction_frame_result_2_673.tup1;
  struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes result_2_679;
  if (tx.is_create) {
    result_2_679 = run_create_transaction_frame(tx, v.sender, v.nonce_before, enter_transaction_frame_result_2_673.tup0, initial_state_gas, enter_transaction_frame_result_2_673.tup2, enter_transaction_frame_result_2_673.tup3, enter_transaction_frame_result_2_673.tup4, enter_transaction_frame_result_2_673.tup5, enter_transaction_frame_result_2_673.tup6, EMPTY_CODE, EMPTY_CALLDATA, initial_state_gas);
  } else {
    result_2_679 = run_call_transaction_frame(tx, v.sender, false, enter_transaction_frame_result_2_673.tup0, initial_state_gas, enter_transaction_frame_result_2_673.tup2, enter_transaction_frame_result_2_673.tup3, enter_transaction_frame_result_2_673.tup4, enter_transaction_frame_result_2_673.tup5, enter_transaction_frame_result_2_673.tup6, tx.recipient, EMPTY_CODE, EMPTY_CALLDATA, initial_state_gas);
  }
  uint64_t state_gas_after = result_2_679.tup1;
  bool success = frame_succeeded(result_2_679.tup4);
  if (success) {
    k_journal_commit();
  } else {
    k_journal_revert();
  }
  __int128 state_delta = frame_state_gas_used(initial_state_gas, state_gas_after, result_2_679.tup2);
  struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_result_2_680 = tx_frame_gas_snapshot_struct_TransactionInitialGasFields_uint64_t_uint64_t___int128_to_struct_TxFrameGasSnapshotFields(v.gas, result_2_679.tup0, state_gas_after, state_delta);
  __int128 result_2_681 = success ? result_2_679.tup3 : GAS_REFUND_ZERO;
  return ((struct TxFrameResultFields){.gas = tx_frame_gas_snapshot_result_2_680, .refund = result_2_681, .success = success});
}

struct TxFrameResultFields run_amsterdam_transaction_frame(struct TransactionFields tx, struct TxValidityFields v, struct TxUpfrontResult upfront, PreparedAuthorizationList authorizations)
{
  struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_uint_32_uint_32 enter_transaction_frame_result_2_652 = enter_transaction_frame(v);
  uint64_t gas_after = enter_transaction_frame_result_2_652.tup0;
  uint64_t state_gas_after = enter_transaction_frame_result_2_652.tup1;
  uint32_t state_spill_after = enter_transaction_frame_result_2_652.tup2;
  __int128 refund_after = enter_transaction_frame_result_2_652.tup3;
  struct FrameStatus status_after = Running(UNIT);
  struct TransactionInitialGasFields initial_gas = v.gas;
  k_journal_checkpoint();
  uint64_t preparation_reservoir = state_gas_after;
  bytes20 current_target;
  if (tx.is_create) {
    current_target = k_create_addr(v.sender, v.nonce_before);
  } else {
    current_target = tx.recipient;
  }
  authorization_tracker_reset(authorizations.count);
  bool transfers_value = word_nonzero(tx.value);
  bool preparation_ready = false;
  struct tuple_bool_uint_64_uint_64_uint_32 result_2_656 = process_amsterdam_auth_cursor(authorizations, authorizations.count, v.sender, current_target, transfers_value, gas_after, state_gas_after, state_spill_after);
  preparation_ready = result_2_656.tup0;
  gas_after = result_2_656.tup1;
  state_gas_after = result_2_656.tup2;
  state_spill_after = result_2_656.tup3;
  __int128 authorization_state_gas = FRAME_STATE_GAS_DELTA_ZERO;
  bool delegated = false;
  uint64_t execution_reservoir = state_gas_after;
  bytes20 prepared_code_address = current_target;
  struct CodeFields prepared_code = EMPTY_CODE;
  struct CalldataSlice prepared_calldata = EMPTY_CALLDATA;
  if (preparation_ready) {
    authorization_state_gas = frame_state_gas_used(preparation_reservoir, state_gas_after, state_spill_after);
    execution_reservoir = state_gas_after;
    state_spill_after = (uint32_t)STATE_GAS_SPILL_ZERO;
    struct tuple_TransactionPreparation_uint_64_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice prepare_amsterdam_transaction_dispatch_result_2_657 = prepare_amsterdam_transaction_dispatch(tx, v, upfront, gas_after, state_gas_after, state_spill_after);
    struct TransactionPreparation preparation = prepare_amsterdam_transaction_dispatch_result_2_657.tup0;
    gas_after = prepare_amsterdam_transaction_dispatch_result_2_657.tup1;
    state_gas_after = prepare_amsterdam_transaction_dispatch_result_2_657.tup2;
    state_spill_after = prepare_amsterdam_transaction_dispatch_result_2_657.tup3;
    preparation_ready = preparation.ready;
    delegated = preparation.delegated;
    prepared_code_address = prepare_amsterdam_transaction_dispatch_result_2_657.tup5;
    prepared_code = prepare_amsterdam_transaction_dispatch_result_2_657.tup6;
    prepared_calldata = prepare_amsterdam_transaction_dispatch_result_2_657.tup7;
  }
  if (!preparation_ready) {
    k_journal_revert();
    state_gas_after = preparation_reservoir;
    state_spill_after = (uint32_t)STATE_GAS_SPILL_ZERO;
    struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_result_2_658 = tx_frame_gas_snapshot_struct_TransactionInitialGasFields_uint8_t_uint8_t___int128_to_struct_TxFrameGasSnapshotFields(initial_gas, GAS_ZERO, STATE_GAS_ZERO, FRAME_STATE_GAS_DELTA_ZERO);
    return ((struct TxFrameResultFields){.gas = tx_frame_gas_snapshot_result_2_658, .refund = GAS_REFUND_ZERO, .success = false});
  }
  k_journal_checkpoint();
  if (tx.is_create) {
    struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes result_2_663 = run_create_transaction_frame(tx, v.sender, v.nonce_before, gas_after, state_gas_after, state_spill_after, refund_after, enter_transaction_frame_result_2_652.tup4, enter_transaction_frame_result_2_652.tup5, enter_transaction_frame_result_2_652.tup6, prepared_code, prepared_calldata, execution_reservoir);
    gas_after = result_2_663.tup0;
    state_gas_after = result_2_663.tup1;
    state_spill_after = result_2_663.tup2;
    refund_after = result_2_663.tup3;
    status_after = result_2_663.tup4;
  } else {
    struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes result_2_665 = run_call_transaction_frame(tx, v.sender, delegated, gas_after, state_gas_after, state_spill_after, refund_after, enter_transaction_frame_result_2_652.tup4, enter_transaction_frame_result_2_652.tup5, enter_transaction_frame_result_2_652.tup6, prepared_code_address, prepared_code, prepared_calldata, execution_reservoir);
    gas_after = result_2_665.tup0;
    state_gas_after = result_2_665.tup1;
    state_spill_after = result_2_665.tup2;
    refund_after = result_2_665.tup3;
    status_after = result_2_665.tup4;
  }
  bool success = frame_succeeded(status_after);
  if (success) {
    k_journal_commit();
  } else {
    k_journal_revert();
  }
  k_journal_commit();
  __int128 frame_state_gas_used_result_2_668 = frame_state_gas_used(execution_reservoir, state_gas_after, state_spill_after);
  struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_result_2_666 = tx_frame_gas_snapshot(initial_gas, gas_after, state_gas_after, (authorization_state_gas + frame_state_gas_used_result_2_668));
  __int128 result_2_667 = success ? refund_after : GAS_REFUND_ZERO;
  return ((struct TxFrameResultFields){.gas = tx_frame_gas_snapshot_result_2_666, .refund = result_2_667, .success = success});
}

struct TxFrameResultFields run_transaction_frame(struct TransactionFields tx, struct TxValidityFields v, struct TxUpfrontResult upfront, PreparedAuthorizationList authorizations)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  if (execution_profile.protocol.fork >= Amsterdam) {
    return run_amsterdam_transaction_frame(tx, v, upfront, authorizations);
  }
  return run_legacy_transaction_frame(tx, v);
}

uint64_t remaining_gas_after_refund(__int128 total, uint64_t remaining, uint64_t cap)
{
  uint64_t refund;
  if (!(UINT8_C(0) < total)) {
    refund = UINT64_C(0);
  } else if (!(cap < total)) {
    refund = (uint64_t)total;
  } else {
    refund = cap;
  }
  return (refund + remaining);
}

struct ReceiptFields settle_transaction(struct TransactionFields tx, struct TxValidityFields v, uint64_t authorization_refund, struct TxFrameResultFields fr)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  struct TxFrameGasSnapshotFields gas_snapshot = fr.gas;
  uint64_t gas_limit = gas_snapshot.admitted_limit;
  uint64_t gas_left_1_4 = remaining_gas_after_refund((fr.refund + (__int128)authorization_refund), gas_snapshot.remaining, ((gas_limit - gas_snapshot.remaining) / (uint64_t)profile.refund_divisor));
  uint64_t floor;
  bool result_2_646 = (bool)(profile.fork >= Prague);
  floor = result_2_646 ? gas_snapshot.calldata_floor : UINT64_C(0);
  uint64_t gas_used = (gas_limit - gas_left_1_4) < floor ? floor : (gas_limit - gas_left_1_4);
  uint64_t tx_state_gas = gas_snapshot.state_used;
  uint64_t execution_gas = ((gas_limit - gas_snapshot.remaining) - tx_state_gas) < floor ? floor : ((gas_limit - gas_snapshot.remaining) - tx_state_gas);
  u256 sender_refund = validated_word_product_u256_uint64_t_to_u256(v.gas_price, (gas_limit - gas_used));
  k_add_balance(v.sender, sender_refund);
  bytes20 coinbase = k_coinbase();
  u256 priority_payment = validated_word_product_u256_uint64_t_to_u256(v.priority_fee, gas_used);
  k_add_balance(coinbase, priority_payment);
  k_tx_merge();
  struct LogSeriesRef logs = read_logs();
  if (!u128_lt_u64(u128_add_u64_u64(execution_gas, tx_state_gas), gas_used)) {
    return receipt_within(tx.tx_type, fr.success, gas_used, execution_gas, tx_state_gas, logs);
  }
  fatal_error(ExecutionInvalid);
}

struct ReceiptFields process_transaction(struct TransactionFields tx, struct TransactionGasAllowanceFields allowance)
{
  k_tx_reset();
  struct TxValidityFields validity = check_transaction_validity(tx, allowance);
  PreparedAuthorizationList authorizations = prepare_authorizations_(tx.authorizations);
  struct TxEnvFields environment = tx_env(tx.sender, validity.gas_price, tx.blob_hashes);
  k_set_tx(environment);
  struct TxUpfrontResult upfront = apply_transaction_upfront_effects(tx, validity, authorizations);
  struct TxFrameResultFields frame_result = run_transaction_frame(tx, validity, upfront, authorizations);
  return settle_transaction(tx, validity, upfront.authorization_refund, frame_result);
}

struct tuple_TransactionPreparation_uint_64_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice prepare_amsterdam_transaction_dispatch(struct TransactionFields tx, struct TxValidityFields v, struct TxUpfrontResult upfront, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill)
{
  struct tuple_TransactionPreparation_uint_64_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice result_8_1185;
  uint64_t gas_after = carried_gas;
  uint64_t state_gas_after = carried_state_gas;
  uint32_t state_spill_after = carried_state_spill;
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bytes20 current_target;
  if (tx.is_create) {
    current_target = k_create_addr(v.sender, v.nonce_before);
  } else {
    current_target = tx.recipient;
  }
  if (tx.is_create) {
    if (upfront.create_target_prestate_empty) {
      struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_result_2_720 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32_variant_2(gas_after, state_gas_after, state_spill_after, G_amsterdam_state_new_account);
      gas_after = charge_state_gas_result_2_720.tup1;
      state_gas_after = charge_state_gas_result_2_720.tup2;
      state_spill_after = charge_state_gas_result_2_720.tup3;
      if (charge_state_gas_result_2_720.tup0) {
        return ((struct tuple_TransactionPreparation_uint_64_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice){.tup0 = ((struct TransactionPreparation){.delegated = false, .ready = false}), .tup1 = gas_after, .tup2 = state_gas_after, .tup3 = state_spill_after, .tup4 = current_target, .tup5 = current_target, .tup6 = EMPTY_CODE, .tup7 = EMPTY_CALLDATA});
      }
    }
    Bytes initcode = transaction_initcode_slice(tx.input_src);
    bytes32 code_id = code_db_insert(initcode, execution_profile.protocol.fork);
    struct CodeFields code = code_db_resolve(code_id);
    result_8_1185 = ((struct tuple_TransactionPreparation_uint_64_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice){.tup0 = ((struct TransactionPreparation){.delegated = false, .ready = true}), .tup1 = gas_after, .tup2 = state_gas_after, .tup3 = state_spill_after, .tup4 = current_target, .tup5 = current_target, .tup6 = code, .tup7 = EMPTY_CALLDATA});
  } else {
    struct CalldataSlice calldata = InputCalldata(tx.input_src);
    bool transfers_value = word_nonzero(tx.value);
    bool recipient_empty = k_account_is_empty(tx.recipient);
    if (transfers_value && recipient_empty) {
      struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_result_2_724 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32_variant_2(gas_after, state_gas_after, state_spill_after, G_amsterdam_state_new_account);
      gas_after = charge_state_gas_result_2_724.tup1;
      state_gas_after = charge_state_gas_result_2_724.tup2;
      state_spill_after = charge_state_gas_result_2_724.tup3;
      if (charge_state_gas_result_2_724.tup0) {
        return ((struct tuple_TransactionPreparation_uint_64_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice){.tup0 = ((struct TransactionPreparation){.delegated = false, .ready = false}), .tup1 = gas_after, .tup2 = state_gas_after, .tup3 = state_spill_after, .tup4 = current_target, .tup5 = current_target, .tup6 = EMPTY_CODE, .tup7 = calldata});
      }
    }
    struct tuple_bool_bytes20 result_2_726 = k_deleg_target(tx.recipient);
    bool delegated = result_2_726.tup0;
    bytes20 delegate = result_2_726.tup1;
    if (delegated) {
      bool warm = k_account_is_warm(delegate);
      uint16_t access_cost = account_cost(warm);
      if (gas_after < access_cost) {
        struct tuple_TransactionPreparation_uint_8_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice tmp_3_1198 = ((struct tuple_TransactionPreparation_uint_8_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice){.tup0 = ((struct TransactionPreparation){.delegated = false, .ready = false}), .tup1 = GAS_ZERO, .tup2 = state_gas_after, .tup3 = state_spill_after, .tup4 = current_target, .tup5 = current_target, .tup6 = EMPTY_CODE, .tup7 = calldata});
        /* conversions */
        result_8_1185.tup0 = tmp_3_1198.tup0;
        result_8_1185.tup1 = (uint64_t)tmp_3_1198.tup1;
        result_8_1185.tup2 = tmp_3_1198.tup2;
        result_8_1185.tup3 = tmp_3_1198.tup3;
        result_8_1185.tup4 = tmp_3_1198.tup4;
        result_8_1185.tup5 = tmp_3_1198.tup5;
        result_8_1185.tup6 = tmp_3_1198.tup6;
        result_8_1185.tup7 = tmp_3_1198.tup7;
        /* end conversions */
        goto end_function_1582;
      }
      gas_after = gas_sub_uint64_t_uint16_t_to_uint64_t(gas_after, access_cost);
      k_account_mark_warm(delegate);
    }
    bytes20 code_address = delegated ? delegate : current_target;
    struct CodeFields code_3_1217 = executable_code(tx.recipient, delegated, delegate);
    result_8_1185 = ((struct tuple_TransactionPreparation_uint_64_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice){.tup0 = ((struct TransactionPreparation){.delegated = delegated, .ready = true}), .tup1 = gas_after, .tup2 = state_gas_after, .tup3 = state_spill_after, .tup4 = current_target, .tup5 = code_address, .tup6 = code_3_1217, .tup7 = calldata});
  }
end_function_1582: ;
  return result_8_1185;
}

struct TransactionGasAllowanceFields transaction_gas_allowance(uint64_t value, uint64_t total_limit, uint64_t regular_limit)
{
  if (total_limit < value) {
    fatal_error(ExecutionInvalid);
  }
  return transaction_gas_allowance_fields(value, regular_limit);
}

struct TransactionInitialGasFields transaction_initial_gas(struct TransactionGasAllowanceFields allowance, uint64_t intrinsic_execution, uint64_t intrinsic_state, uint64_t calldata_floor)
{
  if (allowance.total < intrinsic_execution) {
    fatal_error(ExecutionInvalid);
  }
  if (((__int128)allowance.total - (__int128)intrinsic_execution) < (__int128)intrinsic_state) {
    fatal_error(ExecutionInvalid);
  }
  if ((allowance.regular < intrinsic_execution) || (allowance.regular < calldata_floor)) {
    fatal_error(ExecutionInvalid);
  }
  if (((allowance.total - intrinsic_execution) - intrinsic_state) < (allowance.regular - intrinsic_execution)) {
    return transaction_initial_gas_fields_uint64_t_uint64_t_uint64_t_uint64_t_uint64_t_uint64_t_uint8_t_to_struct_TransactionInitialGasFields(allowance.total, allowance.regular, intrinsic_execution, intrinsic_state, calldata_floor, ((allowance.total - intrinsic_execution) - intrinsic_state), UINT8_C(0));
  }
  return transaction_initial_gas_fields(allowance.total, allowance.regular, intrinsic_execution, intrinsic_state, calldata_floor, (allowance.regular - intrinsic_execution), (uint64_t)((((__int128)allowance.total - (__int128)intrinsic_execution) - (__int128)intrinsic_state) - ((__int128)allowance.regular - (__int128)intrinsic_execution)));
}

struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_struct_TransactionInitialGasFields_uint64_t_uint64_t___int128_to_struct_TxFrameGasSnapshotFields(struct TransactionInitialGasFields initial, uint64_t execution, uint64_t state, __int128 state_delta)
{
  if (execution <= initial.admitted_limit) {
    if (state <= (initial.admitted_limit - execution)) {
      if (!(UINT8_C(0) < (state_delta + (__int128)initial.intrinsic_state))) {
        if ((initial.admitted_limit - (state + execution)) <= initial.regular_limit) {
          return tx_frame_gas_snapshot_fields_uint64_t_uint64_t_uint64_t_uint64_t_uint8_t_to_struct_TxFrameGasSnapshotFields(initial.admitted_limit, initial.regular_limit, initial.calldata_floor, (state + execution), UINT8_C(0));
        }
        fatal_error(ExecutionInvalid);
      }
      if (!u64_lt_u128((initial.admitted_limit - (state + execution)), ((u128){{(uint64_t)((state_delta + (__int128)initial.intrinsic_state)), (uint64_t)(((unsigned __int128)((state_delta + (__int128)initial.intrinsic_state))) >> 64)}}))) {
        if (((initial.admitted_limit - (state + execution)) - (uint64_t)u128_to_u64_unchecked(((u128){{(uint64_t)((state_delta + (__int128)initial.intrinsic_state)), (uint64_t)(((unsigned __int128)((state_delta + (__int128)initial.intrinsic_state))) >> 64)}}))) <= initial.regular_limit) {
          return tx_frame_gas_snapshot_fields(initial.admitted_limit, initial.regular_limit, initial.calldata_floor, (state + execution), (uint64_t)u128_to_u64_unchecked(((u128){{(uint64_t)((state_delta + (__int128)initial.intrinsic_state)), (uint64_t)(((unsigned __int128)((state_delta + (__int128)initial.intrinsic_state))) >> 64)}})));
        }
        fatal_error(ExecutionInvalid);
      }
      fatal_error(ExecutionInvalid);
    }
    fatal_error(ExecutionInvalid);
  }
  fatal_error(ExecutionInvalid);
}

struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_struct_TransactionInitialGasFields_uint8_t_uint8_t___int128_to_struct_TxFrameGasSnapshotFields(struct TransactionInitialGasFields initial, uint8_t execution, uint8_t state, __int128 state_delta)
{
  if (!(UINT8_C(0) < (state_delta + (__int128)initial.intrinsic_state))) {
    if ((initial.admitted_limit - ((uint64_t)state + (uint64_t)execution)) <= initial.regular_limit) {
      return tx_frame_gas_snapshot_fields_uint64_t_uint64_t_uint64_t_uint64_t_uint8_t_to_struct_TxFrameGasSnapshotFields(initial.admitted_limit, initial.regular_limit, initial.calldata_floor, ((uint64_t)state + (uint64_t)execution), UINT8_C(0));
    }
    fatal_error(ExecutionInvalid);
  }
  if (!u64_lt_u128((initial.admitted_limit - ((uint64_t)state + (uint64_t)execution)), ((u128){{(uint64_t)((state_delta + (__int128)initial.intrinsic_state)), (uint64_t)(((unsigned __int128)((state_delta + (__int128)initial.intrinsic_state))) >> 64)}}))) {
    if (((initial.admitted_limit - ((uint64_t)state + (uint64_t)execution)) - (uint64_t)u128_to_u64_unchecked(((u128){{(uint64_t)((state_delta + (__int128)initial.intrinsic_state)), (uint64_t)(((unsigned __int128)((state_delta + (__int128)initial.intrinsic_state))) >> 64)}}))) <= initial.regular_limit) {
      return tx_frame_gas_snapshot_fields(initial.admitted_limit, initial.regular_limit, initial.calldata_floor, ((uint64_t)state + (uint64_t)execution), (uint64_t)u128_to_u64_unchecked(((u128){{(uint64_t)((state_delta + (__int128)initial.intrinsic_state)), (uint64_t)(((unsigned __int128)((state_delta + (__int128)initial.intrinsic_state))) >> 64)}})));
    }
    fatal_error(ExecutionInvalid);
  }
  fatal_error(ExecutionInvalid);
}

u256 validated_word_product_u256_uint32_t_to_u256(u256 value, uint32_t factor)
{
  bool word_is_zero_result_2_911 = eq_u256(value, WORD_ZERO);
  if (word_is_zero_result_2_911 || u256_eq_u64(WORD_ZERO, factor)) {
    return WORD_ZERO;
  }
  bool result_2_914;
  u256 word_div_word_result_2_913 = word_div_word_u256_uint32_t_to_u256(WORD_ALL_ONES, factor);
  result_2_914 = word_ule(value, word_div_word_result_2_913);
  if (result_2_914) {
    return word_mul_word_u256_uint32_t_to_u256(value, factor);
  }
  fatal_error(ExecutionInvalid);
}

u256 validated_word_product_u256_uint32_t_to_u256_variant_2(u256 value, uint32_t factor)
{
  bool word_is_zero_result_2_911 = eq_u256(value, WORD_ZERO);
  if (word_is_zero_result_2_911 || u256_eq_u64(WORD_ZERO, factor)) {
    return WORD_ZERO;
  }
  bool result_2_914;
  u256 word_div_word_result_2_913 = word_div_word_u256_uint32_t_to_u256_variant_2(WORD_ALL_ONES, factor);
  result_2_914 = word_ule(value, word_div_word_result_2_913);
  if (result_2_914) {
    return word_mul_word_u256_uint32_t_to_u256(value, factor);
  }
  fatal_error(ExecutionInvalid);
}

u256 validated_word_product_u256_uint64_t_to_u256(u256 value, uint64_t factor)
{
  bool word_is_zero_result_2_911 = eq_u256(value, WORD_ZERO);
  if (word_is_zero_result_2_911 || u256_eq_u64(WORD_ZERO, factor)) {
    return WORD_ZERO;
  }
  bool result_2_914;
  u256 word_div_word_result_2_913 = word_div_word_u256_uint64_t_to_u256(WORD_ALL_ONES, factor);
  result_2_914 = word_ule(value, word_div_word_result_2_913);
  if (result_2_914) {
    return word_mul_word_u256_uint64_t_to_u256(value, factor);
  }
  fatal_error(ExecutionInvalid);
}

