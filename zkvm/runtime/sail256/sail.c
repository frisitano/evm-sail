/* ======================================================================== */
/* sail256: the HOST-OPTIMIZED variant of the sailfix runtime.                */
/*                                                                           */
/* Same representation and surface as sailfix (sail_int = 512-bit sign-       */
/* magnitude, lbits = inline 256-bit, identical sail.h) but with the hot      */
/* cores rewritten for speed:                                                 */
/*   - all magnitude ops are SIZED (loop over significant limbs, not all 8);   */
/*   - division is single-pass for 1-limb divisors and Knuth Algorithm D       */
/*     (base 2^64, Hacker's Delight divmnu64 shape) for multi-limb ones --     */
/*     sailfix's bit-by-bit restoring division was 512 iterations;             */
/*   - get_slice_int (= word_of_int) extracts limb-wise, not bit-by-bit;       */
/*   - decimal parsing accumulates 19-digit chunks (the EEST runner parses     */
/*     millions of stream integers), not a full 8x8 multiply per digit.        */
/* Differentially validated against the GMP build over the full EEST suite.   */
/* ======================================================================== */

#include "sail.h"
#include <string.h>

extern void *malloc(unsigned long);
extern void  free(void *);

/* ===================== 512-bit unsigned magnitude ====================== */
#define NI SAIL_INT_LIMBS   /* 8 limbs = 512 bits */

/* number of significant limbs (0 for the value 0) */
static inline int mag_len(const uint64_t *a) {
  int l = NI;
  while (l > 0 && !a[l - 1]) l--;
  return l;
}
static inline void mag_zero(uint64_t *a, int n) { for (int i = 0; i < n; i++) a[i] = 0; }
static int mag_cmp(const uint64_t *a, const uint64_t *b, int n) {
  (void)n;
  int la = mag_len(a), lb = mag_len(b);
  if (la != lb) return la < lb ? -1 : 1;
  for (int i = la - 1; i >= 0; i--) { if (a[i] < b[i]) return -1; if (a[i] > b[i]) return 1; }
  return 0;
}
/* r = a + b over NI limbs (sized loop; the carry tail is propagated) */
static uint64_t mag_add(uint64_t *r, const uint64_t *a, const uint64_t *b, int n) {
  (void)n;
  int la = mag_len(a), lb = mag_len(b);
  int l = la > lb ? la : lb;
  unsigned __int128 c = 0;
  for (int i = 0; i < l; i++) {
    unsigned __int128 s = (unsigned __int128)a[i] + b[i] + c;
    r[i] = (uint64_t)s; c = s >> 64;
  }
  if (l < NI && c) { r[l] = (uint64_t)c; c = 0; l++; }
  for (int i = l; i < NI; i++) r[i] = 0;
  return (uint64_t)c;
}
/* r = a - b (a >= b), sized */
static void mag_sub(uint64_t *r, const uint64_t *a, const uint64_t *b, int n) {
  (void)n;
  int la = mag_len(a);
  unsigned __int128 bor = 0;
  for (int i = 0; i < la; i++) {
    unsigned __int128 d = (unsigned __int128)a[i] - b[i] - bor;
    r[i] = (uint64_t)d; bor = (d >> 64) & 1;
  }
  for (int i = la; i < NI; i++) r[i] = 0;
}
/* r = a * b truncated to NI limbs, sized schoolbook */
static void mag_mul(uint64_t *r, const uint64_t *a, const uint64_t *b, int n) {
  (void)n;
  int la = mag_len(a), lb = mag_len(b);
  uint64_t t[NI]; mag_zero(t, NI);
  for (int i = 0; i < la; i++) {
    if (!a[i]) continue;
    unsigned __int128 carry = 0;
    int jmax = lb < NI - i ? lb : NI - i;
    for (int j = 0; j < jmax; j++) {
      unsigned __int128 s = (unsigned __int128)a[i] * b[j] + t[i + j] + carry;
      t[i + j] = (uint64_t)s; carry = s >> 64;
    }
    if (i + jmax < NI) t[i + jmax] += (uint64_t)carry;
  }
  for (int i = 0; i < NI; i++) r[i] = t[i];
}
/* r = a * m + add, single-limb multiplier (decimal-parse helper), sized */
static void mag_mul_small(uint64_t *r, const uint64_t *a, uint64_t m, uint64_t add) {
  int la = mag_len(a);
  unsigned __int128 carry = add;
  for (int i = 0; i < la; i++) {
    unsigned __int128 s = (unsigned __int128)a[i] * m + carry;
    r[i] = (uint64_t)s; carry = s >> 64;
  }
  int l = la;
  while (carry && l < NI) { r[l++] = (uint64_t)carry; carry >>= 64; }
  for (int i = l; i < NI; i++) r[i] = 0;
}

/* q = a / b, rem = a % b (unsigned), b != 0 */
static void mag_divmod(uint64_t *q, uint64_t *rem, const uint64_t *a, const uint64_t *b, int nn) {
  (void)nn;
  int m = mag_len(a), n = mag_len(b);
  mag_zero(q, NI); mag_zero(rem, NI);
  if (m < n || m == 0) { for (int i = 0; i < n && i < NI; i++) rem[i] = a[i]; return; }
  if (n == 1) {
    /* single-limb divisor: one pass, 128-bit intermediate */
    uint64_t d = b[0];
    unsigned __int128 r = 0;
    for (int i = m - 1; i >= 0; i--) {
      unsigned __int128 cur = (r << 64) | a[i];
      q[i] = (uint64_t)(cur / d);
      r = cur % d;
    }
    rem[0] = (uint64_t)r;
    return;
  }
  /* Knuth Algorithm D, base 2^64 (Hacker's Delight divmnu64 shape) */
  int s = __builtin_clzll(b[n - 1]);
  uint64_t v[NI], u[NI + 1];
  if (s) {
    for (int i = n - 1; i > 0; i--) v[i] = (b[i] << s) | (b[i - 1] >> (64 - s));
    v[0] = b[0] << s;
    u[m] = a[m - 1] >> (64 - s);
    for (int i = m - 1; i > 0; i--) u[i] = (a[i] << s) | (a[i - 1] >> (64 - s));
    u[0] = a[0] << s;
  } else {
    for (int i = 0; i < n; i++) v[i] = b[i];
    for (int i = 0; i < m; i++) u[i] = a[i];
    u[m] = 0;
  }
  for (int j = m - n; j >= 0; j--) {
    unsigned __int128 num = ((unsigned __int128)u[j + n] << 64) | u[j + n - 1];
    unsigned __int128 qhat = num / v[n - 1];
    unsigned __int128 rhat = num % v[n - 1];
    while ((qhat >> 64) != 0 ||
           (unsigned __int128)(uint64_t)qhat * v[n - 2] > ((rhat << 64) | u[j + n - 2])) {
      qhat--; rhat += v[n - 1];
      if ((rhat >> 64) != 0) break;
    }
    /* multiply-subtract u[j..j+n] -= qhat * v */
    uint64_t qh = (uint64_t)qhat;
    uint64_t mul_carry = 0;
    unsigned __int128 borrow = 0;
    for (int i = 0; i < n; i++) {
      unsigned __int128 p = (unsigned __int128)qh * v[i] + mul_carry;
      mul_carry = (uint64_t)(p >> 64);
      unsigned __int128 d = (unsigned __int128)u[i + j] - (uint64_t)p - borrow;
      u[i + j] = (uint64_t)d;
      borrow = (d >> 64) & 1;
    }
    unsigned __int128 d = (unsigned __int128)u[j + n] - mul_carry - borrow;
    u[j + n] = (uint64_t)d;
    if ((d >> 64) & 1) {                       /* went negative: add back */
      qh--;
      unsigned __int128 c = 0;
      for (int i = 0; i < n; i++) {
        unsigned __int128 ss = (unsigned __int128)u[i + j] + v[i] + c;
        u[i + j] = (uint64_t)ss; c = ss >> 64;
      }
      u[j + n] += (uint64_t)c;
    }
    q[j] = qh;
  }
  /* remainder = u[0..n-1] >> s */
  if (s) {
    for (int i = 0; i < n - 1; i++) rem[i] = (u[i] >> s) | (u[i + 1] << (64 - s));
    rem[n - 1] = u[n - 1] >> s;
  } else {
    for (int i = 0; i < n; i++) rem[i] = u[i];
  }
}

/* ===================== sail_int (sign-magnitude) ======================= */
typedef sail_int_struct si;
static void si_norm(si *x) { if (mag_len(x->d) == 0) x->neg = 0; }

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

/* chunked decimal parse: up to 19 digits per 64-bit chunk, then one sized
 * mul_small + add per chunk (sailfix did a full 8x8 multiply PER DIGIT). */
static void parse_dec(si *r, const char *str) {
  static const uint64_t POW10[20] = {
    1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL, 1000000ULL, 10000000ULL,
    100000000ULL, 1000000000ULL, 10000000000ULL, 100000000000ULL, 1000000000000ULL,
    10000000000000ULL, 100000000000000ULL, 1000000000000000ULL, 10000000000000000ULL,
    100000000000000000ULL, 1000000000000000000ULL, 10000000000000000000ULL };
  mag_zero(r->d, NI); r->neg = 0;
  const char *p = str; int neg = 0;
  if (*p == '-') { neg = 1; p++; }
  uint64_t chunk = 0; int k = 0;
  for (; *p; p++) {
    if (*p < '0' || *p > '9') continue;
    chunk = chunk * 10 + (uint64_t)(*p - '0');
    if (++k == 19) { mag_mul_small(r->d, r->d, POW10[19], chunk); chunk = 0; k = 0; }
  }
  if (k) mag_mul_small(r->d, r->d, POW10[k], chunk);
  r->neg = (neg && mag_len(r->d) != 0) ? 1 : 0;
}
void convert_sail_int_of_sail_string(sail_int *rop, const_sail_string str) { parse_dec((*rop), str); }
int mpz_set_str(sail_int rop, const char *str, int base) { (void)base; parse_dec((si *)rop, str); return 0; }

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
  int rzero = mag_len(rm) == 0;
  if (a->neg && !rzero) {                       /* adjust toward -inf */
    uint64_t one[NI]; mag_zero(one, NI); one[0] = 1; mag_add(qm, qm, one, NI);
  }
  for (int i = 0; i < NI; i++) r->d[i] = qm[i];
  r->neg = (a->neg ^ b->neg); si_norm(r);
}
void emod_int(sail_int *rop, const sail_int a, const sail_int b) {
  si *r = (*rop); uint64_t qm[NI], rm[NI]; mag_divmod(qm, rm, a->d, b->d, NI);
  int rzero = mag_len(rm) == 0;
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

/* limb-wise get_slice_int (sailfix looped per BIT; this is word_of_int, one of
 * the hottest runtime calls in the model). Two's-complement limbs of n are
 * materialized once, then the [start, start+len) window is limb-extracted. */
void get_slice_int(lbits *rop, const sail_int len, const sail_int n, const sail_int start) {
  int L = (int)len->d[0], st = (int)start->d[0];
  rop->len = (uint64_t)L;
  rop->d[0] = rop->d[1] = rop->d[2] = rop->d[3] = 0;
  uint64_t tw[NI];
  uint64_t ext;                                /* sign extension beyond 512 bits */
  if (!n->neg) {
    for (int i = 0; i < NI; i++) tw[i] = n->d[i];
    ext = 0;
  } else {                                     /* two's complement: ~(|n|-1) */
    uint64_t one[NI], m1[NI];
    mag_zero(one, NI); one[0] = 1;
    mag_sub(m1, n->d, one, NI);
    for (int i = 0; i < NI; i++) tw[i] = ~m1[i];
    ext = ~0ULL;
  }
  int nw = (L + 63) / 64;
  for (int i = 0; i < nw && i < 4; i++) {
    int bitpos = st + i * 64;
    int w = bitpos >> 6, b = bitpos & 63;
    uint64_t lo = (w >= 0 && w < NI) ? tw[w] : ext;
    uint64_t hi = (w + 1 >= 0 && w + 1 < NI) ? tw[w + 1] : ext;
    rop->d[i] = b ? ((lo >> b) | (hi << (64 - b))) : lo;
  }
  lb_mask(rop);
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

/* ===================== strings / assert ================================ */
void create_sail_string(sail_string *str) { char *s = (char *)malloc(1); s[0] = 0; *str = s; }
void kill_sail_string(sail_string *str) { free((void *)*str); }
