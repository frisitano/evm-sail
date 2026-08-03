/* Hash host operations implemented by this backend's hash.c. */
#ifndef EVMSAIL_OPTIMIZED_PRIMITIVES_CRYPTO_H
#define EVMSAIL_OPTIMIZED_PRIMITIVES_CRYPTO_H

#include "evmsail/prelude.h"
#include <stdbool.h>
#include <stdint.h>

struct zStatelessInputSliceFields;
struct zScratchSliceFields;
struct zEvmMemorySliceFields;
struct zCodeRegionSliceFields;
struct zOutputSliceFields;
struct zLogDataSliceFields;

Hash32 host_keccak_stateless_input(
    struct zStatelessInputSliceFields input);
Hash32 host_keccak_scratch(struct zScratchSliceFields input);
Hash32 host_keccak_memory(struct zEvmMemorySliceFields input);
Hash32 host_keccak_code(struct zCodeRegionSliceFields input);
Hash32 host_keccak_output(struct zOutputSliceFields input);
Hash32 host_keccak_log_data(struct zLogDataSliceFields input);
Hash32 host_sha256_stateless_input(
    struct zStatelessInputSliceFields input);
Hash32 host_sha256_scratch(struct zScratchSliceFields input);
Hash32 host_sha256_memory(struct zEvmMemorySliceFields input);
Hash32 host_keccak_word(const U256 input);
Hash32 host_keccak_address(Address input);
Hash32 host_sha256_pair(
    Hash32 left, Hash32 right);

#endif
