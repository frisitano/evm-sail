/* Optimized receipt-RLP bloom writer boundary. */

#include "evmsail/host/region_access.h"
#include <stdint.h>
struct ScratchSliceFields scratch_store_receipt_logs_bloom(uint32_t off, uint64_t start,
                                                           uint64_t count)
{
  return region_scratch_store_receipt_logs_bloom(off, start, count);
}
