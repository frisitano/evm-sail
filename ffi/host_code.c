/* C-backed CODE buffer + JUMPDEST bitmap for the evm-sail model.
 *
 * set_code streams the frame's code bytes here ONCE; afterwards every fetch
 * is an O(1) array read (the Sail-list path needed a sequential cursor to
 * avoid O(pc) list walks), PUSH immediates are assembled into a whole word by
 * one call (hc_word), and JUMP validity is an O(1) bit test against a bitmap
 * built during the same streaming pass (PUSH immediate data skipped). Frames
 * save/restore code by re-streaming on set_code, exactly as before. */
#include "sail.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define HJ_MAXCODE 65536          /* > MAX_INITCODE_SIZE (49152) */

static uint8_t  hc_code[HJ_MAXCODE];
static uint8_t  hj_bm[HJ_MAXCODE / 8];
static uint32_t hj_len;           /* bytes streamed so far        */
static uint32_t hj_skip;          /* PUSH immediate bytes to skip */

unit hj_begin(const unit u) {
  (void)u;
  memset(hj_bm, 0, sizeof hj_bm);
  hj_len = 0;
  hj_skip = 0;
  return UNIT;
}

unit hj_byte(uint64_t b) {
  if (hj_len < HJ_MAXCODE) {
    hc_code[hj_len] = (uint8_t)b;
    if (hj_skip) hj_skip--;
    else if (b == 0x5b) hj_bm[hj_len >> 3] |= (uint8_t)(1u << (hj_len & 7));
    else if (b >= 0x60 && b <= 0x7f) hj_skip = (uint32_t)(b - 0x5f);
  }
  hj_len++;
  return UNIT;
}

bool hj_valid(uint64_t i) {
  return i < hj_len && i < HJ_MAXCODE && (hj_bm[i >> 3] >> (i & 7)) & 1;
}

/* code byte at offset i (0x00 past the end, the EVM's implicit STOP padding) */
uint64_t hc_byte(uint64_t i) {
  return (i < hj_len && i < HJ_MAXCODE) ? hc_code[i] : 0;
}

#ifdef SAIL_INT_LIMBS
/* the n-byte PUSH immediate starting at offset i, as a right-aligned word
 * (fixed-width runtimes; the GMP build assembles in Sail) */
void hc_word(lbits *rop, uint64_t i, uint64_t n) {
  rop->len = 256;
  rop->d[0] = rop->d[1] = rop->d[2] = rop->d[3] = 0;
  for (uint64_t k = 0; k < n && k < 32; k++) {
    uint64_t byte = (i + k < hj_len && i + k < HJ_MAXCODE) ? hc_code[i + k] : 0;
    uint64_t bit = (n - 1 - k) * 8;            /* big-endian within the value */
    rop->d[bit >> 6] |= byte << (bit & 63);
  }
}
#else
void hc_word(lbits *rop, uint64_t i, uint64_t n) {
  rop->len = 256;
  mpz_set_ui(*rop->bits, 0);
  for (uint64_t k = 0; k < n && k < 32; k++) {
    uint64_t byte = (i + k < hj_len && i + k < HJ_MAXCODE) ? hc_code[i + k] : 0;
    mpz_mul_2exp(*rop->bits, *rop->bits, 8);
    mpz_add_ui(*rop->bits, *rop->bits, byte);
  }
}
#endif
