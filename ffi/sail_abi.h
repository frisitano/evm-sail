/* Shared declarations for the Sail model's nominal C domain types.
 *
 * The ordinary model deliberately keeps Sail's canonical representations:
 *   word    -> lbits
 *   address -> vector(20, dec, byte)
 *   hash    -> vector(32, dec, byte)
 *
 * The c_optimized splice lowers the same nominal types to inline values:
 *   word    -> sail_u256
 *   address -> sail_fixed_bytes_20
 *   hash    -> sail_fixed_bytes_32
 *
 * FFI headers are injected before the generated model header.  Define the
 * guarded representation types here so both sides see identical declarations
 * regardless of include order.
 */
#ifndef EVMSAIL_SAIL_ABI_H
#define EVMSAIL_SAIL_ABI_H

#include "sail.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

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

/* Encoded C identifier for vector(_, _, bits(8)) in the generated model. */
#ifndef SAIL_VECTOR_ZZ5VECZ8Z5BV8Z9_DEFINED
#define SAIL_VECTOR_ZZ5VECZ8Z5BV8Z9_DEFINED
struct zz5vecz8z5bv8z9 {
  size_t len;
  uint64_t *data;
};
typedef struct zz5vecz8z5bv8z9 zz5vecz8z5bv8z9;
#endif

/* Keep the generated list(byte) spelling behind a stable host-side alias.
 * The standard extraction uses the generic bits element representation;
 * --c-specialize keeps bits(8) fixed-width in the optimized extraction. */
#ifdef EVMSAIL_STANDARD_ABI
struct node_zz5listz8z5bvz9;
typedef struct node_zz5listz8z5bvz9 *evmsail_byte_list;
#else
struct node_zz5listz8z5bv8z9;
typedef struct node_zz5listz8z5bv8z9 *evmsail_byte_list;
#endif

struct zAddressResult;

#ifdef EVMSAIL_STANDARD_ABI
typedef lbits sail_word;
typedef zz5vecz8z5bv8z9 sail_address;
typedef zz5vecz8z5bv8z9 sail_b256;
typedef sail_b256 sail_hash;

#define EVMSAIL_WORD_RETURN void
#define EVMSAIL_WORD_RESULT(name) sail_word *name,
#define EVMSAIL_HASH_RETURN void
#define EVMSAIL_HASH_RESULT(name) sail_hash *name,
#define EVMSAIL_ADDRESS_RETURN void
#define EVMSAIL_ADDRESS_RESULT(name) sail_address *name,
#define EVMSAIL_ADDRESS_RESULT_RECORD_RETURN void
#define EVMSAIL_ADDRESS_RESULT_RECORD_RESULT(name) struct zAddressResult *name,
#else
typedef sail_u256 sail_word;
typedef sail_fixed_bytes_20 sail_address;
typedef sail_fixed_bytes_32 sail_b256;
typedef sail_b256 sail_hash;

#define EVMSAIL_WORD_RETURN sail_word
#define EVMSAIL_WORD_RESULT(name)
#define EVMSAIL_HASH_RETURN sail_hash
#define EVMSAIL_HASH_RESULT(name)
#define EVMSAIL_ADDRESS_RETURN sail_address
#define EVMSAIL_ADDRESS_RESULT(name)
#define EVMSAIL_ADDRESS_RESULT_RECORD_RETURN struct zAddressResult
#define EVMSAIL_ADDRESS_RESULT_RECORD_RESULT(name)
#endif

/* Raw bits(256) values share the same ABI split as the nominal EVM word:
 * heap-backed lbits in a standard extraction, four inline limbs in the
 * optimized extraction. */
typedef sail_word sail_bits256;

static inline void evmsail_expect_lbits_len(const lbits value, uint64_t len) {
  (void)value;
  (void)len;
  assert(value.len == len);
}

#ifdef EVMSAIL_STANDARD_ABI
static inline uint8_t evmsail_byte_value(const lbits value) {
  evmsail_expect_lbits_len(value, 8);
  return (uint8_t)(value.d[0] & UINT64_C(0xff));
}
#else
static inline uint8_t evmsail_byte_value(const uint64_t value) {
  return (uint8_t)(value & UINT64_C(0xff));
}
#endif

static inline void evmsail_standard_bytes_resize(zz5vecz8z5bv8z9 *out,
                                                 size_t len) {
  if (out->data != NULL) sail_free(out->data);
  out->len = len;
  out->data = sail_new_array(uint64_t, len);
}

static inline void evmsail_address_to_be_bytes(uint8_t out[20],
                                               sail_address value) {
#ifdef EVMSAIL_STANDARD_ABI
  assert(value.len == 20);
  for (size_t i = 0; i < 20; i++)
    out[i] = (uint8_t)(value.data[19 - i] & UINT64_C(0xff));
#else
  for (size_t i = 0; i < 20; i++) out[i] = value.bytes[19 - i];
#endif
}

static inline void evmsail_hash_to_be_bytes(uint8_t out[32], sail_hash value) {
#ifdef EVMSAIL_STANDARD_ABI
  assert(value.len == 32);
  for (size_t i = 0; i < 32; i++)
    out[i] = (uint8_t)(value.data[31 - i] & UINT64_C(0xff));
#else
  for (size_t i = 0; i < 32; i++) out[i] = value.bytes[31 - i];
#endif
}

static inline void evmsail_address_set_be_bytes(sail_address *out,
                                                const uint8_t in[20]) {
#ifdef EVMSAIL_STANDARD_ABI
  evmsail_standard_bytes_resize(out, 20);
  for (size_t i = 0; i < 20; i++) out->data[i] = in[19 - i];
#else
  for (size_t i = 0; i < 20; i++) out->bytes[i] = in[19 - i];
#endif
}

static inline void evmsail_hash_set_be_bytes(sail_hash *out,
                                             const uint8_t in[32]) {
#ifdef EVMSAIL_STANDARD_ABI
  evmsail_standard_bytes_resize(out, 32);
  for (size_t i = 0; i < 32; i++) out->data[i] = in[31 - i];
#else
  for (size_t i = 0; i < 32; i++) out->bytes[i] = in[31 - i];
#endif
}

static inline void evmsail_address_to_lbits(lbits *out, sail_address value) {
  uint8_t bytes[20];
  evmsail_address_to_be_bytes(bytes, value);
  out->len = 160;
  memset(out->d, 0, sizeof(out->d));
  for (size_t i = 0; i < 20; i++) {
    size_t bit = 8 * (19 - i);
    out->d[bit / 64] |= (uint64_t)bytes[i] << (bit % 64);
  }
}

static inline void evmsail_hash_to_lbits(lbits *out, sail_hash value) {
  uint8_t bytes[32];
  evmsail_hash_to_be_bytes(bytes, value);
  out->len = 256;
  memset(out->d, 0, sizeof(out->d));
  for (size_t i = 0; i < 32; i++) {
    size_t bit = 8 * (31 - i);
    out->d[bit / 64] |= (uint64_t)bytes[i] << (bit % 64);
  }
}

static inline void evmsail_address_set_lbits(sail_address *out, lbits value) {
  uint8_t bytes[20];
  evmsail_expect_lbits_len(value, 160);
  for (size_t i = 0; i < 20; i++) {
    size_t bit = 8 * (19 - i);
    bytes[i] = (uint8_t)(value.d[bit / 64] >> (bit % 64));
  }
  evmsail_address_set_be_bytes(out, bytes);
}

static inline void evmsail_hash_set_lbits(sail_hash *out, lbits value) {
  uint8_t bytes[32];
  evmsail_expect_lbits_len(value, 256);
  for (size_t i = 0; i < 32; i++) {
    size_t bit = 8 * (31 - i);
    bytes[i] = (uint8_t)(value.d[bit / 64] >> (bit % 64));
  }
  evmsail_hash_set_be_bytes(out, bytes);
}

static inline void evmsail_word_to_lbits(lbits *out, sail_word value) {
#ifdef EVMSAIL_STANDARD_ABI
  evmsail_expect_lbits_len(value, 256);
  *out = value;
#else
  out->len = 256;
  for (size_t i = 0; i < 4; i++) out->d[i] = value.limbs[i];
#endif
}

static inline sail_word evmsail_word_from_lbits(lbits value) {
  evmsail_expect_lbits_len(value, 256);
#ifdef EVMSAIL_STANDARD_ABI
  return value;
#else
  sail_word out = {{value.d[0], value.d[1], value.d[2], value.d[3]}};
  return out;
#endif
}

static inline sail_address evmsail_address_from_lbits(lbits value) {
#ifdef EVMSAIL_STANDARD_ABI
  sail_address out = {0, NULL};
#else
  sail_address out;
#endif
  evmsail_address_set_lbits(&out, value);
  return out;
}

static inline sail_hash evmsail_hash_from_lbits(lbits value) {
#ifdef EVMSAIL_STANDARD_ABI
  sail_hash out = {0, NULL};
#else
  sail_hash out;
#endif
  evmsail_hash_set_lbits(&out, value);
  return out;
}

static inline void evmsail_address_assign(sail_address *out,
                                          sail_address value) {
#ifdef EVMSAIL_STANDARD_ABI
  assert(value.len == 20);
  evmsail_standard_bytes_resize(out, 20);
  memcpy(out->data, value.data, 20 * sizeof(uint64_t));
#else
  *out = value;
#endif
}

static inline void evmsail_hash_assign(sail_hash *out, sail_hash value) {
#ifdef EVMSAIL_STANDARD_ABI
  assert(value.len == 32);
  evmsail_standard_bytes_resize(out, 32);
  memcpy(out->data, value.data, 32 * sizeof(uint64_t));
#else
  *out = value;
#endif
}

#ifdef EVMSAIL_STANDARD_ABI
#define EVMSAIL_RETURN_WORD(result, value)                                    \
  do {                                                                        \
    *(result) = (value);                                                       \
    return;                                                                    \
  } while (0)
#define EVMSAIL_RETURN_HASH(result, value)                                    \
  do {                                                                        \
    evmsail_hash_assign((result), (value));                                    \
    return;                                                                    \
  } while (0)
#define EVMSAIL_RETURN_ADDRESS(result, value)                                 \
  do {                                                                        \
    evmsail_address_assign((result), (value));                                 \
    return;                                                                    \
  } while (0)
#define EVMSAIL_RETURN_HASH_BE_BYTES(result, value)                           \
  do {                                                                        \
    evmsail_hash_set_be_bytes((result), (value));                              \
    return;                                                                    \
  } while (0)
#define EVMSAIL_RETURN_ADDRESS_BE_BYTES(result, value)                        \
  do {                                                                        \
    evmsail_address_set_be_bytes((result), (value));                           \
    return;                                                                    \
  } while (0)
#define EVMSAIL_RETURN_WORD_LBITS(result, value) EVMSAIL_RETURN_WORD((result), (value))
#define EVMSAIL_RETURN_HASH_LBITS(result, value)                              \
  do {                                                                        \
    evmsail_hash_set_lbits((result), (value));                                 \
    return;                                                                    \
  } while (0)
#define EVMSAIL_RETURN_ADDRESS_LBITS(result, value)                           \
  do {                                                                        \
    evmsail_address_set_lbits((result), (value));                              \
    return;                                                                    \
  } while (0)
#else
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
#define EVMSAIL_RETURN_WORD_LBITS(result, value) return evmsail_word_from_lbits(value)
#define EVMSAIL_RETURN_HASH_LBITS(result, value) return evmsail_hash_from_lbits(value)
#define EVMSAIL_RETURN_ADDRESS_LBITS(result, value) return evmsail_address_from_lbits(value)
#endif

#endif
