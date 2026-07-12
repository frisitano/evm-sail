/* Shared lbits <-> big-endian conversions for the host FFI.
 *
 * Every C hook that passes 256-bit words / addresses / hashes across the Sail
 * FFI as whole lbits values uses these helpers, so the two runtime
 * representations are handled in exactly one place:
 *
 *   - fixed-width runtimes (sailfix / sail256, SAIL_INT_LIMBS defined):
 *     lbits = { len, d[] } with d[0] the LEAST significant 64-bit limb;
 *   - stock GMP runtime: lbits = { len, mpz_t* }.
 *
 * "be_words" are 64-bit limbs most-significant first (w[0] = bits N-1..N-64),
 * matching the order the FFI's uint64_t[4] key/value arrays are stored in.
 */
#ifndef LBITS_CONVERT_H
#define LBITS_CONVERT_H

#include "sail.h"
#include <stdint.h>
#include <string.h>

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
#ifdef SAIL_INT_LIMBS
  w[0] = v.d[3];
  w[1] = v.d[2];
  w[2] = v.d[1];
  w[3] = v.d[0];
#else
  uint64_t le[4] = {0, 0, 0, 0};
  size_t n = 0;
  mpz_export(le, &n, -1, sizeof(uint64_t), 0, 0, *v.bits);
  w[0] = le[3];
  w[1] = le[2];
  w[2] = le[1];
  w[3] = le[0];
#endif
}

/* build a 256-bit lbits from 4 big-endian-ordered 64-bit words */
static inline void be_words4_to_lbits(lbits *rop, const uint64_t w[4]) {
  rop->len = 256;
#ifdef SAIL_INT_LIMBS
  rop->d[0] = w[3];
  rop->d[1] = w[2];
  rop->d[2] = w[1];
  rop->d[3] = w[0];
#else
  uint64_t le[4] = {w[3], w[2], w[1], w[0]};
  mpz_import(*rop->bits, 4, -1, sizeof(uint64_t), 0, 0, le);
#endif
}

/* extract a 256-bit lbits into 4 little-endian-ordered words (w[0] = LSB) */
static inline void lbits_to_le_words4(uint64_t w[4], const lbits v) {
#ifdef SAIL_INT_LIMBS
  w[0] = v.d[0];
  w[1] = v.d[1];
  w[2] = v.d[2];
  w[3] = v.d[3];
#else
  w[0] = w[1] = w[2] = w[3] = 0;
  size_t n = 0;
  mpz_export(w, &n, -1, sizeof(uint64_t), 0, 0, *v.bits);
#endif
}

/* extract a bits(64) lbits as its scalar value */
static inline uint64_t lbits_to_u64(const lbits v) {
  sail_expect_len(v, 64);
#ifdef SAIL_INT_LIMBS
  return v.d[0];
#else
  uint64_t out = 0;
  size_t n = 0;
  mpz_export(&out, &n, -1, sizeof(out), 0, 0, *v.bits);
  return out;
#endif
}

/* build a 256-bit lbits from 4 little-endian-ordered words */
static inline void le_words4_to_lbits(lbits *rop, const uint64_t w[4]) {
  rop->len = 256;
#ifdef SAIL_INT_LIMBS
  rop->d[0] = w[0];
  rop->d[1] = w[1];
  rop->d[2] = w[2];
  rop->d[3] = w[3];
#else
  mpz_import(*rop->bits, 4, -1, sizeof(uint64_t), 0, 0, w);
#endif
}

/* extract the low `nbytes` (<= 32) of an lbits as big-endian bytes */
static inline void lbits_to_be_bytes(uint8_t *out, size_t nbytes, const lbits v) {
#ifdef SAIL_INT_LIMBS
  for (size_t i = 0; i < nbytes; i++) {
    size_t bit = 8 * (nbytes - 1 - i);
    out[i] = (uint8_t)(v.d[bit / 64] >> (bit % 64));
  }
#else
  memset(out, 0, nbytes);
  size_t n = 0;
  uint8_t tmp[64];
  mpz_export(tmp, &n, 1, 1, 0, 0, *v.bits); /* big-endian, minimal length */
  if (n <= nbytes) {
    memcpy(out + (nbytes - n), tmp, n);
  } else {
    memcpy(out, tmp + (n - nbytes), nbytes);
  }
#endif
}

/* build an lbits of `len_bits` from big-endian bytes */
static inline void be_bytes_to_lbits(lbits *rop, uint64_t len_bits,
                                     const uint8_t *in, size_t nbytes) {
  rop->len = len_bits;
#ifdef SAIL_INT_LIMBS
  rop->d[0] = rop->d[1] = rop->d[2] = rop->d[3] = 0;
  for (size_t i = 0; i < nbytes; i++) {
    size_t bit = 8 * (nbytes - 1 - i);
    rop->d[bit / 64] |= (uint64_t)in[i] << (bit % 64);
  }
#else
  mpz_import(*rop->bits, nbytes, 1, 1, 0, 0, in);
#endif
}

#endif
