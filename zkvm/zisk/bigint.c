/* ZisK provider for the extractions/c/zkvm_bigint.h 256-bit arithmetic contract.
 *
 * The ziskos zisklib uint256 module (pinned v1.0.0-alpha) exports these
 * C-ABI wrappers over the arith256/arith256_mod syscalls; they link from the
 * ziskos crate graph in the Rust-linked guest and from libziskos.a in the
 * C-direct guest. Operands are [u64; 4] limbs, least significant first --
 * the contract's exact layout, so calls pass through directly.
 *
 * div_rem256_c panics on a zero divisor and the modular ops return 0 for a
 * zero modulus; the contract's nonzero preconditions (enforced in Sail)
 * keep both cases unreachable. */

#include <stdint.h>

#include "zkvm_bigint.h"

extern void div_rem256_c(const uint64_t *a, const uint64_t *b, uint64_t *quo,
                         uint64_t *rem);
extern void mul_mod256_c(const uint64_t *a, const uint64_t *b,
                         const uint64_t *m, uint64_t *out);
extern void add_mod256_c(const uint64_t *a, const uint64_t *b,
                         const uint64_t *m, uint64_t *out);

void zkvm_u256_divrem(const uint64_t a[4], const uint64_t b[4],
                      uint64_t quotient[4], uint64_t remainder[4])
{
    div_rem256_c(a, b, quotient, remainder);
}

void zkvm_u256_mulmod(const uint64_t a[4], const uint64_t b[4],
                      const uint64_t n[4], uint64_t out[4])
{
    mul_mod256_c(a, b, n, out);
}

void zkvm_u256_addmod(const uint64_t a[4], const uint64_t b[4],
                      const uint64_t n[4], uint64_t out[4])
{
    add_mod256_c(a, b, n, out);
}
