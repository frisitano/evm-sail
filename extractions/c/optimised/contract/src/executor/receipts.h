/* Backend-internal receipt-record table surface.
 *
 * Receipt accumulation owns encoded bytes; the MPT builder borrows records
 * through this narrow iterator and owns only ordering and trie construction. */
#ifndef EVMSAIL_OPTIMIZED_EXECUTOR_RECEIPTS_INTERNAL_H
#define EVMSAIL_OPTIMIZED_EXECUTOR_RECEIPTS_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void receipt_records_workspace_bind(uint32_t transaction_count);
void receipt_bytes_workspace_bind(size_t capacity);
uint64_t receipt_record_count(void);
bool receipt_record_span(uint64_t index, const uint8_t **bytes, uint64_t *length);

#endif
