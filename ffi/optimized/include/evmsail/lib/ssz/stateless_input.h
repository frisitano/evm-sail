#ifndef EVMSAIL_OPTIMIZED_LIB_SSZ_STATELESS_INPUT_H
#define EVMSAIL_OPTIMIZED_LIB_SSZ_STATELESS_INPUT_H

#include "evmsail/prelude.h"

Hash32 sha256_request_digest(uint64_t request_type, struct StatelessInputSliceFields request);

#endif
