/* C externs for the evm-sail EEST runner. The test input arrives on stdin as a
 * stream of varints ([len][big-endian bytes]) -- the same byte source the model
 * reads everywhere via ssz_src (io.sail); the run_eest.py / revm-eest encoders
 * produce it. We buffer all of stdin once and serve it by index. */
#include "sail.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned char *g_in = NULL;
static long g_len = -1;
static void ensure(void) {
    if (g_len >= 0) return;
    size_t cap = 1u << 16, n = 0;
    g_in = (unsigned char *)malloc(cap);
    int c;
    while ((c = getchar()) != EOF) {
        if (n >= cap) { cap <<= 1; g_in = (unsigned char *)realloc(g_in, cap); }
        g_in[n++] = (unsigned char)c;
    }
    g_len = (long)n;
}
uint64_t ssz_src_byte(sail_int idx) {
    ensure();
    long i = (long)mpz_get_ui(idx);
    return (i >= 0 && i < g_len) ? (uint64_t)g_in[i] : 0;
}
void ssz_src_len(sail_int rop, const unit u) {
    (void)u; ensure(); mpz_set_ui(rop, (unsigned long)(g_len < 0 ? 0 : g_len));
}
uint64_t ssz_src_le(sail_int off, sail_int n) {
    ensure();
    long o = (long)mpz_get_ui(off), k = (long)mpz_get_ui(n); uint64_t v = 0;
    for (long i = 0; i < k; i++) { uint64_t b = (o + i >= 0 && o + i < g_len) ? g_in[o + i] : 0; v |= b << (8 * i); }
    return v;
}
uint64_t ssz_src_be(sail_int off, sail_int n) {
    ensure();
    long o = (long)mpz_get_ui(off), k = (long)mpz_get_ui(n); uint64_t v = 0;
    for (long i = 0; i < k; i++) { uint64_t b = (o + i >= 0 && o + i < g_len) ? g_in[o + i] : 0; v = (v << 8) | b; }
    return v;
}
