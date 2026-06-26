/* Native conformance driver for the evm-sail zkVM stateless block guest.
 *
 * Reads argv[1] = a file containing the RAW statelessInputBytes (already
 * hex-decoded, INCLUDING the leading 0x0001 schema prefix — the Sail SSZ
 * decoder strips it). Runs the guest entry main() over those bytes and
 * prints the guest's canonical SSZ output as a single lowercase hex line.
 *
 * Mirrors zkvm/ere-guest/ere_bridge.c's flow:
 *   set_input -> model_init -> main -> flush_output -> model_fini
 * but with a host file/stdout I/O harness instead of the ere SDK.
 *
 * If a Sail assertion/exception fires, the GMP-free runtime calls
 * exit(EXIT_FAILURE); the driver treats a nonzero exit / empty output as
 * "the guest errored on this fixture".
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail.h" /* unit / UNIT */

/* Version banner. Printed on -v / --version / version so the EEST `consume
 * direct` harness can identify this binary (its FixtureConsumerTool matches
 * `detect_binary_pattern = ^evm-sail`). Keep the leading token "evm-sail". */
#define EVMSAIL_VERSION "evm-sail zkvm stateless guest 0.1.0"

/* model entry points (Sail-generated + sail256 runtime) */
extern unit zmain(unit);
extern void model_init(void);
extern void model_fini(void);

/* zkvm_input.c, compiled -DERE_GUEST */
extern void evmsail_set_input(const unsigned char *p, unsigned long n);
extern void evmsail_flush_output(void);

/* native_io.c */
extern const unsigned char *zkvm_output_buffer(void);
extern size_t zkvm_output_size(void);

static unsigned char *read_file(const char *path, unsigned long *out_len)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "native-runner: cannot open '%s'\n", path);
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long n = ftell(f);
    if (n < 0) { fclose(f); return NULL; }
    rewind(f);
    unsigned char *buf = (unsigned char *)malloc((size_t)n ? (size_t)n : 1);
    if (!buf) { fclose(f); return NULL; }
    size_t got = fread(buf, 1, (size_t)n, f);
    fclose(f);
    if (got != (size_t)n) { free(buf); return NULL; }
    *out_len = (unsigned long)n;
    return buf;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <stateless-input-bytes-file>\n", argv[0]);
        return 2;
    }
    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0 ||
        strcmp(argv[1], "version") == 0) {
        printf("%s\n", EVMSAIL_VERSION);
        return 0;
    }

    unsigned long in_len = 0;
    unsigned char *in = read_file(argv[1], &in_len);
    if (!in) return 2;

    evmsail_set_input(in, in_len);
    model_init();
    (void)zmain(UNIT);
    evmsail_flush_output();
    model_fini();

    const unsigned char *out = zkvm_output_buffer();
    size_t out_len = zkvm_output_size();
    for (size_t i = 0; i < out_len; i++) {
        printf("%02x", out[i]);
    }
    printf("\n");

    free(in);
    return 0;
}
