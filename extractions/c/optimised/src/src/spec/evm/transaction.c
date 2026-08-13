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
  uint64_t result_8_932;
  uint32_t nonzeroes = slice_count_nonzero(input);
  if (nonzeroes <= input.len) {
    int64_t integer_result_3_3874 = ((int64_t)G_txdatazero * ((int64_t)input.len - (int64_t)nonzeroes));
    uint64_t mult_atom_result_2_984 = ((uint64_t)G_txdatanonzero * (uint64_t)nonzeroes);
    result_8_932 = (uint64_t)((int64_t)(uint64_t)integer_result_3_3874 + (int64_t)mult_atom_result_2_984);
  } else {
    fatal_error(ExecutionInvalid);
  }
  return result_8_932;
}

uint64_t legacy_intrinsic_gas(struct TransactionFields tx)
{
  uint64_t result_8_933;
  uint64_t data_cost = calldata_cost(tx.input_src);
  uint64_t address_cost = ((uint64_t)G_access_list_address * (uint64_t)tx.access_list.address_count);
  uint64_t slot_cost = ((uint64_t)G_access_list_storage_key * (uint64_t)tx.access_list.slot_count);
  uint64_t auth_cost = ((uint64_t)PER_EMPTY_ACCOUNT * (uint64_t)tx.authorizations.count);
  __int128 integer_result_3_3876 = ((__int128)data_cost + (__int128)G_transaction);
  if (tx.is_create) {
    __int128 integer_result_3_3880 = ((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)integer_result_3_3876 + (__int128)address_cost) + (__int128)slot_cost) + (__int128)auth_cost) + (__int128)G_txcreate);
    uint32_t transaction_initcode_gas_result_2_972 = transaction_initcode_gas(tx.input_src.len);
    result_8_933 = (uint64_t)((__int128)(uint64_t)integer_result_3_3880 + (__int128)transaction_initcode_gas_result_2_972);
  } else {
    result_8_933 = (uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)integer_result_3_3876 + (__int128)address_cost) + (__int128)slot_cost) + (__int128)auth_cost);
  }
  return result_8_933;
}

uint64_t legacy_calldata_floor(Bytes input)
{
  uint64_t result_8_934;
  uint32_t nonzeroes = slice_count_nonzero(input);
  if (nonzeroes <= input.len) {
    int64_t integer_result_3_3884 = ((int64_t)(uint64_t)((int64_t)(uint64_t)((int64_t)UINT8_C(10) * ((int64_t)input.len - (int64_t)nonzeroes)) + (int64_t)(UINT64_C(40) * (uint64_t)nonzeroes)) + (int64_t)G_transaction);
    result_8_934 = (uint64_t)integer_result_3_3884;
  } else {
    fatal_error(ExecutionInvalid);
  }
  return result_8_934;
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
  bool result_2_963 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C20__(tx.recipient, tx.sender);
  if (result_2_963) {
    if (transfers_value) {
      uint16_t add_atom_result_2_964 = ((uint16_t)((uint32_t)AMSTERDAM_TX_VALUE_COST + (uint32_t)AMSTERDAM_COLD_ACCOUNT_ACCESS));
      return ((uint16_t)((uint32_t)AMSTERDAM_TRANSFER_LOG_COST + (uint32_t)add_atom_result_2_964));
    }
    return AMSTERDAM_COLD_ACCOUNT_ACCESS;
  }
  return UINT16_C(0);
}

struct IntrinsicGasCost intrinsic_gas(struct TransactionFields tx)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool result_2_933 = (bool)(execution_profile.protocol.fork < Amsterdam);
  if (result_2_933) {
    uint64_t legacy_intrinsic_gas_result_2_934 = legacy_intrinsic_gas(tx);
    uint64_t result_2_936 = legacy_calldata_floor(tx.input_src);
    return ((struct IntrinsicGasCost){.calldata_floor = result_2_936, .execution = legacy_intrinsic_gas_result_2_934, .state = UINT64_C(0)});
  }
  Bytes input = tx.input_src;
  uint16_t recipient = amsterdam_recipient_execution_cost(tx);
  uint32_t address_count = tx.access_list.address_count;
  uint32_t slot_count = tx.access_list.slot_count;
  uint64_t mult_atom_result_2_952 = ((uint64_t)AMSTERDAM_ACCESS_LIST_ADDRESS * (uint64_t)address_count);
  uint64_t mult_atom_result_2_953 = ((uint64_t)AMSTERDAM_ACCESS_LIST_SLOT * (uint64_t)slot_count);
  uint64_t mult_atom_result_2_955 = ((uint64_t)AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR * (uint64_t)address_count);
  uint64_t mult_atom_result_2_957 = ((uint64_t)AMSTERDAM_ACCESS_LIST_SLOT_FLOOR * (uint64_t)slot_count);
  uint64_t authorization_execution = ((uint64_t)AMSTERDAM_AUTH_BASE * (uint64_t)tx.authorizations.count);
  uint32_t create_execution;
  if (tx.is_create) {
    create_execution = transaction_initcode_gas(input.len);
  } else {
    create_execution = UINT32_C(0);
  }
  uint64_t result_2_944 = calldata_cost(tx.input_src);
  __int128 integer_result_3_3885 = ((__int128)result_2_944 + (__int128)AMSTERDAM_TX_BASE);
  uint64_t result_2_938;
  uint64_t mult_atom_result_2_937 = ((uint64_t)AMSTERDAM_CALLDATA_FLOOR_BYTE * (uint64_t)input.len);
  result_2_938 = (mult_atom_result_2_937 + (uint64_t)AMSTERDAM_TX_BASE);
  uint64_t mult_atom_result_2_940 = ((uint64_t)AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR * (uint64_t)address_count);
  uint64_t mult_atom_result_2_942 = ((uint64_t)AMSTERDAM_ACCESS_LIST_SLOT_FLOOR * (uint64_t)slot_count);
  return ((struct IntrinsicGasCost){.calldata_floor = (mult_atom_result_2_942 + (mult_atom_result_2_940 + (result_2_938 + (uint64_t)recipient))), .execution = (uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)integer_result_3_3885 + (__int128)recipient) + ((__int128)mult_atom_result_2_957 + ((__int128)mult_atom_result_2_955 + ((__int128)mult_atom_result_2_953 + (__int128)mult_atom_result_2_952)))) + (__int128)authorization_execution) + (__int128)create_execution), .state = UINT64_C(0)});
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
    bool result_2_922 = word_ule(blob_price, tx.max_blob_fee);
    if (result_2_922) {
      blob_fee = validated_word_product_u256_uint32_t_to_u256_variant_2(blob_price, blob_gas);
    } else {
      fatal_error(ExecutionInvalid);
    }
  }
  u256 execution_cap = validated_word_product_u256_uint64_t_to_u256(tx.max_fee, gas_limit);
  u256 blob_cap = validated_word_product_u256_uint32_t_to_u256(tx.max_blob_fee, blob_gas);
  u256 execution_and_value;
  bool result_2_916;
  u256 word_sub_word_result_2_915;
  bool lteq_int_result_2_2658 = (bool)(!u256_lt(WORD_ALL_ONES, execution_cap));
  if (lteq_int_result_2_2658) {
    word_sub_word_result_2_915 = u256_sub(WORD_ALL_ONES, execution_cap);
  } else {
    u256 sub_atom_result_2_2659 = u256_sub(execution_cap, WORD_ALL_ONES);
    word_sub_word_result_2_915 = u256_of_u320_unchecked(u320_add(u320_of_u256(u256_sub((u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}}, sub_atom_result_2_2659)), u320_of_u64(UINT8_C(1))));
  }
  result_2_916 = word_ule(tx.value, word_sub_word_result_2_915);
  if (result_2_916) {
    execution_and_value = word_add_word(execution_cap, tx.value);
  } else {
    fatal_error(ExecutionInvalid);
  }
  u256 upfront;
  bool result_2_913;
  u256 word_sub_word_result_2_912;
  bool lteq_int_result_8_195 = (bool)(!u256_lt(WORD_ALL_ONES, execution_and_value));
  if (lteq_int_result_8_195) {
    word_sub_word_result_2_912 = u256_sub(WORD_ALL_ONES, execution_and_value);
  } else {
    u256 sub_atom_result_8_198 = u256_sub(execution_and_value, WORD_ALL_ONES);
    word_sub_word_result_2_912 = u256_of_u320_unchecked(u320_add(u320_of_u256(u256_sub((u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}}, sub_atom_result_8_198)), u320_of_u64(UINT8_C(1))));
  }
  result_2_913 = word_ule(blob_cap, word_sub_word_result_2_912);
  if (result_2_913) {
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
    struct tuple_bool_bytes20 k_deleg_target_result_2_871 = k_deleg_target(au.authority);
    bytes32 code_key = k_code_key(au.authority);
    uint64_t nonce = k_get_nonce(au.authority);
    bool eq_anything_result_2_872 = eq_bytes32(code_key, KECCAK_EMPTY);
    bool tmp_3_1340 = (bool)((eq_anything_result_2_872 || k_deleg_target_result_2_871.tup0) && (nonce == au.nonce));
    if (tmp_3_1340) {
      bool existed = k_account_exists(au.authority);
      bool result_2_877 = eq_bytes20(au.address, ZERO_ADDRESS);
      if (result_2_877) {
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
  uint64_t gas_after = carried_gas;
  uint64_t state_gas_after = carried_state_gas;
  uint32_t state_spill_after = carried_state_spill;
  bool chain_id_is_zero = eq_u256(au.chain_id, WORD_ZERO);
  uint64_t expected_chain_id = word_of_chain_identifier(k_chain_id);
  if (au.valid_sig && (chain_id_is_zero || u256_eq_u64(au.chain_id, expected_chain_id))) {
    k_account_mark_warm(au.authority);
    struct tuple_bool_bytes20 k_deleg_target_result_2_845 = k_deleg_target(au.authority);
    bool currently_delegated = k_deleg_target_result_2_845.tup0;
    bytes32 code_key = k_code_key(au.authority);
    uint64_t nonce = k_get_nonce(au.authority);
    bool eq_anything_result_2_846 = eq_bytes32(code_key, KECCAK_EMPTY);
    bool tmp_3_1304 = (bool)((eq_anything_result_2_846 || currently_delegated) && (nonce == au.nonce));
    if (tmp_3_1304) {
      bool seen = authorization_tracker_seen(au.authority);
      bool delegated_before_tx;
      if (seen) {
        delegated_before_tx = authorization_tracker_originally_delegated(au.authority);
      } else {
        delegated_before_tx = currently_delegated;
      }
      bool tmp_3_1308 = (bool)(seen || (eq_bytes20(au.authority, sender) || (transfers_value && eq_bytes20(au.authority, current_target))));
      bool account_exists = k_account_exists(au.authority);
      if (!account_exists) {
        struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_result_2_850 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after, state_spill_after, G_amsterdam_state_new_account);
        gas_after = charge_state_gas_result_2_850.tup1;
        state_gas_after = charge_state_gas_result_2_850.tup2;
        state_spill_after = charge_state_gas_result_2_850.tup3;
        if (charge_state_gas_result_2_850.tup0) {
          return ((struct tuple_bool_uint_64_uint_64_uint_32){.tup0 = false, .tup1 = gas_after, .tup2 = state_gas_after, .tup3 = state_spill_after});
        }
      }
      if (!tmp_3_1308) {
        struct tuple_bool_uint_64 charge_result_2_851;
        bool lteq_int_result_2_1971 = (bool)(G_amsterdam_account_write <= gas_after);
        if (lteq_int_result_2_1971) {
          uint64_t sub_atom_result_2_1972 = (gas_after - (uint64_t)G_amsterdam_account_write);
          charge_result_2_851 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = sub_atom_result_2_1972});
        } else {
          struct tuple_bool_uint_8 tmp_3_3313 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
          /* conversions */
          charge_result_2_851.tup0 = tmp_3_3313.tup0;
          charge_result_2_851.tup1 = (uint64_t)tmp_3_3313.tup1;
          /* end conversions */
        }
        gas_after = charge_result_2_851.tup1;
        if (charge_result_2_851.tup0) {
          return ((struct tuple_bool_uint_64_uint_64_uint_32){.tup0 = false, .tup1 = gas_after, .tup2 = state_gas_after, .tup3 = state_spill_after});
        }
      }
      bool delegation_set = authorization_tracker_delegation_set(au.authority);
      bool result_2_853 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C20__(au.address, ZERO_ADDRESS);
      if (result_2_853 && (!delegated_before_tx && !delegation_set)) {
        struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_result_2_855 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after, state_spill_after, G_amsterdam_state_auth_base);
        gas_after = charge_state_gas_result_2_855.tup1;
        state_gas_after = charge_state_gas_result_2_855.tup2;
        state_spill_after = charge_state_gas_result_2_855.tup3;
        if (charge_state_gas_result_2_855.tup0) {
          return ((struct tuple_bool_uint_64_uint_64_uint_32){.tup0 = false, .tup1 = gas_after, .tup2 = state_gas_after, .tup3 = state_spill_after});
        }
      }
      bool result_2_857 = eq_bytes20(au.address, ZERO_ADDRESS);
      if (result_2_857) {
        k_clear_code(au.authority);
      } else {
        k_set_delegation(au.authority, au.address);
      }
      k_bump_nonce(au.authority);
      bool result_2_860 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C20__(au.address, ZERO_ADDRESS);
      authorization_tracker_commit(au.authority, (bool)(!seen && currently_delegated), result_2_860);
    }
  }
  return ((struct tuple_bool_uint_64_uint_64_uint_32){.tup0 = true, .tup1 = gas_after, .tup2 = state_gas_after, .tup3 = state_spill_after});
}

struct tuple_bool_uint_64_uint_64_uint_32 process_amsterdam_auth_cursor(PreparedAuthorizationList authorizations, uint16_t count, bytes20 sender, bytes20 current_target, bool transfers_value, uint64_t gas, uint64_t state_gas, uint32_t state_spill)
{
  if (count == UINT8_C(0)) {
    return ((struct tuple_bool_uint_64_uint_64_uint_32){.tup0 = true, .tup1 = gas, .tup2 = state_gas, .tup3 = state_spill});
  }
  struct Authorization authorization = prepared_authorization_head_(authorizations);
  PreparedAuthorizationList remaining = prepared_authorization_tail_(authorizations, count);
  struct tuple_bool_uint_64_uint_64_uint_32 process_amsterdam_auth_result_2_841 = process_amsterdam_auth(authorization, sender, current_target, transfers_value, gas, state_gas, state_spill);
  if (process_amsterdam_auth_result_2_841.tup0) {
    return process_amsterdam_auth_cursor(remaining, ((uint16_t)((uint32_t)count - (uint32_t)UINT16_C(1))), sender, current_target, transfers_value, process_amsterdam_auth_result_2_841.tup1, process_amsterdam_auth_result_2_841.tup2, process_amsterdam_auth_result_2_841.tup3);
  }
  return ((struct tuple_bool_uint_64_uint_64_uint_32){.tup0 = false, .tup1 = process_amsterdam_auth_result_2_841.tup1, .tup2 = process_amsterdam_auth_result_2_841.tup2, .tup3 = process_amsterdam_auth_result_2_841.tup3});
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
  bool result_2_826 = (bool)(execution_profile.protocol.fork >= Shanghai);
  if (result_2_826) {
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
  struct tuple_u256_u256 result_2_771;
  u256 result_2_768 = k_header.base_fee;
  result_2_771 = eff_gas_price_for(result_2_768, tx.max_fee, tx.max_priority_fee);
  bytes20 sender = tx.sender;
  uint64_t nonce_before = k_get_nonce(sender);
  struct TransactionCosts costs;
  uint32_t result_2_814 = k_header.excess_blob_gas;
  costs = transaction_costs(profile, tx, gas_limit, result_2_814);
  uint64_t expected_nonce = word_of_account_nonce(nonce_before);
  if (!u256_eq_u64(tx.nonce, expected_nonce)) {
    fatal_error(ExecutionInvalid);
  }
  struct tuple_bool_bytes20 k_deleg_target_result_2_774 = k_deleg_target(sender);
  if (tx_semantics.blob) {
    bool result_2_780 = (bool)(profile.fork < Cancun);
    bool tmp_3_1265 = (bool)(result_2_780 || ((tx.blob_hashes.count == UINT8_C(0)) || tx.is_create));
    if (tmp_3_1265) {
      fatal_error(ExecutionInvalid);
    }
  }
  bool result_2_784 = (bool)(profile.fork >= Prague);
  bool tmp_3_1261 = (bool)(result_2_784 && (gas_limit < costs.calldata_floor));
  if (tmp_3_1261) {
    fatal_error(ExecutionInvalid);
  }
  u256 sender_balance = k_get_balance(sender);
  bool upfront_affordable = word_ule(costs.upfront, sender_balance);
  if (!upfront_affordable) {
    fatal_error(ExecutionInvalid);
  }
  bytes32 sender_code_key = k_code_key(sender);
  bool eq_anything_result_2_812 = eq_bytes32(sender_code_key, KECCAK_EMPTY);
  if (!eq_anything_result_2_812 && !k_deleg_target_result_2_774.tup0) {
    fatal_error(ExecutionInvalid);
  }
  if (allowance.regular < costs.calldata_floor) {
    fatal_error(ExecutionInvalid);
  }
  bool base_fee_affordable;
  u256 result_2_810 = k_header.base_fee;
  base_fee_affordable = word_ule(result_2_810, tx.max_fee);
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
  bool tmp_3_1277 = (bool)(tx_semantics.set_code && (tx.authorizations.count == UINT8_C(0)));
  if (tmp_3_1277) {
    fatal_error(ExecutionInvalid);
  }
  bool result_2_801 = eq_TxSignatureScheme(tx_semantics.signature, TypedSignature);
  bool tmp_3_1278 = (bool)(result_2_801 && (tx.chain_id != k_chain_id));
  if (tmp_3_1278) {
    fatal_error(ExecutionInvalid);
  }
  if (nonce_before == UINT64_C(18446744073709551615)) {
    fatal_error(ExecutionInvalid);
  }
  struct TransactionInitialGasFields initial_gas = transaction_initial_gas(allowance, costs.intrinsic_execution, costs.intrinsic_state, costs.calldata_floor);
  return tx_validity_fields(sender, nonce_before, initial_gas, costs.blob_fee, result_2_771.tup0, result_2_771.tup1);
}

struct TxUpfrontResult apply_transaction_upfront_effects(struct TransactionFields tx, struct TxValidityFields v, PreparedAuthorizationList authorizations)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  bool create_target_prestate_empty;
  bool result_2_764 = (bool)(profile.fork >= Amsterdam);
  if (result_2_764 && tx.is_create) {
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
  bool result_2_759 = (bool)(profile.fork < Amsterdam);
  if (result_2_759) {
    authorization_refund = process_auth_list(authorizations);
  } else {
    authorization_refund = UINT64_C(0);
  }
  return ((struct TxUpfrontResult){.authorization_refund = authorization_refund, .create_target_prestate_empty = create_target_prestate_empty});
}

struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_Bytes enter_transaction_frame(struct TxValidityFields v)
{
  struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_Bytes result_8_952;
  StackPointer stack = stack_reset();
  Bytes memory = memory_reset();
  struct tuple_uint_64_uint_64_uint_8_int_128_StackPointer_Bytes tmp_3_1250 = ((struct tuple_uint_64_uint_64_uint_8_int_128_StackPointer_Bytes){.tup0 = v.gas.execution_remaining, .tup1 = v.gas.state_remaining, .tup2 = STATE_GAS_SPILL_ZERO, .tup3 = GAS_REFUND_ZERO, .tup4 = stack, .tup5 = memory});
  /* conversions */
  result_8_952.tup0 = tmp_3_1250.tup0;
  result_8_952.tup1 = tmp_3_1250.tup1;
  result_8_952.tup2 = (uint32_t)tmp_3_1250.tup2;
  result_8_952.tup3 = tmp_3_1250.tup3;
  result_8_952.tup4 = tmp_3_1250.tup4;
  result_8_952.tup5 = tmp_3_1250.tup5;
  /* end conversions */
  return result_8_952;
}

struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes run_create_transaction_frame(struct TransactionFields tx, bytes20 sender, uint64_t nonce_before, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_stack, Bytes carried_memory, struct CodeFields carried_code, struct CalldataSlice carried_calldata, uint64_t state_gas_reservoir)
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
    struct tuple_uint_64_uint_32_FrameStatus result_2_715;
    struct ExecutionProfileFields execution_profile_8_205 = k_execution_profile;
    bool result_2_2039 = (bool)(execution_profile_8_205.protocol.fork >= Amsterdam);
    if (result_2_2039) {
      struct FrameStatus Exceptional_result_2_2040 = Exceptional(AddressCollision);
      struct tuple_uint_64_uint_8_FrameStatus tmp_3_3353 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_2_2040});
      /* conversions */
      result_2_715.tup0 = tmp_3_3353.tup0;
      result_2_715.tup1 = (uint32_t)tmp_3_3353.tup1;
      result_2_715.tup2 = tmp_3_3353.tup2;
      /* end conversions */
    } else {
      struct FrameStatus Exceptional_result_2_2041 = Exceptional(AddressCollision);
      result_2_715 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after, .tup1 = state_spill_after, .tup2 = Exceptional_result_2_2041});
    }
    state_gas_after = result_2_715.tup0;
    state_spill_after = result_2_715.tup1;
    status_after = result_2_715.tup2;
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
    bool result_2_718 = (bool)(execution_profile.protocol.fork < Amsterdam);
    if (result_2_718) {
      Bytes initcode = transaction_initcode_slice(tx.input_src);
      bytes32 code_id = code_db_insert(initcode, execution_profile.protocol.fork);
      frame_code = code_db_resolve(code_id);
      frame_calldata = EMPTY_CALLDATA;
    }
    struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes result_2_722 = interpret_uint64_t_uint64_t_uint32_t___int128_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint8_t_struct_CodeFields_struct_CalldataSlice_to_struct_tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes(gas_after, state_gas_after, state_spill_after, refund_after, carried_stack, carried_memory, sender, new_addr, new_addr, tx.value, state_gas_reservoir, false, UINT8_C(0), frame_code, frame_calldata);
    gas_after = result_2_722.tup0;
    state_gas_after = result_2_722.tup1;
    state_spill_after = result_2_722.tup2;
    refund_after = result_2_722.tup3;
    status_after = result_2_722.tup4;
    output_after = result_2_722.tup5;
    bool initcode_succeeded = frame_succeeded(status_after);
    if (initcode_succeeded) {
      uint32_t dep_len = output_after.len;
      bool valid_deployed_size = deployed_code_size_allowed(dep_len);
      bool valid_prefix;
      bool result_2_732 = (bool)(execution_profile.protocol.fork < London);
      if (result_2_732 || (dep_len == UINT8_C(0))) {
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
          struct tuple_bool_uint_64_uint_64_uint_32 charge_deployment_state_gas_result_2_725 = charge_deployment_state_gas(gas_after, state_gas_after, state_spill_after, state_deposit);
          gas_after = charge_deployment_state_gas_result_2_725.tup1;
          state_gas_after = charge_deployment_state_gas_result_2_725.tup2;
          state_spill_after = charge_deployment_state_gas_result_2_725.tup3;
          if (charge_deployment_state_gas_result_2_725.tup0) {
            gas_after = (uint64_t)GAS_ZERO;
            struct tuple_uint_64_uint_32_FrameStatus result_2_726;
            struct ExecutionProfileFields execution_profile_8_207 = k_execution_profile;
            bool result_8_209 = (bool)(execution_profile_8_207.protocol.fork >= Amsterdam);
            if (result_8_209) {
              struct FrameStatus Exceptional_result_8_211 = Exceptional(OutOfGas);
              struct tuple_uint_64_uint_8_FrameStatus tmp_8_212 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_211});
              /* conversions */
              result_2_726.tup0 = tmp_8_212.tup0;
              result_2_726.tup1 = (uint32_t)tmp_8_212.tup1;
              result_2_726.tup2 = tmp_8_212.tup2;
              /* end conversions */
            } else {
              struct FrameStatus Exceptional_result_8_213 = Exceptional(OutOfGas);
              result_2_726 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after, .tup1 = state_spill_after, .tup2 = Exceptional_result_8_213});
            }
            state_gas_after = result_2_726.tup0;
            state_spill_after = result_2_726.tup1;
            status_after = result_2_726.tup2;
          }
          bool deployment_succeeded = frame_succeeded(status_after);
          if (deployment_succeeded) {
            Bytes stored_code = code_db_intern_output(output_after);
            k_deploy_code(new_addr, stored_code);
          }
        } else {
          gas_after = (uint64_t)GAS_ZERO;
          struct tuple_uint_64_uint_32_FrameStatus result_2_729;
          struct ExecutionProfileFields execution_profile_8_215 = k_execution_profile;
          bool result_8_217 = (bool)(execution_profile_8_215.protocol.fork >= Amsterdam);
          if (result_8_217) {
            struct FrameStatus Exceptional_result_8_219 = Exceptional(OutOfGas);
            struct tuple_uint_64_uint_8_FrameStatus tmp_8_220 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_219});
            /* conversions */
            result_2_729.tup0 = tmp_8_220.tup0;
            result_2_729.tup1 = (uint32_t)tmp_8_220.tup1;
            result_2_729.tup2 = tmp_8_220.tup2;
            /* end conversions */
          } else {
            struct FrameStatus Exceptional_result_8_221 = Exceptional(OutOfGas);
            result_2_729 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after, .tup1 = state_spill_after, .tup2 = Exceptional_result_8_221});
          }
          state_gas_after = result_2_729.tup0;
          state_spill_after = result_2_729.tup1;
          status_after = result_2_729.tup2;
        }
      } else {
        gas_after = (uint64_t)GAS_ZERO;
        struct tuple_uint_64_uint_32_FrameStatus result_2_730;
        struct ExecutionProfileFields execution_profile_8_223 = k_execution_profile;
        bool result_8_225 = (bool)(execution_profile_8_223.protocol.fork >= Amsterdam);
        if (result_8_225) {
          struct FrameStatus Exceptional_result_8_227 = Exceptional(OutOfGas);
          struct tuple_uint_64_uint_8_FrameStatus tmp_8_228 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_227});
          /* conversions */
          result_2_730.tup0 = tmp_8_228.tup0;
          result_2_730.tup1 = (uint32_t)tmp_8_228.tup1;
          result_2_730.tup2 = tmp_8_228.tup2;
          /* end conversions */
        } else {
          struct FrameStatus Exceptional_result_8_229 = Exceptional(OutOfGas);
          result_2_730 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after, .tup1 = state_spill_after, .tup2 = Exceptional_result_8_229});
        }
        state_gas_after = result_2_730.tup0;
        state_spill_after = result_2_730.tup1;
        status_after = result_2_730.tup2;
      }
    }
  }
  return ((struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes){.tup0 = gas_after, .tup1 = state_gas_after, .tup2 = state_spill_after, .tup3 = refund_after, .tup4 = status_after, .tup5 = output_after});
}

struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes run_call_transaction_frame(struct TransactionFields tx, bytes20 sender, bool delegated, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_stack, Bytes carried_memory, bytes20 carried_code_address, struct CodeFields carried_code, struct CalldataSlice carried_calldata, uint64_t state_gas_reservoir)
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
  bool tmp_3_1118;
  if (delegated) {
    tmp_3_1118 = false;
  } else {
    tmp_3_1118 = neq_anything_EPrecompileId__(selected_precompile, NotPrecompile);
  }
  if (tmp_3_1118) {
    struct CalldataSlice precompile_input = InputCalldata(tx.input_src);
    struct GasCharge precompile_charge = precompile_gas(selected_precompile, precompile_input, gas_after);
    if (precompile_charge.affordable) {
      struct PrecompileResult result = run_precompile_slice(selected_precompile, precompile_input);
      if (result.success) {
        gas_after = gas_sub(gas_after, precompile_charge.cost);
        output_after = result.output;
        struct HaltKind result_2_700 = HaltReturn(result.output);
        status_after = Halted(result_2_700);
      } else {
        gas_after = (uint64_t)GAS_ZERO;
        struct tuple_uint_64_uint_32_FrameStatus result_2_701;
        struct ExecutionProfileFields execution_profile_8_231 = k_execution_profile;
        bool result_2_2039 = (bool)(execution_profile_8_231.protocol.fork >= Amsterdam);
        if (result_2_2039) {
          struct FrameStatus Exceptional_result_2_2040 = Exceptional(OutOfGas);
          struct tuple_uint_64_uint_8_FrameStatus tmp_3_3353 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_2_2040});
          /* conversions */
          result_2_701.tup0 = tmp_3_3353.tup0;
          result_2_701.tup1 = (uint32_t)tmp_3_3353.tup1;
          result_2_701.tup2 = tmp_3_3353.tup2;
          /* end conversions */
        } else {
          struct FrameStatus Exceptional_result_2_2041 = Exceptional(OutOfGas);
          result_2_701 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after, .tup1 = state_spill_after, .tup2 = Exceptional_result_2_2041});
        }
        state_gas_after = result_2_701.tup0;
        state_spill_after = result_2_701.tup1;
        status_after = result_2_701.tup2;
      }
    } else {
      gas_after = (uint64_t)GAS_ZERO;
      struct tuple_uint_64_uint_32_FrameStatus result_2_702;
      struct ExecutionProfileFields execution_profile_8_233 = k_execution_profile;
      bool result_8_235 = (bool)(execution_profile_8_233.protocol.fork >= Amsterdam);
      if (result_8_235) {
        struct FrameStatus Exceptional_result_8_237 = Exceptional(OutOfGas);
        struct tuple_uint_64_uint_8_FrameStatus tmp_8_238 = ((struct tuple_uint_64_uint_8_FrameStatus){.tup0 = state_gas_reservoir, .tup1 = STATE_GAS_SPILL_ZERO, .tup2 = Exceptional_result_8_237});
        /* conversions */
        result_2_702.tup0 = tmp_8_238.tup0;
        result_2_702.tup1 = (uint32_t)tmp_8_238.tup1;
        result_2_702.tup2 = tmp_8_238.tup2;
        /* end conversions */
      } else {
        struct FrameStatus Exceptional_result_8_239 = Exceptional(OutOfGas);
        result_2_702 = ((struct tuple_uint_64_uint_32_FrameStatus){.tup0 = state_gas_after, .tup1 = state_spill_after, .tup2 = Exceptional_result_8_239});
      }
      state_gas_after = result_2_702.tup0;
      state_spill_after = result_2_702.tup1;
      status_after = result_2_702.tup2;
    }
  } else {
    bool result_2_704 = (bool)(execution_profile.protocol.fork < Amsterdam);
    if (result_2_704) {
      frame_calldata = InputCalldata(tx.input_src);
      code_address = tx.recipient;
      struct tuple_bool_bytes20 result_2_707 = k_deleg_target(tx.recipient);
      bool tx_deleg = result_2_707.tup0;
      bytes20 tx_dtgt = result_2_707.tup1;
      if (tx_deleg) {
        k_account_mark_warm(tx_dtgt);
        k_aload_(tx_dtgt);
      }
      if (tx_deleg) {
        code_address = tx_dtgt;
      }
      frame_code = executable_code(tx.recipient, tx_deleg, tx_dtgt);
    }
    struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes result_2_712 = interpret_uint64_t_uint64_t_uint32_t___int128_StackPointer_Bytes_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint8_t_struct_CodeFields_struct_CalldataSlice_to_struct_tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes(gas_after, state_gas_after, state_spill_after, refund_after, carried_stack, carried_memory, sender, tx.recipient, code_address, tx.value, state_gas_reservoir, false, UINT8_C(0), frame_code, frame_calldata);
    gas_after = result_2_712.tup0;
    state_gas_after = result_2_712.tup1;
    state_spill_after = result_2_712.tup2;
    refund_after = result_2_712.tup3;
    status_after = result_2_712.tup4;
    output_after = result_2_712.tup5;
  }
  return ((struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes){.tup0 = gas_after, .tup1 = state_gas_after, .tup2 = state_spill_after, .tup3 = refund_after, .tup4 = status_after, .tup5 = output_after});
}

struct TxFrameResultFields run_legacy_transaction_frame(struct TransactionFields tx, struct TxValidityFields v)
{
  k_journal_checkpoint();
  struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_Bytes enter_transaction_frame_result_2_683 = enter_transaction_frame(v);
  uint64_t initial_state_gas = enter_transaction_frame_result_2_683.tup1;
  struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes result_2_689;
  if (tx.is_create) {
    result_2_689 = run_create_transaction_frame(tx, v.sender, v.nonce_before, enter_transaction_frame_result_2_683.tup0, initial_state_gas, enter_transaction_frame_result_2_683.tup2, enter_transaction_frame_result_2_683.tup3, enter_transaction_frame_result_2_683.tup4, enter_transaction_frame_result_2_683.tup5, EMPTY_CODE, EMPTY_CALLDATA, initial_state_gas);
  } else {
    result_2_689 = run_call_transaction_frame(tx, v.sender, false, enter_transaction_frame_result_2_683.tup0, initial_state_gas, enter_transaction_frame_result_2_683.tup2, enter_transaction_frame_result_2_683.tup3, enter_transaction_frame_result_2_683.tup4, enter_transaction_frame_result_2_683.tup5, tx.recipient, EMPTY_CODE, EMPTY_CALLDATA, initial_state_gas);
  }
  uint64_t state_gas_after = result_2_689.tup1;
  bool success = frame_succeeded(result_2_689.tup4);
  if (success) {
    k_journal_commit();
  } else {
    k_journal_revert();
  }
  __int128 state_delta = frame_state_gas_used(initial_state_gas, state_gas_after, result_2_689.tup2);
  struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_result_2_690 = tx_frame_gas_snapshot_struct_TransactionInitialGasFields_uint64_t_uint64_t___int128_to_struct_TxFrameGasSnapshotFields(v.gas, result_2_689.tup0, state_gas_after, state_delta);
  __int128 result_2_691 = success ? result_2_689.tup3 : GAS_REFUND_ZERO;
  return ((struct TxFrameResultFields){.gas = tx_frame_gas_snapshot_result_2_690, .refund = result_2_691, .success = success});
}

struct TxFrameResultFields run_amsterdam_transaction_frame(struct TransactionFields tx, struct TxValidityFields v, struct TxUpfrontResult upfront, PreparedAuthorizationList authorizations)
{
  struct tuple_uint_64_uint_64_uint_32_int_128_StackPointer_Bytes enter_transaction_frame_result_2_662 = enter_transaction_frame(v);
  uint64_t gas_after = enter_transaction_frame_result_2_662.tup0;
  uint64_t state_gas_after = enter_transaction_frame_result_2_662.tup1;
  uint32_t state_spill_after = enter_transaction_frame_result_2_662.tup2;
  __int128 refund_after = enter_transaction_frame_result_2_662.tup3;
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
  struct tuple_bool_uint_64_uint_64_uint_32 result_2_666 = process_amsterdam_auth_cursor(authorizations, authorizations.count, v.sender, current_target, transfers_value, gas_after, state_gas_after, state_spill_after);
  preparation_ready = result_2_666.tup0;
  gas_after = result_2_666.tup1;
  state_gas_after = result_2_666.tup2;
  state_spill_after = result_2_666.tup3;
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
    struct tuple_TransactionPreparation_uint_64_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice prepare_amsterdam_transaction_dispatch_result_2_667 = prepare_amsterdam_transaction_dispatch(tx, v, upfront, gas_after, state_gas_after, state_spill_after);
    struct TransactionPreparation preparation = prepare_amsterdam_transaction_dispatch_result_2_667.tup0;
    gas_after = prepare_amsterdam_transaction_dispatch_result_2_667.tup1;
    state_gas_after = prepare_amsterdam_transaction_dispatch_result_2_667.tup2;
    state_spill_after = prepare_amsterdam_transaction_dispatch_result_2_667.tup3;
    preparation_ready = preparation.ready;
    delegated = preparation.delegated;
    prepared_code_address = prepare_amsterdam_transaction_dispatch_result_2_667.tup5;
    prepared_code = prepare_amsterdam_transaction_dispatch_result_2_667.tup6;
    prepared_calldata = prepare_amsterdam_transaction_dispatch_result_2_667.tup7;
  }
  if (!preparation_ready) {
    k_journal_revert();
    state_gas_after = preparation_reservoir;
    state_spill_after = (uint32_t)STATE_GAS_SPILL_ZERO;
    struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_result_2_668 = tx_frame_gas_snapshot_struct_TransactionInitialGasFields_uint8_t_uint8_t___int128_to_struct_TxFrameGasSnapshotFields(initial_gas, GAS_ZERO, STATE_GAS_ZERO, FRAME_STATE_GAS_DELTA_ZERO);
    return ((struct TxFrameResultFields){.gas = tx_frame_gas_snapshot_result_2_668, .refund = GAS_REFUND_ZERO, .success = false});
  }
  k_journal_checkpoint();
  if (tx.is_create) {
    struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes result_2_673 = run_create_transaction_frame(tx, v.sender, v.nonce_before, gas_after, state_gas_after, state_spill_after, refund_after, enter_transaction_frame_result_2_662.tup4, enter_transaction_frame_result_2_662.tup5, prepared_code, prepared_calldata, execution_reservoir);
    gas_after = result_2_673.tup0;
    state_gas_after = result_2_673.tup1;
    state_spill_after = result_2_673.tup2;
    refund_after = result_2_673.tup3;
    status_after = result_2_673.tup4;
  } else {
    struct tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes result_2_675 = run_call_transaction_frame(tx, v.sender, delegated, gas_after, state_gas_after, state_spill_after, refund_after, enter_transaction_frame_result_2_662.tup4, enter_transaction_frame_result_2_662.tup5, prepared_code_address, prepared_code, prepared_calldata, execution_reservoir);
    gas_after = result_2_675.tup0;
    state_gas_after = result_2_675.tup1;
    state_spill_after = result_2_675.tup2;
    refund_after = result_2_675.tup3;
    status_after = result_2_675.tup4;
  }
  bool success = frame_succeeded(status_after);
  if (success) {
    k_journal_commit();
  } else {
    k_journal_revert();
  }
  k_journal_commit();
  __int128 frame_state_gas_used_result_2_678 = frame_state_gas_used(execution_reservoir, state_gas_after, state_spill_after);
  struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_result_2_676 = tx_frame_gas_snapshot(initial_gas, gas_after, state_gas_after, (authorization_state_gas + frame_state_gas_used_result_2_678));
  __int128 result_2_677 = success ? refund_after : GAS_REFUND_ZERO;
  return ((struct TxFrameResultFields){.gas = tx_frame_gas_snapshot_result_2_676, .refund = result_2_677, .success = success});
}

struct TxFrameResultFields run_transaction_frame(struct TransactionFields tx, struct TxValidityFields v, struct TxUpfrontResult upfront, PreparedAuthorizationList authorizations)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool result_2_661 = (bool)(execution_profile.protocol.fork >= Amsterdam);
  if (result_2_661) {
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
  bool result_2_656 = (bool)(profile.fork >= Prague);
  floor = result_2_656 ? gas_snapshot.calldata_floor : UINT64_C(0);
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
      struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_result_2_737 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32_variant_2(gas_after, state_gas_after, state_spill_after, G_amsterdam_state_new_account);
      gas_after = charge_state_gas_result_2_737.tup1;
      state_gas_after = charge_state_gas_result_2_737.tup2;
      state_spill_after = charge_state_gas_result_2_737.tup3;
      if (charge_state_gas_result_2_737.tup0) {
        return ((struct tuple_TransactionPreparation_uint_64_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice){.tup0 = ((struct TransactionPreparation){.delegated = false, .ready = false}), .tup1 = gas_after, .tup2 = state_gas_after, .tup3 = state_spill_after, .tup4 = current_target, .tup5 = current_target, .tup6 = EMPTY_CODE, .tup7 = EMPTY_CALLDATA});
      }
    }
    Bytes initcode = transaction_initcode_slice(tx.input_src);
    bytes32 code_id = code_db_insert(initcode, execution_profile.protocol.fork);
    struct CodeFields code = code_db_resolve(code_id);
    return ((struct tuple_TransactionPreparation_uint_64_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice){.tup0 = ((struct TransactionPreparation){.delegated = false, .ready = true}), .tup1 = gas_after, .tup2 = state_gas_after, .tup3 = state_spill_after, .tup4 = current_target, .tup5 = current_target, .tup6 = code, .tup7 = EMPTY_CALLDATA});
  }
  struct CalldataSlice calldata = InputCalldata(tx.input_src);
  bool transfers_value = word_nonzero(tx.value);
  bool recipient_empty = k_account_is_empty(tx.recipient);
  if (transfers_value && recipient_empty) {
    struct tuple_bool_uint_64_uint_64_uint_32 charge_state_gas_result_2_741 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32_variant_2(gas_after, state_gas_after, state_spill_after, G_amsterdam_state_new_account);
    gas_after = charge_state_gas_result_2_741.tup1;
    state_gas_after = charge_state_gas_result_2_741.tup2;
    state_spill_after = charge_state_gas_result_2_741.tup3;
    if (charge_state_gas_result_2_741.tup0) {
      return ((struct tuple_TransactionPreparation_uint_64_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice){.tup0 = ((struct TransactionPreparation){.delegated = false, .ready = false}), .tup1 = gas_after, .tup2 = state_gas_after, .tup3 = state_spill_after, .tup4 = current_target, .tup5 = current_target, .tup6 = EMPTY_CODE, .tup7 = calldata});
    }
  }
  struct tuple_bool_bytes20 result_2_743 = k_deleg_target(tx.recipient);
  bool delegated = result_2_743.tup0;
  bytes20 delegate = result_2_743.tup1;
  if (delegated) {
    bool warm = k_account_is_warm(delegate);
    uint16_t access_cost = account_cost(warm);
    struct tuple_bool_uint_64 charge_result_2_744;
    bool lteq_int_result_2_1971 = (bool)(access_cost <= gas_after);
    if (lteq_int_result_2_1971) {
      charge_result_2_744 = ((struct tuple_bool_uint_64){.tup0 = false, .tup1 = (gas_after - (uint64_t)access_cost)});
    } else {
      struct tuple_bool_uint_8 tmp_3_3313 = ((struct tuple_bool_uint_8){.tup0 = true, .tup1 = GAS_ZERO});
      /* conversions */
      charge_result_2_744.tup0 = tmp_3_3313.tup0;
      charge_result_2_744.tup1 = (uint64_t)tmp_3_3313.tup1;
      /* end conversions */
    }
    gas_after = charge_result_2_744.tup1;
    if (charge_result_2_744.tup0) {
      return ((struct tuple_TransactionPreparation_uint_64_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice){.tup0 = ((struct TransactionPreparation){.delegated = false, .ready = false}), .tup1 = gas_after, .tup2 = state_gas_after, .tup3 = state_spill_after, .tup4 = current_target, .tup5 = current_target, .tup6 = EMPTY_CODE, .tup7 = calldata});
    }
    k_account_mark_warm(delegate);
  }
  bytes20 code_address = delegated ? delegate : current_target;
  struct CodeFields code_3_1249 = executable_code(tx.recipient, delegated, delegate);
  return ((struct tuple_TransactionPreparation_uint_64_uint_64_uint_32_bytes20_bytes20_CodeFields_CalldataSlice){.tup0 = ((struct TransactionPreparation){.delegated = delegated, .ready = true}), .tup1 = gas_after, .tup2 = state_gas_after, .tup3 = state_spill_after, .tup4 = current_target, .tup5 = code_address, .tup6 = code_3_1249, .tup7 = calldata});
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
  bool tmp_3_1344 = (bool)((allowance.regular < intrinsic_execution) || (allowance.regular < calldata_floor));
  if (tmp_3_1344) {
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
  bool word_is_zero_result_2_928 = eq_u256(value, WORD_ZERO);
  bool tmp_3_1351 = (bool)(word_is_zero_result_2_928 || u256_eq_u64(WORD_ZERO, factor));
  if (tmp_3_1351) {
    return WORD_ZERO;
  }
  bool result_2_931;
  u256 word_div_word_result_2_930 = word_div_word_u256_uint32_t_to_u256(WORD_ALL_ONES, factor);
  result_2_931 = word_ule(value, word_div_word_result_2_930);
  if (result_2_931) {
    return word_mul_word_u256_uint32_t_to_u256(value, factor);
  }
  fatal_error(ExecutionInvalid);
}

u256 validated_word_product_u256_uint32_t_to_u256_variant_2(u256 value, uint32_t factor)
{
  bool word_is_zero_result_2_928 = eq_u256(value, WORD_ZERO);
  bool tmp_3_1351 = (bool)(word_is_zero_result_2_928 || u256_eq_u64(WORD_ZERO, factor));
  if (tmp_3_1351) {
    return WORD_ZERO;
  }
  bool result_2_931;
  u256 word_div_word_result_2_930 = word_div_word_u256_uint32_t_to_u256_variant_2(WORD_ALL_ONES, factor);
  result_2_931 = word_ule(value, word_div_word_result_2_930);
  if (result_2_931) {
    return word_mul_word_u256_uint32_t_to_u256(value, factor);
  }
  fatal_error(ExecutionInvalid);
}

u256 validated_word_product_u256_uint64_t_to_u256(u256 value, uint64_t factor)
{
  bool word_is_zero_result_2_928 = eq_u256(value, WORD_ZERO);
  bool tmp_3_1351 = (bool)(word_is_zero_result_2_928 || u256_eq_u64(WORD_ZERO, factor));
  if (tmp_3_1351) {
    return WORD_ZERO;
  }
  bool result_2_931;
  u256 word_div_word_result_2_930 = word_div_word_u256_uint64_t_to_u256(WORD_ALL_ONES, factor);
  result_2_931 = word_ule(value, word_div_word_result_2_930);
  if (result_2_931) {
    return word_mul_word_u256_uint64_t_to_u256(value, factor);
  }
  fatal_error(ExecutionInvalid);
}

