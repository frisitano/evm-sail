#ifndef EVMSAIL_OPTIMIZED_EXECUTOR_RECEIPTS_H
#define EVMSAIL_OPTIMIZED_EXECUTOR_RECEIPTS_H

#include "evmsail/prelude.h"

unit receipt_table_reset(unit ignored);
unit receipt_table_push(uint64_t index,
                                struct zScratchSliceFields value);
Hash32 receipt_table_root(uint64_t count);
unit block_logs_bloom_reset(unit ignored);
LogsBloom block_logs_bloom(unit ignored);

#endif
