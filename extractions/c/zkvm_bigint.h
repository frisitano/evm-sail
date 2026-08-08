/**
 * zkVM 256-bit Integer Arithmetic C Interface
 *
 * This header defines the standard C interface for guest programs to access
 * native 256-bit division and modular arithmetic in zkVMs. On platforms with
 * arithmetic precompiles (e.g. ZisK's arith256 syscalls) these operations
 * cost a handful of steps instead of a limb-level software routine; other
 * platforms provide a portable software implementation behind the unchanged
 * header.
 *
 * NOTE: this contract awaits AGENTS.md contract-list ratification. The extractions/c/
 * root is documented as holding only the standardized zkvm_accelerators.h and
 * zkvm_io.h platform contracts; adding this header extends that list and
 * needs maintainer sign-off.
 *
 * Design Notes:
 * - Values are unsigned 256-bit integers as four 64-bit limbs, least
 *   significant limb first (matching the optimized model's u256 layout and
 *   ZisK's [u64; 4] arith256 operand layout).
 * - Divisor/modulus preconditions are the caller's responsibility: the EVM's
 *   zero-divisor rules (DIV/MOD/ADDMOD/MULMOD yield 0) stay in the proven
 *   Sail specification, so providers never see a zero divisor or modulus.
 *
 * Usage Notes:
 * - Caller MUST ensure all pointers are valid and caller-allocated.
 * - Output arrays MAY alias each other or the inputs only where a provider
 *   documents it; portable callers SHOULD pass distinct arrays.
 */

#ifndef ZKVM_BIGINT_H
#define ZKVM_BIGINT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Unsigned 256-bit Euclidean division.
 *
 * Computes quotient = a / b and remainder = a % b.
 *
 * @param a Dividend limbs, least significant first
 * @param b Divisor limbs; precondition b != 0
 * @param[out] quotient Quotient limbs
 * @param[out] remainder Remainder limbs (remainder < b)
 */
void zkvm_u256_divrem(const uint64_t a[4], const uint64_t b[4],
                      uint64_t quotient[4], uint64_t remainder[4]);

/**
 * Unsigned 256-bit modular multiplication.
 *
 * Computes out = (a * b) mod n over the full 512-bit product (no
 * intermediate 2^256 truncation).
 *
 * @param a First factor limbs
 * @param b Second factor limbs
 * @param n Modulus limbs; precondition n != 0
 * @param[out] out Result limbs (out < n)
 */
void zkvm_u256_mulmod(const uint64_t a[4], const uint64_t b[4],
                      const uint64_t n[4], uint64_t out[4]);

/**
 * Unsigned 256-bit modular addition.
 *
 * Computes out = (a + b) mod n over the full 257-bit sum (no intermediate
 * 2^256 truncation).
 *
 * @param a First addend limbs
 * @param b Second addend limbs
 * @param n Modulus limbs; precondition n != 0
 * @param[out] out Result limbs (out < n)
 */
void zkvm_u256_addmod(const uint64_t a[4], const uint64_t b[4],
                      const uint64_t n[4], uint64_t out[4]);

#ifdef __cplusplus
}
#endif

#endif /* ZKVM_BIGINT_H */
