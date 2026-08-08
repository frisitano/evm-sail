#pragma once

#include "evmsail/spec/lib/rlp/codecs/withdrawals.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

uint32_t topics_rlp_content_size(uint64_t index);

uint32_t topics_rlp_size(uint64_t index);

uint32_t log_entry_rlp_content_size(uint64_t index);

uint32_t log_entry_rlp_size(uint64_t index);

uint32_t logs_rlp_content_size(struct LogSeriesRef logs);

uint32_t logs_rlp_size(struct LogSeriesRef logs);

void rlp_write_topics(uint64_t index);

void rlp_write_log_entry(uint64_t index);

void rlp_write_logs(struct LogSeriesRef logs);

void receipt_write_logs_bloom(struct ReceiptFields receipt);

uint32_t receipt_payload_content_size(struct ReceiptFields r, u128 cumulative_gas_used);

uint32_t receipt_encoded_length(struct ReceiptFields r, u128 cumulative_gas_used);

void receipt_write_encoded(struct ReceiptFields r, u128 cumulative_gas_used);

Bytes receipt_encoded(struct ReceiptFields r, u128 cumulative_gas_used);


#ifdef __cplusplus
}
#endif
