#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

uint32_t topics_rlp_content_size(uint64_t index)
{
  uint32_t size = UINT32_C(0);
  uint64_t topic = UINT64_C(0);
  uint64_t topic_count = log_topic_count(index);
  while (topic < topic_count) {
    size = rlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t_variant_3(size, UINT32_C(33));
    topic = log_store_index_increment(topic);
  }
  return size;
}

uint32_t topics_rlp_size(uint64_t index)
{
  uint32_t content_size = topics_rlp_content_size(index);
  return rlp_scratch_list_size(content_size);
}

uint32_t log_entry_rlp_content_size(uint64_t index)
{
  uint32_t topics_length = topics_rlp_size(index);
  Bytes data = read_log_data(index);
  uint32_t data_length = rlp_log_scratch_slice_size(data);
  uint32_t address_and_topics_length = rlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t_variant_2(UINT32_C(21), topics_length);
  return rlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t_variant_3(address_and_topics_length, data_length);
}

uint32_t log_entry_rlp_size(uint64_t index)
{
  uint32_t content_size = log_entry_rlp_content_size(index);
  return rlp_scratch_list_size(content_size);
}

uint32_t logs_rlp_content_size(struct LogSeriesRef logs)
{
  uint32_t size = UINT32_C(0);
  uint64_t offset = UINT64_C(0);
  while (offset < logs.count) {
    uint64_t index = log_store_index_add(logs.start, offset);
    uint32_t entry_size = log_entry_rlp_size(index);
    size = rlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t_variant_3(size, entry_size);
    offset = log_store_index_increment(offset);
  }
  return size;
}

uint32_t logs_rlp_size(struct LogSeriesRef logs)
{
  uint32_t content_size = logs_rlp_content_size(logs);
  return rlp_scratch_list_size(content_size);
}

void rlp_write_topics(uint64_t index)
{
  uint32_t content_size = topics_rlp_content_size(index);
  rlp_write_list_prefix_uint32_t_to_unit(content_size);
  uint64_t topic = UINT64_C(0);
  uint64_t topic_count = log_topic_count(index);
  while (topic < topic_count) {
    u256 value = log_topic(index, topic);
    rlp_write_word(value);
    topic = log_store_index_increment(topic);
  }
}

void rlp_write_log_entry(uint64_t index)
{
  uint32_t content_size = log_entry_rlp_content_size(index);
  rlp_write_list_prefix_uint32_t_to_unit(content_size);
  bytes20 address = log_addr(index);
  rlp_write_addr(address);
  rlp_write_topics(index);
  Bytes data = read_log_data(index);
  rlp_write_log_data_slice(data);
}

void rlp_write_logs(struct LogSeriesRef logs)
{
  uint32_t content_size = logs_rlp_content_size(logs);
  rlp_write_list_prefix_uint32_t_to_unit(content_size);
  uint64_t offset = UINT64_C(0);
  while (offset < logs.count) {
    uint64_t index = log_store_index_add(logs.start, offset);
    rlp_write_log_entry(index);
    offset = log_store_index_increment(offset);
  }
}

void receipt_write_logs_bloom(struct ReceiptFields receipt)
{
  rlp_write_string_prefix_uint16_t_uint64_t_to_unit(LOGS_BLOOM_BYTE_LENGTH);
  Bytes arena = scratch_arena;
  scratch_arena = scratch_store_receipt_logs_bloom(arena.len, receipt.logs.start, receipt.logs.count);
}

uint32_t receipt_payload_content_size(struct ReceiptFields r, u128 cumulative_gas_used)
{
  uint8_t status = r.success ? UINT8_C(1) : UINT8_C(0);
  uint8_t status_size = rlp_uint_word_size_uint8_t_to_uint8_t(status);
  uint32_t status_length = rlp_scratch_small_length(status_size);
  u256 gas_word = word_of_nat_byte_count_u128_to_u256(cumulative_gas_used);
  uint8_t gas_size = rlp_uint_word_size(gas_word);
  uint32_t gas_length = rlp_scratch_small_length(gas_size);
  uint8_t bloom_prefix_size = rlp_length_prefix_len_uint16_t_to_uint8_t(LOGS_BLOOM_BYTE_LENGTH);
  uint32_t bloom_prefix_length = rlp_scratch_small_length(bloom_prefix_size);
  uint32_t bloom_length = rlp_scratch_length_add_uint16_t_uint32_t_to_uint32_t(LOGS_BLOOM_BYTE_LENGTH, bloom_prefix_length);
  uint32_t logs_length = logs_rlp_size(r.logs);
  uint32_t fixed_length = rlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t(status_length, gas_length);
  uint32_t fixed_and_bloom_length = rlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t_variant_3(fixed_length, bloom_length);
  return rlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t_variant_3(fixed_and_bloom_length, logs_length);
}

uint32_t receipt_encoded_length(struct ReceiptFields r, u128 cumulative_gas_used)
{
  uint32_t content_len = receipt_payload_content_size(r, cumulative_gas_used);
  uint64_t envelope_type = tx_envelope_type(r.tx_type);
  uint32_t payload_len = rlp_scratch_list_size(content_len);
  if (envelope_type != UINT64_C(0x00)) {
    return rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(payload_len, UINT8_C(1));
  }
  return payload_len;
}

void receipt_write_encoded(struct ReceiptFields r, u128 cumulative_gas_used)
{
  uint8_t status = r.success ? UINT8_C(1) : UINT8_C(0);
  uint32_t content_len = receipt_payload_content_size(r, cumulative_gas_used);
  uint64_t envelope_type = tx_envelope_type(r.tx_type);
  if (envelope_type != UINT64_C(0x00)) {
    scratch_push_byte(envelope_type);
  }
  rlp_write_list_prefix_uint32_t_to_unit(content_len);
  rlp_write_uint_word_uint8_t_to_unit(status);
  rlp_write_uint_word_u128_to_unit(cumulative_gas_used);
  receipt_write_logs_bloom(r);
  rlp_write_logs(r.logs);
}

Bytes receipt_encoded(struct ReceiptFields r, u128 cumulative_gas_used)
{
  uint32_t encoded_len = receipt_encoded_length(r, cumulative_gas_used);
  struct RlpEncoder encoder = rlp_encoder_begin(encoded_len);
  receipt_write_encoded(r, cumulative_gas_used);
  return rlp_encoder_finish(encoder);
}

