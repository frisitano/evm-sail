/* ===========================================================================
 * Implementation of the eth-act zkVM IO interface (zkvm_io.h) for the spike
 * validation harness.
 *
 *   void read_input(const uint8_t** buf_ptr, size_t* buf_size);
 *   void write_output(const uint8_t* output, size_t size);
 *
 * Per the io-interface standard:
 *   - read_input returns the private input buffer (read-only to the guest),
 *     is idempotent, and may be called multiple times.  If buf_size is 0 the
 *     buffer pointer is invalid and must not be dereferenced.
 *   - write_output appends bytes to the public output; multiple calls
 *     concatenate.
 *
 * The private input is supplied through a fixed, preloaded region
 * (`__zkvm_input`).  A real zkVM host would populate it from the proof's
 * private witness; in this harness it defaults to empty, and the block under
 * test is constructed inside the Sail model.  The public output is accumulated
 * into a buffer and mirrored to the HTIF console so the result is observable
 * when running on spike.
 * =========================================================================== */

#include "zkvm_io.h"
#include "htif.h"

/* ---- private input region ----------------------------------------------- */

/* Preloaded private input: the schema-prefixed SSZ stateless input, embedded at
 * build time as `zkvm_input_bytes` (generated from vectors/*.ssz). A real zkVM
 * host would instead populate this region from the proof's private witness. */
extern const unsigned char zkvm_input_bytes[];
extern const unsigned long  zkvm_input_bytes_len;

void read_input(const uint8_t **buf_ptr, size_t *buf_size)
{
    /* Idempotent, no side effects beyond setting the out-params. */
    *buf_ptr  = (const uint8_t *)zkvm_input_bytes;
    *buf_size = (size_t)zkvm_input_bytes_len;
}

/* ---- public output accumulator ------------------------------------------ */

#ifndef ZKVM_OUTPUT_CAPACITY
#define ZKVM_OUTPUT_CAPACITY 65536
#endif
static unsigned char zkvm_output[ZKVM_OUTPUT_CAPACITY];
static size_t        zkvm_output_len = 0;

void write_output(const uint8_t *output, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        if (zkvm_output_len < ZKVM_OUTPUT_CAPACITY) {
            zkvm_output[zkvm_output_len++] = output[i];
        }
        /* Mirror to the host console so the public output is observable on
         * spike. A real zkVM commits these bytes as the public output. */
        htif_putchar((char)output[i]);
    }
}

/* Accessors for the harness (not part of the standard interface). */
const unsigned char *zkvm_output_buffer(void) { return zkvm_output; }
size_t               zkvm_output_size(void)   { return zkvm_output_len; }
