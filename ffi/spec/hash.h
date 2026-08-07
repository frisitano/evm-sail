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
struct zEvmMemorySliceFields;
struct zCodeRegionSliceFields;
struct zOutputSliceFields;
struct zLogDataSliceFields;

fixed_bytes_32 host_keccak_stateless_input(
    struct zStatelessInputSliceFields input);
fixed_bytes_32 host_keccak_scratch(struct zScratchSliceFields input);
fixed_bytes_32 host_keccak_memory(struct zEvmMemorySliceFields input);
fixed_bytes_32 host_keccak_code(struct zCodeRegionSliceFields input);
fixed_bytes_32 host_keccak_output(struct zOutputSliceFields input);
fixed_bytes_32 host_keccak_log_data(struct zLogDataSliceFields input);
fixed_bytes_32 host_sha256_stateless_input(
    struct zStatelessInputSliceFields input);
fixed_bytes_32 host_sha256_scratch(struct zScratchSliceFields input);
fixed_bytes_32 host_sha256_memory(struct zEvmMemorySliceFields input);
fixed_bytes_32 host_keccak_word(const u256 input);
fixed_bytes_32 host_keccak_address(fixed_bytes_20 input);
fixed_bytes_32 host_sha256_pair(
    fixed_bytes_32 left, fixed_bytes_32 right);

#endif
