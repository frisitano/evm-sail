#ifndef EVMSAIL_OPTIMIZED_LIB_RLP_ENCODING_H
#define EVMSAIL_OPTIMIZED_LIB_RLP_ENCODING_H

#include "evmsail/prelude.h"
#include <stdbool.h>
#include <stdint.h>

/* C-private primitives used by optimized high-level encoders. */
bytes32 zero_hash(void);
bytes32 keccak_bytes(const uint8_t *bytes, uint64_t len);
bytes32 sha256_bytes(const uint8_t *bytes, uint64_t len);
uint64_t rlp_quantity_size_u64(uint64_t value);
uint64_t rlp_quantity_size_u256(u256 value);
uint64_t rlp_string_prefix_size(uint64_t len, uint8_t first);
uint64_t rlp_string_size(uint64_t len, uint8_t first);
uint64_t rlp_list_prefix_size(uint64_t content_len);
uint8_t *rlp_write_string_prefix(uint8_t *out, uint64_t len, uint8_t first);
uint8_t *rlp_write_list_prefix(uint8_t *out, uint64_t content_len);
uint8_t *rlp_write_u64(uint8_t *out, uint64_t value);
uint8_t *rlp_write_u256(uint8_t *out, u256 value);

#endif
