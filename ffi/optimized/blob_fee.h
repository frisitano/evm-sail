#ifndef EVMSAIL_OPTIMIZED_BLOB_FEE_H
#define EVMSAIL_OPTIMIZED_BLOB_FEE_H

#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H
#include <stdbool.h>

/*
 * Optimized EIP-4844 blob-base-fee calculation. The exact recurrence is
 * evaluated up to the specification's native EVM-word observation boundary.
 */
sail_u256 evmsail_blob_base_fee_word(uint64_t excess_blob_gas);

bool evmsail_blob_reserve_price_for_excess(
    sail_u256 base_fee, uint64_t excess_blob_gas);

#endif
