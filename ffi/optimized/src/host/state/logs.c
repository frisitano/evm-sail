/* Optimized LOG series and receipt-bloom accumulator.
 *
 * Log records borrow ranges from block-lifetime topic and data arenas. Frame
 * rollback removes appended records through the shared state journal;
 * transaction reset merely starts a new receipt-visible view. */
#include "host/state/logs.h"

#include "evmsail/prelude.h"
#include "evmsail/host/types.h"
#include "host/state/journal.h"
#include "primitives/value.h"
#include "workspace.h"
#include "zkvm_accelerators.h"

#include <stdint.h>
#include <string.h>

typedef struct {
  bytes20 address;
  uint32_t topic_offset;
  uint32_t topic_count;
  uint32_t data_offset;
  uint32_t data_length;
} LogRecord;

typedef struct {
  LogRecord *records;
  u256 *topics;
  uint8_t *data;
  uint32_t record_count;
  uint32_t topic_count;
  uint32_t data_length;
  uint32_t transaction_start;
  uint32_t record_capacity;
  uint32_t topic_capacity;
  uint32_t data_capacity;
} LogTable;

static LogTable log_table;
static uint64_t block_logs_bloom_words[32];

void state_logs_workspace_bind(uint32_t record_capacity, uint32_t topic_capacity,
                               uint32_t data_capacity)
{
  log_table.record_capacity = record_capacity;
  log_table.topic_capacity = topic_capacity;
  log_table.data_capacity = data_capacity;
  WORKSPACE_BIND(log_table.records, record_capacity);
  WORKSPACE_BIND(log_table.topics, topic_capacity);
  WORKSPACE_BIND(log_table.data, data_capacity);
}

void logs_reset(void)
{
  log_table.record_count = 0;
  log_table.topic_count = 0;
  log_table.data_length = 0;
  log_table.transaction_start = 0;
  memset(block_logs_bloom_words, 0, sizeof(block_logs_bloom_words));
}
void logs_tx_reset(void)
{
  log_table.transaction_start = log_table.record_count;
}

uint64_t logs_tx_start(void)
{
  return log_table.transaction_start;
}

uint64_t logs_tx_count(void)
{
  return log_table.record_count - log_table.transaction_start;
}

void log_begin(bytes20 address)
{
  if (log_table.record_count >= log_table.record_capacity) {
    GUEST_ABORT();
  }
  state_journal_push_log_appended();
  LogRecord *record = &log_table.records[log_table.record_count++];
  record->address = address;
  record->topic_offset = log_table.topic_count;
  record->topic_count = 0;
  record->data_offset = log_table.data_length;
  record->data_length = 0;
}

void log_add_topic(u256 topic)
{
  if (log_table.record_count == 0 || log_table.topic_count >= log_table.topic_capacity) {
    GUEST_ABORT();
  }
  log_table.topics[log_table.topic_count++] = topic;
  log_table.records[log_table.record_count - 1].topic_count++;
}

void log_add_data_bulk(const uint8_t *bytes, uint64_t length)
{
  if (log_table.record_count == 0 || length > log_table.data_capacity - log_table.data_length) {
    GUEST_ABORT();
  }
  if (length == 0) {
    return;
  }
  memcpy(log_table.data + log_table.data_length, bytes, (size_t)length);
  log_table.data_length += (uint32_t)length;
  log_table.records[log_table.record_count - 1].data_length += (uint32_t)length;
}

void log_add_data_word(u256 value)
{
  uint8_t bytes[32];
  sail_word_to_be_bytes(bytes, value);
  log_add_data_bulk(bytes, sizeof(bytes));
}

/* LOG payloads are copied from active-frame memory into the log-data arena. */
void log_add_data_memory(Bytes data)
{
  const uint64_t len = data.len;
  const uint8_t *p = data.bytes;
  if (p) {
    log_add_data_bulk(p, len);
  }
}

void logs_revert_last(void)
{
  if (log_table.record_count == 0) {
    GUEST_ABORT();
  }
  const LogRecord *record = &log_table.records[--log_table.record_count];
  log_table.topic_count = record->topic_offset;
  log_table.data_length = record->data_offset;
}

uint64_t log_count(void)
{
  return log_table.record_count;
}

bytes20 log_addr(uint64_t index)
{
  return log_table.records[index].address;
}

uint64_t log_topic_count(uint64_t index)
{
  return log_table.records[index].topic_count;
}

u256 log_topic(uint64_t log_index, uint64_t topic_index)
{
  return log_table.topics[log_table.records[log_index].topic_offset + topic_index];
}

static void bloom_write_digest(uint8_t out[256], const zkvm_keccak256_hash *digest)
{
  for (size_t i = 0; i < 3; ++i) {
    const uint16_t bit =
        (uint16_t)(((uint16_t)(digest->data[2 * i] & 0x07) << 8) | digest->data[(2 * i) + 1]);
    out[255 - (bit >> 3)] |= (uint8_t)(UINT8_C(1) << (bit & 7));
    block_logs_bloom_words[bit >> 6] |= UINT64_C(1) << (bit & 63);
  }
}

static bool bloom_write_bytes(uint8_t out[256], const uint8_t *bytes, size_t length)
{
  zkvm_keccak256_hash digest = {{0}};
  if (zkvm_keccak256(bytes, length, &digest) != ZKVM_EOK) {
    return false;
  }
  bloom_write_digest(out, &digest);
  return true;
}

void receipt_runtime_block_bloom_reset(void)
{
  memset(block_logs_bloom_words, 0, sizeof(block_logs_bloom_words));
}

bool receipt_runtime_block_bloom_matches(const uint8_t *reference, uint64_t length)
{
  if (length != sizeof(block_logs_bloom_words) || !reference) {
    return false;
  }
  for (size_t word = 0; word < sizeof(block_logs_bloom_words) / sizeof(uint64_t); ++word) {
    uint64_t reference_word = 0;
    memcpy(&reference_word,
           reference + sizeof(block_logs_bloom_words) - ((word + 1) * sizeof(uint64_t)),
           sizeof(reference_word));
    if (block_logs_bloom_words[word] != __builtin_bswap64(reference_word)) {
      return false;
    }
  }
  return true;
}

bool receipt_runtime_bloom_write(uint64_t start, uint64_t count, uint8_t out[256])
{
  memset(out, 0, 256);
  for (uint64_t offset = 0; offset < count; ++offset) {
    const LogRecord *record = &log_table.records[start + offset];
    if (!bloom_write_bytes(out, bytes20_data(&record->address), 20)) {
      return false;
    }
    for (uint32_t topic = 0; topic < record->topic_count; ++topic) {
      uint8_t bytes[32];
      sail_word_to_be_bytes(bytes, log_table.topics[record->topic_offset + topic]);
      if (!bloom_write_bytes(out, bytes, sizeof(bytes))) {
        return false;
      }
    }
  }
  return true;
}

uint32_t log_data_len(uint64_t index)
{
  return log_table.records[index].data_length;
}

uint32_t log_data_off(uint64_t index)
{
  return log_table.records[index].data_offset;
}

Bytes host_log_data_slice(uint64_t index)
{
  if (index >= log_table.record_count) {
    GUEST_ABORT();
  }
  const LogRecord *record = &log_table.records[index];
  if (record->data_offset > log_table.data_length ||
      record->data_length > log_table.data_length - record->data_offset) {
    GUEST_ABORT();
  }
  return (Bytes){
      .bytes = record->data_length == 0 ? NULL : log_table.data + record->data_offset,
      .len = record->data_length,
  };
}

const uint8_t *log_data_base(void)
{
  return log_table.data;
}

uint32_t log_data_capacity(void)
{
  return log_table.data_capacity;
}
