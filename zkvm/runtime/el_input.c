/* Private-input accessor FFI for the Sail SSZ decoder. Reads the same preloaded
 * buffer read_input() returns (zkvm_input_bytes, generated from the SSZ test
 * vector at build time). A real zkVM host supplies these bytes as the witness. */
#include "el_input.h"
#include "zkvm_io.h"

/* Input source. On spike the SSZ witness is a vector baked in at build time
 * (zkvm_input_bytes). Under ere/a real zkVM there is no baked vector -- the host
 * supplies it at runtime via the zkvm-standards read_input(), and the guest
 * wrapper hands it to evmsail_set_input() before running. */
#ifdef ERE_GUEST
static const unsigned char *g_in = 0;
static unsigned long        g_in_len = 0;
void evmsail_set_input(const unsigned char *p, unsigned long n) { g_in = p; g_in_len = n; }
#define IN_PTR g_in
#define IN_LEN g_in_len
#else
extern const unsigned char zkvm_input_bytes[];
extern const unsigned long  zkvm_input_bytes_len;
#define IN_PTR zkvm_input_bytes
#define IN_LEN zkvm_input_bytes_len
#endif

/* point a tx-input slot at a span of the stateless SSZ input -- the tx executes
 * directly over the witness bytes, no copy. (txin_view: host_mem.h via el_input.h) */
/* store the code for a codeHash as a view of the witness span [off, off+len). */
unit cs_view_input(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0,
                   uint64_t off, uint64_t len) {
  uint64_t o = (off < IN_LEN) ? off : IN_LEN;
  uint64_t avail = IN_LEN - o;
  if (len > avail) len = avail;
  cs_view_hash(h3, h2, h1, h0, IN_PTR + o, len);
  return UNIT;
}
uint64_t txin_view_input(uint64_t idx, uint64_t off, uint64_t len) {
  const unsigned char *base = IN_PTR + (off < IN_LEN ? off : IN_LEN);
  uint64_t avail = IN_LEN - (off < IN_LEN ? off : IN_LEN);
  if (len > avail) len = avail;
  txin_view(idx, base, len);
  return len;
}

/* The guest emits the canonical SSZ result through the standard write_output. */
#ifdef ERE_GUEST
/* ere/zkvm-standards write_output is commit-once: buffer the SSZ result and
 * flush it in a single call (evmsail_flush_output, from the guest wrapper). */
static unsigned char g_out[1u << 17];
static unsigned long g_out_len = 0;
unit el_emit_out(uint64_t b)
{
    if (g_out_len < sizeof g_out) g_out[g_out_len++] = (unsigned char)(b & 0xff);
    return UNIT;
}
void evmsail_flush_output(void) { write_output(g_out, g_out_len); }
#else
unit el_emit_out(uint64_t b)
{
    uint8_t byte = (uint8_t)(b & 0xff);
    write_output(&byte, 1);
    return UNIT;
}
#endif

void ssz_src_len(sail_int rop, const unit u)
{
    (void)u;
    mpz_set_ui(rop, IN_LEN);
}

uint64_t ssz_src_byte(sail_int idx)
{
    unsigned long i = mpz_get_ui(idx);
    if (i >= IN_LEN) {
        return 0;   /* out-of-range reads return 0 (decoder guards lengths) */
    }
    return (uint64_t)IN_PTR[i];
}

/* Bulk slice readers: read n (<=8) bytes from the input in ONE FFI call instead
 * of n per-byte crossings — the hot path of SSZ decoding (offsets, fields). */
uint64_t ssz_src_le(sail_int off, sail_int n)   /* little-endian */
{
    unsigned long o = mpz_get_ui(off);
    long k = (long)mpz_get_ui(n);
    uint64_t v = 0;
    for (long i = 0; i < k; i++) {
        uint64_t b = (o + (unsigned long)i < IN_LEN) ? IN_PTR[o + i] : 0;
        v |= b << (8 * i);
    }
    return v;
}
uint64_t ssz_src_be(sail_int off, sail_int n)   /* big-endian */
{
    unsigned long o = mpz_get_ui(off);
    long k = (long)mpz_get_ui(n);
    uint64_t v = 0;
    for (long i = 0; i < k; i++) {
        uint64_t b = (o + (unsigned long)i < IN_LEN) ? IN_PTR[o + i] : 0;
        v = (v << 8) | b;
    }
    return v;
}
