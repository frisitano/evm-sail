/* Optimized LOG series and receipt-bloom accumulator.
 *
 * Log records borrow ranges from block-lifetime topic and data arenas. Frame
 * rollback removes appended records through the shared state journal;
 * transaction reset merely starts a new receipt-visible view. */
#include "host/state/logs.h"

#include "evmsail/exceptions.h"
#include "host/state/journal.h"
#include "primitives/hash.h"
#include "primitives/value.h"
#include "workspace.h"
#include "zkvm_accelerators.h"

#include <stdint.h>
#include <string.h>

typedef struct {
  Address address;
  uint32_t topic_offset;
  uint32_t topic_count;
  uint32_t data_offset;
  uint32_t data_length;
} LogRecord;

typedef struct {
  LogRecord *records;
  U256 *topics;
  uint8_t *data;
  uint32_t record_count;
  uint32_t topic_count;
  uint32_t data_length;
  uint32_t transaction_start;
} LogTable;

static LogTable log_table;
static uint64_t block_logs_bloom_words[32];

void state_logs_workspace_bind(void) {
  WORKSPACE_BIND(log_table.records, GUEST_LOG_RECORDS);
  WORKSPACE_BIND(log_table.topics, GUEST_LOG_TOPICS);
  WORKSPACE_BIND(log_table.data, GUEST_LOG_DATA_BYTES);
}

unit logs_reset(unit ignored) {
  (void)ignored;
  log_table.record_count = 0;
  log_table.topic_count = 0;
  log_table.data_length = 0;
  log_table.transaction_start = 0;
  memset(block_logs_bloom_words, 0, sizeof(block_logs_bloom_words));
  return UNIT;
}

unit logs_tx_reset(unit ignored) {
  (void)ignored;
  log_table.transaction_start = log_table.record_count;
  return UNIT;
}

uint64_t logs_tx_start(unit ignored) {
  (void)ignored;
  return log_table.transaction_start;
}

uint64_t logs_tx_count(unit ignored) {
  (void)ignored;
  return log_table.record_count - log_table.transaction_start;
}

unit log_begin(Address address) {
  if (log_table.record_count == GUEST_LOG_RECORDS) GUEST_ABORT();
  state_journal_push_log_appended();
  LogRecord *record = &log_table.records[log_table.record_count++];
  record->address = address;
  record->topic_offset = log_table.topic_count;
  record->topic_count = 0;
  record->data_offset = log_table.data_length;
  record->data_length = 0;
  return UNIT;
}

unit log_add_topic(U256 topic) {
  if (log_table.record_count == 0 ||
      log_table.topic_count == GUEST_LOG_TOPICS)
    GUEST_ABORT();
  log_table.topics[log_table.topic_count++] = topic;
  log_table.records[log_table.record_count - 1].topic_count++;
  return UNIT;
}

unit log_add_data_bulk(const uint8_t *bytes, uint64_t length) {
  if (log_table.record_count == 0 ||
      length > GUEST_LOG_DATA_BYTES - log_table.data_length)
    GUEST_ABORT();
  if (length == 0) return UNIT;
  memcpy(log_table.data + log_table.data_length, bytes, (size_t)length);
  log_table.data_length += (uint32_t)length;
  log_table.records[log_table.record_count - 1].data_length +=
      (uint32_t)length;
  return UNIT;
}

unit log_add_data_word(U256 value) {
  uint8_t bytes[32];
  sail_word_to_be_bytes(bytes, value);
  return log_add_data_bulk(bytes, sizeof(bytes));
}

void logs_revert_last(void) {
  if (log_table.record_count == 0) GUEST_ABORT();
  const LogRecord *record = &log_table.records[--log_table.record_count];
  log_table.topic_count = record->topic_offset;
  log_table.data_length = record->data_offset;
}

uint64_t log_count(unit ignored) {
  (void)ignored;
  return log_table.record_count;
}

Address log_addr(uint64_t index) {
  static const Address zero = {{0}};
  return index < log_table.record_count ? log_table.records[index].address
                                        : zero;
}

uint64_t log_topic_count(uint64_t index) {
  return index < log_table.record_count ? log_table.records[index].topic_count
                                        : 0;
}

U256 log_topic(uint64_t log_index, uint64_t topic_index) {
  static const U256 zero = {{0}};
  if (log_index >= log_table.record_count ||
      topic_index >= log_table.records[log_index].topic_count)
    return zero;
  return log_table.topics[log_table.records[log_index].topic_offset +
                          topic_index];
}

static void bloom_write_digest(uint8_t out[256],
                               const zkvm_keccak256_hash *digest) {
  for (size_t i = 0; i < 3; ++i) {
    const uint16_t bit =
        (uint16_t)(((uint16_t)(digest->data[2 * i] & 0x07) << 8) |
                   digest->data[2 * i + 1]);
    out[255 - (bit >> 3)] |= (uint8_t)(UINT8_C(1) << (bit & 7));
    block_logs_bloom_words[bit >> 6] |= UINT64_C(1) << (bit & 63);
  }
}

static bool bloom_write_bytes(uint8_t out[256], const uint8_t *bytes,
                              size_t length) {
  zkvm_keccak256_hash digest = {{0}};
  if (zkvm_keccak256(bytes, length, &digest) != ZKVM_EOK) return false;
  bloom_write_digest(out, &digest);
  return true;
}

unit receipt_runtime_block_bloom_reset(unit ignored) {
  (void)ignored;
  memset(block_logs_bloom_words, 0, sizeof(block_logs_bloom_words));
  return UNIT;
}

LogsBloom receipt_runtime_block_bloom(unit ignored) {
  (void)ignored;
  LogsBloom bloom;
  memcpy(bloom.bytes, block_logs_bloom_words, sizeof(bloom.bytes));
  return bloom;
}

bool receipt_runtime_bloom_write(uint64_t start, uint64_t count,
                                 uint8_t out[256]) {
  if (start > log_table.record_count ||
      count > log_table.record_count - start)
    return false;

  memset(out, 0, 256);
  for (uint64_t offset = 0; offset < count; ++offset) {
    const LogRecord *record = &log_table.records[start + offset];
    if (!bloom_write_bytes(out, address_bytes_const(&record->address), 20))
      return false;
    for (uint32_t topic = 0; topic < record->topic_count; ++topic) {
      uint8_t bytes[32];
      sail_word_to_be_bytes(
          bytes, log_table.topics[record->topic_offset + topic]);
      if (!bloom_write_bytes(out, bytes, sizeof(bytes))) return false;
    }
  }
  return true;
}

uint32_t log_data_len(uint64_t index) {
  return index < log_table.record_count ? log_table.records[index].data_length
                                        : 0;
}

uint32_t log_data_off(uint64_t index) {
  if (index >= log_table.record_count ||
      log_table.records[index].data_length == 0)
    return 0;
  return log_table.records[index].data_offset;
}

const uint8_t *log_data_region(uint64_t offset, uint64_t length) {
  static const uint8_t empty = 0;
  if (offset > log_table.data_length ||
      length > log_table.data_length - offset)
    return NULL;
  return length ? log_table.data + offset : &empty;
}

const uint8_t *log_data_base(void) { return log_table.data; }
