#ifndef EVMSAIL_OPTIMIZED_PRELUDE_H
#define EVMSAIL_OPTIMIZED_PRELUDE_H

#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H

#include <stdint.h>

/*
 * Semantic names for the fixed representations selected by the optimized Sail
 * backend. Generated names stop at this boundary: host modules speak in EVM
 * values, not in backend lowering details.
 */
typedef sail_fixed_bytes_u64_lanes_20 Address;
typedef sail_fixed_bytes_u64_lanes_32 Hash32;
typedef sail_fixed_bytes_256 LogsBloom;
typedef sail_u256 U256;

/* Optimized bindings owned by sail/optimised/prelude.sail. */
U256 hash_to_word(Hash32 bytes);
Hash32 word_to_hash(U256 value);
uint64_t u64_bit_length(uint64_t value);
Address word_to_address(U256 value);
U256 address_to_word(Address bytes);

#endif
