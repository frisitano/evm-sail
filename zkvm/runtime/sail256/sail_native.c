/* ======================================================================== */
/* sailfix native support: the string/print/rts surface a hosted Sail program  */
/* (e.g. the EEST runner) needs beyond the freestanding guest's ~40 functions.  */
/* Lets the native EEST runner link against the GMP-free sailfix runtime so the  */
/* full EEST suite validates sailfix's int/bits arithmetic (not just the guest). */
/* Linked ONLY in native hosted builds, never in the freestanding zkVM guest.   */
/* ======================================================================== */

#include "sail.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* divide an 8-limb magnitude by 10 in place, return the remainder digit */
static uint64_t div_small(uint64_t *mag, int n, uint64_t d)
{
    unsigned __int128 rem = 0;
    for (int i = n - 1; i >= 0; i--) {
        unsigned __int128 cur = (rem << 64) | mag[i];
        mag[i] = (uint64_t)(cur / d);
        rem = cur % d;
    }
    return (uint64_t)rem;
}
static int mag_zero_p(const uint64_t *mag, int n)
{
    for (int i = 0; i < n; i++) if (mag[i]) return 0;
    return 1;
}

void dec_str(sail_string *out, const sail_int n)
{
    char buf[176];
    int pos = (int)sizeof(buf);
    uint64_t mag[SAIL_INT_LIMBS];
    for (int i = 0; i < SAIL_INT_LIMBS; i++) mag[i] = n->d[i];
    if (mag_zero_p(mag, SAIL_INT_LIMBS)) {
        buf[--pos] = '0';
    } else {
        while (!mag_zero_p(mag, SAIL_INT_LIMBS)) {
            uint64_t dig = div_small(mag, SAIL_INT_LIMBS, 10);
            buf[--pos] = (char)('0' + dig);
        }
        if (n->neg) buf[--pos] = '-';
    }
    int len = (int)sizeof(buf) - pos;
    char *s = (char *)malloc((size_t)len + 1);
    memcpy(s, buf + pos, (size_t)len);
    s[len] = '\0';
    *out = s;
}

static void hex_str_impl(sail_string *out, const sail_int n, const char *digits)
{
    /* highest non-zero limb */
    int hi = SAIL_INT_LIMBS - 1;
    while (hi > 0 && n->d[hi] == 0) hi--;
    char buf[160];
    int pos = 0;
    buf[pos++] = '0'; buf[pos++] = 'x';
    if (n->d[hi] == 0) {
        buf[pos++] = '0';
    } else {
        int started = 0;
        for (int limb = hi; limb >= 0; limb--) {
            for (int nib = 15; nib >= 0; nib--) {
                int v = (int)((n->d[limb] >> (nib * 4)) & 0xf);
                if (!started && v == 0 && !(limb == 0 && nib == 0)) continue;
                started = 1;
                buf[pos++] = digits[v];
            }
        }
    }
    char *s = (char *)malloc((size_t)pos + 1);
    memcpy(s, buf, (size_t)pos);
    s[pos] = '\0';
    *out = s;
}
void hex_str(sail_string *out, const sail_int n)       { hex_str_impl(out, n, "0123456789abcdef"); }
void hex_str_upper(sail_string *out, const sail_int n) { hex_str_impl(out, n, "0123456789ABCDEF"); }

void concat_str(sail_string *out, const_sail_string a, const_sail_string b)
{
    size_t la = strlen(a), lb = strlen(b);
    char *s = (char *)malloc(la + lb + 1);
    memcpy(s, a, la); memcpy(s + la, b, lb); s[la + lb] = '\0';
    *out = s;
}

unit print_endline(const_sail_string str) { fputs(str, stdout); fputc('\n', stdout); fflush(stdout); return UNIT; }

/* rts stubs (the GMP-heavy rts.c is not linked; sailfix needs none of it) */
void setup_rts(void)   {}
void cleanup_rts(void) {}
int  process_arguments(int argc, char **argv) { (void)argc; (void)argv; return 0; }  /* 0 = ok */
