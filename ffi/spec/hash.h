/* Hash host operations implemented by this backend's hash.c. */
#ifndef EVMSAIL_SPEC_HASH_H
#define EVMSAIL_SPEC_HASH_H

#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H
#include <stdbool.h>
#include <stdint.h>

struct zStatelessInputSliceFields;
struct zScratchSliceFields;
struct zMemorySliceFields;
struct zCodeRegionSliceFields;
struct zOutputSliceFields;
struct zLogDataSliceFields;

sail_fixed_bytes_32 host_keccak_stateless_input(
    struct zStatelessInputSliceFields input);
sail_fixed_bytes_32 host_keccak_scratch(struct zScratchSliceFields input);
sail_fixed_bytes_32 host_keccak_memory(struct zMemorySliceFields input);
sail_fixed_bytes_32 host_keccak_code(struct zCodeRegionSliceFields input);
sail_fixed_bytes_32 host_keccak_output(struct zOutputSliceFields input);
sail_fixed_bytes_32 host_keccak_log_data(struct zLogDataSliceFields input);
sail_fixed_bytes_32 host_sha256_stateless_input(
    struct zStatelessInputSliceFields input);
sail_fixed_bytes_32 host_sha256_scratch(struct zScratchSliceFields input);
sail_fixed_bytes_32 host_sha256_memory(struct zMemorySliceFields input);
sail_fixed_bytes_32 host_keccak_word(const sail_u256 input);
sail_fixed_bytes_32 host_keccak_address(sail_fixed_bytes_20 input);
sail_fixed_bytes_32 host_sha256_pair(
    sail_fixed_bytes_32 left, sail_fixed_bytes_32 right);

#endif
