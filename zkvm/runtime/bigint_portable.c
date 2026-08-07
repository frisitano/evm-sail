/* Portable software provider for the ffi/zkvm_bigint.h 256-bit arithmetic
 * contract: Knuth Algorithm D division over 64-bit limbs plus full-width
 * (512-bit product / 257-bit sum) modular reduction.
 *
 * Linked ONLY into the Spike guest (zkvm/build.sh cmd_guest), which has no
 * arithmetic precompile. The ZisK guest links zkvm/zisk/bigint.c instead;
 * linking both would collide on the zkvm_u256_* definitions. The native
 * harness resolves the contract from the accel-host Rust cdylib.
 *
 * Contract preconditions (divisor/modulus nonzero) are enforced in Sail;
 * a zero divisor here would divide by zero in hardware. */

#include <stdint.h>

#include "zkvm_bigint.h"

typedef unsigned __int128 u128;

/* Number of significant limbs (0 for the value zero). */
static int limbs_used(const uint64_t *x, int n)
{
    while (n > 0 && x[n - 1] == 0) {
        n--;
    }
    return n;
}

/* Knuth Algorithm D (TAOCP 4.3.1) in base 2^64.
 *
 * Divides u (m limbs) by v (n limbs), writing an (m - n + 1)-limb quotient
 * and an n-limb remainder. Requires 2 <= n <= m <= 8 and v[n - 1] != 0. */
static void knuth_divrem(const uint64_t *u, int m, const uint64_t *v, int n,
                         uint64_t *q, uint64_t *r)
{
    uint64_t un[9];
    uint64_t vn[8];
    const int s = __builtin_clzll(v[n - 1]);

    /* D1: normalize so the divisor's top limb has its high bit set. */
    if (s == 0) {
        for (int i = 0; i < n; i++) {
            vn[i] = v[i];
        }
        for (int i = 0; i < m; i++) {
            un[i] = u[i];
        }
        un[m] = 0;
    } else {
        for (int i = n - 1; i > 0; i--) {
            vn[i] = (v[i] << s) | (v[i - 1] >> (64 - s));
        }
        vn[0] = v[0] << s;
        un[m] = u[m - 1] >> (64 - s);
        for (int i = m - 1; i > 0; i--) {
            un[i] = (u[i] << s) | (u[i - 1] >> (64 - s));
        }
        un[0] = u[0] << s;
    }

    /* D2-D7: one quotient limb per round, most significant first. */
    for (int j = m - n; j >= 0; j--) {
        /* D3: estimate the quotient limb from the top two dividend limbs. */
        const u128 num = ((u128)un[j + n] << 64) | un[j + n - 1];
        u128 qhat = num / vn[n - 1];
        u128 rhat = num % vn[n - 1];
        while ((qhat >> 64) != 0 ||
               (u128)(uint64_t)qhat * vn[n - 2] >
                   ((rhat << 64) | un[j + n - 2])) {
            qhat--;
            rhat += vn[n - 1];
            if ((rhat >> 64) != 0) {
                break;
            }
        }

        /* D4: multiply and subtract. */
        uint64_t qh = (uint64_t)qhat;
        uint64_t mul_carry = 0;
        uint64_t borrow = 0;
        for (int i = 0; i < n; i++) {
            const u128 p = (u128)qh * vn[i] + mul_carry;
            mul_carry = (uint64_t)(p >> 64);
            const u128 d = (u128)un[i + j] - (uint64_t)p - borrow;
            un[i + j] = (uint64_t)d;
            borrow = (uint64_t)((d >> 64) & 1);
        }
        const u128 d = (u128)un[j + n] - mul_carry - borrow;
        un[j + n] = (uint64_t)d;

        /* D5-D6: the estimate was one too large; add the divisor back. */
        if ((d >> 64) != 0) {
            qh--;
            uint64_t carry = 0;
            for (int i = 0; i < n; i++) {
                const u128 t = (u128)un[i + j] + vn[i] + carry;
                un[i + j] = (uint64_t)t;
                carry = (uint64_t)(t >> 64);
            }
            un[j + n] += carry;
        }
        q[j] = qh;
    }

    /* D8: denormalize the remainder. */
    if (s == 0) {
        for (int i = 0; i < n; i++) {
            r[i] = un[i];
        }
    } else {
        for (int i = 0; i < n - 1; i++) {
            r[i] = (un[i] >> s) | (un[i + 1] << (64 - s));
        }
        r[n - 1] = un[n - 1] >> s;
    }
}

/* General divrem over trimmed operands: u (up to 8 limbs) by the nonzero
 * 4-limb divisor v, writing 8 quotient limbs and 4 remainder limbs. */
static void divrem_wide(const uint64_t *u, int u_limbs, const uint64_t v[4],
                        uint64_t q[8], uint64_t r[4])
{
    const int m = limbs_used(u, u_limbs);
    const int n = limbs_used(v, 4);

    for (int i = 0; i < 8; i++) {
        q[i] = 0;
    }
    for (int i = 0; i < 4; i++) {
        r[i] = 0;
    }
    if (m < n) {
        for (int i = 0; i < m; i++) {
            r[i] = u[i];
        }
        return;
    }
    if (n == 1) {
        const uint64_t d = v[0];
        uint64_t rem = 0;
        for (int j = m - 1; j >= 0; j--) {
            const u128 cur = ((u128)rem << 64) | u[j];
            q[j] = (uint64_t)(cur / d);
            rem = (uint64_t)(cur % d);
        }
        r[0] = rem;
        return;
    }
    knuth_divrem(u, m, v, n, q, r);
}

void zkvm_u256_divrem(const uint64_t a[4], const uint64_t b[4],
                      uint64_t quotient[4], uint64_t remainder[4])
{
    uint64_t q[8];
    uint64_t r[4];
    divrem_wide(a, 4, b, q, r);
    for (int i = 0; i < 4; i++) {
        quotient[i] = q[i];
        remainder[i] = r[i];
    }
}

void zkvm_u256_mulmod(const uint64_t a[4], const uint64_t b[4],
                      const uint64_t n[4], uint64_t out[4])
{
    /* Full 512-bit schoolbook product; each partial sum fits 128 bits. */
    uint64_t product[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 4; i++) {
        uint64_t carry = 0;
        for (int j = 0; j < 4; j++) {
            const u128 t = (u128)a[i] * b[j] + product[i + j] + carry;
            product[i + j] = (uint64_t)t;
            carry = (uint64_t)(t >> 64);
        }
        product[i + 4] = carry;
    }
    uint64_t q[8];
    divrem_wide(product, 8, n, q, out);
}

void zkvm_u256_addmod(const uint64_t a[4], const uint64_t b[4],
                      const uint64_t n[4], uint64_t out[4])
{
    /* Full 257-bit sum: no 2^256 truncation before the reduction. */
    uint64_t sum[5];
    uint64_t carry = 0;
    for (int i = 0; i < 4; i++) {
        const u128 t = (u128)a[i] + b[i] + carry;
        sum[i] = (uint64_t)t;
        carry = (uint64_t)(t >> 64);
    }
    sum[4] = carry;
    uint64_t q[8];
    divrem_wide(sum, 5, n, q, out);
}
