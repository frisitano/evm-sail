#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

uint32_t receipt_store_begin(void)
{
  receipt_table_reset();
  return UINT32_C(0);
}

void receipt_store_append(struct ReceiptFields receipt, u128 cumulative_gas_used, uint32_t index)
{
  uint32_t mark = scratch_begin();
  Bytes value = receipt_encoded(receipt, cumulative_gas_used);
  receipt_table_push(index, value);
  scratch_rewind(mark);
}

bytes32 receipt_store_root(uint32_t count)
{
  return receipt_table_root(count);
}

bool block_logs_bloom_matches_(Bytes reference)
{
  return block_logs_bloom_matches(reference);
}

