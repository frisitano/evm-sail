#ifndef EVMSAIL_OPTIMIZED_PRIMITIVES_BLOCK_H
#define EVMSAIL_OPTIMIZED_PRIMITIVES_BLOCK_H

#include "evmsail/prelude.h"

bool logs_bloom_matches_ref(
    LogsBloom computed, struct StatelessInputSliceFields reference);

#endif
