/* Private-input accessor FFI for the Sail SSZ decoder (RISC-V guest). Reads
 * the same preloaded buffer read_input() returns (zkvm_input_bytes, baked from
 * the VEC file at build time). A real zkVM host supplies these bytes as the
 * witness. The host builds get the equivalent surface from the ctypes harness
 * (native-runner/test_utils.c); the deleted ere-guest target's -DERE_GUEST
 * runtime-input branch lives in git history. */
#include "zkvm_input.h"
#include "zkvm_io.h"

extern const unsigned char zkvm_input_bytes[];
extern const unsigned long  zkvm_input_bytes_len;

/* The guest emits the canonical SSZ result through the standard write_output. */
unit el_emit_out(uint64_t b)
{
    uint8_t byte = (uint8_t)(b & 0xff);
    write_output(&byte, 1);
    return UNIT;
}

uint64_t ssz_src_len(const unit u)
{
    (void)u;
    return (uint64_t)zkvm_input_bytes_len;
}

uint64_t ssz_src_byte(uint64_t idx)
{
    uint64_t i = idx;
    return (i < zkvm_input_bytes_len) ? (uint64_t)zkvm_input_bytes[i] : 0;
}

const uint8_t *evmsail_stateless_input_ptr(uint64_t off, uint64_t len)
{
    uint64_t total = (uint64_t)zkvm_input_bytes_len;
    if (off > total || len > total - off) return NULL;
    return zkvm_input_bytes + off;
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
