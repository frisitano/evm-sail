/* Backend-internal receipt-record table surface.
 *
 * Receipt accumulation owns encoded bytes; the MPT builder borrows records
 * through this narrow iterator and owns only ordering and trie construction. */
#ifndef EVMSAIL_OPTIMIZED_EXECUTOR_RECEIPTS_H
#define EVMSAIL_OPTIMIZED_EXECUTOR_RECEIPTS_H

#include <stdbool.h>
#include <stdint.h>

void receipts_workspace_bind(void);
uint64_t receipt_record_count(void);
bool receipt_record_span(uint64_t index, const uint8_t **bytes,
                         uint64_t *length);

#endif
