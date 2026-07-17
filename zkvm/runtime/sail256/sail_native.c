/* Hosted-only string, printing, and RTS support for sail256. */

#include "sail.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void dec_str(sail_string *out, const sail_int n)
{
    uint64_t magnitude[SAIL_INT_LIMBS] = {0};
    for (uint8_t i = 0; i < n->length; i++) magnitude[i] = n->limbs[i];

    /* ceil(log10(2^768)) = 232, plus sign and terminator. */
    char reversed[234];
    size_t length = 0;
    uint8_t limbs = n->length;
    do {
        unsigned __int128 remainder = 0;
        for (int i = (int)limbs - 1; i >= 0; i--) {
            unsigned __int128 dividend = (remainder << 64) | magnitude[i];
            magnitude[i] = (uint64_t)(dividend / 10);
            remainder = dividend % 10;
        }
        while (limbs > 0 && magnitude[limbs - 1] == 0) limbs--;
        reversed[length++] = (char)('0' + remainder);
    } while (limbs != 0);
    if (n->negative) reversed[length++] = '-';

    char *string = (char *)malloc(length + 1);
    for (size_t i = 0; i < length; i++) string[i] = reversed[length - i - 1];
    string[length] = '\0';
    free(*out);
    *out = string;
}

static void hex_str_impl(sail_string *out, const sail_int n, const char *digits)
{
    const size_t prefix = n->negative ? 3 : 2;
    const size_t digits_length = n->length == 0
        ? 1
        : (size_t)(n->length - 1) * 16
            + (size_t)((64 - __builtin_clzll(n->limbs[n->length - 1]) + 3) / 4);
    char *string = (char *)malloc(prefix + digits_length + 1);
    size_t position = 0;
    if (n->negative) string[position++] = '-';
    string[position++] = '0';
    string[position++] = 'x';
    if (n->length == 0) {
        string[position++] = '0';
    } else {
        bool leading = true;
        for (int limb = (int)n->length - 1; limb >= 0; limb--) {
            for (int shift = 60; shift >= 0; shift -= 4) {
                uint8_t nibble = (uint8_t)((n->limbs[limb] >> shift) & 0xf);
                if (leading && nibble == 0) continue;
                leading = false;
                string[position++] = digits[nibble];
            }
        }
    }
    string[position] = '\0';
    free(*out);
    *out = string;
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

/* rts stubs (the GMP-heavy rts.c is not linked; sail256 needs none of it) */
void setup_rts(void)   {}
void cleanup_rts(void) {}
int  process_arguments(int argc, char **argv) { (void)argc; (void)argv; return 0; }  /* 0 = ok */

/* Declared in sail.h but only defined in the toolchain's GMP sail.c (not linked
 * here): right-shift returning 0 when the count >= word width, avoiding C's UB on
 * over-wide shifts. The generated model references it (e.g. zsszz_addr). Defined
 * in the runtime so every sail256-linked binary gets it, instead of each app
 * (test_utils.c) backfilling its own copy. */
fbits safe_rshift(const fbits x, const fbits n) { return (n >= 64) ? (fbits)0 : (x >> n); }
