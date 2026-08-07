/* Optimized receipt accumulation and block-bloom ABI.
 *
 * Encoded receipts are copied from temporary executor scratch into one
 * block-lifetime table. The MPT module consumes them through receipts.h and
 * remains concerned only with ordering and trie construction. */
#include "evmsail/host/types.h"
#include "evmsail/prelude.h"

#include "evmsail/spec/exceptions.h"
#include "executor/receipts.h"
#include "host/state/logs.h"
#include "lib/mpt/trie.h"
#include "workspace.h"

#include <stdint.h>
#include <string.h>

typedef struct {
  size_t value_offset;
  size_t value_length;
} ReceiptRecord;

typedef struct {
  ReceiptRecord *records;
  uint8_t *bytes;
  size_t count;
  size_t bytes_length;
  size_t record_capacity;
  size_t bytes_capacity;
} ReceiptTable;

static ReceiptTable receipt_table;

void receipt_records_workspace_bind(uint32_t transaction_count)
{
  receipt_table.record_capacity = (size_t)transaction_count;
  WORKSPACE_BIND(receipt_table.records, receipt_table.record_capacity);
}

void receipt_bytes_workspace_bind(size_t capacity)
{
  receipt_table.bytes_capacity = capacity;
  WORKSPACE_BIND(receipt_table.bytes, capacity);
}

void receipt_table_reset(void)
{
  mpt_reset();
  receipt_runtime_block_bloom_reset();
  receipt_table.count = 0;
  receipt_table.bytes_length = 0;
}

void receipt_table_push(uint32_t index, Bytes value)
{
  const uint64_t length = value.len;
  const uint8_t *source = value.bytes;
  if (index != receipt_table.count || length > SIZE_MAX || !source ||
      receipt_table.bytes_length > SIZE_MAX - (size_t)length ||
      receipt_table.count >= receipt_table.record_capacity ||
      (size_t)length > receipt_table.bytes_capacity - receipt_table.bytes_length) {
    fatal_error(WitnessDeficient);
  }

  ReceiptRecord *record = &receipt_table.records[receipt_table.count];
  record->value_offset = receipt_table.bytes_length;
  record->value_length = (size_t)length;
  memmove(receipt_table.bytes + receipt_table.bytes_length, source, (size_t)length);
  receipt_table.bytes_length += (size_t)length;
  receipt_table.count++;
}

uint64_t receipt_record_count(void)
{
  return receipt_table.count;
}

bool receipt_record_span(uint64_t index, const uint8_t **bytes, uint64_t *length)
{
  if (index >= receipt_table.count) {
    return false;
  }
  const ReceiptRecord *record = &receipt_table.records[index];
  *bytes = receipt_table.bytes + record->value_offset;
  *length = record->value_length;
  return true;
}

bytes32 receipt_table_root(uint32_t count)
{
  return mpt_receipt_table_root(count);
}

bool block_logs_bloom_matches(Bytes reference)
{
  return receipt_runtime_block_bloom_matches(reference.bytes, reference.len);
}
