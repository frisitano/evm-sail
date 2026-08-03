/* Optimized receipt-RLP bloom writer boundary. */
#include "evmsail/prelude.h"

#include "evmsail/host/region_access.h"
#include "evmsail/lib/rlp/codecs/receipts.h"

struct ScratchRegionResult scratch_store_receipt_logs_bloom(
    uint32_t off, uint64_t start, uint64_t count) {
  struct ScratchRegionResult result = {0};
  region_scratch_store_receipt_logs_bloom(&result, off, start, count);
  return result;
}
