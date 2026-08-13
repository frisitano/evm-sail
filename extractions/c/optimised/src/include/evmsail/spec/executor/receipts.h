#pragma once

#include "evmsail/spec/lib/rlp/codecs/receipts.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

uint32_t receipt_store_begin(void);

void receipt_store_append(struct ReceiptFields receipt, u128 cumulative_gas_used, uint32_t index);

bytes32 receipt_store_root(uint32_t count);

bool block_logs_bloom_matches_(Bytes reference);


#ifdef __cplusplus
}
#endif
