/* Shared lbits <-> big-endian conversions for the host FFI.
 *
 * Every C hook that passes 256-bit words / addresses / hashes across the Sail
 * FFI as whole lbits values uses these helpers. The active sail256 ABI is
 * lbits = { len, d[] }, with d[0] the least-significant 64-bit limb.
 *
 * "be_words" are 64-bit limbs most-significant first (w[0] = bits N-1..N-64),
 * matching the order the FFI's uint64_t[4] key/value arrays are stored in.
 */
#ifndef LBITS_CONVERT_H
#define LBITS_CONVERT_H

#include "sail_abi.h"
#include <stdint.h>

static inline void sail_expect_len(const lbits v, uint64_t n) {
  evmsail_expect_lbits_len(v, n);
}

/* extract a 256-bit lbits into 4 big-endian-ordered 64-bit words */
static inline void lbits_to_be_words4(uint64_t w[4], const lbits v) {
  w[0] = v.d[3];
  w[1] = v.d[2];
  w[2] = v.d[1];
  w[3] = v.d[0];
}

/* Native word conversions.  These are deliberately separate from the lbits
   helpers so word-valued FFI code cannot accidentally re-introduce the Sail
   runtime representation. */
static inline void sail_word_to_be_words4(uint64_t w[4], const sail_word v) {
#ifdef EVMSAIL_STANDARD_ABI
  sail_expect_len(v, 256);
  w[0] = v.d[3];
  w[1] = v.d[2];
  w[2] = v.d[1];
  w[3] = v.d[0];
#else
  w[0] = v.limbs[3];
  w[1] = v.limbs[2];
  w[2] = v.limbs[1];
  w[3] = v.limbs[0];
#endif
}

static inline void be_words4_to_be_bytes(uint8_t out[32],
                                         const uint64_t w[4]) {
  for (size_t word = 0; word < 4; word++)
    for (size_t byte = 0; byte < 8; byte++)
      out[word * 8 + byte] =
          (uint8_t)(w[word] >> (56 - (byte * 8)));
}

static inline void sail_hash_to_be_words4(uint64_t w[4], sail_hash value) {
  uint8_t bytes[32];
  evmsail_hash_to_be_bytes(bytes, value);
  for (size_t word = 0; word < 4; word++) {
    w[word] = 0;
    for (size_t byte = 0; byte < 8; byte++)
      w[word] = (w[word] << 8) | bytes[word * 8 + byte];
  }
}

static inline void sail_hash_to_le_words4(uint64_t w[4], sail_hash value) {
  uint64_t be[4];
  sail_hash_to_be_words4(be, value);
  w[0] = be[3];
  w[1] = be[2];
  w[2] = be[1];
  w[3] = be[0];
}

static inline sail_hash be_words4_to_sail_hash(const uint64_t w[4]) {
  uint8_t bytes[32];
#ifdef EVMSAIL_STANDARD_ABI
  sail_hash result = {0, NULL};
#else
  sail_hash result;
#endif
  be_words4_to_be_bytes(bytes, w);
  evmsail_hash_set_be_bytes(&result, bytes);
  return result;
}

static inline sail_hash le_words4_to_sail_hash(const uint64_t w[4]) {
  const uint64_t be[4] = {w[3], w[2], w[1], w[0]};
  return be_words4_to_sail_hash(be);
}

static inline sail_address be_bytes_to_sail_address(const uint8_t in[20]) {
#ifdef EVMSAIL_STANDARD_ABI
  sail_address result = {0, NULL};
#else
  sail_address result;
#endif
  evmsail_address_set_be_bytes(&result, in);
  return result;
}

static inline sail_word be_words4_to_sail_word(const uint64_t w[4]) {
#ifdef EVMSAIL_STANDARD_ABI
  sail_word result = {.len = 256, .d = {w[3], w[2], w[1], w[0]}};
#else
  sail_word result = {{w[3], w[2], w[1], w[0]}};
#endif
  return result;
}

static inline void sail_word_to_le_words4(uint64_t w[4], const sail_word v) {
#ifdef EVMSAIL_STANDARD_ABI
  sail_expect_len(v, 256);
  w[0] = v.d[0];
  w[1] = v.d[1];
  w[2] = v.d[2];
  w[3] = v.d[3];
#else
  w[0] = v.limbs[0];
  w[1] = v.limbs[1];
  w[2] = v.limbs[2];
  w[3] = v.limbs[3];
#endif
}

static inline sail_word le_words4_to_sail_word(const uint64_t w[4]) {
#ifdef EVMSAIL_STANDARD_ABI
  sail_word result = {.len = 256, .d = {w[0], w[1], w[2], w[3]}};
#else
  sail_word result = {{w[0], w[1], w[2], w[3]}};
#endif
  return result;
}

static inline void sail_word_to_be_bytes(uint8_t out[32], const sail_word v) {
#ifdef EVMSAIL_STANDARD_ABI
  sail_expect_len(v, 256);
  for (size_t i = 0; i < 32; i++) {
    size_t bit = 8 * (31 - i);
    out[i] = (uint8_t)(v.d[bit / 64] >> (bit % 64));
  }
#else
  for (size_t i = 0; i < 32; i++) {
    size_t bit = 8 * (31 - i);
    out[i] = (uint8_t)(v.limbs[bit / 64] >> (bit % 64));
  }
#endif
}

static inline sail_word be_bytes_to_sail_word(const uint8_t in[32]) {
#ifdef EVMSAIL_STANDARD_ABI
  sail_word result = {.len = 256, .d = {0, 0, 0, 0}};
  for (size_t i = 0; i < 32; i++) {
    size_t bit = 8 * (31 - i);
    result.d[bit / 64] |= (uint64_t)in[i] << (bit % 64);
  }
#else
  sail_word result = {{0}};
  for (size_t i = 0; i < 32; i++) {
    size_t bit = 8 * (31 - i);
    result.limbs[bit / 64] |= (uint64_t)in[i] << (bit % 64);
  }
#endif
  return result;
}

/* build a 256-bit lbits from 4 big-endian-ordered 64-bit words */
static inline void be_words4_to_lbits(lbits *rop, const uint64_t w[4]) {
  rop->len = 256;
  rop->d[0] = w[3];
  rop->d[1] = w[2];
  rop->d[2] = w[1];
  rop->d[3] = w[0];
}

/* extract a 256-bit lbits into 4 little-endian-ordered words (w[0] = LSB) */
static inline void lbits_to_le_words4(uint64_t w[4], const lbits v) {
  w[0] = v.d[0];
  w[1] = v.d[1];
  w[2] = v.d[2];
  w[3] = v.d[3];
}

/* extract a bits(64) lbits as its scalar value */
static inline uint64_t lbits_to_u64(const lbits v) {
  sail_expect_len(v, 64);
  return v.d[0];
}

/* build a 256-bit lbits from 4 little-endian-ordered words */
static inline void le_words4_to_lbits(lbits *rop, const uint64_t w[4]) {
  rop->len = 256;
  rop->d[0] = w[0];
  rop->d[1] = w[1];
  rop->d[2] = w[2];
  rop->d[3] = w[3];
}

/* extract the low `nbytes` (<= 32) of an lbits as big-endian bytes */
static inline void lbits_to_be_bytes(uint8_t *out, size_t nbytes, const lbits v) {
  for (size_t i = 0; i < nbytes; i++) {
    size_t bit = 8 * (nbytes - 1 - i);
    out[i] = (uint8_t)(v.d[bit / 64] >> (bit % 64));
  }
}

/* build an lbits of `len_bits` from big-endian bytes */
static inline void be_bytes_to_lbits(lbits *rop, uint64_t len_bits,
                                     const uint8_t *in, size_t nbytes) {
  rop->len = len_bits;
  rop->d[0] = rop->d[1] = rop->d[2] = rop->d[3] = 0;
  for (size_t i = 0; i < nbytes; i++) {
    size_t bit = 8 * (nbytes - 1 - i);
    rop->d[bit / 64] |= (uint64_t)in[i] << (bit % 64);
  }
}

#endif
