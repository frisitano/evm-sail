/* Optimized block primitive boundaries. */
#include "evmsail/prelude.h"

#include "evmsail/host/region_access.h"
#include "evmsail/primitives/block.h"

bool logs_bloom_matches_ref(
    LogsBloom computed, struct zStatelessInputSliceFields reference) {
  return region_logs_bloom_matches_ref(computed, reference);
}
