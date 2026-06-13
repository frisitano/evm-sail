/* Private-input accessor FFI for the Sail SSZ decoder. Reads the same preloaded
 * buffer read_input() returns (zkvm_input_bytes, generated from the SSZ test
 * vector at build time). A real zkVM host supplies these bytes as the witness. */
#include "el_input.h"
#include "zkvm_io.h"

extern const unsigned char zkvm_input_bytes[];
extern const unsigned long  zkvm_input_bytes_len;

/* point a tx-input slot at a span of the stateless SSZ input -- the tx executes
 * directly over the witness bytes, no copy. (txin_view: host_mem.h via el_input.h) */
/* store the code for a codeHash as a view of the witness span [off, off+len). */
unit cs_view_input(uint64_t h3, uint64_t h2, uint64_t h1, uint64_t h0,
                   uint64_t off, uint64_t len) {
  uint64_t o = (off < zkvm_input_bytes_len) ? off : zkvm_input_bytes_len;
  uint64_t avail = zkvm_input_bytes_len - o;
  if (len > avail) len = avail;
  cs_view_hash(h3, h2, h1, h0, zkvm_input_bytes + o, len);
  return UNIT;
}
uint64_t txin_view_input(uint64_t idx, uint64_t off, uint64_t len) {
  const unsigned char *base = zkvm_input_bytes + (off < zkvm_input_bytes_len ? off : zkvm_input_bytes_len);
  uint64_t avail = zkvm_input_bytes_len - (off < zkvm_input_bytes_len ? off : zkvm_input_bytes_len);
  if (len > avail) len = avail;
  txin_view(idx, base, len);
  return len;
}

/* The guest emits the canonical SSZ result through the standard write_output. */
unit el_emit_out(uint64_t b)
{
    uint8_t byte = (uint8_t)(b & 0xff);
    write_output(&byte, 1);
    return UNIT;
}

void ssz_src_len(sail_int rop, const unit u)
{
    (void)u;
    mpz_set_ui(rop, zkvm_input_bytes_len);
}

uint64_t ssz_src_byte(sail_int idx)
{
    unsigned long i = mpz_get_ui(idx);
    if (i >= zkvm_input_bytes_len) {
        return 0;   /* out-of-range reads return 0 (decoder guards lengths) */
    }
    return (uint64_t)zkvm_input_bytes[i];
}

/* Bulk slice readers: read n (<=8) bytes from the input in ONE FFI call instead
 * of n per-byte crossings — the hot path of SSZ decoding (offsets, fields). */
uint64_t ssz_src_le(sail_int off, sail_int n)   /* little-endian */
{
    unsigned long o = mpz_get_ui(off);
    long k = (long)mpz_get_ui(n);
    uint64_t v = 0;
    for (long i = 0; i < k; i++) {
        uint64_t b = (o + (unsigned long)i < zkvm_input_bytes_len) ? zkvm_input_bytes[o + i] : 0;
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
        uint64_t b = (o + (unsigned long)i < zkvm_input_bytes_len) ? zkvm_input_bytes[o + i] : 0;
        v = (v << 8) | b;
    }
    return v;
}
