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

#include "sail.h"
#include <stdint.h>

/* Domain-typed views of lbits, mirroring the Sail aliases (word/address/
   hash). Transparent typedefs: same ABI as the generated callers, but the C
   signatures read one-for-one against the Sail val declarations. The runtime
   width in .len is the one piece of type information that survives to C --
   sail_expect_len makes it a debug-build assertion (compiled out under
   NDEBUG, e.g. the zkVM guest). */
typedef lbits sail_word;    /* bits(256): word            */
typedef lbits sail_address; /* bits(160): address         */
typedef lbits sail_hash;    /* bits(256): hash / trie key */

#include <assert.h>
static inline void sail_expect_len(const lbits v, uint64_t n) {
  (void)v; (void)n;
  assert(v.len == n);
}

/* extract a 256-bit lbits into 4 big-endian-ordered 64-bit words */
static inline void lbits_to_be_words4(uint64_t w[4], const lbits v) {
  w[0] = v.d[3];
  w[1] = v.d[2];
  w[2] = v.d[1];
  w[3] = v.d[0];
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
