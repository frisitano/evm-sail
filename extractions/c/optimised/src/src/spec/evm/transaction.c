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
  uint64_t result_8_502;
  uint32_t nonzeroes = slice_count_nonzero(input);
  if (nonzeroes <= input.len) {
    int64_t integer_result_3_3678 = ((int64_t)G_txdatazero * ((int64_t)input.len - (int64_t)nonzeroes));
    uint64_t mult_atom_result_2_957 = ((uint64_t)G_txdatanonzero * (uint64_t)nonzeroes);
    result_8_502 = (uint64_t)((int64_t)(uint64_t)integer_result_3_3678 + (int64_t)mult_atom_result_2_957);
  } else {
    fatal_error(ExecutionInvalid);
  }
  return result_8_502;
}

uint64_t legacy_intrinsic_gas(struct TransactionFields tx)
{
  uint64_t result_8_503;
  uint64_t data_cost = calldata_cost(tx.input_src);
  uint64_t address_cost = ((uint64_t)G_access_list_address * (uint64_t)tx.access_list.address_count);
  uint64_t slot_cost = ((uint64_t)G_access_list_storage_key * (uint64_t)tx.access_list.slot_count);
  uint64_t auth_cost = ((uint64_t)PER_EMPTY_ACCOUNT * (uint64_t)tx.authorizations.count);
  __int128 integer_result_3_3680 = ((__int128)data_cost + (__int128)G_transaction);
  if (tx.is_create) {
    __int128 integer_result_3_3684 = ((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)integer_result_3_3680 + (__int128)address_cost) + (__int128)slot_cost) + (__int128)auth_cost) + (__int128)G_txcreate);
    uint32_t transaction_initcode_gas_result_2_945 = transaction_initcode_gas(tx.input_src.len);
    result_8_503 = (uint64_t)((__int128)(uint64_t)integer_result_3_3684 + (__int128)transaction_initcode_gas_result_2_945);
  } else {
    result_8_503 = (uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)integer_result_3_3680 + (__int128)address_cost) + (__int128)slot_cost) + (__int128)auth_cost);
  }
  return result_8_503;
}

uint64_t legacy_calldata_floor(Bytes input)
{
  uint64_t result_8_504;
  uint32_t nonzeroes = slice_count_nonzero(input);
  if (nonzeroes <= input.len) {
    int64_t integer_result_3_3688 = ((int64_t)(uint64_t)((int64_t)(uint64_t)((int64_t)UINT8_C(10) * ((int64_t)input.len - (int64_t)nonzeroes)) + (int64_t)(UINT64_C(40) * (uint64_t)nonzeroes)) + (int64_t)G_transaction);
    result_8_504 = (uint64_t)integer_result_3_3688;
  } else {
    fatal_error(ExecutionInvalid);
  }
  return result_8_504;
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
  bool result_2_936 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C20__(tx.recipient, tx.sender);
  if (result_2_936) {
    if (transfers_value) {
      uint16_t add_atom_result_2_937 = ((uint16_t)((uint32_t)AMSTERDAM_TX_VALUE_COST + (uint32_t)AMSTERDAM_COLD_ACCOUNT_ACCESS));
      return ((uint16_t)((uint32_t)AMSTERDAM_TRANSFER_LOG_COST + (uint32_t)add_atom_result_2_937));
    }
    return AMSTERDAM_COLD_ACCOUNT_ACCESS;
  }
  return UINT16_C(0);
}

struct IntrinsicGasCost intrinsic_gas(struct TransactionFields tx)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool result_2_906 = (bool)(execution_profile.protocol.fork < Amsterdam);
  if (result_2_906) {
    uint64_t legacy_intrinsic_gas_result_2_907 = legacy_intrinsic_gas(tx);
    uint64_t result_2_909 = legacy_calldata_floor(tx.input_src);
    return ((struct IntrinsicGasCost){.calldata_floor = result_2_909, .execution = legacy_intrinsic_gas_result_2_907, .state = UINT64_C(0)});
  }
  Bytes input = tx.input_src;
  uint16_t recipient = amsterdam_recipient_execution_cost(tx);
  uint32_t address_count = tx.access_list.address_count;
  uint32_t slot_count = tx.access_list.slot_count;
  uint64_t mult_atom_result_2_925 = ((uint64_t)AMSTERDAM_ACCESS_LIST_ADDRESS * (uint64_t)address_count);
  uint64_t mult_atom_result_2_926 = ((uint64_t)AMSTERDAM_ACCESS_LIST_SLOT * (uint64_t)slot_count);
  uint64_t mult_atom_result_2_928 = ((uint64_t)AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR * (uint64_t)address_count);
  uint64_t mult_atom_result_2_930 = ((uint64_t)AMSTERDAM_ACCESS_LIST_SLOT_FLOOR * (uint64_t)slot_count);
  uint64_t authorization_execution = ((uint64_t)AMSTERDAM_AUTH_BASE * (uint64_t)tx.authorizations.count);
  uint32_t create_execution;
  if (tx.is_create) {
    create_execution = transaction_initcode_gas(input.len);
  } else {
    create_execution = UINT32_C(0);
  }
  uint64_t result_2_917 = calldata_cost(tx.input_src);
  __int128 integer_result_3_3689 = ((__int128)result_2_917 + (__int128)AMSTERDAM_TX_BASE);
  uint64_t result_2_911;
  uint64_t mult_atom_result_2_910 = ((uint64_t)AMSTERDAM_CALLDATA_FLOOR_BYTE * (uint64_t)input.len);
  result_2_911 = (mult_atom_result_2_910 + (uint64_t)AMSTERDAM_TX_BASE);
  uint64_t mult_atom_result_2_913 = ((uint64_t)AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR * (uint64_t)address_count);
  uint64_t mult_atom_result_2_915 = ((uint64_t)AMSTERDAM_ACCESS_LIST_SLOT_FLOOR * (uint64_t)slot_count);
  return ((struct IntrinsicGasCost){.calldata_floor = (mult_atom_result_2_915 + (mult_atom_result_2_913 + (result_2_911 + (uint64_t)recipient))), .execution = (uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)integer_result_3_3689 + (__int128)recipient) + ((__int128)mult_atom_result_2_930 + ((__int128)mult_atom_result_2_928 + ((__int128)mult_atom_result_2_926 + (__int128)mult_atom_result_2_925)))) + (__int128)authorization_execution) + (__int128)create_execution), .state = UINT64_C(0)});
}

struct TransactionCosts transaction_costs(struct ProtocolProfileFields profile, struct TransactionFields tx, uint64_t gas_limit, uint32_t excess_blob_gas)
{
  struct IntrinsicGasCost intrinsic = intrinsic_gas(tx);
  uint32_t blob_gas = (UINT32_C(131072) * (uint32_t)tx.blob_hashes.count);
  u256 blob_fee;
  if (blob_gas == UINT8_C(0)) {
    blob_fee = WORD_ZERO;
  } else {
    u256 blob_price = blob_base_fee(profile, excess_blob_gas);
    bool result_2_898 = word_ule(blob_price, tx.max_blob_fee);
    if (result_2_898) {
      blob_fee = validated_word_product_u256_uint32_t_to_u256_variant_2(blob_price, blob_gas);
    } else {
      fatal_error(ExecutionInvalid);
    }
  }
  u256 execution_cap = validated_word_product_u256_uint64_t_to_u256(tx.max_fee, gas_limit);
  u256 blob_cap = validated_word_product_u256_uint32_t_to_u256(tx.max_blob_fee, blob_gas);
  u256 execution_and_value;
  bool result_2_892;
  u256 word_sub_word_result_2_891 = word_sub_word(WORD_ALL_ONES, execution_cap);
  result_2_892 = word_ule(tx.value, word_sub_word_result_2_891);
  if (result_2_892) {
    execution_and_value = word_add_word(execution_cap, tx.value);
  } else {
    fatal_error(ExecutionInvalid);
  }
  u256 upfront;
  bool result_2_889;
  u256 word_sub_word_result_2_888 = word_sub_word(WORD_ALL_ONES, execution_and_value);
  result_2_889 = word_ule(blob_cap, word_sub_word_result_2_888);
  if (result_2_889) {
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
  bool chain_id_is_zero = word_is_zero(au.chain_id);
  uint64_t expected_chain_id = word_of_chain_identifier(k_chain_id);
  if (au.valid_sig && (chain_id_is_zero || u256_eq_u64(au.chain_id, expected_chain_id))) {
    k_account_mark_warm(au.authority);
    struct tuple_bool_bytes20 k_deleg_target_result_2_847 = k_deleg_target(au.authority);
    bytes32 code_key = k_code_key(au.authority);
    uint64_t nonce = k_get_nonce(au.authority);
    bool eq_anything_result_2_848 = eq_bytes32(code_key, KECCAK_EMPTY);
    bool tmp_3_1237 = (bool)((eq_anything_result_2_848 || k_deleg_target_result_2_847.tup0) && (nonce == au.nonce));
    if (tmp_3_1237) {
      bool existed = k_account_exists(au.authority);
      bool result_2_853 = eq_bytes20(au.address, ZERO_ADDRESS);
      if (result_2_853) {
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

bool process_amsterdam_auth(struct Authorization au, bytes20 sender, bytes20 current_target, bool transfers_value)
{
  bool chain_id_is_zero = word_is_zero(au.chain_id);
  uint64_t expected_chain_id = word_of_chain_identifier(k_chain_id);
  if (au.valid_sig && (chain_id_is_zero || u256_eq_u64(au.chain_id, expected_chain_id))) {
    k_account_mark_warm(au.authority);
    struct tuple_bool_bytes20 k_deleg_target_result_2_821 = k_deleg_target(au.authority);
    bool currently_delegated = k_deleg_target_result_2_821.tup0;
    bytes32 code_key = k_code_key(au.authority);
    uint64_t nonce = k_get_nonce(au.authority);
    bool eq_anything_result_2_822 = eq_bytes32(code_key, KECCAK_EMPTY);
    bool tmp_3_1209 = (bool)((eq_anything_result_2_822 || currently_delegated) && (nonce == au.nonce));
    if (tmp_3_1209) {
      bool seen = authorization_tracker_seen(au.authority);
      bool delegated_before_tx;
      if (seen) {
        delegated_before_tx = authorization_tracker_originally_delegated(au.authority);
      } else {
        delegated_before_tx = currently_delegated;
      }
      bool tmp_3_1213 = (bool)(seen || (eq_bytes20(au.authority, sender) || (transfers_value && eq_bytes20(au.authority, current_target))));
      bool account_exists = k_account_exists(au.authority);
      if (!account_exists) {
        struct tuple_bool_uint_64 charge_state_gas_result_2_826 = charge_state_gas(gas_remaining, G_amsterdam_state_new_account);
        gas_remaining = charge_state_gas_result_2_826.tup1;
        if (!charge_state_gas_result_2_826.tup0) {
          return false;
        }
      }
      if (!tmp_3_1213) {
        struct tuple_bool_uint_64 charge_result_2_827 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_remaining, G_amsterdam_account_write);
        gas_remaining = charge_result_2_827.tup1;
        if (!charge_result_2_827.tup0) {
          return false;
        }
      }
      bool delegation_set = authorization_tracker_delegation_set(au.authority);
      bool result_2_829 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C20__(au.address, ZERO_ADDRESS);
      if (result_2_829 && (!delegated_before_tx && !delegation_set)) {
        struct tuple_bool_uint_64 charge_state_gas_result_2_831 = charge_state_gas(gas_remaining, G_amsterdam_state_auth_base);
        gas_remaining = charge_state_gas_result_2_831.tup1;
        if (!charge_state_gas_result_2_831.tup0) {
          return false;
        }
      }
      bool result_2_833 = eq_bytes20(au.address, ZERO_ADDRESS);
      if (result_2_833) {
        k_clear_code(au.authority);
      } else {
        k_set_delegation(au.authority, au.address);
      }
      k_bump_nonce(au.authority);
      bool result_2_836 = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C20__(au.address, ZERO_ADDRESS);
      authorization_tracker_commit(au.authority, (bool)(!seen && currently_delegated), result_2_836);
    }
  }
  return true;
}

bool process_amsterdam_auth_cursor(PreparedAuthorizationList authorizations, uint16_t count, bytes20 sender, bytes20 current_target, bool transfers_value)
{
  if (count == UINT8_C(0)) {
    return true;
  }
  struct Authorization authorization = prepared_authorization_head_(authorizations);
  PreparedAuthorizationList remaining = prepared_authorization_tail_(authorizations, count);
  bool processed = process_amsterdam_auth(authorization, sender, current_target, transfers_value);
  if (processed) {
    return process_amsterdam_auth_cursor(remaining, ((uint16_t)((uint32_t)count - (uint32_t)UINT16_C(1))), sender, current_target, transfers_value);
  }
  return false;
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
  bool result_2_803 = (bool)(execution_profile.protocol.fork >= Shanghai);
  if (result_2_803) {
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
    u256 available_priority = word_sub_word(max_fee, base_fee);
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
    priority = word_sub_word(price, base_fee);
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
  struct tuple_u256_u256 result_2_748;
  u256 result_2_745 = k_header.base_fee;
  result_2_748 = eff_gas_price_for(result_2_745, tx.max_fee, tx.max_priority_fee);
  bytes20 sender = tx.sender;
  uint64_t nonce_before = k_get_nonce(sender);
  struct TransactionCosts costs;
  uint32_t result_2_791 = k_header.excess_blob_gas;
  costs = transaction_costs(profile, tx, gas_limit, result_2_791);
  uint64_t expected_nonce = word_of_account_nonce(nonce_before);
  if (!u256_eq_u64(tx.nonce, expected_nonce)) {
    fatal_error(ExecutionInvalid);
  }
  struct tuple_bool_bytes20 k_deleg_target_result_2_751 = k_deleg_target(sender);
  if (tx_semantics.blob) {
    bool result_2_757 = (bool)(profile.fork < Cancun);
    bool tmp_3_1174 = (bool)(result_2_757 || ((tx.blob_hashes.count == UINT8_C(0)) || tx.is_create));
    if (tmp_3_1174) {
      fatal_error(ExecutionInvalid);
    }
  }
  bool result_2_761 = (bool)(profile.fork >= Prague);
  bool tmp_3_1170 = (bool)(result_2_761 && (gas_limit < costs.calldata_floor));
  if (tmp_3_1170) {
    fatal_error(ExecutionInvalid);
  }
  u256 sender_balance = k_get_balance(sender);
  bool upfront_affordable = word_ule(costs.upfront, sender_balance);
  if (!upfront_affordable) {
    fatal_error(ExecutionInvalid);
  }
  bytes32 sender_code_key = k_code_key(sender);
  bool eq_anything_result_2_789 = eq_bytes32(sender_code_key, KECCAK_EMPTY);
  if (!eq_anything_result_2_789 && !k_deleg_target_result_2_751.tup0) {
    fatal_error(ExecutionInvalid);
  }
  if (allowance.regular < costs.calldata_floor) {
    fatal_error(ExecutionInvalid);
  }
  bool base_fee_affordable;
  u256 result_2_787 = k_header.base_fee;
  base_fee_affordable = word_ule(result_2_787, tx.max_fee);
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
  bool tmp_3_1186 = (bool)(tx_semantics.set_code && (tx.authorizations.count == UINT8_C(0)));
  if (tmp_3_1186) {
    fatal_error(ExecutionInvalid);
  }
  bool result_2_778 = eq_TxSignatureScheme(tx_semantics.signature, TypedSignature);
  bool tmp_3_1187 = (bool)(result_2_778 && (tx.chain_id != k_chain_id));
  if (tmp_3_1187) {
    fatal_error(ExecutionInvalid);
  }
  if (nonce_before == UINT64_C(18446744073709551615)) {
    fatal_error(ExecutionInvalid);
  }
  struct TransactionInitialGasFields initial_gas = transaction_initial_gas(allowance, costs.intrinsic_execution, costs.intrinsic_state, costs.calldata_floor);
  return tx_validity_fields(sender, nonce_before, initial_gas, costs.blob_fee, result_2_748.tup0, result_2_748.tup1);
}

struct TxUpfrontResult apply_transaction_upfront_effects(struct TransactionFields tx, struct TxValidityFields v, PreparedAuthorizationList authorizations)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  bool create_target_prestate_empty;
  bool result_2_741 = (bool)(profile.fork >= Amsterdam);
  if (result_2_741 && tx.is_create) {
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
  bool result_2_736 = (bool)(profile.fork < Amsterdam);
  if (result_2_736) {
    authorization_refund = process_auth_list(authorizations);
  } else {
    authorization_refund = UINT64_C(0);
  }
  return ((struct TxUpfrontResult){.authorization_refund = authorization_refund, .create_target_prestate_empty = create_target_prestate_empty});
}

void enter_transaction_frame(struct TxValidityFields v)
{
  struct TransactionInitialGasFields initial_gas = v.gas;
  pc = UINT32_C(0);
  call_depth = UINT16_C(0);
  gas_remaining = initial_gas.execution_remaining;
  state_gas_remaining = initial_gas.state_remaining;
  state_gas_spilled = STATE_GAS_SPILL_ZERO;
  message = ((struct Message){.address = ZERO_ADDRESS, .caller = ZERO_ADDRESS, .code_address = ZERO_ADDRESS, .depth = UINT16_C(0), .is_static = false, .state_gas_reservoir = state_gas_remaining, .value = ZERO_WORD});
  stack_top = stack_reset();
  memory_reset();
  returndata_clear();
  calldata = EMPTY_CALLDATA;
  frame_code = EMPTY_CODE;
  frame_refund = GAS_REFUND_ZERO;
  frame_status = Running(UNIT);
}

struct TransactionPreparation prepare_amsterdam_transaction_dispatch(struct TransactionFields tx, struct TxValidityFields v, struct TxUpfrontResult upfront)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bytes20 current_target;
  if (tx.is_create) {
    current_target = k_create_addr(v.sender, v.nonce_before);
  } else {
    current_target = tx.recipient;
  }
  message = ((struct Message){.address = current_target, .caller = v.sender, .code_address = current_target, .depth = UINT16_C(0), .is_static = false, .state_gas_reservoir = state_gas_remaining, .value = tx.value});
  if (tx.is_create) {
    if (upfront.create_target_prestate_empty) {
      struct tuple_bool_uint_64 charge_state_gas_result_2_716 = charge_state_gas(gas_remaining, G_amsterdam_state_new_account);
      gas_remaining = charge_state_gas_result_2_716.tup1;
      if (!charge_state_gas_result_2_716.tup0) {
        return ((struct TransactionPreparation){.delegated = false, .ready = false});
      }
    }
    Bytes initcode = transaction_initcode_slice(tx.input_src);
    bytes32 code_id = code_db_insert(initcode, execution_profile.protocol.fork);
    frame_code = code_db_resolve(code_id);
    return ((struct TransactionPreparation){.delegated = false, .ready = true});
  }
  calldata = InputCalldata(tx.input_src);
  bool transfers_value = word_nonzero(tx.value);
  bool recipient_empty = k_account_is_empty(tx.recipient);
  if (transfers_value && recipient_empty) {
    struct tuple_bool_uint_64 charge_state_gas_result_2_721 = charge_state_gas(gas_remaining, G_amsterdam_state_new_account);
    gas_remaining = charge_state_gas_result_2_721.tup1;
    if (!charge_state_gas_result_2_721.tup0) {
      return ((struct TransactionPreparation){.delegated = false, .ready = false});
    }
  }
  struct tuple_bool_bytes20 result_2_723 = k_deleg_target(tx.recipient);
  bool delegated = result_2_723.tup0;
  bytes20 delegate = result_2_723.tup1;
  if (delegated) {
    bool warm = k_account_is_warm(delegate);
    uint16_t access_cost = account_cost(warm);
    struct tuple_bool_uint_64 charge_result_2_724 = charge_uint64_t_uint16_t_to_struct_tuple_bool_uint_64(gas_remaining, access_cost);
    gas_remaining = charge_result_2_724.tup1;
    if (!charge_result_2_724.tup0) {
      return ((struct TransactionPreparation){.delegated = false, .ready = false});
    }
    k_account_mark_warm(delegate);
  }
  if (delegated) {
    message.code_address = delegate;
  }
  frame_code = executable_code(tx.recipient, delegated, delegate);
  return ((struct TransactionPreparation){.delegated = delegated, .ready = true});
}

void run_create_transaction_frame(struct TransactionFields tx, bytes20 sender, uint64_t nonce_before)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  bytes20 new_addr = k_create_addr(sender, nonce_before);
  k_account_mark_warm(new_addr);
  bool occupied = k_account_occupied(new_addr);
  if (occupied) {
    gas_remaining = exc_halt(gas_remaining, AddressCollision);
    return;
  }
  k_mark_created(new_addr);
  k_clear_storage(new_addr);
  k_bump_nonce(new_addr);
  bool transfers_value = word_nonzero(tx.value);
  if (transfers_value) {
    k_transfer(sender, new_addr, tx.value);
  }
  bool result_2_699 = (bool)(profile.fork < Amsterdam);
  if (result_2_699) {
    message = ((struct Message){.address = new_addr, .caller = sender, .code_address = new_addr, .depth = UINT16_C(0), .is_static = false, .state_gas_reservoir = state_gas_remaining, .value = tx.value});
    Bytes initcode = transaction_initcode_slice(tx.input_src);
    bytes32 code_id = code_db_insert(initcode, profile.fork);
    frame_code = code_db_resolve(code_id);
  }
  Bytes deployed_code = interpret_();
  bool initcode_succeeded = frame_succeeded();
  if (initcode_succeeded) {
    uint32_t dep_len = deployed_code.len;
    bool valid_deployed_size = deployed_code_size_allowed(dep_len);
    bool valid_prefix;
    bool result_2_709 = (bool)(profile.fork < London);
    if (result_2_709 || (dep_len == UINT8_C(0))) {
      valid_prefix = true;
    } else {
      uint64_t first_byte = output_byte(deployed_code, UINT8_C(0));
      valid_prefix = (bool)(first_byte != UINT64_C(0xEF));
    }
    if (valid_deployed_size && valid_prefix) {
      struct GasCharge deployment_charge = code_deployment_execution_cost(dep_len, gas_remaining);
      if (deployment_charge.affordable) {
        gas_remaining = gas_sub(gas_remaining, deployment_charge.cost);
        uint64_t state_deposit = code_deployment_state_cost(dep_len);
        struct tuple_bool_uint_64 charge_deployment_state_gas_result_2_705 = charge_deployment_state_gas(gas_remaining, state_deposit);
        gas_remaining = charge_deployment_state_gas_result_2_705.tup1;
        bool deployment_succeeded = frame_succeeded();
        if (deployment_succeeded) {
          Bytes stored_code = code_db_intern_output(deployed_code);
          k_deploy_code(new_addr, stored_code);
        }
        return;
      }
      gas_remaining = exc_halt(gas_remaining, OutOfGas);
      return;
    }
    gas_remaining = exc_halt(gas_remaining, OutOfGas);
    return;
  }
}

void run_call_transaction_frame(struct TransactionFields tx, bytes20 sender, bool delegated)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  k_aload_(tx.recipient);
  bool transfers_value = word_nonzero(tx.value);
  if (transfers_value) {
    k_transfer(sender, tx.recipient, tx.value);
  }
  enum PrecompileId selected_precompile = precompile_id_for_address(tx.recipient);
  bool tmp_3_1088;
  if (delegated) {
    tmp_3_1088 = false;
  } else {
    tmp_3_1088 = neq_anything_EPrecompileId__(selected_precompile, NotPrecompile);
  }
  if (tmp_3_1088) {
    struct CalldataSlice precompile_input = InputCalldata(tx.input_src);
    struct GasCharge precompile_charge = precompile_gas(selected_precompile, precompile_input, gas_remaining);
    if (precompile_charge.affordable) {
      struct PrecompileResult result = run_precompile_slice(selected_precompile, precompile_input);
      if (result.success) {
        gas_remaining = gas_sub(gas_remaining, precompile_charge.cost);
        struct HaltKind halt = HaltReturn(result.output);
        frame_status = Halted(halt);
      } else {
        gas_remaining = exc_halt(gas_remaining, OutOfGas);
      }
    } else {
      gas_remaining = exc_halt(gas_remaining, OutOfGas);
    }
  } else {
    bool result_2_685 = (bool)(execution_profile.protocol.fork < Amsterdam);
    if (result_2_685) {
      calldata = InputCalldata(tx.input_src);
      message = ((struct Message){.address = tx.recipient, .caller = sender, .code_address = tx.recipient, .depth = UINT16_C(0), .is_static = false, .state_gas_reservoir = state_gas_remaining, .value = tx.value});
      struct tuple_bool_bytes20 result_2_691 = k_deleg_target(tx.recipient);
      bool tx_deleg = result_2_691.tup0;
      bytes20 tx_dtgt = result_2_691.tup1;
      if (tx_deleg) {
        k_account_mark_warm(tx_dtgt);
        k_aload_(tx_dtgt);
      }
      frame_code = executable_code(tx.recipient, tx_deleg, tx_dtgt);
    }
    interpret_();
  }
}

struct TxFrameResultFields run_legacy_transaction_frame(struct TransactionFields tx, struct TxValidityFields v)
{
  k_journal_checkpoint();
  enter_transaction_frame(v);
  if (tx.is_create) {
    run_create_transaction_frame(tx, v.sender, v.nonce_before);
  } else {
    run_call_transaction_frame(tx, v.sender, false);
  }
  bool success = frame_succeeded();
  if (success) {
    k_journal_commit();
  } else {
    k_journal_revert();
  }
  __int128 state_delta = frame_state_gas_used();
  struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_result_2_674 = tx_frame_gas_snapshot_struct_TransactionInitialGasFields_uint64_t_uint64_t___int128_to_struct_TxFrameGasSnapshotFields(v.gas, gas_remaining, state_gas_remaining, state_delta);
  __int128 result_2_675 = success ? frame_refund : GAS_REFUND_ZERO;
  return ((struct TxFrameResultFields){.gas = tx_frame_gas_snapshot_result_2_674, .refund = result_2_675, .success = success});
}

struct TxFrameResultFields run_amsterdam_transaction_frame(struct TransactionFields tx, struct TxValidityFields v, struct TxUpfrontResult upfront, PreparedAuthorizationList authorizations)
{
  enter_transaction_frame(v);
  struct TransactionInitialGasFields initial_gas = v.gas;
  k_journal_checkpoint();
  uint64_t preparation_reservoir = state_gas_remaining;
  bytes20 current_target;
  if (tx.is_create) {
    current_target = k_create_addr(v.sender, v.nonce_before);
  } else {
    current_target = tx.recipient;
  }
  authorization_tracker_reset(authorizations.count);
  bool transfers_value = word_nonzero(tx.value);
  bool preparation_ready = process_amsterdam_auth_cursor(authorizations, authorizations.count, v.sender, current_target, transfers_value);
  __int128 authorization_state_gas = FRAME_STATE_GAS_DELTA_ZERO;
  bool delegated = false;
  if (preparation_ready) {
    authorization_state_gas = frame_state_gas_used();
    message.state_gas_reservoir = state_gas_remaining;
    state_gas_spilled = STATE_GAS_SPILL_ZERO;
    struct TransactionPreparation preparation = prepare_amsterdam_transaction_dispatch(tx, v, upfront);
    preparation_ready = preparation.ready;
    delegated = preparation.delegated;
  }
  if (!preparation_ready) {
    k_journal_revert();
    message.state_gas_reservoir = preparation_reservoir;
    state_gas_remaining = preparation_reservoir;
    state_gas_spilled = STATE_GAS_SPILL_ZERO;
    struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_result_2_655 = tx_frame_gas_snapshot(initial_gas, gas_remaining, state_gas_remaining, FRAME_STATE_GAS_DELTA_ZERO);
    return ((struct TxFrameResultFields){.gas = tx_frame_gas_snapshot_result_2_655, .refund = GAS_REFUND_ZERO, .success = false});
  }
  k_journal_checkpoint();
  if (tx.is_create) {
    run_create_transaction_frame(tx, v.sender, v.nonce_before);
  } else {
    run_call_transaction_frame(tx, v.sender, delegated);
  }
  bool success = frame_succeeded();
  if (success) {
    k_journal_commit();
  } else {
    k_journal_revert();
  }
  k_journal_commit();
  __int128 frame_state_gas_used_result_2_663 = frame_state_gas_used();
  struct TxFrameGasSnapshotFields tx_frame_gas_snapshot_result_2_661 = tx_frame_gas_snapshot(initial_gas, gas_remaining, state_gas_remaining, (authorization_state_gas + frame_state_gas_used_result_2_663));
  __int128 result_2_662 = success ? frame_refund : GAS_REFUND_ZERO;
  return ((struct TxFrameResultFields){.gas = tx_frame_gas_snapshot_result_2_661, .refund = result_2_662, .success = success});
}

struct TxFrameResultFields run_transaction_frame(struct TransactionFields tx, struct TxValidityFields v, struct TxUpfrontResult upfront, PreparedAuthorizationList authorizations)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  bool result_2_653 = (bool)(execution_profile.protocol.fork >= Amsterdam);
  if (result_2_653) {
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
  bool result_2_648 = (bool)(profile.fork >= Prague);
  floor = result_2_648 ? gas_snapshot.calldata_floor : UINT64_C(0);
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
  bool tmp_3_1241 = (bool)((allowance.regular < intrinsic_execution) || (allowance.regular < calldata_floor));
  if (tmp_3_1241) {
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

u256 validated_word_product_u256_uint32_t_to_u256(u256 value, uint32_t factor)
{
  bool word_is_zero_result_2_901 = word_is_zero(value);
  bool tmp_3_1248;
  if (word_is_zero_result_2_901) {
    tmp_3_1248 = true;
  } else {
    tmp_3_1248 = word_is_zero_uint32_t_to_bool(factor);
  }
  if (tmp_3_1248) {
    return WORD_ZERO;
  }
  bool result_2_904;
  u256 word_div_word_result_2_903 = word_div_word_u256_uint32_t_to_u256(WORD_ALL_ONES, factor);
  result_2_904 = word_ule(value, word_div_word_result_2_903);
  if (result_2_904) {
    return word_mul_word_u256_uint32_t_to_u256(value, factor);
  }
  fatal_error(ExecutionInvalid);
}

u256 validated_word_product_u256_uint32_t_to_u256_variant_2(u256 value, uint32_t factor)
{
  bool word_is_zero_result_2_901 = word_is_zero(value);
  bool tmp_3_1248;
  if (word_is_zero_result_2_901) {
    tmp_3_1248 = true;
  } else {
    tmp_3_1248 = word_is_zero_uint32_t_to_bool(factor);
  }
  if (tmp_3_1248) {
    return WORD_ZERO;
  }
  bool result_2_904;
  u256 word_div_word_result_2_903 = word_div_word_u256_uint32_t_to_u256_variant_2(WORD_ALL_ONES, factor);
  result_2_904 = word_ule(value, word_div_word_result_2_903);
  if (result_2_904) {
    return word_mul_word_u256_uint32_t_to_u256(value, factor);
  }
  fatal_error(ExecutionInvalid);
}

u256 validated_word_product_u256_uint64_t_to_u256(u256 value, uint64_t factor)
{
  bool word_is_zero_result_2_901 = word_is_zero(value);
  bool tmp_3_1248;
  if (word_is_zero_result_2_901) {
    tmp_3_1248 = true;
  } else {
    tmp_3_1248 = word_is_zero_uint64_t_to_bool(factor);
  }
  if (tmp_3_1248) {
    return WORD_ZERO;
  }
  bool result_2_904;
  u256 word_div_word_result_2_903 = word_div_word_u256_uint64_t_to_u256(WORD_ALL_ONES, factor);
  result_2_904 = word_ule(value, word_div_word_result_2_903);
  if (result_2_904) {
    return word_mul_word_u256_uint64_t_to_u256(value, factor);
  }
  fatal_error(ExecutionInvalid);
}

