#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

bool historical_schema_has_slot(Bytes cursor, u256 slot)
{
  if (cursor.len == UINT8_C(0)) {
    return false;
  }
  struct RlpFieldRef entry = rlp_decode_item(cursor);
  Bytes next = rlp_cursor_advance(cursor, entry.source.len);
  Bytes fields = bal_ref_cursor(entry);
  struct RlpFieldRef field = rlp_decode_item(fields);
  u256 candidate = bal_ref_word(field);
  if (eq_u256(candidate, slot)) {
    return true;
  }
  if (word_ult(slot, candidate)) {
    return false;
  }
  return historical_schema_has_slot(next, slot);
}

bool historical_schema_has_access(Bytes cursor, u256 address, struct option_R__sail_c_repr_u256_ slot)
{
  if (cursor.len == UINT8_C(0)) {
    return false;
  }
  struct RlpFieldRef entry = rlp_decode_item(cursor);
  Bytes next = rlp_cursor_advance(cursor, entry.source.len);
  Bytes fields = bal_ref_cursor(entry);
  struct RlpFieldRef field = rlp_decode_item(fields);
  u256 candidate = rlp_decode_word(field);
  if (eq_u256(candidate, address)) {
    Bytes fields_1_21 = rlp_cursor_advance(fields, field.source.len);
    struct RlpFieldRef slots = rlp_decode_item(fields_1_21);
    switch (slot.kind) {
    case Kind_None_R__sail_c_repr_u256_:
      return true;
    case Kind_Some_R__sail_c_repr_u256_:
    {
      Bytes bal_ref_cursor_result_2_2393 = bal_ref_cursor(slots);
      return historical_schema_has_slot(bal_ref_cursor_result_2_2393, slot.variants.Some_R__sail_c_repr_u256_);
    }
    }
  }
  if (word_ult(address, candidate)) {
    return false;
  }
  return historical_schema_has_access(next, address, slot);
}

void require_historical_state_access(bytes20 address, struct option_R__sail_c_repr_u256_ slot)
{
  if (k_execution_profile.protocol.fork < Amsterdam) {
    bool tmp_3_3390;
    switch (slot.kind) {
    case Kind_None_R__sail_c_repr_u256_:
      tmp_3_3390 = account_is_warm(address);
      break;
    case Kind_Some_R__sail_c_repr_u256_:
      tmp_3_3390 = storage_is_warm(address, slot.variants.Some_R__sail_c_repr_u256_);
      break;
    }
    if (tmp_3_3390) {
      return;
    }
    struct RlpFieldRef root = rlp_single_ref(k_historical_access_schema);
    bool found;
    Bytes bal_ref_cursor_result_2_2388 = bal_ref_cursor(root);
    found = historical_schema_has_access(bal_ref_cursor_result_2_2388, (address_to_word(address)), slot);
    if (found) {
      return;
    }
    fatal_error(InvalidBlockAccessList);
  }
}

void validate_historical_schema_slots(Bytes cursor, u256 previous, bool have_previous, bytes32 storage_root)
{
  if (cursor.len == UINT8_C(0)) {
    return;
  }
  struct RlpFieldRef entry = rlp_decode_item(cursor);
  Bytes next = rlp_cursor_advance(cursor, entry.source.len);
  Bytes fields = bal_ref_cursor(entry);
  struct RlpFieldRef slot_field = rlp_decode_item(fields);
  Bytes fields_1_5 = rlp_cursor_advance(fields, slot_field.source.len);
  struct RlpFieldRef markers = rlp_decode_item(fields_1_5);
  bal_expect_end((rlp_cursor_advance(fields_1_5, markers.source.len)));
  u256 slot = bal_ref_word(slot_field);
  bool tmp_3_748;
  if (have_previous) {
    tmp_3_748 = (bool)(!(word_ult(previous, slot)));
  } else {
    tmp_3_748 = false;
  }
  if (tmp_3_748) {
    fatal_error(InvalidBlockAccessList);
  }
  Bytes values = bal_ref_cursor(markers);
  struct RlpFieldRef marker = rlp_decode_item(values);
  bal_expect_end((rlp_cursor_advance(values, marker.source.len)));
  bal_compare_index_word(marker, UINT8_C(0), ZERO_WORD);
  bytes32 keccak256_word_result_2_376 = host_keccak_word(slot);
  stateless_storage_by_key(storage_root, keccak256_word_result_2_376);
  validate_historical_schema_slots(next, slot, true, storage_root);
}

void validate_historical_schema_accounts(Bytes cursor, u256 previous, bool have_previous)
{
  if (cursor.len == UINT8_C(0)) {
    return;
  }
  struct RlpFieldRef entry = rlp_decode_item(cursor);
  Bytes next = rlp_cursor_advance(cursor, entry.source.len);
  Bytes fields = bal_ref_cursor(entry);
  struct RlpFieldRef address_field = rlp_decode_item(fields);
  Bytes fields_1_0 = rlp_cursor_advance(fields, address_field.source.len);
  struct RlpFieldRef slots_field = rlp_decode_item(fields_1_0);
  Bytes fields_1_1 = rlp_cursor_advance(fields_1_0, slots_field.source.len);
  struct RlpFieldRef reads = rlp_decode_item(fields_1_1);
  Bytes fields_1_2 = rlp_cursor_advance(fields_1_1, reads.source.len);
  struct RlpFieldRef balances = rlp_decode_item(fields_1_2);
  Bytes fields_1_3 = rlp_cursor_advance(fields_1_2, balances.source.len);
  struct RlpFieldRef nonces = rlp_decode_item(fields_1_3);
  Bytes fields_1_4 = rlp_cursor_advance(fields_1_3, nonces.source.len);
  struct RlpFieldRef codes = rlp_decode_item(fields_1_4);
  bal_expect_end((rlp_cursor_advance(fields_1_4, codes.source.len)));
  Bytes address_bytes = bal_ref_bytes(address_field);
  u256 address = rlp_decode_word(address_field);
  if (address_bytes.len != UINT8_C(20)) {
    fatal_error(InvalidBlockAccessList);
  }
  bool tmp_3_738;
  if (have_previous) {
    tmp_3_738 = (bool)(!(word_ult(previous, address)));
  } else {
    tmp_3_738 = false;
  }
  if (tmp_3_738) {
    fatal_error(InvalidBlockAccessList);
  }
  struct Account account;
  bytes32 result_2_353;
  bytes20 word_to_address_result_2_352 = word_to_address(address);
  result_2_353 = host_keccak_address(word_to_address_result_2_352);
  account = stateless_account_by_key(k_parent_state_root, result_2_353);
  Bytes bal_ref_cursor_result_2_346 = bal_ref_cursor(slots_field);
  validate_historical_schema_slots(bal_ref_cursor_result_2_346, ZERO_WORD, false, account.info.storage_root);
  Bytes bal_ref_cursor_result_2_348 = bal_ref_cursor(reads);
  bal_expect_end(bal_ref_cursor_result_2_348);
  Bytes bal_ref_cursor_result_2_349 = bal_ref_cursor(balances);
  bal_expect_end(bal_ref_cursor_result_2_349);
  Bytes bal_ref_cursor_result_2_350 = bal_ref_cursor(nonces);
  bal_expect_end(bal_ref_cursor_result_2_350);
  Bytes bal_ref_cursor_result_2_351 = bal_ref_cursor(codes);
  bal_expect_end(bal_ref_cursor_result_2_351);
  validate_historical_schema_accounts(next, address, true);
}

void validate_historical_access_schema(Bytes bytes)
{
  struct RlpFieldRef root = rlp_single_ref(bytes);
  Bytes bal_ref_cursor_result_2_336 = bal_ref_cursor(root);
  validate_historical_schema_accounts(bal_ref_cursor_result_2_336, ZERO_WORD, false);
}

void validate_block_access_list_(Bytes bytes, uint64_t block_gas_limit)
{
  validate_block_access_list(bytes, block_gas_limit);
}

void bal_compare_index_word(struct RlpFieldRef pair, uint8_t index, u256 value)
{
  Bytes fields = bal_ref_cursor(pair);
  struct RlpFieldRef index_field = rlp_decode_item(fields);
  Bytes fields_1_6 = rlp_cursor_advance(fields, index_field.source.len);
  struct RlpFieldRef value_field = rlp_decode_item(fields_1_6);
  bal_expect_end((rlp_cursor_advance(fields_1_6, value_field.source.len)));
  uint64_t decoded_index = bal_ref_uint64(index_field);
  u256 decoded_value = bal_ref_word(value_field);
  if ((decoded_index != index) || (!eq_u256(decoded_value, value))) {
    fatal_error(InvalidBlockAccessList);
  }
}

