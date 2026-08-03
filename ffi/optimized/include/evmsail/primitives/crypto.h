/* Hash host operations implemented by this backend's hash.c. */
#ifndef EVMSAIL_OPTIMIZED_PRIMITIVES_CRYPTO_H
#define EVMSAIL_OPTIMIZED_PRIMITIVES_CRYPTO_H

#include "evmsail/prelude.h"
#include <stdbool.h>
#include <stdint.h>

struct StatelessInputSliceFields;
struct ScratchSliceFields;
struct EvmMemorySliceFields;
struct CodeRegionSliceFields;
struct OutputSliceFields;
struct LogDataSliceFields;

Hash32 host_keccak_stateless_input(
    struct StatelessInputSliceFields input);
Hash32 host_keccak_scratch(struct ScratchSliceFields input);
Hash32 host_keccak_memory(struct EvmMemorySliceFields input);
Hash32 host_keccak_code(struct CodeRegionSliceFields input);
Hash32 host_keccak_output(struct OutputSliceFields input);
Hash32 host_keccak_log_data(struct LogDataSliceFields input);
Hash32 host_sha256_stateless_input(
    struct StatelessInputSliceFields input);
Hash32 host_sha256_scratch(struct ScratchSliceFields input);
Hash32 host_sha256_memory(struct EvmMemorySliceFields input);
Hash32 host_keccak_word(const U256 input);
Hash32 host_keccak_address(Address input);
Hash32 host_sha256_pair(
    Hash32 left, Hash32 right);

#endif
