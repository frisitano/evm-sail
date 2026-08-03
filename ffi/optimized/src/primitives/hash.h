/* Helpers for canonical protocol-order hash bytes in C host stores. */
#ifndef EVMSAIL_OPTIMIZED_PRIMITIVES_HASH_H
#define EVMSAIL_OPTIMIZED_PRIMITIVES_HASH_H

#include "evmsail/prelude.h"
#include "primitives/value.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const Hash32 EVMSAIL_EMPTY_TRIE_ROOT = {{
    0x56, 0xe8, 0x1f, 0x17, 0x1b, 0xcc, 0x55, 0xa6,
    0xff, 0x83, 0x45, 0xe6, 0x92, 0xc0, 0xf8, 0x6e,
    0x5b, 0x48, 0xe0, 0x1b, 0x99, 0x6c, 0xad, 0xc0,
    0x01, 0x62, 0x2f, 0xb5, 0xe3, 0x63, 0xb4, 0x21,
}};

static const Hash32 EVMSAIL_KECCAK_EMPTY = {{
    0xc5, 0xd2, 0x46, 0x01, 0x86, 0xf7, 0x23, 0x3c,
    0x92, 0x7e, 0x7d, 0xb2, 0xdc, 0xc7, 0x03, 0xc0,
    0xe5, 0x00, 0xb6, 0x53, 0xca, 0x82, 0x27, 0x3b,
    0x7b, 0xfa, 0xd8, 0x04, 0x5d, 0x85, 0xa4, 0x70,
}};

static inline int hash_compare(const Hash32 *a,
                                       const Hash32 *b) {
  return memcmp(a->bytes, b->bytes, sizeof(a->bytes));
}

static inline int hash_equal(const Hash32 *a,
                                     const Hash32 *b) {
  return memcmp(a->bytes, b->bytes, sizeof(a->bytes)) == 0;
}

static inline Hash32 hash_from_sail_word(U256 value) {
  Hash32 result;
  sail_word_to_be_bytes(result.bytes, value);
  return result;
}

static inline U256 hash_to_sail_word(const Hash32 *value) {
  return be_bytes_to_sail_word(value->bytes);
}

/* The low 160 bits of the digest, retaining canonical Ethereum byte order. */
static inline Address hash_low_address(
    const Hash32 *value) {
  Address result;
  memcpy(result.bytes, value->bytes + 12, sizeof(result.bytes));
  return result;
}

#endif
