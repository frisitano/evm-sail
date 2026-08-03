/* Optimized receipt-RLP bloom writer boundary. */
#include "evmsail/prelude.h"

#include "evmsail/host/region_access.h"
#include "evmsail/lib/rlp/codecs/receipts.h"

void scratch_store_receipt_logs_bloom(struct zScratchRegionResult *result,
                                      uint64_t off, uint64_t start,
                                      uint64_t count) {
  region_scratch_store_receipt_logs_bloom(result, off, start, count);
}
