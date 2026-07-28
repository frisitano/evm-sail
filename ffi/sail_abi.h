/* Fixed C representations inferred by Sail from the model's type bounds.
 *
 * Both C build modes run the specialization pass. A word's 256-bit unsigned
 * range and the statically sized address/hash byte vectors therefore have the
 * same by-value ABI in both modes; the optimized splice only tightens otherwise
 * unbounded protocol quantities.
 */
#ifndef EVMSAIL_SAIL_ABI_H
#define EVMSAIL_SAIL_ABI_H

#include "sail.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef SAIL_U128_DEFINED
#define SAIL_U128_DEFINED
typedef struct { uint64_t limbs[2]; } sail_u128;
#endif

#ifndef SAIL_U256_DEFINED
#define SAIL_U256_DEFINED
typedef struct { uint64_t limbs[4]; } sail_u256;
#endif

#ifndef SAIL_FIXED_BYTES_20_DEFINED
#define SAIL_FIXED_BYTES_20_DEFINED
typedef struct { uint8_t bytes[20]; } sail_fixed_bytes_20;
#endif

#ifndef SAIL_FIXED_BYTES_32_DEFINED
#define SAIL_FIXED_BYTES_32_DEFINED
typedef struct { uint8_t bytes[32]; } sail_fixed_bytes_32;
#endif

#ifndef SAIL_FIXED_BYTES_256_DEFINED
#define SAIL_FIXED_BYTES_256_DEFINED
typedef struct { uint8_t bytes[256]; } sail_fixed_bytes_256;
#endif

struct zAddressResult;

typedef sail_u256 sail_word;
typedef sail_fixed_bytes_20 sail_address;
typedef sail_fixed_bytes_32 sail_b256;
typedef sail_fixed_bytes_256 sail_logs_bloom;
typedef sail_b256 sail_hash;
typedef sail_word sail_bits256;

#define EVMSAIL_WORD_RETURN sail_word
#define EVMSAIL_WORD_RESULT(name)
#define EVMSAIL_WORD_PARAM(name) const sail_word name
#define EVMSAIL_WORD_VALUE(name) (name)
#define EVMSAIL_WORD_FIELD_VALUE(name) (name)
#define EVMSAIL_WORD_FIELD_ASSIGN(name, value) ((name) = (value))
#define EVMSAIL_HASH_RETURN sail_hash
#define EVMSAIL_HASH_RESULT(name)
#define EVMSAIL_ADDRESS_RETURN sail_address
#define EVMSAIL_ADDRESS_RESULT(name)
#define EVMSAIL_ADDRESS_RESULT_RECORD_RETURN struct zAddressResult
#define EVMSAIL_ADDRESS_RESULT_RECORD_RESULT(name)

static inline uint8_t evmsail_byte_value(const uint64_t value) {
  return (uint8_t)value;
}

static inline void evmsail_address_to_be_bytes(uint8_t out[20],
                                               sail_address value) {
  memcpy(out, value.bytes, 20);
}

static inline void evmsail_hash_to_be_bytes(uint8_t out[32], sail_hash value) {
  memcpy(out, value.bytes, 32);
}

static inline void evmsail_address_set_be_bytes(sail_address *out,
                                                const uint8_t in[20]) {
  memcpy(out->bytes, in, 20);
}

static inline void evmsail_hash_set_be_bytes(sail_hash *out,
                                             const uint8_t in[32]) {
  memcpy(out->bytes, in, 32);
}

static inline void evmsail_address_assign(sail_address *out,
                                          sail_address value) {
  *out = value;
}

static inline void evmsail_hash_assign(sail_hash *out, sail_hash value) {
  *out = value;
}

#define EVMSAIL_RETURN_WORD(result, value) return (value)
#define EVMSAIL_RETURN_HASH(result, value) return (value)
#define EVMSAIL_RETURN_ADDRESS(result, value) return (value)
#define EVMSAIL_RETURN_HASH_BE_BYTES(result, value)                           \
  do {                                                                        \
    sail_hash evmsail_result;                                                  \
    evmsail_hash_set_be_bytes(&evmsail_result, (value));                       \
    return evmsail_result;                                                     \
  } while (0)
#define EVMSAIL_RETURN_ADDRESS_BE_BYTES(result, value)                        \
  do {                                                                        \
    sail_address evmsail_result;                                               \
    evmsail_address_set_be_bytes(&evmsail_result, (value));                    \
    return evmsail_result;                                                     \
  } while (0)

#endif
