#ifndef EVMSAIL_OPTIMIZED_PRIMITIVES_BLOCK_H
#define EVMSAIL_OPTIMIZED_PRIMITIVES_BLOCK_H

#include "evmsail/prelude.h"

bool logs_bloom_matches_ref(bytes256 computed, Bytes reference);

#endif
