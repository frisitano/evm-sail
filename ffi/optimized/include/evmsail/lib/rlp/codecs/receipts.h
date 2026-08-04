#ifndef EVMSAIL_OPTIMIZED_LIB_RLP_CODECS_RECEIPTS_H
#define EVMSAIL_OPTIMIZED_LIB_RLP_CODECS_RECEIPTS_H

#include "evmsail/prelude.h"

struct ScratchRegionResult scratch_store_receipt_logs_bloom(
    uint32_t off, uint64_t start, uint64_t count);

#endif
