#ifndef EVMSAIL_BLOB_FEE_GLUE_H
#define EVMSAIL_BLOB_FEE_GLUE_H

#include "sail_abi.h"

/*
 * Optimized EIP-4844 blob-base-fee calculation. The optimized ABI follows
 * Reth/Revm's u128 block-context representation; the Sail wrapper widens the
 * result to the specification's EVM word.
 */
sail_u128 evmsail_blob_base_fee(uint64_t excess_blob_gas);

#endif
