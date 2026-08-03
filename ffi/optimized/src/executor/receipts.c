/* Optimized receipt accumulation and block-bloom ABI.
 *
 * Encoded receipts are copied from temporary executor scratch into one
 * block-lifetime table. The MPT module consumes them through receipts.h and
 * remains concerned only with ordering and trie construction. */
#include "evmsail/prelude.h"

#include "evmsail/exceptions.h"
#include "evmsail/executor/receipts.h"
#include "evmsail/host/scratch.h"
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
} ReceiptTable;

static ReceiptTable receipt_table;

void receipts_workspace_bind(void) {
  WORKSPACE_BIND(receipt_table.records, GUEST_RECEIPT_RECORDS);
  WORKSPACE_BIND(receipt_table.bytes, GUEST_RECEIPT_BYTES);
}

unit receipt_table_reset(unit ignored) {
  mpt_reset(ignored);
  receipt_table.count = 0;
  receipt_table.bytes_length = 0;
  return UNIT;
}

unit receipt_table_push(uint32_t index, struct ScratchSliceFields value) {
  const uint64_t offset = value.off;
  const uint64_t length = value.len;
  const uint8_t *source = NULL;
  if (index != receipt_table.count || length > SIZE_MAX ||
      !(source = scratch_region(offset, length)) ||
      receipt_table.bytes_length > SIZE_MAX - (size_t)length ||
      receipt_table.count == GUEST_RECEIPT_RECORDS ||
      (size_t)length > GUEST_RECEIPT_BYTES - receipt_table.bytes_length) {
    throw_invalid_block(WitnessDeficient,
                        "optimized receipt accumulator push");
    return UNIT;
  }

  ReceiptRecord *record = &receipt_table.records[receipt_table.count];
  record->value_offset = receipt_table.bytes_length;
  record->value_length = (size_t)length;
  memmove(receipt_table.bytes + receipt_table.bytes_length, source,
          (size_t)length);
  receipt_table.bytes_length += (size_t)length;
  receipt_table.count++;
  return UNIT;
}

uint64_t receipt_record_count(void) { return receipt_table.count; }

bool receipt_record_span(uint64_t index, const uint8_t **bytes,
                         uint64_t *length) {
  if (index >= receipt_table.count) return false;
  const ReceiptRecord *record = &receipt_table.records[index];
  if (record->value_offset > receipt_table.bytes_length ||
      record->value_length > receipt_table.bytes_length - record->value_offset)
    return false;
  *bytes = receipt_table.bytes + record->value_offset;
  *length = record->value_length;
  return true;
}

Hash32 receipt_table_root(uint32_t count) {
  return mpt_receipt_table_root(count);
}

unit block_logs_bloom_reset(unit ignored) {
  return receipt_runtime_block_bloom_reset(ignored);
}

LogsBloom block_logs_bloom(unit ignored) {
  return receipt_runtime_block_bloom(ignored);
}
