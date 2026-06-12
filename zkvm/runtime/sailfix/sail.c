/* ======================================================================== */
/* sailfix: a GMP-free, fixed-width implementation of the Sail C runtime's     */
/* int + bits primitives, replacing the mpz-backed sail.c + mini-gmp.          */
/*                                                                           */
/*   sail_int : 512-bit sign-magnitude (array-of-1, mpz_t-style ABI). 512 bits */
/*              covers every value the EVM produces (MUL/MULMOD/EXP compute     */
/*              unsigned(a)*unsigned(b) <= 2^512 before truncation).           */
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

/* ===================== 512-bit unsigned magnitude ====================== */
#define NI SAIL_INT_LIMBS   /* 8 limbs = 512 bits */

static void mag_zero(uint64_t *a, int n) { for (int i = 0; i < n; i++) a[i] = 0; }
static int  mag_is_zero(const uint64_t *a, int n) { for (int i = 0; i < n; i++) if (a[i]) return 0; return 1; }
static int  mag_cmp(const uint64_t *a, const uint64_t *b, int n) {
  for (int i = n - 1; i >= 0; i--) { if (a[i] < b[i]) return -1; if (a[i] > b[i]) return 1; }
  return 0;
}
static uint64_t mag_add(uint64_t *r, const uint64_t *a, const uint64_t *b, int n) {
  unsigned __int128 c = 0;
  for (int i = 0; i < n; i++) { unsigned __int128 s = (unsigned __int128)a[i] + b[i] + c; r[i] = (uint64_t)s; c = s >> 64; }
  return (uint64_t)c;
}
static void mag_sub(uint64_t *r, const uint64_t *a, const uint64_t *b, int n) { /* a >= b */
  unsigned __int128 bor = 0;
  for (int i = 0; i < n; i++) { unsigned __int128 d = (unsigned __int128)a[i] - b[i] - bor; r[i] = (uint64_t)d; bor = (d >> 64) & 1; }
}
static void mag_mul(uint64_t *r, const uint64_t *a, const uint64_t *b, int n) {
  uint64_t t[NI]; mag_zero(t, n);
  for (int i = 0; i < n; i++) {
    if (!a[i]) continue;
    unsigned __int128 carry = 0;
    for (int j = 0; i + j < n; j++) {
      unsigned __int128 s = (unsigned __int128)a[i] * b[j] + t[i + j] + carry;
      t[i + j] = (uint64_t)s; carry = s >> 64;
    }
  }
  for (int i = 0; i < n; i++) r[i] = t[i];
}
/* q = a / b, rem = a % b (unsigned), b != 0 */
static void mag_divmod(uint64_t *q, uint64_t *rem, const uint64_t *a, const uint64_t *b, int n) {
  /* fast path: single-limb operands */
  int ahi = n - 1, bhi = n - 1;
  while (ahi > 0 && !a[ahi]) ahi--;
  while (bhi > 0 && !b[bhi]) bhi--;
  mag_zero(q, n); mag_zero(rem, n);
  if (ahi == 0 && bhi == 0) { q[0] = a[0] / b[0]; rem[0] = a[0] % b[0]; return; }
  uint64_t r[NI]; mag_zero(r, n);
  for (int bit = n * 64 - 1; bit >= 0; bit--) {
    uint64_t carry = 0;                          /* r <<= 1 */
    for (int i = 0; i < n; i++) { uint64_t nc = r[i] >> 63; r[i] = (r[i] << 1) | carry; carry = nc; }
    r[0] |= (a[bit >> 6] >> (bit & 63)) & 1;      /* bring in bit of a */
    if (mag_cmp(r, b, n) >= 0) { mag_sub(r, r, b, n); q[bit >> 6] |= (uint64_t)1 << (bit & 63); }
  }
  for (int i = 0; i < n; i++) rem[i] = r[i];
}

/* ===================== sail_int (sign-magnitude) ======================= */
typedef sail_int_struct si;
static void si_norm(si *x) { if (mag_is_zero(x->d, NI)) x->neg = 0; }

void create_sail_int(sail_int *rop) { mag_zero((*rop)->d, NI); (*rop)->neg = 0; }
void kill_sail_int(sail_int *rop)   { (void)rop; }
void copy_sail_int(sail_int *rop, const sail_int op) {
  for (int i = 0; i < NI; i++) (*rop)->d[i] = op->d[i]; (*rop)->neg = op->neg;
}
uint64_t sail_int_get_ui(const sail_int op) { return op->d[0]; }
mach_int convert_mach_int_of_sail_int(const sail_int op) {
  int64_t v = (int64_t)op->d[0];
  return op->neg ? -v : v;
}
void convert_sail_int_of_mach_int(sail_int *rop, const mach_int op) {
  si *r = (*rop); mag_zero(r->d, NI);
  if (op < 0) { r->neg = 1; r->d[0] = (uint64_t)(-(op + 1)) + 1; }
  else { r->neg = 0; r->d[0] = (uint64_t)op; }
}
void convert_sail_int_of_sail_string(sail_int *rop, const_sail_string str) {
  si *r = (*rop); mag_zero(r->d, NI); r->neg = 0;
  const char *p = str; int neg = 0;
  if (*p == '-') { neg = 1; p++; }
  for (; *p; p++) {
    if (*p < '0' || *p > '9') continue;
    uint64_t ten[NI]; mag_zero(ten, NI); ten[0] = 10;
    uint64_t t[NI];   mag_mul(t, r->d, ten, NI);
    uint64_t dig[NI]; mag_zero(dig, NI); dig[0] = (uint64_t)(*p - '0');
    mag_add(r->d, t, dig, NI);
  }
  r->neg = (neg && !mag_is_zero(r->d, NI)) ? 1 : 0;
}

void add_int(sail_int *rop, const sail_int a, const sail_int b) {
  si *r = (*rop);
  if (a->neg == b->neg) { mag_add(r->d, a->d, b->d, NI); r->neg = a->neg; }
  else { int c = mag_cmp(a->d, b->d, NI);
    if (c >= 0) { mag_sub(r->d, a->d, b->d, NI); r->neg = a->neg; }
    else        { mag_sub(r->d, b->d, a->d, NI); r->neg = b->neg; } }
  si_norm(r);
}
void neg_int(sail_int *rop, const sail_int a) { copy_sail_int(rop, a); (*rop)->neg ^= 1; si_norm((*rop)); }
void sub_int(sail_int *rop, const sail_int a, const sail_int b) {
  sail_int nb; copy_sail_int(&nb, b); nb->neg ^= 1; si_norm(nb);
  add_int(rop, a, nb);
}
void mult_int(sail_int *rop, const sail_int a, const sail_int b) {
  si *r = (*rop); mag_mul(r->d, a->d, b->d, NI); r->neg = (a->neg ^ b->neg); si_norm(r);
}
/* truncated division (toward zero): SDIV / SMOD */
void tdiv_int(sail_int *rop, const sail_int a, const sail_int b) {
  si *r = (*rop); uint64_t rem[NI]; mag_divmod(r->d, rem, a->d, b->d, NI);
  r->neg = (a->neg ^ b->neg); si_norm(r);
}
void tmod_int(sail_int *rop, const sail_int a, const sail_int b) {
  si *r = (*rop); uint64_t q[NI]; mag_divmod(q, r->d, a->d, b->d, NI);
  r->neg = a->neg; si_norm(r);  /* remainder takes the dividend's sign */
}
/* Euclidean division: floor quotient, remainder in [0, |b|) */
void ediv_int(sail_int *rop, const sail_int a, const sail_int b) {
  si *r = (*rop); uint64_t qm[NI], rm[NI]; mag_divmod(qm, rm, a->d, b->d, NI);
  int rzero = mag_is_zero(rm, NI);
  if (a->neg && !rzero) {                       /* adjust toward -inf */
    uint64_t one[NI]; mag_zero(one, NI); one[0] = 1; mag_add(qm, qm, one, NI);
  }
  for (int i = 0; i < NI; i++) r->d[i] = qm[i];
  r->neg = (a->neg ^ b->neg); si_norm(r);
}
void emod_int(sail_int *rop, const sail_int a, const sail_int b) {
  si *r = (*rop); uint64_t qm[NI], rm[NI]; mag_divmod(qm, rm, a->d, b->d, NI);
  int rzero = mag_is_zero(rm, NI);
  if (a->neg && !rzero) { mag_sub(r->d, b->d, rm, NI); }   /* |b| - rm */
  else { for (int i = 0; i < NI; i++) r->d[i] = rm[i]; }
  r->neg = 0; si_norm(r);
}
void pow2(sail_int *rop, const sail_int op) {
  si *r = (*rop); mag_zero(r->d, NI); r->neg = 0;
  int e = (int)op->d[0];
  if (e >= 0 && e < NI * 64) r->d[e >> 6] = (uint64_t)1 << (e & 63);
}

static int si_cmp(const sail_int a, const sail_int b) {
  if (a->neg != b->neg) return a->neg ? -1 : 1;
  int c = mag_cmp(a->d, b->d, NI); return a->neg ? -c : c;
}
bool eq_int(const sail_int a, const sail_int b) { return si_cmp(a, b) == 0; }
bool lt(const sail_int a, const sail_int b)     { return si_cmp(a, b) <  0; }
bool gt(const sail_int a, const sail_int b)     { return si_cmp(a, b) >  0; }
bool lteq(const sail_int a, const sail_int b)   { return si_cmp(a, b) <= 0; }
bool gteq(const sail_int a, const sail_int b)   { return si_cmp(a, b) >= 0; }

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
  rop->len = op->d[0]; rop->d[0] = rop->d[1] = rop->d[2] = rop->d[3] = 0;
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

void shiftl(lbits *rop, const lbits a, const sail_int n) { *rop = a; d256_shl(rop->d, (int)n->d[0]); lb_mask(rop); }
void shiftr(lbits *rop, const lbits a, const sail_int n) { *rop = a; d256_shr(rop->d, (int)n->d[0]); }
void zero_extend(lbits *rop, const lbits a, const sail_int len) { *rop = a; rop->len = len->d[0]; lb_mask(rop); }
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
  int h = (int)hi->d[0], l = (int)lo->d[0];
  *rop = op; d256_shr(rop->d, l); rop->len = (uint64_t)(h - l + 1); lb_mask(rop);
}
fbits bitvector_access(const lbits op, const sail_int n) {
  int i = (int)n->d[0]; return (op.d[i >> 6] >> (i & 63)) & 1;
}

/* bit i of the two's-complement representation of sail_int x (magm1 = |x|-1) */
static int si_twos_bit(const si *x, const uint64_t *magm1, int i) {
  if (!x->neg) return (i < NI * 64) ? (int)((x->d[i >> 6] >> (i & 63)) & 1) : 0;
  uint64_t b = (i < NI * 64) ? ((magm1[i >> 6] >> (i & 63)) & 1) : 0;
  return (int)(b ^ 1);
}
void get_slice_int(lbits *rop, const sail_int len, const sail_int n, const sail_int start) {
  int L = (int)len->d[0], st = (int)start->d[0];
  rop->len = (uint64_t)L; rop->d[0] = rop->d[1] = rop->d[2] = rop->d[3] = 0;
  uint64_t magm1[NI];
  if (n->neg) { uint64_t one[NI]; mag_zero(one, NI); one[0] = 1; mag_sub(magm1, n->d, one, NI); }
  for (int i = 0; i < L; i++)
    if (si_twos_bit(n, magm1, st + i)) rop->d[i >> 6] |= (uint64_t)1 << (i & 63);
}
void sail_unsigned(sail_int *rop, const lbits op) {
  si *r = (*rop); mag_zero(r->d, NI); r->neg = 0;
  for (int i = 0; i < 4; i++) r->d[i] = op.d[i];
}
void sail_signed(sail_int *rop, const lbits op) {
  si *r = (*rop); mag_zero(r->d, NI); r->neg = 0;
  for (int i = 0; i < 4; i++) r->d[i] = op.d[i];
  int top = (int)op.len - 1;
  if (top >= 0 && ((op.d[top >> 6] >> (top & 63)) & 1)) {  /* negative: value - 2^len */
    uint64_t p2[NI]; mag_zero(p2, NI); p2[op.len >> 6] = (uint64_t)1 << (op.len & 63);
    mag_sub(r->d, p2, r->d, NI); r->neg = 1; si_norm(r);
  }
}

/* GMP-API shim used by shared FFI (runner_ffi.c next_int): decimal string -> int */
int mpz_set_str(sail_int rop, const char *str, int base) {
  (void)base; si *r = (si *)rop; mag_zero(r->d, NI); r->neg = 0;
  const char *p = str; int neg = 0;
  if (*p == '-') { neg = 1; p++; }
  for (; *p; p++) {
    if (*p < '0' || *p > '9') continue;
    uint64_t ten[NI]; mag_zero(ten, NI); ten[0] = 10;
    uint64_t t[NI];   mag_mul(t, r->d, ten, NI);
    uint64_t dig[NI]; mag_zero(dig, NI); dig[0] = (uint64_t)(*p - '0');
    mag_add(r->d, t, dig, NI);
  }
  r->neg = (neg && !mag_is_zero(r->d, NI)) ? 1 : 0;
  return 0;
}

/* ===================== strings / assert ================================ */
void create_sail_string(sail_string *str) { char *s = (char *)malloc(1); s[0] = 0; *str = s; }
void kill_sail_string(sail_string *str) { free((void *)*str); }
