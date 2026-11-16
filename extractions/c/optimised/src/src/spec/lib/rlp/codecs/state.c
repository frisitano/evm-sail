#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

Bytes encode_state_account(struct AccountInfo info, bytes32 storage_root)
{
  uint8_t nonce_length = rlp_uint_word_size_uint64_t_to_uint8_t(info.nonce);
  uint8_t content_len = ((uint8_t)((uint32_t)UINT8_C(33) + ((uint32_t)UINT8_C(33) + ((uint32_t)(rlp_uint_word_size(info.balance)) + (uint32_t)nonce_length))));
  uint8_t encoded_length = rlp_list_size_uint8_t_to_uint8_t_variant_3(content_len);
  struct RlpEncoder encoder = rlp_encoder_begin_uint8_t_to_struct_RlpEncoder(encoded_length);
  rlp_write_list_prefix_uint8_t_to_unit_variant_3(content_len);
  rlp_write_uint_word_uint64_t_to_unit(info.nonce);
  rlp_write_uint_word(info.balance);
  rlp_write_word((hash_to_word(storage_root)));
  rlp_write_word((hash_to_word(info.code_hash)));
  return rlp_encoder_finish(encoder);
}

Bytes encode_storage_value(u256 value)
{
  uint8_t encoded_len = rlp_uint_word_size(value);
  struct RlpEncoder encoder = rlp_encoder_begin_uint8_t_to_struct_RlpEncoder(encoded_len);
  rlp_write_uint_word(value);
  return rlp_encoder_finish(encoder);
}

struct AccountInfo decode_state_account(Bytes value)
{
  Bytes fields = rlp_node_cursor(value);
  struct RlpFieldRef nonce = rlp_decode_item(fields);
  Bytes fields_1_9 = rlp_cursor_advance(fields, nonce.source.len);
  struct RlpFieldRef balance = rlp_decode_item(fields_1_9);
  Bytes fields_1_10 = rlp_cursor_advance(fields_1_9, balance.source.len);
  struct RlpFieldRef storage = rlp_decode_item(fields_1_10);
  Bytes fields_1_11 = rlp_cursor_advance(fields_1_10, storage.source.len);
  struct RlpFieldRef code = rlp_decode_item(fields_1_11);
  rlp_cursor_expect_end((rlp_cursor_advance(fields_1_11, code.source.len)));
  bytes32 storage_root;
  if (storage.content_len == UINT8_C(0)) {
    storage_root = EMPTY_TRIE_ROOT;
  } else {
    u256 storage_word = rlp_decode_word(storage);
    storage_root = word_to_hash(storage_word);
  }
  bytes32 code_hash;
  if (code.content_len == UINT8_C(0)) {
    code_hash = KECCAK_EMPTY;
  } else {
    u256 code_word = rlp_decode_word(code);
    code_hash = word_to_hash(code_word);
  }
  uint64_t rlp_decode_uint64_result_2_627 = rlp_decode_uint64(nonce);
  return ((struct AccountInfo){.balance = (rlp_decode_u256(balance)), .code_hash = code_hash, .nonce = rlp_decode_uint64_result_2_627, .storage_root = storage_root});
}

