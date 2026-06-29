/* ======================================================================== */
/* sailfix: a GMP-free, fixed-width implementation of the Sail C runtime's     */
/* int + bits primitives, replacing the mpz-backed sail.c + mini-gmp.          */
/*                                                                           */
/*   sail_int : 512-bit sign-magnitude with an EXACT significant-limb count    */
/*              `len` (d[len-1] != 0, or len==0 for zero); limbs above len are  */
/*              UNDEFINED and never read. 512 bits covers every value the EVM   */
/*              produces (MUL/MULMOD/EXP compute unsigned(a)*unsigned(b) <=      */
/*              2^512 before truncation). Tracking len lets the common small     */
/*              values (gas, pc, opcode bytes, indices, comparisons) touch one   */
/*              limb instead of eight.                                          */
/*   lbits    : 256-bit inline { len, d[4] } (max bitvector width in the EVM).  */
/*                                                                           */
/* Only the ~40 functions the generated guest references are implemented; the  */
/* generated C is unchanged and recompiles against the modified sail.h. Real/   */
/* rational ops are intentionally absent (the EVM guest uses none).            */
/* ======================================================================== */

#include "sail.h"
#include <string.h>

extern void *malloc(unsigned long);
extern void  free(void *);

/* ===================== length-aware unsigned magnitude ================= */
#define NI SAIL_INT_LIMBS   /* 8 limbs = 512 bits */

/* significant-limb count of d[0..upto-1] (trims trailing zero limbs). Only
 * reads written limbs, so callers pass the count they actually filled. */
static int mag_norm(const uint64_t *d, int upto) {
  while (upto > 0 && !d[upto - 1]) upto--; return upto;
}
/* compare magnitudes given EXACT lengths: longer (normalized) magnitude wins. */
static int mag_cmp(const uint64_t *a, int la, const uint64_t *b, int lb) {
  if (la != lb) return la < lb ? -1 : 1;
  for (int i = la - 1; i >= 0; i--) { if (a[i] < b[i]) return -1; if (a[i] > b[i]) return 1; }
  return 0;
}
/* r = a + b, returns result length. r may alias a or b (reads i before write i). */
static int mag_add(uint64_t *r, const uint64_t *a, int la, const uint64_t *b, int lb) {
  int n = la > lb ? la : lb;
  unsigned __int128 c = 0;
  for (int i = 0; i < n; i++) {
    unsigned __int128 s = (unsigned __int128)(i < la ? a[i] : 0) + (i < lb ? b[i] : 0) + c;
    r[i] = (uint64_t)s; c = s >> 64;
  }
  if (c) { r[n] = (uint64_t)c; return n + 1; }
  return mag_norm(r, n);
}
/* r = a - b (requires a >= b), returns result length. r may alias a or b. */
static int mag_sub(uint64_t *r, const uint64_t *a, int la, const uint64_t *b, int lb) {
  unsigned __int128 bor = 0;
  for (int i = 0; i < la; i++) {
    unsigned __int128 d = (unsigned __int128)a[i] - (i < lb ? b[i] : 0) - bor;
    r[i] = (uint64_t)d; bor = (d >> 64) & 1;
  }
  return mag_norm(r, la);
}
/* materialize a full NI-limb magnitude (len limbs + zero fill) for the complex
 * ops (mul/divmod) that are simpler and safer expressed full-width. */
static void mag_full(uint64_t out[NI], const uint64_t *d, int len) {
  for (int i = 0; i < NI; i++) out[i] = i < len ? d[i] : 0;
}
static int magf_cmp(const uint64_t *a, const uint64_t *b) {
  for (int i = NI - 1; i >= 0; i--) { if (a[i] < b[i]) return -1; if (a[i] > b[i]) return 1; }
  return 0;
}
static void magf_sub(uint64_t *r, const uint64_t *a, const uint64_t *b) {
  unsigned __int128 bor = 0;
  for (int i = 0; i < NI; i++) { unsigned __int128 d = (unsigned __int128)a[i] - b[i] - bor; r[i] = (uint64_t)d; bor = (d >> 64) & 1; }
}
/* r = a * b, returns result length. */
static int mag_mul(uint64_t *r, const uint64_t *a, int la, const uint64_t *b, int lb) {
  uint64_t af[NI], bf[NI], t[NI];
  mag_full(af, a, la); mag_full(bf, b, lb);
  for (int i = 0; i < NI; i++) t[i] = 0;
  for (int i = 0; i < NI; i++) {
    if (!af[i]) continue;
    unsigned __int128 carry = 0;
    for (int j = 0; i + j < NI; j++) {
      unsigned __int128 s = (unsigned __int128)af[i] * bf[j] + t[i + j] + carry;
      t[i + j] = (uint64_t)s; carry = s >> 64;
    }
  }
  for (int i = 0; i < NI; i++) r[i] = t[i];
  return mag_norm(r, NI);
}
/* q = a / b, rem = a % b (unsigned, b != 0); lengths via *ql,*rl. */
static void mag_divmod(uint64_t *q, int *ql, uint64_t *rem, int *rl,
                       const uint64_t *a, int la, const uint64_t *b, int lb) {
  uint64_t af[NI], bf[NI], r[NI];
  mag_full(af, a, la); mag_full(bf, b, lb);
  for (int i = 0; i < NI; i++) { q[i] = 0; r[i] = 0; }
  int ahi = NI - 1, bhi = NI - 1;
  while (ahi > 0 && !af[ahi]) ahi--;
  while (bhi > 0 && !bf[bhi]) bhi--;
  if (ahi == 0 && bhi == 0) { q[0] = af[0] / bf[0]; r[0] = af[0] % bf[0]; }
  else {
    for (int bit = NI * 64 - 1; bit >= 0; bit--) {
      uint64_t carry = 0;                          /* r <<= 1 */
      for (int i = 0; i < NI; i++) { uint64_t nc = r[i] >> 63; r[i] = (r[i] << 1) | carry; carry = nc; }
      r[0] |= (af[bit >> 6] >> (bit & 63)) & 1;     /* bring in bit of a */
      if (magf_cmp(r, bf) >= 0) { magf_sub(r, r, bf); q[bit >> 6] |= (uint64_t)1 << (bit & 63); }
    }
  }
  for (int i = 0; i < NI; i++) rem[i] = r[i];
  *ql = mag_norm(q, NI); *rl = mag_norm(rem, NI);
}

/* ===================== sail_int (sign-magnitude) ======================= */
typedef sail_int_struct si;

void create_sail_int(sail_int *rop) { (*rop)->neg = 0; (*rop)->len = 0; }
void recreate_sail_int(sail_int *rop) { (*rop)->neg = 0; (*rop)->len = 0; }
void kill_sail_int(sail_int *rop)   { (void)rop; }
void copy_sail_int(sail_int *rop, const sail_int op) {
  int L = op->len;
  for (int i = 0; i < L; i++) (*rop)->d[i] = op->d[i];
  (*rop)->len = L; (*rop)->neg = op->neg;
}
uint64_t sail_int_get_ui(const sail_int op) { return op->len ? op->d[0] : 0; }
mach_int convert_mach_int_of_sail_int(const sail_int op) {
  int64_t v = (int64_t)(op->len ? op->d[0] : 0);
  return op->neg ? -v : v;
}
void convert_sail_int_of_mach_int(sail_int *rop, const mach_int op) {
  si *r = (*rop);
  if (op < 0) { r->neg = 1; r->d[0] = (uint64_t)(-(op + 1)) + 1; r->len = 1; }
  else if (op > 0) { r->neg = 0; r->d[0] = (uint64_t)op; r->len = 1; }
  else { r->neg = 0; r->len = 0; }
}
void convert_sail_int_of_sail_string(sail_int *rop, const_sail_string str) {
  si *r = (*rop); r->len = 0; r->neg = 0;
  const char *p = str; int neg = 0;
  if (*p == '-') { neg = 1; p++; }
  uint64_t ten[1] = {10};
  for (; *p; p++) {
    if (*p < '0' || *p > '9') continue;
    uint64_t t[NI]; int tl = mag_mul(t, r->d, r->len, ten, 1);
    uint64_t dig[1] = {(uint64_t)(*p - '0')};
    r->len = mag_add(r->d, t, tl, dig, dig[0] ? 1 : 0);
  }
  r->neg = (neg && r->len) ? 1 : 0;
}

void add_int(sail_int *rop, const sail_int a, const sail_int b) {
  si *r = (*rop);
  if (a->neg == b->neg) { r->len = mag_add(r->d, a->d, a->len, b->d, b->len); r->neg = r->len ? a->neg : 0; }
  else {
    int c = mag_cmp(a->d, a->len, b->d, b->len);
    if (c == 0) { r->len = 0; r->neg = 0; }
    else if (c > 0) { r->len = mag_sub(r->d, a->d, a->len, b->d, b->len); r->neg = r->len ? a->neg : 0; }
    else { r->len = mag_sub(r->d, b->d, b->len, a->d, a->len); r->neg = r->len ? b->neg : 0; }
  }
}
void neg_int(sail_int *rop, const sail_int a) {
  copy_sail_int(rop, a); (*rop)->neg = (*rop)->len ? !(*rop)->neg : 0;
}
void sub_int(sail_int *rop, const sail_int a, const sail_int b) {
  sail_int nb; copy_sail_int(&nb, b); nb->neg = nb->len ? !nb->neg : 0;
  add_int(rop, a, nb);
}
void mult_int(sail_int *rop, const sail_int a, const sail_int b) {
  si *r = (*rop); r->len = mag_mul(r->d, a->d, a->len, b->d, b->len);
  r->neg = r->len ? (a->neg ^ b->neg) : 0;
}
/* truncated division (toward zero): SDIV / SMOD */
void tdiv_int(sail_int *rop, const sail_int a, const sail_int b) {
  si *r = (*rop); uint64_t rem[NI]; int rl;
  mag_divmod(r->d, &r->len, rem, &rl, a->d, a->len, b->d, b->len);
  r->neg = r->len ? (a->neg ^ b->neg) : 0;
}
void tmod_int(sail_int *rop, const sail_int a, const sail_int b) {
  si *r = (*rop); uint64_t q[NI]; int ql;
  mag_divmod(q, &ql, r->d, &r->len, a->d, a->len, b->d, b->len);
  r->neg = r->len ? a->neg : 0;  /* remainder takes the dividend's sign */
}
/* Euclidean division: floor quotient, remainder in [0, |b|) */
void ediv_int(sail_int *rop, const sail_int a, const sail_int b) {
  si *r = (*rop); uint64_t qm[NI], rm[NI]; int ql, rl;
  mag_divmod(qm, &ql, rm, &rl, a->d, a->len, b->d, b->len);
  if (a->neg && rl) {                            /* adjust toward -inf */
    uint64_t one[1] = {1}; ql = mag_add(qm, qm, ql, one, 1);
  }
  for (int i = 0; i < ql; i++) r->d[i] = qm[i];
  r->len = ql; r->neg = r->len ? (a->neg ^ b->neg) : 0;
}
void emod_int(sail_int *rop, const sail_int a, const sail_int b) {
  si *r = (*rop); uint64_t qm[NI], rm[NI]; int ql, rl;
  mag_divmod(qm, &ql, rm, &rl, a->d, a->len, b->d, b->len);
  if (a->neg && rl) { r->len = mag_sub(r->d, b->d, b->len, rm, rl); }   /* |b| - rm */
  else { for (int i = 0; i < rl; i++) r->d[i] = rm[i]; r->len = rl; }
  r->neg = 0;
}
void pow2(sail_int *rop, const sail_int op) {
  si *r = (*rop); r->neg = 0;
  int e = op->len ? (int)op->d[0] : 0;
  if (e < 0 || e >= NI * 64) { r->len = 0; return; }
  int w = e >> 6;
  for (int i = 0; i <= w; i++) r->d[i] = 0;
  r->d[w] |= (uint64_t)1 << (e & 63);
  r->len = w + 1;
}

static int si_cmp(const sail_int a, const sail_int b) {
  if (a->neg != b->neg) return a->neg ? -1 : 1;
  int c = mag_cmp(a->d, a->len, b->d, b->len); return a->neg ? -c : c;
}
bool eq_int(const sail_int a, const sail_int b) { return si_cmp(a, b) == 0; }
bool lt(const sail_int a, const sail_int b)     { return si_cmp(a, b) <  0; }
bool gt(const sail_int a, const sail_int b)     { return si_cmp(a, b) >  0; }
bool lteq(const sail_int a, const sail_int b)   { return si_cmp(a, b) <= 0; }
bool gteq(const sail_int a, const sail_int b)   { return si_cmp(a, b) >= 0; }

/* low limb of a sail_int treated as a small unsigned index/shift/width. */
static uint64_t si_lo(const sail_int x) { return x->len ? x->d[0] : 0; }

/* ===================== lbits (256-bit inline) ========================== */
static uint64_t lb_word_mask(int bits) { return bits >= 64 ? ~0ULL : (((uint64_t)1 << bits) - 1); }
static void lb_mask(lbits *x) {
  int len = (int)x->len;
  for (int i = 0; i < 4; i++) {
    int lo = i * 64;
    if (len <= lo) x->d[i] = 0;
    else if (len < lo + 64) x->d[i] &= lb_word_mask(len - lo);
  }
}
static void d256_shl(uint64_t *d, int n) {
  if (n >= 256) { d[0] = d[1] = d[2] = d[3] = 0; return; }
  int w = n / 64, b = n % 64; uint64_t r[4] = {0, 0, 0, 0};
  for (int i = 3; i >= 0; i--) { int s = i - w; if (s < 0) continue;
    r[i] |= d[s] << b; if (b && s - 1 >= 0) r[i] |= d[s - 1] >> (64 - b); }
  for (int i = 0; i < 4; i++) d[i] = r[i];
}
static void d256_shr(uint64_t *d, int n) {
  if (n >= 256) { d[0] = d[1] = d[2] = d[3] = 0; return; }
  int w = n / 64, b = n % 64; uint64_t r[4] = {0, 0, 0, 0};
  for (int i = 0; i < 4; i++) { int s = i + w; if (s > 3) continue;
    r[i] |= d[s] >> b; if (b && s + 1 <= 3) r[i] |= d[s + 1] << (64 - b); }
  for (int i = 0; i < 4; i++) d[i] = r[i];
}

void create_lbits(lbits *rop) { rop->len = 0; rop->d[0] = rop->d[1] = rop->d[2] = rop->d[3] = 0; }
void recreate_lbits(lbits *rop) { create_lbits(rop); }
void kill_lbits(lbits *rop) { (void)rop; }
void copy_lbits(lbits *rop, const lbits op) { *rop = op; }

void zeros(lbits *rop, const sail_int op) {
  rop->len = si_lo(op); rop->d[0] = rop->d[1] = rop->d[2] = rop->d[3] = 0;
}
fbits convert_fbits_of_lbits(const lbits op, const bool direction) { (void)direction; return op.d[0]; }
void convert_lbits_of_fbits(lbits *rop, const fbits op, const uint64_t len, const bool direction) {
  (void)direction; rop->len = len; rop->d[0] = op; rop->d[1] = rop->d[2] = rop->d[3] = 0; lb_mask(rop);
}

void and_bits(lbits *rop, const lbits a, const lbits b) { rop->len = a.len; for (int i = 0; i < 4; i++) rop->d[i] = a.d[i] & b.d[i]; }
void or_bits(lbits *rop, const lbits a, const lbits b)  { rop->len = a.len; for (int i = 0; i < 4; i++) rop->d[i] = a.d[i] | b.d[i]; }
void xor_bits(lbits *rop, const lbits a, const lbits b) { rop->len = a.len; for (int i = 0; i < 4; i++) rop->d[i] = a.d[i] ^ b.d[i]; }
void not_bits(lbits *rop, const lbits a) { rop->len = a.len; for (int i = 0; i < 4; i++) rop->d[i] = ~a.d[i]; lb_mask(rop); }
void add_bits(lbits *rop, const lbits a, const lbits b) {
  rop->len = a.len; unsigned __int128 c = 0;
  for (int i = 0; i < 4; i++) { unsigned __int128 s = (unsigned __int128)a.d[i] + b.d[i] + c; rop->d[i] = (uint64_t)s; c = s >> 64; }
  lb_mask(rop);
}
void sub_bits(lbits *rop, const lbits a, const lbits b) {
  rop->len = a.len; unsigned __int128 bor = 0;
  for (int i = 0; i < 4; i++) { unsigned __int128 d = (unsigned __int128)a.d[i] - b.d[i] - bor; rop->d[i] = (uint64_t)d; bor = (d >> 64) & 1; }
  lb_mask(rop);
}
bool eq_bits(const lbits a, const lbits b)  { for (int i = 0; i < 4; i++) if (a.d[i] != b.d[i]) return false; return true; }
bool neq_bits(const lbits a, const lbits b) { return !eq_bits(a, b); }

void shiftl(lbits *rop, const lbits a, const sail_int n) { *rop = a; d256_shl(rop->d, (int)si_lo(n)); lb_mask(rop); }
void shiftr(lbits *rop, const lbits a, const sail_int n) { *rop = a; d256_shr(rop->d, (int)si_lo(n)); }
void zero_extend(lbits *rop, const lbits a, const sail_int len) { *rop = a; rop->len = si_lo(len); lb_mask(rop); }
void append(lbits *rop, const lbits a, const lbits b) {        /* a (high) ++ b (low) */
  lbits hi = a; d256_shl(hi.d, (int)b.len);
  rop->len = a.len + b.len;
  for (int i = 0; i < 4; i++) rop->d[i] = hi.d[i] | b.d[i];
  lb_mask(rop);
}
void append_64(lbits *rop, const lbits a, const fbits chunk) {
  lbits c = {64, {chunk, 0, 0, 0}}; append(rop, a, c);
}
void vector_subrange_lbits(lbits *rop, const lbits op, const sail_int hi, const sail_int lo) {
  int h = (int)si_lo(hi), l = (int)si_lo(lo);
  *rop = op; d256_shr(rop->d, l); rop->len = (uint64_t)(h - l + 1); lb_mask(rop);
}
fbits bitvector_access(const lbits op, const sail_int n) {
  int i = (int)si_lo(n); return (op.d[i >> 6] >> (i & 63)) & 1;
}

/* bit i of the two's-complement representation of sail_int x (magm1 = |x|-1,
 * with mlen its significant-limb count). */
static int si_twos_bit(const si *x, const uint64_t *magm1, int mlen, int i) {
  if (!x->neg) return (i < x->len * 64) ? (int)((x->d[i >> 6] >> (i & 63)) & 1) : 0;
  uint64_t b = (i < mlen * 64) ? ((magm1[i >> 6] >> (i & 63)) & 1) : 0;
  return (int)(b ^ 1);   /* two's complement of -|x| is ~(|x|-1) */
}
void get_slice_int(lbits *rop, const sail_int len, const sail_int n, const sail_int start) {
  int L = (int)si_lo(len), st = (int)si_lo(start);
  rop->len = (uint64_t)L; rop->d[0] = rop->d[1] = rop->d[2] = rop->d[3] = 0;
  uint64_t magm1[NI]; int mlen = 0;
  if (n->neg) { uint64_t one[1] = {1}; mlen = mag_sub(magm1, n->d, n->len, one, 1); }
  for (int i = 0; i < L; i++)
    if (si_twos_bit(n, magm1, mlen, st + i)) rop->d[i >> 6] |= (uint64_t)1 << (i & 63);
}
void sail_unsigned(sail_int *rop, const lbits op) {
  si *r = (*rop); r->neg = 0;
  for (int i = 0; i < 4; i++) r->d[i] = op.d[i];
  r->len = mag_norm(r->d, 4);
}
void sail_signed(sail_int *rop, const lbits op) {
  si *r = (*rop); r->neg = 0;
  for (int i = 0; i < 4; i++) r->d[i] = op.d[i];
  r->len = mag_norm(r->d, 4);
  int top = (int)op.len - 1;
  if (top >= 0 && ((op.d[top >> 6] >> (top & 63)) & 1)) {  /* negative: value - 2^len */
    uint64_t p2[NI]; for (int i = 0; i < NI; i++) p2[i] = 0;
    p2[op.len >> 6] = (uint64_t)1 << (op.len & 63);
    int p2len = (int)(op.len >> 6) + 1;
    r->len = mag_sub(r->d, p2, p2len, r->d, r->len); r->neg = r->len ? 1 : 0;
  }
}

/* GMP-API shim used by shared FFI (runner_ffi.c next_int): decimal string -> int */
int mpz_set_str(sail_int rop, const char *str, int base) {
  (void)base; si *r = (si *)rop; r->len = 0; r->neg = 0;
  const char *p = str; int neg = 0;
  if (*p == '-') { neg = 1; p++; }
  uint64_t ten[1] = {10};
  for (; *p; p++) {
    if (*p < '0' || *p > '9') continue;
    uint64_t t[NI]; int tl = mag_mul(t, r->d, r->len, ten, 1);
    uint64_t dig[1] = {(uint64_t)(*p - '0')};
    r->len = mag_add(r->d, t, tl, dig, dig[0] ? 1 : 0);
  }
  r->neg = (neg && r->len) ? 1 : 0;
  return 0;
}

/* ===================== strings / assert ================================ */
void create_sail_string(sail_string *str) { char *s = (char *)malloc(1); s[0] = 0; *str = s; }
void kill_sail_string(sail_string *str) { free((void *)*str); }
