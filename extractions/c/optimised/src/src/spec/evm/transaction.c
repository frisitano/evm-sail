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
  return code_db_intern_input((stateless_input_slice(input.bytes, input.len)));
}

uint64_t calldata_cost(Bytes input)
{
  uint32_t nonzeroes = slice_count_nonzero(input);
  if (nonzeroes <= input.len) {
    return (uint64_t)((int64_t)(uint64_t)((int64_t)G_txdatazero * ((int64_t)input.len - (int64_t)nonzeroes)) + (int64_t)((uint64_t)G_txdatanonzero * (uint64_t)nonzeroes));
  }
  fatal_error(ExecutionInvalid);
}

uint64_t legacy_intrinsic_gas(struct TransactionFields tx)
{
  uint64_t data_cost = calldata_cost(tx.input_src);
  if (tx.is_create) {
    return (uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)data_cost + (__int128)G_transaction) + ((__int128)(uint64_t)G_access_list_address * (__int128)(uint64_t)tx.access_list.address_count)) + ((__int128)(uint64_t)G_access_list_storage_key * (__int128)(uint64_t)tx.access_list.slot_count)) + ((__int128)(uint64_t)PER_EMPTY_ACCOUNT * (__int128)(uint64_t)tx.authorizations.count)) + (__int128)G_txcreate) + (__int128)(transaction_initcode_gas(tx.input_src.len)));
  }
  return (uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)data_cost + (__int128)G_transaction) + ((__int128)(uint64_t)G_access_list_address * (__int128)(uint64_t)tx.access_list.address_count)) + ((__int128)(uint64_t)G_access_list_storage_key * (__int128)(uint64_t)tx.access_list.slot_count)) + ((__int128)(uint64_t)PER_EMPTY_ACCOUNT * (__int128)(uint64_t)tx.authorizations.count));
}

uint64_t legacy_calldata_floor(Bytes input)
{
  uint32_t nonzeroes = slice_count_nonzero(input);
  if (nonzeroes <= input.len) {
    return (uint64_t)((int64_t)(uint64_t)((int64_t)(uint64_t)((int64_t)UINT8_C(10) * ((int64_t)input.len - (int64_t)nonzeroes)) + (int64_t)(UINT64_C(40) * (uint64_t)nonzeroes)) + (int64_t)G_transaction);
  }
  fatal_error(ExecutionInvalid);
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
  if (neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C20__(tx.recipient, tx.sender)) {
    if (transfers_value) {
      return ((uint16_t)((uint32_t)AMSTERDAM_TRANSFER_LOG_COST + ((uint32_t)AMSTERDAM_TX_VALUE_COST + (uint32_t)AMSTERDAM_COLD_ACCOUNT_ACCESS)));
    }
    return AMSTERDAM_COLD_ACCOUNT_ACCESS;
  }
  return UINT16_C(0);
}

struct IntrinsicGasCost intrinsic_gas(struct TransactionFields tx)
{
  if (k_execution_profile.protocol.fork < Amsterdam) {
    uint64_t execution = legacy_intrinsic_gas(tx);
    return ((struct IntrinsicGasCost){.calldata_floor = (legacy_calldata_floor(tx.input_src)), .execution = execution, .state = UINT64_C(0)});
  }
  Bytes input = tx.input_src;
  uint16_t recipient = amsterdam_recipient_execution_cost(tx);
  uint32_t create_execution;
  if (tx.is_create) {
    create_execution = transaction_initcode_gas(input.len);
  } else {
    create_execution = UINT32_C(0);
  }
  return ((struct IntrinsicGasCost){.calldata_floor = (((uint64_t)AMSTERDAM_ACCESS_LIST_SLOT_FLOOR * (uint64_t)tx.access_list.slot_count) + (((uint64_t)AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR * (uint64_t)tx.access_list.address_count) + ((((uint64_t)AMSTERDAM_CALLDATA_FLOOR_BYTE * (uint64_t)input.len) + (uint64_t)AMSTERDAM_TX_BASE) + (uint64_t)recipient))), .execution = (uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(uint64_t)((__int128)(calldata_cost(tx.input_src)) + (__int128)AMSTERDAM_TX_BASE) + (__int128)recipient) + (((__int128)(uint64_t)AMSTERDAM_ACCESS_LIST_SLOT_FLOOR * (__int128)(uint64_t)tx.access_list.slot_count) + (((__int128)(uint64_t)AMSTERDAM_ACCESS_LIST_ADDRESS_FLOOR * (__int128)(uint64_t)tx.access_list.address_count) + (((__int128)(uint64_t)AMSTERDAM_ACCESS_LIST_SLOT * (__int128)(uint64_t)tx.access_list.slot_count) + ((__int128)(uint64_t)AMSTERDAM_ACCESS_LIST_ADDRESS * (__int128)(uint64_t)tx.access_list.address_count))))) + ((__int128)(uint64_t)AMSTERDAM_AUTH_BASE * (__int128)(uint64_t)tx.authorizations.count)) + (__int128)create_execution), .state = UINT64_C(0)});
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
    if (word_ule(blob_price, tx.max_blob_fee)) {
      blob_fee = validated_word_product_u256_uint32_t_to_u256_variant_2(blob_price, blob_gas);
    } else {
      fatal_error(ExecutionInvalid);
    }
  }
  u256 execution_cap = validated_word_product_u256_uint64_t_to_u256(tx.max_fee, gas_limit);
  u256 blob_cap = validated_word_product_u256_uint32_t_to_u256(tx.max_blob_fee, blob_gas);
  u256 maximum_value = !u256_lt(WORD_ALL_ONES, execution_cap) ? u256_sub(WORD_ALL_ONES, execution_cap) : u256_of_u320(u320_add(u320_of_u256(u256_sub((u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}}, u256_sub(execution_cap, WORD_ALL_ONES))), u320_of_u64(UINT8_C(1))));
  bool value_fits = word_ule(tx.value, maximum_value);
  u256 execution_and_value;
  if (value_fits) {
    execution_and_value = word_add_word(execution_cap, tx.value);
  } else {
    fatal_error(ExecutionInvalid);
  }
  u256 maximum_blob_cap = !u256_lt(WORD_ALL_ONES, execution_and_value) ? u256_sub(WORD_ALL_ONES, execution_and_value) : u256_of_u320(u320_add(u320_of_u256(u256_sub((u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}}, u256_sub(execution_and_value, WORD_ALL_ONES))), u320_of_u64(UINT8_C(1))));
  bool blob_cap_fits = word_ule(blob_cap, maximum_blob_cap);
  u256 upfront;
  if (blob_cap_fits) {
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
  uint64_t expected_chain_id = word_of_chain_identifier(k_chain_id);
  if (au.valid_sig && (eq_u256(au.chain_id, WORD_ZERO) || u256_eq_u64(au.chain_id, expected_chain_id))) {
    k_account_mark_warm(au.authority);
    bool _8_1363_8_1660;
    bytes20 _8_1364_8_1661 = k_deleg_target(au.authority, &_8_1363_8_1660);
    bytes32 code_key = k_code_key(au.authority);
    uint64_t nonce = k_get_nonce(au.authority);
    if ((eq_bytes32(code_key, KECCAK_EMPTY) || _8_1363_8_1660) && (nonce == au.nonce)) {
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

uint32_t process_amsterdam_auth(struct Authorization au, bytes20 sender, bytes20 current_target, bool transfers_value, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, bool *restrict condition_8_1464, uint64_t *restrict field_1_8_1465, uint64_t *restrict field_2_8_1466)
{
  uint64_t gas_after = carried_gas;
  uint64_t state_gas_after = carried_state_gas;
  uint32_t state_spill_after = carried_state_spill;
  uint64_t expected_chain_id = word_of_chain_identifier(k_chain_id);
  if (au.valid_sig && (eq_u256(au.chain_id, WORD_ZERO) || u256_eq_u64(au.chain_id, expected_chain_id))) {
    k_account_mark_warm(au.authority);
    bool _8_1363_8_1662;
    bytes20 _8_1364_8_1663 = k_deleg_target(au.authority, &_8_1363_8_1662);
    bytes32 code_key = k_code_key(au.authority);
    uint64_t nonce = k_get_nonce(au.authority);
    if ((eq_bytes32(code_key, KECCAK_EMPTY) || _8_1363_8_1662) && (nonce == au.nonce)) {
      bool seen = authorization_tracker_seen(au.authority);
      bool delegated_before_tx;
      if (seen) {
        delegated_before_tx = authorization_tracker_originally_delegated(au.authority);
      } else {
        delegated_before_tx = _8_1363_8_1662;
      }
      if (!(k_account_exists(au.authority))) {
        bool _8_1511_8_1668;
        uint64_t _8_1512_8_1669;
        uint64_t _8_1513_8_1670;
        uint32_t _8_1514_8_1671 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after, state_spill_after, G_amsterdam_state_new_account, &_8_1511_8_1668, &_8_1512_8_1669, &_8_1513_8_1670);
        gas_after = _8_1512_8_1669;
        state_gas_after = _8_1513_8_1670;
        state_spill_after = _8_1514_8_1671;
        if (_8_1511_8_1668) {
          (*condition_8_1464) = false;
          (*field_1_8_1465) = gas_after;
          (*field_2_8_1466) = state_gas_after;
          return state_spill_after;
        }
      }
      if (!seen && ((!eq_bytes20(au.authority, sender)) && (!transfers_value || (!eq_bytes20(au.authority, current_target))))) {
        if (gas_after < G_amsterdam_account_write) {
          (*condition_8_1464) = false;
          (*field_1_8_1465) = (uint64_t)GAS_ZERO;
          (*field_2_8_1466) = state_gas_after;
          return state_spill_after;
        }
        gas_after = gas_sub_uint64_t_uint16_t_to_uint64_t(gas_after, G_amsterdam_account_write);
      }
      bool delegation_set = authorization_tracker_delegation_set(au.authority);
      bool creates_delegation = neq_anything_R__sail_c_repr_fixed_bytes_u64_lanes_C20__(au.address, ZERO_ADDRESS);
      if (creates_delegation && (!delegated_before_tx && !delegation_set)) {
        bool _8_1511_8_1664;
        uint64_t _8_1512_8_1665;
        uint64_t _8_1513_8_1666;
        uint32_t _8_1514_8_1667 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after, state_spill_after, G_amsterdam_state_auth_base, &_8_1511_8_1664, &_8_1512_8_1665, &_8_1513_8_1666);
        gas_after = _8_1512_8_1665;
        state_gas_after = _8_1513_8_1666;
        state_spill_after = _8_1514_8_1667;
        if (_8_1511_8_1664) {
          (*condition_8_1464) = false;
          (*field_1_8_1465) = gas_after;
          (*field_2_8_1466) = state_gas_after;
          return state_spill_after;
        }
      }
      if (eq_bytes20(au.address, ZERO_ADDRESS)) {
        k_clear_code(au.authority);
      } else {
        k_set_delegation(au.authority, au.address);
      }
      k_bump_nonce(au.authority);
      authorization_tracker_commit(au.authority, (bool)(!seen && _8_1363_8_1662), creates_delegation);
    }
  }
  (*condition_8_1464) = true;
  (*field_1_8_1465) = gas_after;
  (*field_2_8_1466) = state_gas_after;
  return state_spill_after;
}

uint32_t process_amsterdam_auth_cursor(PreparedAuthorizationList authorizations, uint16_t count, bytes20 sender, bytes20 current_target, bool transfers_value, uint64_t gas, uint64_t state_gas, uint32_t state_spill, bool *restrict condition_8_1468, uint64_t *restrict field_1_8_1469, uint64_t *restrict field_2_8_1470)
{
  if (count == UINT8_C(0)) {
    (*condition_8_1468) = true;
    (*field_1_8_1469) = gas;
    (*field_2_8_1470) = state_gas;
    return state_spill;
  }
  struct Authorization authorization = prepared_authorization_head_(authorizations);
  PreparedAuthorizationList remaining = prepared_authorization_tail_(authorizations, count);
  bool _8_1464_8_1672;
  uint64_t _8_1465_8_1673;
  uint64_t _8_1466_8_1674;
  uint32_t _8_1467_8_1675 = process_amsterdam_auth(authorization, sender, current_target, transfers_value, gas, state_gas, state_spill, &_8_1464_8_1672, &_8_1465_8_1673, &_8_1466_8_1674);
  if (_8_1464_8_1672) {
    return process_amsterdam_auth_cursor(remaining, ((uint16_t)((uint32_t)count - (uint32_t)UINT16_C(1))), sender, current_target, transfers_value, _8_1465_8_1673, _8_1466_8_1674, _8_1467_8_1675, condition_8_1468, field_1_8_1469, field_2_8_1470);
  }
  (*condition_8_1468) = false;
  (*field_1_8_1469) = _8_1465_8_1673;
  (*field_2_8_1470) = _8_1466_8_1674;
  return _8_1467_8_1675;
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
  rlp_cursor_expect_end((rlp_cursor_advance(fields_1_6, keys_f.source.len)));
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

u256 eff_gas_price_for(u256 base_fee, u256 max_fee, u256 max_priority_fee, u256 *restrict __sail_c_repr_u256_8_1472)
{
  bool max_fee_below_base = word_ule(max_fee, base_fee);
  u256 price;
  if (max_fee_below_base) {
    price = max_fee;
  } else {
    u256 available_priority = !u256_lt(max_fee, base_fee) ? u256_sub(max_fee, base_fee) : u256_of_u320(u320_add(u320_of_u256(u256_sub((u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}}, u256_sub(base_fee, max_fee))), u320_of_u64(UINT8_C(1))));
    if (word_ule(max_priority_fee, available_priority)) {
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
  (*__sail_c_repr_u256_8_1472) = price;
  return priority;
}

struct TxValidityFields check_transaction_validity(struct TransactionFields tx, struct TransactionGasAllowanceFields allowance)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  struct TxTypeSemantics tx_semantics = tx_type_semantics(tx.tx_type);
  uint8_t parity = tx_signature_parity(k_chain_id, tx_semantics.signature, tx.sig_v);
  if (!(tx_auth_valid(tx.sender, tx.signing_hash, parity, tx.sig_r, tx.sig_s))) {
    fatal_error(InvalidSignature);
  }
  uint64_t gas_limit = allowance.total;
  u256 _8_1472_8_1676;
  u256 _8_1473_8_1677 = eff_gas_price_for((k_header.base_fee), tx.max_fee, tx.max_priority_fee, &_8_1472_8_1676);
  bytes20 sender = tx.sender;
  uint64_t nonce_before = k_get_nonce(sender);
  struct TransactionCosts costs = transaction_costs(profile, tx, gas_limit, (k_header.excess_blob_gas));
  if (!u256_eq_u64(tx.nonce, (word_of_account_nonce(nonce_before)))) {
    fatal_error(ExecutionInvalid);
  }
  bool _8_1363_8_1678;
  bytes20 _8_1364_8_1679 = k_deleg_target(sender, &_8_1363_8_1678);
  if (tx_semantics.blob && ((profile.fork < Cancun) || ((tx.blob_hashes.count == UINT8_C(0)) || tx.is_create))) {
    fatal_error(ExecutionInvalid);
  }
  if ((profile.fork >= Prague) && (gas_limit < costs.calldata_floor)) {
    fatal_error(ExecutionInvalid);
  }
  u256 sender_balance = k_get_balance(sender);
  if (!(word_ule(costs.upfront, sender_balance))) {
    fatal_error(ExecutionInvalid);
  }
  bytes32 sender_code_key = k_code_key(sender);
  if ((!eq_bytes32(sender_code_key, KECCAK_EMPTY)) && !_8_1363_8_1678) {
    fatal_error(ExecutionInvalid);
  }
  if (allowance.regular < costs.calldata_floor) {
    fatal_error(ExecutionInvalid);
  }
  bool base_fee_affordable = word_ule((k_header.base_fee), tx.max_fee);
  if (!base_fee_affordable) {
    fatal_error(ExecutionInvalid);
  }
  bool valid_initcode_size = initcode_size_allowed(tx.input_src.len);
  if (tx.is_create && !valid_initcode_size) {
    fatal_error(ExecutionInvalid);
  }
  if (!(word_ule(tx.max_priority_fee, tx.max_fee))) {
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
  bool typed_signature = eq_TxSignatureScheme(tx_semantics.signature, TypedSignature);
  if (typed_signature && (tx.chain_id != k_chain_id)) {
    fatal_error(ExecutionInvalid);
  }
  if (nonce_before == UINT64_C(18446744073709551615)) {
    fatal_error(ExecutionInvalid);
  }
  struct TransactionInitialGasFields initial_gas = transaction_initial_gas(allowance, costs.intrinsic_execution, costs.intrinsic_state, costs.calldata_floor);
  return tx_validity_fields(sender, nonce_before, initial_gas, costs.blob_fee, _8_1472_8_1676, _8_1473_8_1677);
}

struct TxUpfrontResult apply_transaction_upfront_effects(struct TransactionFields tx, struct TxValidityFields v, PreparedAuthorizationList authorizations)
{
  struct ExecutionProfileFields execution_profile = k_execution_profile;
  struct ProtocolProfileFields profile = execution_profile.protocol;
  bool create_target_prestate_empty;
  if ((profile.fork >= Amsterdam) && tx.is_create) {
    bytes20 create_target = k_create_addr(v.sender, v.nonce_before);
    create_target_prestate_empty = (bool)(!(k_account_exists(create_target)));
  } else {
    create_target_prestate_empty = false;
  }
  k_sub_balance(v.sender, (validated_word_product_u256_uint64_t_to_u256(v.gas_price, v.gas.admitted_limit)));
  if (word_nonzero(v.blob_fee)) {
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

uint32_t enter_transaction_frame(struct TxValidityFields v, uint64_t *restrict field_0_8_1474, uint64_t *restrict field_1_8_1475, uint32_t *restrict field_2_8_1476, __int128 *restrict field_3_8_1477, StackPointer *restrict stackpointer_8_1478, uint32_t *restrict field_5_8_1479)
{
  (*stackpointer_8_1478) = stack_reset();
  (*field_0_8_1474) = v.gas.execution_remaining;
  (*field_1_8_1475) = v.gas.state_remaining;
  (*field_2_8_1476) = (uint32_t)STATE_GAS_SPILL_ZERO;
  (*field_3_8_1477) = GAS_REFUND_ZERO;
  (*field_5_8_1479) = MEMORY_BASE_ZERO;
  return MEMORY_HEIGHT_ZERO;
}

Bytes run_create_transaction_frame(struct TransactionFields tx, bytes20 sender, uint64_t nonce_before, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_stack, uint32_t carried_memory_base, uint32_t carried_memory_height, struct CodeFields carried_code, struct CalldataSlice carried_calldata, uint64_t state_gas_reservoir, uint64_t *restrict field_0_8_1481, uint64_t *restrict field_1_8_1482, uint32_t *restrict field_2_8_1483, __int128 *restrict field_3_8_1484, struct FrameStatus *restrict framestatus_8_1485)
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
  if (k_account_occupied(new_addr)) {
    gas_after = (uint64_t)GAS_ZERO;
    struct ExceptionalStateTransition exceptional;
    if (k_execution_profile.protocol.fork >= Amsterdam) {
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(AddressCollision))});
    } else {
      exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after, .state_gas_spilled = state_spill_after, .status = (Exceptional(AddressCollision))});
    }
    state_gas_after = exceptional.state_gas_remaining;
    state_spill_after = exceptional.state_gas_spilled;
    status_after = exceptional.status;
  } else {
    k_mark_created(new_addr);
    k_clear_storage(new_addr);
    k_bump_nonce(new_addr);
    if (word_nonzero(tx.value)) {
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
    uint64_t _8_1529_8_1680;
    uint64_t _8_1530_8_1681;
    uint32_t _8_1531_8_1682;
    __int128 _8_1532_8_1683;
    struct FrameStatus _8_1533_8_1684;
    Bytes _8_1534_8_1685 = interpret_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint8_t_struct_CodeFields_struct_CalldataSlice_to_struct_tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes(gas_after, state_gas_after, state_spill_after, refund_after, carried_stack, carried_memory_base, carried_memory_height, sender, new_addr, new_addr, tx.value, state_gas_reservoir, false, UINT8_C(0), frame_code, frame_calldata, &_8_1529_8_1680, &_8_1530_8_1681, &_8_1531_8_1682, &_8_1532_8_1683, &_8_1533_8_1684);
    gas_after = _8_1529_8_1680;
    state_gas_after = _8_1530_8_1681;
    state_spill_after = _8_1531_8_1682;
    refund_after = _8_1532_8_1683;
    status_after = _8_1533_8_1684;
    output_after = _8_1534_8_1685;
    if (frame_succeeded(status_after)) {
      uint32_t dep_len = output_after.len;
      bool valid_deployed_size = deployed_code_size_allowed(dep_len);
      bool valid_prefix;
      if ((execution_profile.protocol.fork < London) || (dep_len == UINT8_C(0))) {
        valid_prefix = true;
      } else {
        valid_prefix = (bool)((output_byte(output_after, UINT8_C(0))) != UINT64_C(0xEF));
      }
      if (valid_deployed_size && valid_prefix) {
        struct GasCharge deployment_charge = code_deployment_execution_cost(dep_len, gas_after);
        if (deployment_charge.affordable) {
          gas_after = gas_sub(gas_after, deployment_charge.cost);
          uint64_t state_deposit = code_deployment_state_cost(dep_len);
          bool deployment_halt = false;
          bool _8_1519_8_1686;
          uint64_t _8_1520_8_1687;
          uint64_t _8_1521_8_1688;
          uint32_t _8_1522_8_1689 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint64_t_to_struct_tuple_bool_uint_64_uint_64_uint_32(gas_after, state_gas_after, state_spill_after, state_deposit, &_8_1519_8_1686, &_8_1520_8_1687, &_8_1521_8_1688);
          deployment_halt = _8_1519_8_1686;
          gas_after = _8_1520_8_1687;
          state_gas_after = _8_1521_8_1688;
          state_spill_after = _8_1522_8_1689;
          if (deployment_halt) {
            gas_after = (uint64_t)GAS_ZERO;
            struct ExceptionalStateTransition exceptional_3_1181;
            if (k_execution_profile.protocol.fork >= Amsterdam) {
              exceptional_3_1181 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
            } else {
              exceptional_3_1181 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after, .state_gas_spilled = state_spill_after, .status = (Exceptional(OutOfGas))});
            }
            state_gas_after = exceptional_3_1181.state_gas_remaining;
            state_spill_after = exceptional_3_1181.state_gas_spilled;
            status_after = exceptional_3_1181.status;
          }
          if (frame_succeeded(status_after)) {
            Bytes stored_code = code_db_intern_output(output_after);
            k_deploy_code(new_addr, stored_code);
          }
        } else {
          gas_after = (uint64_t)GAS_ZERO;
          struct ExceptionalStateTransition exceptional_3_1182;
          if (k_execution_profile.protocol.fork >= Amsterdam) {
            exceptional_3_1182 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
          } else {
            exceptional_3_1182 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after, .state_gas_spilled = state_spill_after, .status = (Exceptional(OutOfGas))});
          }
          state_gas_after = exceptional_3_1182.state_gas_remaining;
          state_spill_after = exceptional_3_1182.state_gas_spilled;
          status_after = exceptional_3_1182.status;
        }
      } else {
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional_3_1183;
        if (k_execution_profile.protocol.fork >= Amsterdam) {
          exceptional_3_1183 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
        } else {
          exceptional_3_1183 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after, .state_gas_spilled = state_spill_after, .status = (Exceptional(OutOfGas))});
        }
        state_gas_after = exceptional_3_1183.state_gas_remaining;
        state_spill_after = exceptional_3_1183.state_gas_spilled;
        status_after = exceptional_3_1183.status;
      }
    }
  }
  (*field_0_8_1481) = gas_after;
  (*field_1_8_1482) = state_gas_after;
  (*field_2_8_1483) = state_spill_after;
  (*field_3_8_1484) = refund_after;
  (*framestatus_8_1485) = status_after;
  return output_after;
}

Bytes run_call_transaction_frame(struct TransactionFields tx, bytes20 sender, bool delegated, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, __int128 carried_refund, StackPointer carried_stack, uint32_t carried_memory_base, uint32_t carried_memory_height, bytes20 carried_code_address, struct CodeFields carried_code, struct CalldataSlice carried_calldata, uint64_t state_gas_reservoir, uint64_t *restrict field_0_8_1487, uint64_t *restrict field_1_8_1488, uint32_t *restrict field_2_8_1489, __int128 *restrict field_3_8_1490, struct FrameStatus *restrict framestatus_8_1491)
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
  if (word_nonzero(tx.value)) {
    k_transfer(sender, tx.recipient, tx.value);
  }
  enum PrecompileId selected_precompile = precompile_id_for_address(tx.recipient);
  bool direct_precompile = false;
  if (!delegated) {
    direct_precompile = neq_anything_EPrecompileId__(selected_precompile, NotPrecompile);
  }
  if (direct_precompile) {
    struct CalldataSlice precompile_input = InputCalldata(tx.input_src);
    struct GasCharge precompile_charge = precompile_gas(selected_precompile, precompile_input, gas_after);
    if (precompile_charge.affordable) {
      struct PrecompileResult result = run_precompile_slice(selected_precompile, precompile_input);
      if (result.success) {
        gas_after = gas_sub(gas_after, precompile_charge.cost);
        output_after = result.output;
        status_after = Halted((HaltReturn(result.output)));
      } else {
        gas_after = (uint64_t)GAS_ZERO;
        struct ExceptionalStateTransition exceptional;
        if (k_execution_profile.protocol.fork >= Amsterdam) {
          exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
        } else {
          exceptional = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after, .state_gas_spilled = state_spill_after, .status = (Exceptional(OutOfGas))});
        }
        state_gas_after = exceptional.state_gas_remaining;
        state_spill_after = exceptional.state_gas_spilled;
        status_after = exceptional.status;
      }
    } else {
      gas_after = (uint64_t)GAS_ZERO;
      struct ExceptionalStateTransition exceptional_3_1137;
      if (k_execution_profile.protocol.fork >= Amsterdam) {
        exceptional_3_1137 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_reservoir, .state_gas_spilled = (uint32_t)STATE_GAS_SPILL_ZERO, .status = (Exceptional(OutOfGas))});
      } else {
        exceptional_3_1137 = ((struct ExceptionalStateTransition){.state_gas_remaining = state_gas_after, .state_gas_spilled = state_spill_after, .status = (Exceptional(OutOfGas))});
      }
      state_gas_after = exceptional_3_1137.state_gas_remaining;
      state_spill_after = exceptional_3_1137.state_gas_spilled;
      status_after = exceptional_3_1137.status;
    }
  } else {
    if (execution_profile.protocol.fork < Amsterdam) {
      frame_calldata = InputCalldata(tx.input_src);
      code_address = tx.recipient;
      bool tx_deleg;
      bytes20 tx_dtgt = k_deleg_target(tx.recipient, &tx_deleg);
      if (tx_deleg) {
        k_account_mark_warm(tx_dtgt);
        k_aload_(tx_dtgt);
      }
      if (tx_deleg) {
        code_address = tx_dtgt;
      }
      frame_code = executable_code(tx.recipient, tx_deleg, tx_dtgt);
    }
    uint64_t _8_1529_8_1690;
    uint64_t _8_1530_8_1691;
    uint32_t _8_1531_8_1692;
    __int128 _8_1532_8_1693;
    struct FrameStatus _8_1533_8_1694;
    Bytes _8_1534_8_1695 = interpret_uint64_t_uint64_t_uint32_t___int128_StackPointer_uint32_t_uint32_t_bytes20_bytes20_bytes20_u256_uint64_t_bool_uint8_t_struct_CodeFields_struct_CalldataSlice_to_struct_tuple_uint_64_uint_64_uint_32_int_128_FrameStatus_Bytes(gas_after, state_gas_after, state_spill_after, refund_after, carried_stack, carried_memory_base, carried_memory_height, sender, tx.recipient, code_address, tx.value, state_gas_reservoir, false, UINT8_C(0), frame_code, frame_calldata, &_8_1529_8_1690, &_8_1530_8_1691, &_8_1531_8_1692, &_8_1532_8_1693, &_8_1533_8_1694);
    gas_after = _8_1529_8_1690;
    state_gas_after = _8_1530_8_1691;
    state_spill_after = _8_1531_8_1692;
    refund_after = _8_1532_8_1693;
    status_after = _8_1533_8_1694;
    output_after = _8_1534_8_1695;
  }
  (*field_0_8_1487) = gas_after;
  (*field_1_8_1488) = state_gas_after;
  (*field_2_8_1489) = state_spill_after;
  (*field_3_8_1490) = refund_after;
  (*framestatus_8_1491) = status_after;
  return output_after;
}

struct TxFrameResultFields run_legacy_transaction_frame(struct TransactionFields tx, struct TxValidityFields v)
{
  k_journal_checkpoint();
  uint64_t _8_1474_8_1696;
  uint64_t initial_state_gas;
  uint32_t _8_1476_8_1697;
  __int128 _8_1477_8_1698;
  StackPointer _8_1478_8_1699;
  uint32_t _8_1479_8_1700;
  uint32_t _8_1480_8_1701 = enter_transaction_frame(v, &_8_1474_8_1696, &initial_state_gas, &_8_1476_8_1697, &_8_1477_8_1698, &_8_1478_8_1699, &_8_1479_8_1700);
  uint64_t field_0_8_1797;
  uint64_t field_1_8_1798;
  uint32_t field_2_8_1799;
  __int128 field_3_8_1800;
  struct FrameStatus framestatus_8_1801;
  Bytes outputslicefields_8_1802;
  if (tx.is_create) {
    outputslicefields_8_1802 = run_create_transaction_frame(tx, v.sender, v.nonce_before, _8_1474_8_1696, initial_state_gas, _8_1476_8_1697, _8_1477_8_1698, _8_1478_8_1699, _8_1479_8_1700, _8_1480_8_1701, EMPTY_CODE, EMPTY_CALLDATA, initial_state_gas, &field_0_8_1797, &field_1_8_1798, &field_2_8_1799, &field_3_8_1800, &framestatus_8_1801);
  } else {
    outputslicefields_8_1802 = run_call_transaction_frame(tx, v.sender, false, _8_1474_8_1696, initial_state_gas, _8_1476_8_1697, _8_1477_8_1698, _8_1478_8_1699, _8_1479_8_1700, _8_1480_8_1701, tx.recipient, EMPTY_CODE, EMPTY_CALLDATA, initial_state_gas, &field_0_8_1797, &field_1_8_1798, &field_2_8_1799, &field_3_8_1800, &framestatus_8_1801);
  }
  uint64_t state_gas_after = field_1_8_1798;
  bool success = frame_succeeded(framestatus_8_1801);
  if (success) {
    k_journal_commit();
  } else {
    k_journal_revert();
  }
  __int128 state_delta = frame_state_gas_used(initial_state_gas, state_gas_after, field_2_8_1799);
  __int128 retained_refund = success ? field_3_8_1800 : GAS_REFUND_ZERO;
  return ((struct TxFrameResultFields){.gas = (tx_frame_gas_snapshot_struct_TransactionInitialGasFields_uint64_t_uint64_t___int128_to_struct_TxFrameGasSnapshotFields(v.gas, field_0_8_1797, state_gas_after, state_delta)), .refund = retained_refund, .success = success});
}

struct TxFrameResultFields run_amsterdam_transaction_frame(struct TransactionFields tx, struct TxValidityFields v, struct TxUpfrontResult upfront, PreparedAuthorizationList authorizations)
{
  uint64_t gas_after;
  uint64_t state_gas_after;
  uint32_t state_spill_after;
  __int128 refund_after;
  StackPointer _8_1478_8_1702;
  uint32_t _8_1479_8_1703;
  uint32_t _8_1480_8_1704 = enter_transaction_frame(v, &gas_after, &state_gas_after, &state_spill_after, &refund_after, &_8_1478_8_1702, &_8_1479_8_1703);
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
  bool _8_1468_8_1705;
  uint64_t _8_1469_8_1706;
  uint64_t _8_1470_8_1707;
  uint32_t _8_1471_8_1708 = process_amsterdam_auth_cursor(authorizations, authorizations.count, v.sender, current_target, transfers_value, gas_after, state_gas_after, state_spill_after, &_8_1468_8_1705, &_8_1469_8_1706, &_8_1470_8_1707);
  preparation_ready = _8_1468_8_1705;
  gas_after = _8_1469_8_1706;
  state_gas_after = _8_1470_8_1707;
  state_spill_after = _8_1471_8_1708;
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
    struct TransactionPreparation preparation;
    uint64_t _8_1544_8_1721;
    uint64_t _8_1545_8_1722;
    uint32_t _8_1546_8_1723;
    bytes20 _8_1547_8_1724;
    bytes20 _8_1548_8_1725;
    struct CodeFields _8_1549_8_1726;
    struct CalldataSlice _8_1550_8_1727 = prepare_amsterdam_transaction_dispatch(tx, v, upfront, gas_after, state_gas_after, state_spill_after, &preparation, &_8_1544_8_1721, &_8_1545_8_1722, &_8_1546_8_1723, &_8_1547_8_1724, &_8_1548_8_1725, &_8_1549_8_1726);
    gas_after = _8_1544_8_1721;
    state_gas_after = _8_1545_8_1722;
    state_spill_after = _8_1546_8_1723;
    preparation_ready = preparation.ready;
    delegated = preparation.delegated;
    prepared_code_address = _8_1548_8_1725;
    prepared_code = _8_1549_8_1726;
    prepared_calldata = _8_1550_8_1727;
  }
  if (!preparation_ready) {
    k_journal_revert();
    state_gas_after = preparation_reservoir;
    state_spill_after = (uint32_t)STATE_GAS_SPILL_ZERO;
    return ((struct TxFrameResultFields){.gas = (tx_frame_gas_snapshot_struct_TransactionInitialGasFields_uint8_t_uint8_t___int128_to_struct_TxFrameGasSnapshotFields(initial_gas, GAS_ZERO, STATE_GAS_ZERO, FRAME_STATE_GAS_DELTA_ZERO)), .refund = GAS_REFUND_ZERO, .success = false});
  }
  k_journal_checkpoint();
  if (tx.is_create) {
    uint64_t _8_1481_8_1715;
    uint64_t _8_1482_8_1716;
    uint32_t _8_1483_8_1717;
    __int128 _8_1484_8_1718;
    struct FrameStatus _8_1485_8_1719;
    Bytes _8_1486_8_1720 = run_create_transaction_frame(tx, v.sender, v.nonce_before, gas_after, state_gas_after, state_spill_after, refund_after, _8_1478_8_1702, _8_1479_8_1703, _8_1480_8_1704, prepared_code, prepared_calldata, execution_reservoir, &_8_1481_8_1715, &_8_1482_8_1716, &_8_1483_8_1717, &_8_1484_8_1718, &_8_1485_8_1719);
    gas_after = _8_1481_8_1715;
    state_gas_after = _8_1482_8_1716;
    state_spill_after = _8_1483_8_1717;
    refund_after = _8_1484_8_1718;
    status_after = _8_1485_8_1719;
  } else {
    uint64_t _8_1487_8_1709;
    uint64_t _8_1488_8_1710;
    uint32_t _8_1489_8_1711;
    __int128 _8_1490_8_1712;
    struct FrameStatus _8_1491_8_1713;
    Bytes _8_1492_8_1714 = run_call_transaction_frame(tx, v.sender, delegated, gas_after, state_gas_after, state_spill_after, refund_after, _8_1478_8_1702, _8_1479_8_1703, _8_1480_8_1704, prepared_code_address, prepared_code, prepared_calldata, execution_reservoir, &_8_1487_8_1709, &_8_1488_8_1710, &_8_1489_8_1711, &_8_1490_8_1712, &_8_1491_8_1713);
    gas_after = _8_1487_8_1709;
    state_gas_after = _8_1488_8_1710;
    state_spill_after = _8_1489_8_1711;
    refund_after = _8_1490_8_1712;
    status_after = _8_1491_8_1713;
  }
  bool success = frame_succeeded(status_after);
  if (success) {
    k_journal_commit();
  } else {
    k_journal_revert();
  }
  k_journal_commit();
  __int128 execution_state_delta = frame_state_gas_used(execution_reservoir, state_gas_after, state_spill_after);
  __int128 retained_refund = success ? refund_after : GAS_REFUND_ZERO;
  return ((struct TxFrameResultFields){.gas = (tx_frame_gas_snapshot(initial_gas, gas_after, state_gas_after, (authorization_state_gas + execution_state_delta))), .refund = retained_refund, .success = success});
}

struct TxFrameResultFields run_transaction_frame(struct TransactionFields tx, struct TxValidityFields v, struct TxUpfrontResult upfront, PreparedAuthorizationList authorizations)
{
  if (k_execution_profile.protocol.fork >= Amsterdam) {
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
  uint64_t floor = profile.fork >= Prague ? gas_snapshot.calldata_floor : UINT64_C(0);
  uint64_t gas_used = (gas_limit - gas_left_1_4) < floor ? floor : (gas_limit - gas_left_1_4);
  uint64_t tx_state_gas = gas_snapshot.state_used;
  uint64_t execution_gas = ((gas_limit - gas_snapshot.remaining) - tx_state_gas) < floor ? floor : ((gas_limit - gas_snapshot.remaining) - tx_state_gas);
  k_add_balance(v.sender, (validated_word_product_u256_uint64_t_to_u256(v.gas_price, (gas_limit - gas_used))));
  bytes20 coinbase = k_coinbase();
  k_add_balance(coinbase, (validated_word_product_u256_uint64_t_to_u256(v.priority_fee, gas_used)));
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
  k_set_tx((tx_env(tx.sender, validity.gas_price, tx.blob_hashes)));
  struct TxUpfrontResult upfront = apply_transaction_upfront_effects(tx, validity, authorizations);
  struct TxFrameResultFields frame_result = run_transaction_frame(tx, validity, upfront, authorizations);
  return settle_transaction(tx, validity, upfront.authorization_refund, frame_result);
}

struct CalldataSlice prepare_amsterdam_transaction_dispatch(struct TransactionFields tx, struct TxValidityFields v, struct TxUpfrontResult upfront, uint64_t carried_gas, uint64_t carried_state_gas, uint32_t carried_state_spill, struct TransactionPreparation *restrict transactionpreparation_8_1543, uint64_t *restrict field_1_8_1544, uint64_t *restrict field_2_8_1545, uint32_t *restrict field_3_8_1546, bytes20 *restrict __sail_c_repr_fixed_bytes_u64_lanes_8_1547, bytes20 *restrict __sail_c_repr_fixed_bytes_u64_lanes_8_1548, struct CodeFields *restrict codefields_8_1549)
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
      bool _8_1515_8_1754;
      uint64_t _8_1516_8_1755;
      uint64_t _8_1517_8_1756;
      uint32_t _8_1518_8_1757 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32_variant_2(gas_after, state_gas_after, state_spill_after, G_amsterdam_state_new_account, &_8_1515_8_1754, &_8_1516_8_1755, &_8_1517_8_1756);
      gas_after = _8_1516_8_1755;
      state_gas_after = _8_1517_8_1756;
      state_spill_after = _8_1518_8_1757;
      if (_8_1515_8_1754) {
        (*transactionpreparation_8_1543) = ((struct TransactionPreparation){.delegated = false, .ready = false});
        (*field_1_8_1544) = gas_after;
        (*field_2_8_1545) = state_gas_after;
        (*field_3_8_1546) = state_spill_after;
        (*__sail_c_repr_fixed_bytes_u64_lanes_8_1547) = current_target;
        (*__sail_c_repr_fixed_bytes_u64_lanes_8_1548) = current_target;
        (*codefields_8_1549) = EMPTY_CODE;
        return EMPTY_CALLDATA;
      }
    }
    Bytes initcode = transaction_initcode_slice(tx.input_src);
    bytes32 code_id = code_db_insert(initcode, execution_profile.protocol.fork);
    (*codefields_8_1549) = code_db_resolve(code_id);
    (*transactionpreparation_8_1543) = ((struct TransactionPreparation){.delegated = false, .ready = true});
    (*field_1_8_1544) = gas_after;
    (*field_2_8_1545) = state_gas_after;
    (*field_3_8_1546) = state_spill_after;
    (*__sail_c_repr_fixed_bytes_u64_lanes_8_1547) = current_target;
    (*__sail_c_repr_fixed_bytes_u64_lanes_8_1548) = current_target;
    return EMPTY_CALLDATA;
  }
  struct CalldataSlice calldata = InputCalldata(tx.input_src);
  bool transfers_value = word_nonzero(tx.value);
  bool recipient_empty = k_account_is_empty(tx.recipient);
  if (transfers_value && recipient_empty) {
    bool _8_1515_8_1750;
    uint64_t _8_1516_8_1751;
    uint64_t _8_1517_8_1752;
    uint32_t _8_1518_8_1753 = charge_state_gas_uint64_t_uint64_t_uint32_t_uint32_t_to_struct_tuple_bool_uint_64_uint_64_uint_32_variant_2(gas_after, state_gas_after, state_spill_after, G_amsterdam_state_new_account, &_8_1515_8_1750, &_8_1516_8_1751, &_8_1517_8_1752);
    gas_after = _8_1516_8_1751;
    state_gas_after = _8_1517_8_1752;
    state_spill_after = _8_1518_8_1753;
    if (_8_1515_8_1750) {
      (*transactionpreparation_8_1543) = ((struct TransactionPreparation){.delegated = false, .ready = false});
      (*field_1_8_1544) = gas_after;
      (*field_2_8_1545) = state_gas_after;
      (*field_3_8_1546) = state_spill_after;
      (*__sail_c_repr_fixed_bytes_u64_lanes_8_1547) = current_target;
      (*__sail_c_repr_fixed_bytes_u64_lanes_8_1548) = current_target;
      (*codefields_8_1549) = EMPTY_CODE;
      return calldata;
    }
  }
  bool delegated;
  bytes20 delegate = k_deleg_target(tx.recipient, &delegated);
  if (delegated) {
    bool warm = k_account_is_warm(delegate);
    uint16_t access_cost = account_cost(warm);
    if (gas_after < access_cost) {
      (*transactionpreparation_8_1543) = ((struct TransactionPreparation){.delegated = false, .ready = false});
      (*field_1_8_1544) = (uint64_t)GAS_ZERO;
      (*field_2_8_1545) = state_gas_after;
      (*field_3_8_1546) = state_spill_after;
      (*__sail_c_repr_fixed_bytes_u64_lanes_8_1547) = current_target;
      (*__sail_c_repr_fixed_bytes_u64_lanes_8_1548) = current_target;
      (*codefields_8_1549) = EMPTY_CODE;
      return calldata;
    }
    gas_after = gas_sub_uint64_t_uint16_t_to_uint64_t(gas_after, access_cost);
    k_account_mark_warm(delegate);
  }
  bytes20 code_address = delegated ? delegate : current_target;
  (*codefields_8_1549) = executable_code(tx.recipient, delegated, delegate);
  (*transactionpreparation_8_1543) = ((struct TransactionPreparation){.delegated = delegated, .ready = true});
  (*field_1_8_1544) = gas_after;
  (*field_2_8_1545) = state_gas_after;
  (*field_3_8_1546) = state_spill_after;
  (*__sail_c_repr_fixed_bytes_u64_lanes_8_1547) = current_target;
  (*__sail_c_repr_fixed_bytes_u64_lanes_8_1548) = code_address;
  return calldata;
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
  if (eq_u256(value, WORD_ZERO) || u256_eq_u64(WORD_ZERO, factor)) {
    return WORD_ZERO;
  }
  u256 maximum_value = word_div_word_u256_uint32_t_to_u256(WORD_ALL_ONES, factor);
  if (word_ule(value, maximum_value)) {
    return word_mul_word_u256_uint32_t_to_u256(value, factor);
  }
  fatal_error(ExecutionInvalid);
}

u256 validated_word_product_u256_uint32_t_to_u256_variant_2(u256 value, uint32_t factor)
{
  if (eq_u256(value, WORD_ZERO) || u256_eq_u64(WORD_ZERO, factor)) {
    return WORD_ZERO;
  }
  u256 maximum_value = word_div_word_u256_uint32_t_to_u256_variant_2(WORD_ALL_ONES, factor);
  if (word_ule(value, maximum_value)) {
    return word_mul_word_u256_uint32_t_to_u256(value, factor);
  }
  fatal_error(ExecutionInvalid);
}

u256 validated_word_product_u256_uint64_t_to_u256(u256 value, uint64_t factor)
{
  if (eq_u256(value, WORD_ZERO) || u256_eq_u64(WORD_ZERO, factor)) {
    return WORD_ZERO;
  }
  u256 maximum_value = word_div_word_u256_uint64_t_to_u256(WORD_ALL_ONES, factor);
  if (word_ule(value, maximum_value)) {
    return word_mul_word_u256_uint64_t_to_u256(value, factor);
  }
  fatal_error(ExecutionInvalid);
}

