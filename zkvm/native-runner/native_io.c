/* Native (host) implementation of the eth-act zkVM IO output sink.
 *
 * The spike target's zkvm_io.c routes write_output() over HTIF (tohost/fromhost),
 * which we must NOT link in a plain host process. zkvm_input.c (compiled with
 * -DERE_GUEST) buffers el_emit_out() output internally and calls write_output()
 * exactly once from evmsail_flush_output(). We capture those bytes here and
 * expose them to main.c.
 */
#include <stdint.h>
#include <stddef.h>

static unsigned char g_out[1u << 17];
static size_t g_outlen = 0;

void write_output(const uint8_t *p, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        if (g_outlen < sizeof g_out) {
            g_out[g_outlen++] = p[i];
        }
    }
}

const unsigned char *zkvm_output_buffer(void) { return g_out; }
size_t zkvm_output_size(void) { return g_outlen; }

/* Sail runtime helper. It is DECLARED in the GMP-free runtime header
 * (zkvm/runtime/sail256/sail.h) but only DEFINED in the toolchain's GMP sail.c,
 * which this GMP-free native build does not link. The generated model
 * references it (zsszz_addr). We provide the canonical definition here (a shift
 * that returns 0 when the count is >= the word width, avoiding C UB) so we do
 * not have to modify any runtime/ffi/.sail file. fbits == uint64_t. */
uint64_t safe_rshift(uint64_t x, uint64_t n)
{
    return (n >= 64) ? 0ull : (x >> n);
}
